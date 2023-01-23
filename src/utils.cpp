#include <Arduino.h>
#include <time.h>

#include "utils.h"
#include "Spiffs.h"
#include "MonitoringWebServer.h"
#include "VoltageSensor.h"
#include "TemperatureSensors.h"
#include "Persistence.h"
#include "DataPoint.h"

RTC_DATA_ATTR Persistence persistence;

MonitoringWebServer monitoringWebServer;

void setupUtils() {

  if(!SPIFFS.begin()) {
      Serial.println("Persistence: SPIFFS begin failed");
  }

  // // Debugging purpose
  // File root = SPIFFS.open("/");
  // File file = root.openNextFile();
  // while(file) {
  //     Serial.printf("File: %s\n", file.name());
  //     file.close();
  //     file = root.openNextFile();
  // }

 // see https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/ 3", 1);
  tzset();

  setupTemperatureSensors();
  setupVoltageSensor();
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
  // TODO stop the timeout user session

  time_t timeToSleep = 60; // TODO restore a reliable 60s
  Serial.printf("Going to sleep for %d seconds\n", timeToSleep);
  esp_sleep_enable_timer_wakeup(timeToSleep * 1000000ULL);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);
  esp_deep_sleep_start();
}

void setTime() {
  
}