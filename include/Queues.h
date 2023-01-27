#ifndef _QUEUES_H
#define _QUEUES_H

#include <Arduino.h>

enum QueueSendMsg: long { trigMeasurementQueueMsg, getMeasurementQueueMsg, clearHistoryQueueMsg };

extern QueueHandle_t xQueue1;
extern QueueHandle_t xQueue2;

extern const QueueSendMsg trigMeasurementMsg;
extern const QueueSendMsg getMeasurementMsg;
extern const QueueSendMsg clearHistoryMsg;

extern void setupQueues();
extern void deleteQueues();

#endif /* _QUEUES_H */