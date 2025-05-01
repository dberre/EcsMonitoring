#include <Arduino.h>
#include <time.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <driver/gpio.h>

#include "utils.h"
#include "LittleFS.h"
#include "AcquisitionTimer.h"
#include "WatchdogTimer.h"
#include "MonitoringWebServer.h"
#include "Queues.h"
#include "Ads1115Board.h"
#include "TemperatureSensors.h"
#include "Persistence.h"
#include "DataPoint.h"
#include "ApplicationSettings.h"

Persistence persistence;

MonitoringWebServer monitoringWebServer;

AcquisitionTimer *acquisitionTimer = NULL;

WatchdogTimer *watchdogTimer = NULL;

// called by the timer in charge of trigging periodic measurements
void makeMeasurementCallback(void *args) {
  RequestQueueMsg req = TrigMeasurementRequest;
  xQueueSendFromISR(requestQueue, &req, 0);
  // the timer is one shot and must be rearmed at each call
  acquisitionTimer->start();
}

void watchdogCallback(void *args) {
  RequestQueueMsg req = GotoLightSleepRequest;
  xQueueSend(requestQueue, &req, 0);
}

void listRootDirectory() {
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while(file) {
      Serial.printf("File: %s\n", file.name());
      file.close();
      file = root.openNextFile();
  }
}

void setupUtils() {
  if (!LittleFS.begin()) {
    Serial.println("Persistence: LittleFS begin failed");
  }

  // Debugging purpose
  // listRootDirectory();

  // see https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/ 3", 1);
  tzset();

  setupTemperatureSensors();
  setupQueues();
}

void setupForRTCWakeup() {
  Serial.println("setupForRTCWakeup");
  DataPoint newPoint = makeMeasurement();
  saveMeasurement(newPoint);
  Serial.printf("RTC: %d\t%d\t%d\t%f\n",
    newPoint.timestamp, newPoint.coldTemperature, newPoint.hotTemperature, newPoint.power);  
  gotoDeepSleep();
}

void setupForUserWakeup() {
  Serial.println("setupForUserWakeup");
  acquisitionTimer = new AcquisitionTimer(&makeMeasurementCallback, ApplicationSettings::instance()->getSamplingPeriod());
  acquisitionTimer->start();

  // watchdog to go to sleep mode after 3 minutes of inactivity
  watchdogTimer = new WatchdogTimer(&watchdogCallback, 180000000ULL);
  // watchdogTimer = new WatchdogTimer(&watchdogCallback, 36000000000ULL);  // FIXME for test only
  monitoringWebServer.start();
}

DataPoint makeMeasurement() {
  DataPoint newPoint;
  // the two instructions below launch the temperature reading in parallel
  // each is 675 ms, so finally 675ms for both when done in parallel
  requestColdTemperature();
  requestHotTemperature();
  newPoint.coldTemperature = (ApplicationSettings::instance()->getColdSensorPresence()) ? (int16_t)(getColdTemperature() * 10.0f) : 0;
  newPoint.hotTemperature = (ApplicationSettings::instance()->getHotSensorPresence()) ? (int16_t)(getHotTemperature() * 10.0f) : 0;
  newPoint.power = (ApplicationSettings::instance()->getVoltageSensorPresence()) ? getPowerConsumption() : 0.0;
  newPoint.timestamp = time(NULL);
  return newPoint;
}

float getPowerConsumption() {
  // make a measurement on 100 samples
  float vrms = Ads1115Board::getInstance()->readRmsVoltage(0, 100);

  // Ratio of the current transformer: 100A -> 0.050A
  // burdern resistor = 150 ohms
  // Irms = (Vrms / 150) * (100A / 0.05A) = Vrms * 13.3333
  float currentA = 13.3333 * vrms;
  
  // Power (W) = Irms * 235V
  float powerW = currentA * 235.0;

  return powerW;
}

