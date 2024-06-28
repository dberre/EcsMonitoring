#include <Arduino.h> // for printf

#include <driver/adc.h>


const float ADC_TO_VOLTAGE_CONVERSION = (float)(3.3 / 4096.0);

bool setupVoltageSensor() {
    if (adc1_config_width(ADC_WIDTH_BIT_12) == ESP_OK) {
        if (adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11) == ESP_OK) {
            return true;
        }
    }
    return false;
}

float getVoltage() {
    uint32_t adc = 0;
    for (int i = 0; i < 10; i++) {
        adc += adc1_get_raw(ADC1_CHANNEL_7);
    }
    adc /= 10;
    float volts = (float)adc * ADC_TO_VOLTAGE_CONVERSION;
    Serial.printf("ADC:%d  %.03f volts\n", adc, volts);
    return volts;
}