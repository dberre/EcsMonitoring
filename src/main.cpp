#include <Arduino.h>

#include "Utils.h"
#include "DataPoint.h"

void setup() {
  Serial.begin(115200);
  
  setupUtils();

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  Serial.printf("esp_sleep_wakeup_cause_t %d\n", wakeup_reason);
  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_TIMER:
    setupForRTCWakeup();
    break;
  
    default:
    setupForUserWakeup();
    break;
  }
}

void loop() {
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE) {
    DataPoint newPoint = makeMeasurement();
    saveMeasurement(newPoint);
    acquisitionTimer->start();
    Serial.printf("loop: %d\t%d\t%d\t%d\n", 
      newPoint.timestamp, newPoint.coldTemperature, newPoint.hotTemperature, (newPoint.heating) ? 1 : 0);  
  }
}