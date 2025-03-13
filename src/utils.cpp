#include <Arduino.h>
#include <time.h>
#include <esp_timer.h>
#include <esp_log.h>

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
  acquisitionTimer->start(ApplicationSettings::instance()->getSamplingPeriod());
}

void watchdogCallback(void *args) {
  gotoSleep();
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
  gotoSleep();
}

void setupForUserWakeup() {
  Serial.println("setupForUserWakeup");
  acquisitionTimer = new AcquisitionTimer(&makeMeasurementCallback);
  acquisitionTimer->start(ApplicationSettings::instance()->getSamplingPeriod());

  // watchdog to go to sleep mode after 120s of inactivity
  // watchdogTimer = new WatchdogTimer(&watchdogCallback, 120000000ULL);
  watchdogTimer = new WatchdogTimer(&watchdogCallback, 36000000000ULL);  // FIXME for test only
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
  bool save = false;
  if (ApplicationSettings::instance()->getStorageMode() == ApplicationSettings::StorageMode::delta) {
    DataPoint lastPoint = persistence.getLastDataPoint();
    // for the temperature, incremental means that difference with previous point > threshold
    if (ApplicationSettings::instance()->getColdSensorPresence()
      && abs(point.coldTemperature - lastPoint.coldTemperature) > ApplicationSettings::instance()->getTemperatureThreshold()) {
      save = true;
    }

    if (ApplicationSettings::instance()->getHotSensorPresence()
      && abs(point.hotTemperature == lastPoint.hotTemperature) > ApplicationSettings::instance()->getTemperatureThreshold()) {
      save = true;
    }

    // for the power, incremental mean that state ON/OFF is different from the previous point
    if (ApplicationSettings::instance()->getVoltageSensorPresence()
      && ((point.power > ApplicationSettings::instance()->getPowerThreshold()) !=  
        (lastPoint.power > ApplicationSettings::instance()->getPowerThreshold()))) {
      save = true;
    }
  } else {
    save = true;
  }

  if (save) {
    persistence.addDataPoint(point);
  }
}

void gotoSleep() {
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
  Serial.printf("Going to sleep for %d seconds\n", timeToSleep);
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