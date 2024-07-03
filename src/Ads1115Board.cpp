#include <Arduino.h>

#include "Ads1115Board.h"

Ads1115Board *Ads1115Board::_instance = 0;

ADS1115 *Ads1115Board::_board = 0;

TaskHandle_t Ads1115Board::_taskToNotifyFromISR = 0;

Ads1115Board::Ads1115Board() {
  _board = new ADS1115(0x48);

  pinMode(_alertInterruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_alertInterruptPin), ISR_RMScallback, FALLING);
}

Ads1115Board *Ads1115Board::getInstance() {
  if (_instance == 0) {
    _instance = new Ads1115Board();
  }
  return _instance;
}

float Ads1115Board::readRmsVoltage(uint channel, uint numberOfSamples) {

  //_board->begin();

  // range 6,144 volts
  _board->setGain(0);

  // set data rate to maximum (860 samples per second)
  _board->setDataRate(7);

  // configure to have the RDY signal after one conversion
  //_board->setComparatorThresholdHigh(0x8000);
  //_board->setComparatorThresholdLow(0x0000);
  _board->setComparatorQueConvert(0);

  // start the continuous mode
  _board->setMode(0);

  // flush any pending notification (interrupt remains active)
  while (ulTaskNotifyTake(pdTRUE, 0) > 0);

  // trigger the first read
  //_board->requestADC(channel);

  uint32_t sumAdc = 0;
  uint32_t sumSquareAdc = 0;
  uint32_t samplesCount = 0;
  float rmsVoltage = 0.0;

  _taskToNotifyFromISR = xTaskGetCurrentTaskHandle();

  for(;;) {
    // block waiting for an interrupt event and leave in case of timeout
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10000)) == 0) break;

    // int adc = _board->getValue();
    int adc = (samplesCount == 1 ? 4000 : 2000); // FIXME
    sumAdc += adc;
    sumSquareAdc += (adc * adc);
    samplesCount++;
    ets_printf("loop %d\n", samplesCount);

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
void Ads1115Board::ISR_RMScallback() {
  // ets_printf("IRS_RMS\n");
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(_taskToNotifyFromISR, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
