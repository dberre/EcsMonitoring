#include "ADS1X15.h"

#ifndef _ADS1115_H
#define _ADS1115_H

class Ads1115Board {
private:    
    byte _alertInterruptPin = 33;

    static Ads1115Board *_instance;

    static ADS1115 *_board;

    static TaskHandle_t _taskToNotifyFromISR;

public:
    static Ads1115Board *getInstance();

    float readRmsVoltage(uint channel, uint numberOfSamples);

    Ads1115Board(const Ads1115Board&) = delete;
    Ads1115Board& operator = (const Ads1115Board&) = delete;

private:
    Ads1115Board();
     
    static void ISR_RMScallback();
};

#endif