#ifndef _QUEUES_H
#define _QUEUES_H

#include <Arduino.h>

#include "DataPoint.h"

struct RequestQueueMsg {
    enum MsgTypes: int { undef, trigMeasurement, getMeasurement, getVoltage, getHistory, clearHistory, getHistoryDepth };

    RequestQueueMsg();
    RequestQueueMsg(MsgTypes type);
    RequestQueueMsg(MsgTypes type, int count, int offset);

    MsgTypes msgType;
    int args[2];
};

struct ResponseQueueMsg {
    enum MsgType: int { undef, series, voltage };

    ResponseQueueMsg();

    MsgType msgType;

    union {
        struct {
            int count;
            DataPoint *points;
        } series;

        float voltage;
    } data;
};

#define TrigMeasurementRequest RequestQueueMsg(RequestQueueMsg::MsgTypes::trigMeasurement)
#define GetMeasurementRequest RequestQueueMsg(RequestQueueMsg::MsgTypes::getMeasurement)
#define GetVoltageRequest RequestQueueMsg(RequestQueueMsg::MsgTypes::getVoltage)
#define GetHistoryRequest(count, offset) RequestQueueMsg(RequestQueueMsg::MsgTypes::getHistory, (count), (offset))
#define GetHistoryDepth RequestQueueMsg(RequestQueueMsg::MsgTypes::getHistoryDepth)
#define ClearHistoryRequest RequestQueueMsg(RequestQueueMsg::MsgTypes::clearHistory)

extern QueueHandle_t requestQueue;
extern QueueHandle_t responseQueue;

extern void setupQueues();
extern void deleteQueues();

#endif /* _QUEUES_H */