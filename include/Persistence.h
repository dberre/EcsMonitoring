#ifndef _PERSISTENCE_H
#define _PERSISTENCE_H

#include "DataPoint.h"

#define PERSISTANCE_BUFFER_SIZE 30

class Persistence {
    public:
    Persistence();
    void addDataPoint(DataPoint& dataPoint);
    DataPoint getLastDataPoint();
    int getDataPoints(DataPoint *dataPoints, int count, int offset);
    void clear();
    
    private:
    // DataPoint dataPoints[PERSISTANCE_BUFFER_SIZE];
    // uint dataCursor;

    void saveToFile();
};

#endif /* _PERSISTENCE_H */