#include <Arduino.h>

#include "Ads1115.h"

Ads1115Sensor *Ads1115Sensor::_defaultInstance = 0;

ADS1115 *Ads1115Sensor::_board = 0;

SemaphoreHandle_t Ads1115Sensor::xBinarySemaphore = 0;

TaskHandle_t Ads1115Sensor::xTaskToNotifyFromISR = 0;

Ads1115Sensor::Ads1115Sensor() {
  _board = new ADS1115(0x48);

  Ads1115Sensor::xBinarySemaphore = xSemaphoreCreateBinary();

  pinMode(_alertInterruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_alertInterruptPin), ISR_RMScallback, FALLING);
}

Ads1115Sensor *Ads1115Sensor::defaultInstance() {
  if (_defaultInstance == 0) {
    _defaultInstance = new Ads1115Sensor();
  }
  return _defaultInstance;
}

float Ads1115Sensor::readRmsVoltage(uint channel, uint numberOfSamples) {

  _board->begin();

  // range 6,144 volts
  _board->setGain(0);

  // set data rate to maximum (860 samples per second)
  _board->setDataRate(7);

  // configure to have the RDY signal after one conversion
  _board->setComparatorThresholdHigh(0x8000);
  _board->setComparatorThresholdLow(0x0000);
  _board->setComparatorQueConvert(0);

  // start the continuous mode
  _board->setMode(0);

  // trigger the first read
  _board->requestADC(channel);

  uint32_t sumAdc = 0;
  uint32_t sumSquareAdc = 0;
  uint32_t samplesCount = 0;
  float rmsVoltage = 0.0;

  xTaskToNotifyFromISR = xTaskGetCurrentTaskHandle();

  for(;;) {
    // block and wait for an interrupt event
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10000)) == 0) break;

    // int adc = ADS.getValue();
    int adc = (samplesCount == 1 ? 4000 : 2000); // FIXME
    sumAdc += adc;
    sumSquareAdc += (adc * adc);
    samplesCount++;
    // ets_printf("loop %d\n", samplesCount);

    // if the expected number of samples is reached then exit
    if (samplesCount == numberOfSamples) {
      rmsVoltage = (_board->getMaxVoltage() / (1 << 16)) * sqrt(((double)sumSquareAdc / numberOfSamples) - pow((double)sumAdc / numberOfSamples, 2));
      ets_printf("%d;%d;%d;%d\n", numberOfSamples, sumAdc, sumSquareAdc, (int32_t)(rmsVoltage * 1000));
      break;
    }
  }

  _board->reset();

  return rmsVoltage;
}

// The ISR routine called when the ADC conversion is ready
void Ads1115Sensor::ISR_RMScallback() {
  // ets_printf("IRS_RMS\n");
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(xTaskToNotifyFromISR, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
