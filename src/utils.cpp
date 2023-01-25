#include <Arduino.h>
#include <time.h>
#include <esp_timer.h>

#include "utils.h"
#include "Spiffs.h"
#include "MonitoringWebServer.h"
#include "VoltageSensor.h"
#include "TemperatureSensors.h"
#include "Persistence.h"
#include "DataPoint.h"

Persistence persistence;

MonitoringWebServer monitoringWebServer;

// Timer dedicated to periodic measurements
esp_timer_handle_t _timer;

volatile SemaphoreHandle_t timerSemaphore;

void listRootDirectory() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file) {
      Serial.printf("File: %s\n", file.name());
      file.close();
      file = root.openNextFile();
  }
}

void setupUtils() {

  if(!SPIFFS.begin()) {
      Serial.println("Persistence: SPIFFS begin failed");
  }

  // Debugging purpose
  listRootDirectory();

 // see https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/ 3", 1);
  tzset();

  setupTemperatureSensors();
  setupVoltageSensor();

  timerSemaphore = xSemaphoreCreateBinary();
}

void makeMeasurementISR() {
  ets_printf("ISR\n");
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

void startAcquisitionTimer() {
  esp_timer_create_args_t _timerConfig;
  _timerConfig.arg = NULL;
  _timerConfig.callback = reinterpret_cast<esp_timer_cb_t>(makeMeasurementISR);
  _timerConfig.dispatch_method = ESP_TIMER_TASK;
  _timerConfig.name = "";
  esp_timer_create(&_timerConfig, &_timer);
  esp_timer_start_once(_timer, computeNextTick() * 1000000ULL); // TODO
}

void stopAcquisitionTimer() {
  if (_timer != NULL) {
    esp_timer_stop(_timer);
    esp_timer_delete(_timer);
    _timer = NULL;
  }
}

DataPoint makeMeasurement() {
  DataPoint newPoint;
  newPoint.coldTemperature = (int16_t)(getColdTemperature() * 10.0f);
  newPoint.hotTemperature = (int16_t)(getHotTemperature() * 10.0f);
  newPoint.heating = getHeaterState();
  newPoint.timestamp = time(NULL);
  return newPoint;
}

void saveMeasurement(DataPoint& point) {
  persistence.addDataPoint(point);
}

void gotoSleep() {
  monitoringWebServer.stop();
  stopAcquisitionTimer();

  time_t timeToSleep = 60; // TODO restore a reliable 60s
  Serial.printf("Going to sleep for %d seconds\n", timeToSleep);
  esp_sleep_enable_timer_wakeup(timeToSleep * 1000000ULL);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);
  esp_deep_sleep_start();
}

time_t computeNextTick() {
  time_t now = time(NULL);
  tm *tnow = localtime(&now);
  tnow->tm_min += 1;
  tnow->tm_sec = 0;
  if (tnow->tm_min > 59) {
    tnow->tm_min = 0;
    tnow->tm_hour += 1;
    if (tnow->tm_hour > 23) {
      tnow->tm_hour = 0;
      tnow->tm_mday += 1;
    }
  }
  return mktime(tnow) - now;
}