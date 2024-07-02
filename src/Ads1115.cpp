#include <Arduino.h>

#include "Ads1115.h"

static uint _numberOfSamples = 0;
static uint _adcChannel = 0;
static float _convertedValue;

static ADS1115 ADS(0x48);
static SemaphoreHandle_t xBinarySemaphore;

static TaskHandle_t xTaskToNotify = NULL;

static byte _alertInterruptPin = 33;

// The ISR routine called when the ADC conversion is ready
void ISR_RMScallback() {
  ets_printf("IRS_RMS\n");
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// The task which performs the RMS calculation
void ISR_RMSprocessing(void * parameter)
{
  uint32_t sumAdc = 0;
  uint32_t sumSquareAdc = 0;
  uint32_t samplesCount = 0;

  for(;;) {
    // block and wait for an interrupt event
    if (xSemaphoreTake(xBinarySemaphore, pdMS_TO_TICKS(10000)) == pdFALSE) break;
    // int adc = ADS.getValue();
    // sumAdc += adc;
    // sumSquareAdc += (adc * adc);
    samplesCount++;
    ets_printf("loop %d\n", samplesCount);

    // if the expected number of samples is reached then exit
    if (samplesCount == _numberOfSamples) {
      float rmsVoltage = (float)(ADS.getMaxVoltage() / (1 << 16)) * sqrt(((double)sumSquareAdc / _numberOfSamples) - pow((double)sumAdc / _numberOfSamples, 2));
      ets_printf("%d;%d;%d;%d\n", _numberOfSamples, sumAdc, sumSquareAdc, (int32_t)(rmsVoltage * 1000));
      break;
    }
  }

  xTaskNotify(xTaskToNotify, 0, eNoAction);
  vTaskDelete(NULL);
}

float readRmsVoltage(uint channel, uint numberOfSamples) {
  _adcChannel = channel;
  _numberOfSamples = numberOfSamples;

  pinMode(_alertInterruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_alertInterruptPin), ISR_RMScallback, FALLING);

  xBinarySemaphore = xSemaphoreCreateBinary();

  xTaskCreate(
      ISR_RMSprocessing,       /* Task function. */
      "ISR_RMSprocessing",     /* name of task. */
      configMINIMAL_STACK_SIZE + 100, /* Stack size of task */
      NULL,                    /* parameter of the task */
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
  vSemaphoreDelete(xBinarySemaphore);

  return _convertedValue;
}

  