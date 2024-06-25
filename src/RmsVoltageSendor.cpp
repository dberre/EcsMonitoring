#include <Arduino.h>
#include <driver/adc.h>

#include "RmsVoltageSensor.h"

RmsVoltageSensor *RmsVoltageSensor::_defaultInstance = 0;

RmsVoltageSensor::RmsVoltageSensor() {
    this->adcChannel = ADC1_CHANNEL_0;
    this->bitWidth = 12;

    this->offsetV = this->bitWidth >> 1;
    this->adcToVoltRatio = (float)(3.3 / (1 << this->bitWidth));
}

RmsVoltageSensor *RmsVoltageSensor::defaultInstance() {
    if (_defaultInstance == 0) {
        _defaultInstance = new RmsVoltageSensor();
    }
    return _defaultInstance;
}

float RmsVoltageSensor::readVoltage(unsigned int crossings, unsigned int timeout) {

    // loop to start measurement +/- 5% arround middle of the ADC range
    int adcRef = 0;
    unsigned long timemax = millis() + timeout;
    int adcCount = 1 << this->bitWidth;
    while(true) {
        adcRef = adc1_get_raw(this->adcChannel);
        if ((adcRef < (adcCount * 0.55)) && (adcRef > (adcCount * 0.45))) break;
        if (millis() > timemax) break;
    }

    unsigned int crossCount = 0;
    unsigned int numberOfSamples = 0;

    double sumSquareAdc = 0.0;

    bool checkVCross = false;
    bool lastVCross = false;

    timemax = millis() + timeout;

    while ((crossCount < crossings) && (millis() < timemax)) {
        numberOfSamples++;

        int adc = adc1_get_raw(this->adcChannel);

        this->offsetV = this->offsetV + ((adc - this->offsetV) / 1024);
        int filteredAdc = adcRef - this->offsetV;

        sumSquareAdc += (filteredAdc * filteredAdc);

        lastVCross = checkVCross;
        if (adc > adcRef) checkVCross = true;
                    else checkVCross = false;
        if (numberOfSamples > 1) {
            if (lastVCross != checkVCross) crossCount++;
        }
    }

    Serial.print(adcRef);
    Serial.print(';');
    Serial.print(numberOfSamples);
    Serial.print(';');
    Serial.print(crossCount);
    Serial.print(';');
    Serial.println(this->offsetV);

    return this->adcToVoltRatio * sqrt(sumSquareAdc / numberOfSamples);
}