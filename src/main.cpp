#include <Arduino.h>

#include "utils.h"
#include "VoltageSensor.h"
#include "Ads1115Board.h"
#include "Queues.h"
#include "DataPoint.h"

void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && ((millis() - start) < 2000)) { delay(100); };
  
  setupUtils();

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  Serial.printf("setup - wakeup cause: %d\n", wakeup_reason);
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
            Serial.printf("trigMeasurement: %d\t%d\t%d\t%f\n", 
              newPoint.timestamp, newPoint.coldTemperature, newPoint.hotTemperature, newPoint.power);
          }
          break;
        case RequestQueueMsg::MsgTypes::getMeasurement: {
            DataPoint newPoint = makeMeasurement();
            ResponseQueueMsg response;
            response.msgType = ResponseQueueMsg::MsgType::series;
            response.data.series.count = 1;
            response.data.series.points = new DataPoint[1];
            response.data.series.points[0] = newPoint;
            xQueueSend(responseQueue, &response, 0);
          }
          break;
        case RequestQueueMsg::MsgTypes::getVoltage: {
            float vrms = Ads1115Board::getInstance()->readRmsVoltage(0, 100);
            ResponseQueueMsg response;
            response.msgType = ResponseQueueMsg::MsgType::voltage;
            response.data.voltage = vrms;
            xQueueSend(responseQueue, &response, 0);
          }
          break;
        case RequestQueueMsg::MsgTypes::getBatteryVoltage: {
            // the is a resistor divider /2 on the board
            float vbatt = 2.0 * Ads1115Board::getInstance()->readVoltage(2, 100);
            ResponseQueueMsg response;
            response.msgType = ResponseQueueMsg::MsgType::voltage;
            response.data.voltage = vbatt;
            xQueueSend(responseQueue, &response, 0);
          }
          break;
        case RequestQueueMsg::MsgTypes::getHistory: {
            int count = msgQueue.args[0];
            int offset = msgQueue.args[1];
            ResponseQueueMsg response;
            response.msgType = ResponseQueueMsg::MsgType::series;
            response.data.series.points = new DataPoint[count];
            response.data.series.count = persistence.getDataPoints(response.data.series.points, count, offset);
            xQueueSend(responseQueue, &response, 0);
          }
          break;
        case RequestQueueMsg::MsgTypes::getHistoryDepth: {
            ResponseQueueMsg response;
            response.msgType = ResponseQueueMsg::MsgType::series;
            response.data.series.points = 0;
            response.data.series.count = persistence.getDataPointsCount();
            xQueueSend(responseQueue, &response, 0);
          }
          break;
        case RequestQueueMsg::MsgTypes::clearHistory: {
            persistence.clear();
          }
          break;
        case RequestQueueMsg::MsgTypes::gotoLightSleep: {
            gotoLightSleep();
          }
          break;
        default:
          break;
      }
    }
}