void saveMeasurement(DataPoint& point) {
  // TODO check threshold for temperature
  bool save = false;
  if (ApplicationSettings::instance()->getStorageMode() == ApplicationSettings::StorageMode::incremental_threshold) {
    DataPoint lastPoint = persistence.getLastDataPoint();
    // for the temperature, incremental means that difference with previous point > threshold
    if (ApplicationSettings::instance()->getColdSensorPresence()
      && abs(point.coldTemperature - lastPoint.coldTemperature) > ApplicationSettings::instance()->getTemperatureThreshold()) {
      save = true;
    }

    if (ApplicationSettings::instance()->getHotSensorPresence()
      && abs(point.hotTemperature - lastPoint.hotTemperature) > ApplicationSettings::instance()->getTemperatureThreshold()) {
      save = true;
    }

    // criteria for saving is state of the measure differs from the state of the last stored measurement
    // state is a bool defined with power >= threshold
    Serial.printf("Threshold: %f %f\n", point.power, lastPoint.power);
    if (ApplicationSettings::instance()->getVoltageSensorPresence()
      && ((point.power > ApplicationSettings::instance()->getPowerThreshold()) !=  
        (lastPoint.power > ApplicationSettings::instance()->getPowerThreshold()))) {
      save = true;
    }
  } else if (ApplicationSettings::instance()->getStorageMode() == ApplicationSettings::StorageMode::incremental_delta) {
    DataPoint lastPoint = persistence.getLastDataPoint();
    // for the temperature, incremental means that difference with previous point > threshold
    if (ApplicationSettings::instance()->getColdSensorPresence()
      && abs(point.coldTemperature - lastPoint.coldTemperature) >= ApplicationSettings::instance()->getTemperatureThreshold()) {
      save = true;
    }

    if (ApplicationSettings::instance()->getHotSensorPresence()
      && abs(point.hotTemperature - lastPoint.hotTemperature) >= ApplicationSettings::instance()->getTemperatureThreshold()) {
      save = true;
    }

    // Criteria for saving is abs(new - previous) >= threshold
    Serial.printf("Delta: %f %f\n", point.power, lastPoint.power);
    if (ApplicationSettings::instance()->getVoltageSensorPresence()
      && fabs(point.power - lastPoint.power) >= ApplicationSettings::instance()->getPowerThreshold()) {
      save = true;
    }
  } else {
    save = true;
  }

  if (save) {
    persistence.addDataPoint(point);
  }
}

void gotoLightSleep() {
  acquisitionTimer->stop();
  watchdogTimer->suspend();
  esp_wifi_stop();
  
  time_t timeToSleep = ApplicationSettings::instance()->getSamplingPeriod();  // TODO stop it
  esp_sleep_enable_timer_wakeup(timeToSleep * 1000000ULL);

  pinMode(GPIO_NUM_2, INPUT_PULLUP);
  ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_wakeup_enable(GPIO_NUM_2, GPIO_INTR_LOW_LEVEL));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_sleep_enable_gpio_wakeup());
  for(;;) {
    Serial.printf("Going to light sleep for %d seconds\n", timeToSleep);
    Serial.flush();

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_light_sleep_start());
    
    // the execution pauses here, and will restart from here at wakeup
    esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
    Serial.printf("light sleep - wakeup cause: %d\n", wakeupCause);
    if (wakeupCause != ESP_SLEEP_WAKEUP_TIMER) {
      break;
    }
    
    DataPoint newPoint = makeMeasurement();
    saveMeasurement(newPoint);
    Serial.printf("Sample: %d\t%d\t%d\t%f\n",
      newPoint.timestamp, newPoint.coldTemperature, newPoint.hotTemperature, newPoint.power);
    Serial.flush();
  }
  gpio_wakeup_disable(GPIO_NUM_2);
  esp_wifi_start();
  acquisitionTimer->start();
  watchdogTimer->restart();
}

void gotoDeepSleep() {
  monitoringWebServer.stop();

  if (acquisitionTimer != NULL) {
    acquisitionTimer->stop();
    delete(acquisitionTimer);
    acquisitionTimer = NULL;
  }

  if (watchdogTimer != NULL) {
    watchdogTimer->stop();
    delete(watchdogTimer);
    watchdogTimer = NULL;
  }

  deleteQueues();

  time_t timeToSleep = computeNextTick();
  Serial.printf("Going to deep sleep for %d seconds\n", timeToSleep);
  esp_sleep_enable_timer_wakeup(timeToSleep * 1000000ULL);
  #ifdef ESP32C3XIAO
  // GPIO2 is DO on the board according to the ESP32 C3 schematic
  esp_err_t status = esp_deep_sleep_enable_gpio_wakeup((1ULL << GPIO_NUM_2), ESP_GPIO_WAKEUP_GPIO_LOW);
  #else
  // will wake if a low level occurs on pin G33
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);
  pinMode(GPIO_NUM_33, INPUT_PULLUP);
  #endif
  esp_deep_sleep_start();
  // the execution stops here, on wakeup it restarts from the setup() in main.
}

time_t computeNextTick() {
  time_t now = time(NULL);
  tm *tnow = localtime(&now);
  tnow->tm_sec += ApplicationSettings::instance()->getSamplingPeriod();
  if (tnow->tm_sec > 59) {
    tnow->tm_min += (tnow->tm_sec / 60);
    tnow->tm_sec %= 60;
    if (tnow->tm_min > 59) {
      tnow->tm_hour += (tnow->tm_min / 60);
      tnow->tm_min %= 60;
      if (tnow->tm_hour > 23) {
        tnow->tm_hour %= 24;
        tnow->tm_mday += 1;
      }
    }
  }
  return mktime(tnow) - now;
}