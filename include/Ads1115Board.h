#include "ADS1X15.h"

#ifndef _ADS1115_H
#define _ADS1115_H

class Ads1115Board {
private:    
    byte _alertInterruptPin = 33;

    static Ads1115Board *_defaultInstance;

    static ADS1115 *_board;

    static TaskHandle_t xTaskToNotifyFromISR;

public:
    Ads1115Board();

    static Ads1115Board *defaultInstance();

    float readRmsVoltage(uint channel, uint numberOfSamples);

private:
    static void ISR_RMScallback();
};

#endif