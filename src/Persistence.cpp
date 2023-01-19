#include <Arduino.h>

#include "Persistence.h"
#include <SPIFFS.h>

// TODO ugly why using the ATTR with a class don't make is persistent ?
RTC_DATA_ATTR int dataCursor = 0;
RTC_DATA_ATTR  DataPoint dataPoints[PERSISTANCE_BUFFER_SIZE];

Persistence::Persistence() {
}

void Persistence::addDataPoint(DataPoint& dataPoint) {
    Serial.printf("addDataPoint: #%d\n", dataCursor);
    dataPoints[dataCursor] = dataPoint;
    dataCursor++;
    if (dataCursor == PERSISTANCE_BUFFER_SIZE) {
        saveToFile();
        dataCursor = 0;
    }
}

DataPoint Persistence::getLastDataPoint() {
    return dataPoints[dataCursor];
}

void Persistence::saveToFile() {
    Serial.println("Persistance save");
    File file = SPIFFS.open("/EcsMonitoring.tsv", "a", true);
    char tmp[80];
    for (int i = 0; i < dataCursor; i++) {
        DataPoint pt = dataPoints[i];
        sprintf(tmp, "%d\t%d\t%d\t%d\n", pt.timestamp, pt.coldTemperature, pt.hotTemperature, (pt.heating) ? 1 : 0);
        file.write((const uint8_t *)tmp, strlen(tmp));
        Serial.printf("SaveToFile: %s", tmp);
    }
    file.close();
}