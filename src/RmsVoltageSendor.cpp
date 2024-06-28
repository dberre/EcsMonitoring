#include <Arduino.h>
#include <driver/adc.h>

#include "RmsVoltageSensor.h"

RmsVoltageSensor *RmsVoltageSensor::_defaultInstance = 0;

RmsVoltageSensor::RmsVoltageSensor() {
    this->adcChannel = ADC1_CHANNEL_7;
    // calculate conversion ratio (3.3v range for a 12 bits ADC)
    this->adcToVoltRatio = (float)(3.3 / (1 << 12));
}

RmsVoltageSensor *RmsVoltageSensor::defaultInstance() {
    if (_defaultInstance == 0) {
        _defaultInstance = new RmsVoltageSensor();
    }
    return _defaultInstance;
}

float RmsVoltageSensor::readVoltage(unsigned int duration) {

    unsigned int numberOfSamples = 0;
    double sumAdc = 0.0;
    double sumSquareAdc = 0.0;

    unsigned long timemax = millis() + duration;

    while ((millis() < timemax)) {
        int adc = adc1_get_raw(this->adcChannel);
        sumAdc += adc;
        sumSquareAdc += (adc * adc);
        numberOfSamples++;
    }

    float rmsVoltage = this->adcToVoltRatio * sqrt((sumSquareAdc / numberOfSamples) - pow(sumAdc / numberOfSamples, 2));
    
    Serial.printf("%04d %.0lf;%04d;%.0lf;%08d;%.04f\n", numberOfSamples, sumAdc, (int)(sumAdc / numberOfSamples), sumSquareAdc, (int)(sumSquareAdc / numberOfSamples), rmsVoltage);

    return rmsVoltage;
}