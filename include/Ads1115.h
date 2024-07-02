#include "ADS1X15.h"

#ifndef _ADS1115_H
#define _ADS1115_H

class Ads1115Sensor {
private:    
    byte _alertInterruptPin = 33;

    static Ads1115Sensor *_defaultInstance;

    static ADS1115 *_board;

    static SemaphoreHandle_t xBinarySemaphore;

    static TaskHandle_t xTaskToNotifyFromISR;

public:
    Ads1115Sensor();

    static Ads1115Sensor *defaultInstance();

    float readRmsVoltage(uint channel, uint numberOfSamples);

private:
    static void ISR_RMScallback();
};

#endif