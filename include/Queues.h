#ifndef _QUEUES_H
#define _QUEUES_H

#include <Arduino.h>

#include "DataPoint.h"

struct RequestQueueMsg {
    enum MsgTypes: long { undef, trigMeasurement, getMeasurement, getHistory, clearHistory };

    RequestQueueMsg();
    RequestQueueMsg(MsgTypes type);
    RequestQueueMsg(MsgTypes type, int count, int offset);

    MsgTypes msgType;
    int args[2];
};

struct ResponseQueueMsg {
    ResponseQueueMsg();

    int count;
    DataPoint *points;
};

#define TrigMeasurementRequest RequestQueueMsg(RequestQueueMsg::MsgTypes::trigMeasurement)
#define GetMeasurementRequest RequestQueueMsg(RequestQueueMsg::MsgTypes::getMeasurement)
#define GetHistoryRequest(count, offset) RequestQueueMsg(RequestQueueMsg::MsgTypes::getHistory, (count), (offset))
#define ClearHistoryRequest RequestQueueMsg(RequestQueueMsg::MsgTypes::clearHistory)

extern QueueHandle_t requestQueue;
extern QueueHandle_t responseQueue;

extern void setupQueues();
extern void deleteQueues();

#endif /* _QUEUES_H */