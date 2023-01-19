#include <driver/adc.h>

const float ADC_TO_VOLTAGE_CONVERSION = 1.0F;

bool setupVoltageSensor() {
    if (adc1_config_width(ADC_WIDTH_BIT_12) == ESP_OK) {
        if (adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0) == ESP_OK) {
            return true;
        }
    }
    return false;
}

float getVoltage() {
    int val = adc1_get_raw(ADC1_CHANNEL_0);
    return (float)val * ADC_TO_VOLTAGE_CONVERSION;
}

bool getHeaterState() {
    return false;
}