#ifndef _UTILS_H
#define _UTILS_H

#include "MonitoringWebServer.h"
#include "Persistence.h"
#include "DataPoint.h"

void setupUtils();
DataPoint makeMeasurement();
void saveMeasurement(DataPoint& point);
void gotoSleep();
void setTime();

extern MonitoringWebServer monitoringWebServer;
extern Persistence persistence;

#endif /* _UTILS_H */