#ifndef _DATA_POINT_H
#define _DATA_POINT_H

#include <Arduino.h>

struct DataPoint {
    time_t timestamp = -1;
    int16_t coldTemperature = 0;
    int16_t hotTemperature = 0;
    float power = 0.0;
    bool heating = false;
};

#endif /* _DATA_POINT_H */