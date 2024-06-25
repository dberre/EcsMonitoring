#ifndef _RMS_VOLTAGE_SENSOR_H
#define _PERSIS_RMS_VOLTAGE_SENSOR_HTENCE_H

#include <driver/adc.h>

class RmsVoltageSensor {

public:
    RmsVoltageSensor();
    float readVoltage(unsigned int crossings, unsigned int timeout);

    static RmsVoltageSensor *defaultInstance();

private:
    adc1_channel_t adcChannel;
    int offsetV;
    int bitWidth;

    float adcToVoltRatio;
    static RmsVoltageSensor *_defaultInstance;
};

#endif /* _RMS_VOLTAGE_SENSOR_H */