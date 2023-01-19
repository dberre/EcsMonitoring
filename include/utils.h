#ifndef _UTILS_H
#define _UTILS_H

#include "MonitoringWebServer.h"
#include "DataPoint.h"

void setupUtils();
DataPoint makeMeasurement();
void saveMeasurement(DataPoint& point);
void gotoSleep();
void setTime();

extern MonitoringWebServer monitoringWebServer;

#endif /* _UTILS_H */