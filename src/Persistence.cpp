#include <Arduino.h>
#include <SPIFFS.h>

#include "Persistence.h"

// TODO ugly why using the ATTR with a class don't make is persistent ?
RTC_DATA_ATTR int dataCursor = 0;
RTC_DATA_ATTR  DataPoint dataPoints[PERSISTANCE_BUFFER_SIZE];

const String dataFilename = "/EcsMonitoring.dat";

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
    File file = SPIFFS.open(dataFilename, "ab", true);
    for (int i = 0; i < dataCursor; i++) {
        DataPoint pt = dataPoints[i];
        file.write((const uint8_t *)(&pt), sizeof(DataPoint));
    }
    file.close();
}

int Persistence::getDataPoints(DataPoint *dataPoints, int count, int offset = 0) {
    File file = SPIFFS.open(dataFilename, "rb");
    
    int count_ = count;
    if ((count_ + offset) * sizeof(DataPoint) > file.available()) {
        // clamp count to remain within bounds
        count_ = file.available() / sizeof(DataPoint) - offset;
        if (count_ < 1) {
            // no data points can be returned with this count and offsete
            return 0;
        }
    }

    DataPoint *dataPtr = dataPoints;
    uint32_t position = file.available() - (offset + 1) * sizeof(DataPoint);
    for (int i = 0; i < count_; i++) {
        file.seek(position);
        file.read((uint8_t *)dataPtr, sizeof(DataPoint));
        position -= sizeof(DataPoint);
        dataPtr++;
    }
    file.close();

    return count_;
}

void Persistence::clear() {
    File file = SPIFFS.open(dataFilename, "wb");
    file.close();
}