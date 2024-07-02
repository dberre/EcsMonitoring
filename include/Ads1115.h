#include "ADS1X15.h"

#ifndef _ADS1115_H
#define _ADS1115_H

class Ads1115Sensor {
private:    
    uint _numberOfSamples = 0;
    uint _adcChannel = 0;
    float _convertedValue;

    byte _alertInterruptPin = 33;

    static Ads1115Sensor *_defaultInstance;

    static ADS1115 *_board;

    static SemaphoreHandle_t xBinarySemaphore;

    static TaskHandle_t xTaskToNotify;

public:
    Ads1115Sensor();

    static Ads1115Sensor *defaultInstance();

    float readRmsVoltage(uint channel, uint numberOfSamples);

private:
    static void ISR_RMScallback();
    static void ISR_RMSprocessing(void * parameter);
};

#endif