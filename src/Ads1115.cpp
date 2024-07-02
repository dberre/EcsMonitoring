#include <Arduino.h>

#include "Ads1115.h"

Ads1115Sensor *Ads1115Sensor::_defaultInstance = 0;

ADS1115 *Ads1115Sensor::_board = 0;

SemaphoreHandle_t Ads1115Sensor::xBinarySemaphore = 0;

TaskHandle_t Ads1115Sensor::xTaskToNotify = 0;

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
  _adcChannel = channel;
  _numberOfSamples = numberOfSamples;

  xTaskCreate(
      ISR_RMSprocessing,       /* Task function. */
      "ISR_RMSprocessing",     /* name of task. */
      configMINIMAL_STACK_SIZE + 100, /* Stack size of task */
      (void *)_numberOfSamples,                    /* parameter of the task */
      4,                       /* priority of the task TODO */
      NULL);

  /* Store the handle of the calling task. */
  xTaskToNotify = xTaskGetCurrentTaskHandle();

  // ADS.begin();

  // // range 6,144 volts
  // ADS.setGain(0);

  // // set data rate to maximum (860 samples per second)
  // ADS.setDataRate(7);

  // // configure to have the RDY signal after one conversion
  // ADS.setComparatorThresholdHigh(0x8000);
  // ADS.setComparatorThresholdLow(0x0000);
  // ADS.setComparatorQueConvert(0);

  // // start the continuous mode
  // ADS.setMode(0);

  // // trigger the first read
  // ADS.requestADC(_adcChannel);

  // wait for the result to be available with a 500ms timeout
  uint32_t ulNotifiedValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
  ets_printf("return:%d\n", ulNotifiedValue);
  // ADS.reset();

  return ulNotifiedValue / 1000.0;
}

// The ISR routine called when the ADC conversion is ready
void Ads1115Sensor::ISR_RMScallback() {
  ets_printf("IRS_RMS\n");
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// The task which performs the RMS calculation
void Ads1115Sensor::ISR_RMSprocessing(void * parameter) {
  uint32_t numberOfSamples = (uint32_t)parameter;
  
  uint32_t sumAdc = 0;
  uint32_t sumSquareAdc = 0;
  uint32_t samplesCount = 0;
  float rmsVoltage = 0.0;

  for(;;) {
    // block and wait for an interrupt event
    if (xSemaphoreTake(xBinarySemaphore, pdMS_TO_TICKS(10000)) == pdFALSE) break;
    // int adc = ADS.getValue();
    int adc = (samplesCount == 1 ? 4000 : 2000); // FIXME
    sumAdc += adc;
    sumSquareAdc += (adc * adc);
    samplesCount++;
    ets_printf("loop %d\n", samplesCount);

    // if the expected number of samples is reached then exit
    if (samplesCount == numberOfSamples) {
      rmsVoltage = (_board->getMaxVoltage() / (1 << 16)) * sqrt(((double)sumSquareAdc / numberOfSamples) - pow((double)sumAdc / numberOfSamples, 2));
      ets_printf("%d;%d;%d;%d\n", numberOfSamples, sumAdc, sumSquareAdc, (int32_t)(rmsVoltage * 1000));
      break;
    }
  }

  xTaskNotify(Ads1115Sensor::xTaskToNotify, (uint32_t)(rmsVoltage * 1000), eSetValueWithOverwrite);
  vTaskDelete(NULL);
}
