#include <Arduino.h>
#include <time.h>

#include "utils.h"
#include "Spiffs.h"
#include "MonitoringWebServer.h"
#include "VoltageSensor.h"
#include "TemperatureSensors.h"
#include "TimeBase.h"
#include "Persistence.h"
#include "DataPoint.h"

RTC_DATA_ATTR Persistence persistence;

TimeBase timeBase;
MonitoringWebServer monitoringWebServer;

void setupUtils() {

  if(!SPIFFS.begin()) {
      Serial.println("Persistence: SPIFFS begin failed");
  }

  // Debugging purpose
  // File root = SPIFFS.open("/");
  // File file = root.openNextFile();
  // while(file) {
  //     Serial.printf("File: %s\n", file.name());
  //     file.close();
  //     file = root.openNextFile();
  // }

  setupTemperatureSensors();
  setupVoltageSensor();
}

DataPoint makeMeasurement() {
  requestTemperatures();
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
  // TODO stop the timeout user session

  // time_t now = time(NULL);
  // uint64_t timeToSleep = difftime(nextTick, now);
  // timeToSleep = (timeToSleep > 0) ? timeToSleep : 1;
  time_t timeToSleep = 30; // TODO restore a reliable 60s
  Serial.printf("Going to sleep for %d seconds\n", timeToSleep);
  esp_sleep_enable_timer_wakeup(timeToSleep * 1000000ULL);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);
  esp_deep_sleep_start();
}

void setTime() {
  
}