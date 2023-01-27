#include <Arduino.h>

#include "Queues.h"
#include "DataPoint.h"

QueueHandle_t xQueue1;

QueueHandle_t xQueue2;

const QueueSendMsg trigMeasurementMsg = trigMeasurementQueueMsg;
const QueueSendMsg getMeasurementMsg = getMeasurementQueueMsg;
const QueueSendMsg clearHistoryMsg = clearHistoryQueueMsg;

void setupQueues() {
    xQueue1 = xQueueCreate(3, sizeof(long));
    xQueue2 = xQueueCreate(1, sizeof(DataPoint));
}

void deleteQueues() {
    vQueueDelete(xQueue1);
    vQueueDelete(xQueue2);
}