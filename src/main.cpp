#include <Arduino.h>

#include "Utils.h"
#include "Queues.h"
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
  QueueSendMsg msgQueue;
  if (xQueueReceive(xQueue1, &msgQueue,  1000 / portTICK_PERIOD_MS) == pdTRUE) {
    Serial.printf("xQueue receive %ld\n", msgQueue);
    switch(msgQueue) {
      case trigMeasurementQueueMsg: {
          DataPoint newPoint = makeMeasurement();
          saveMeasurement(newPoint);
          acquisitionTimer->start();
          Serial.printf("loop: %d\t%d\t%d\t%d\n", 
            newPoint.timestamp, newPoint.coldTemperature, newPoint.hotTemperature, (newPoint.heating) ? 1 : 0);
        }
        break;
      case getMeasurementQueueMsg: {
          DataPoint newPoint = makeMeasurement();
          xQueueSend(xQueue2, &newPoint, 0);
        }
        break;
      case clearHistoryQueueMsg:
        persistence.clear();
        break;
    }
  }
}