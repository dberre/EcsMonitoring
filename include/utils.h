#ifndef _UTILS_H
#define _UTILS_H

#include "MonitoringWebServer.h"
#include "AcquisitionTimer.h"
#include "WatchdogTimer.h"
#include "Persistence.h"
#include "DataPoint.h"

void setupUtils();
void setupForRTCWakeup();
void setupForUserWakeup();
void startAcquisitionTimer();
void stopAcquisitionTimer();
DataPoint makeMeasurement();
void saveMeasurement(DataPoint& point);
void gotoSleep();
void setTime();
time_t computeNextTick();

extern MonitoringWebServer monitoringWebServer;
extern AcquisitionTimer *acquisitionTimer;
extern WatchdogTimer *watchdogTimer;
extern Persistence persistence;
extern volatile SemaphoreHandle_t timerSemaphore;

#endif /* _UTILS_H */