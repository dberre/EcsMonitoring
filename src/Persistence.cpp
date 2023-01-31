#include <Arduino.h>
#include <SPIFFS.h>

#include "Persistence.h"

// TODO ugly why using the ATTR with a class don't make is persistent ?
RTC_DATA_ATTR int dataCursor_ = 0;
RTC_DATA_ATTR  DataPoint dataPoints_[PERSISTANCE_BUFFER_SIZE];

const String dataFilename = "/EcsMonitoring.dat";

Persistence::Persistence() {
}

void Persistence::addDataPoint(DataPoint& dataPoint) {
    Serial.printf("addDataPoint: #%d\n", dataCursor_);
    dataPoints_[dataCursor_] = dataPoint;
    dataCursor_++;
    if (dataCursor_ == PERSISTANCE_BUFFER_SIZE) {
        saveToFile();
        dataCursor_ = 0;
    }
}

DataPoint Persistence::getLastDataPoint() {
    if (dataCursor_ > 0) {
        // available in the local storage
        return dataPoints_[dataCursor_ - 1];
    } else {
        // get it from the file
        DataPoint point;
        if (this->getDataPoints(&point, 1 , 0) == 1) {
            return point;
        } else {
            return DataPoint();
        }
    }
}

void Persistence::saveToFile() {
    Serial.println("Persistance save");
    File file = SPIFFS.open(dataFilename, "ab", true);
    for (int i = 0; i < dataCursor_; i++) {
        DataPoint pt = dataPoints_[i];
        file.write((const uint8_t *)(&pt), sizeof(DataPoint));
    }
    file.close();
}

int Persistence::getDataPoints(DataPoint *dataPoints, int nbToRead, int offset = 0) {
    // take the points available in the local storage
    int stillToRead = nbToRead;
    DataPoint *dataPtr = dataPoints;

    int index = dataCursor_ - 1 - offset;
    if (index >= 0) {
        while ((index >= 0) && (stillToRead > 0)) {
            *dataPtr = dataPoints_[index];
            dataPtr++;
            index--;
            stillToRead--;
        }
        // the offset is now taken into account and must be 0 for the rest of the function
        offset = 0;
    }
    int nbRead = nbToRead - stillToRead;
    
    // if more points are needed then get them from the file
    if (stillToRead > 0) {
        File file = SPIFFS.open(dataFilename, "rb");
        
        // clamp count to remain within the file bounds
        if ((stillToRead + offset) * sizeof(DataPoint) > file.available()) {
            stillToRead = file.available() / sizeof(DataPoint) - offset;
        }
        nbRead += stillToRead;

        uint32_t position = file.available() - (offset + 1) * sizeof(DataPoint);
        for (int i = 0; i < stillToRead; i++) {
            file.seek(position);
            file.read((uint8_t *)dataPtr, sizeof(DataPoint));
            position -= sizeof(DataPoint);
            dataPtr++;
        }
        file.close();
    }
    return nbRead;
}

void Persistence::clear() {
    File file = SPIFFS.open(dataFilename, "wb");
    file.close();
}
