#include <Arduino.h>

#include "Queues.h"
#include "DataPoint.h"

RequestQueueMsg::RequestQueueMsg() {
    this->msgType = MsgTypes::undef;
}

RequestQueueMsg::RequestQueueMsg(MsgTypes type) {
    this->msgType = type;
}

RequestQueueMsg::RequestQueueMsg(MsgTypes type, int count, int offset) {
    this->msgType = type;
    this->args[0] = count;
    this->args[1] = offset;
}

ResponseQueueMsg::ResponseQueueMsg() {
    this->data.series.count = 0;
    this->data.series.points = NULL;
}

QueueHandle_t requestQueue;

QueueHandle_t responseQueue;

void setupQueues() {
    requestQueue = xQueueCreate(1, sizeof(RequestQueueMsg));
    responseQueue = xQueueCreate(1, sizeof(ResponseQueueMsg));
}

void deleteQueues() {
    vQueueDelete(requestQueue);
    vQueueDelete(responseQueue);
}