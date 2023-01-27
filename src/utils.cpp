#include <Arduino.h>
#include <time.h>
#include <esp_timer.h>

#include "utils.h"
#include "Spiffs.h"
#include "AcquisitionTimer.h"
#include "WatchdogTimer.h"
#include "MonitoringWebServer.h"
#include "Queues.h"
#include "VoltageSensor.h"
#include "TemperatureSensors.h"
#include "Persistence.h"
#include "DataPoint.h"

Persistence persistence;

MonitoringWebServer monitoringWebServer;

AcquisitionTimer *acquisitionTimer = NULL;

WatchdogTimer *watchdogTimer = NULL;

void makeMeasurementCallback(void *args) {
  ets_printf("ISR\n");
  xQueueSend(xQueue1, (void *)(&trigMeasurementMsg), 0);
}

void watchdogCallback(void *args) {
  ets_printf("Watchdog expired\n");
  gotoSleep();
}

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
  // listRootDirectory();

  // see https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/ 3", 1);
  tzset();

  setupTemperatureSensors();
  setupVoltageSensor();
  setupQueues();
}

void setupForRTCWakeup() {
  Serial.println("setupForRTCWakeup");
  DataPoint newPoint = makeMeasurement();
  saveMeasurement(newPoint);
  Serial.printf("RTC: %d\t%d\t%d\t%d\n",
    newPoint.timestamp, newPoint.coldTemperature, newPoint.hotTemperature, (newPoint.heating) ? 1 : 0);  
  gotoSleep();
}

void setupForUserWakeup() {
  Serial.println("setupForUserWakeup");

  // timer to trig a measurement every 60s
  acquisitionTimer = new AcquisitionTimer(&makeMeasurementCallback, 60000000ULL);
  acquisitionTimer->start();

  // watchdog to go to sleep mode after 120s of inactivity
  watchdogTimer = new WatchdogTimer(&watchdogCallback, 120000000ULL);

  monitoringWebServer.start();
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