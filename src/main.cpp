#include <Arduino.h>

#include "Utils.h"
#include "VoltageSensor.h"
#include "Ads1115.h"
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
  RequestQueueMsg msgQueue;
  if (xQueueReceive(requestQueue, &msgQueue,  1000 / portTICK_PERIOD_MS) == pdTRUE) {
    //Serial.printf("xQueue receive %ld\n", msgQueue);
    switch(msgQueue.msgType) {
      case RequestQueueMsg::MsgTypes::trigMeasurement: {
          DataPoint newPoint = makeMeasurement();
          saveMeasurement(newPoint);
          acquisitionTimer->start();
          Serial.printf("loop: %d\t%d\t%d\t%d\n", 
            newPoint.timestamp, newPoint.coldTemperature, newPoint.hotTemperature, (newPoint.heating) ? 1 : 0);
        }
        break;
      case RequestQueueMsg::MsgTypes::getMeasurement: {
          DataPoint newPoint = makeMeasurement();
          ResponseQueueMsg response;
          response.count = 1;
          response.points = new DataPoint[1];
          response.points[0] = newPoint;
          xQueueSend(responseQueue, &response, 0);
        }
        break;
      case RequestQueueMsg::MsgTypes::getVoltage: {
          // float response = RmsVoltageSensor::defaultInstance()->readVoltage(40);
          float response = readRmsVoltage(0, 3);
          xQueueSend(responseQueue, &response, 0);
        }
        break;
      case RequestQueueMsg::MsgTypes::getHistory: {
          int count = msgQueue.args[0];
          int offset = msgQueue.args[1];
          ResponseQueueMsg response;
          response.points = new DataPoint[count];
          response.count = persistence.getDataPoints(response.points, count, offset);
          xQueueSend(responseQueue, &response, 0);
        }
        break;
      case RequestQueueMsg::MsgTypes::clearHistory: {
          // persistence.clear(); FIXME
          float response = readRmsVoltage(0, 3);
          Serial.printf("value:%f\n", response);
        }
        break;
      default:
        break;
    }
  }
}