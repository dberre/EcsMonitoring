#include <Arduino.h>

#include "Ads1115Board.h"

Ads1115Board *Ads1115Board::_instance = 0;

ADS1115 *Ads1115Board::_board = 0;

TaskHandle_t Ads1115Board::_taskToNotifyFromISR = 0;

Ads1115Board::Ads1115Board() {
  _board = new ADS1115(0x48);
  #ifdef ESP32C3XIAO
    // pin D1 on the board
    _alertInterruptPin = 3;
  #else 
    // pin G33 on the board
    _alertInterruptPin = 33;
  #endif

  if (Wire.begin() == false) {
    log_e("Failed to initialize Wire");
  }
}

Ads1115Board *Ads1115Board::getInstance() {
  if (_instance == 0) {
    _instance = new Ads1115Board();
  }
  return _instance;
}

float Ads1115Board::readVoltage(uint channel, uint duration) {

  if(_board->begin() == false) {
    log_e("Connection to ADS1115 failed");
    return 0.0;
  }

  // range volts à définir (TODO)
  _board->setGain(1);

  // configure in single shot mode
  _board->setMode(1);

  int32_t sumAdc = 0;
  uint32_t numberOfSamples = 0;
  unsigned long tend = millis() + duration;

  for(;;) {
    _board->requestADC(channel);
    int adc = _board->getValue();
    sumAdc += adc;
    numberOfSamples++;
    if (millis() > tend) break;
  }

  float rmsVoltage = (_board->getMaxVoltage() / 32767) * ((double)sumAdc / numberOfSamples);
  Serial.printf("%d ADC; %.4f V\n", sumAdc / numberOfSamples, rmsVoltage);

  _board->reset();

  return rmsVoltage;
}

float Ads1115Board::readRmsVoltageAlt(uint channel, uint duration) {

  if(_board->begin() == false) {
    log_e("Connection to ADS1115 failed");
    return 0.0;
  }

  // range volts à définir (TODO)
  _board->setGain(1);

  // configure in single shot mode
  _board->setMode(1);

  int32_t sumAdc = 0;
  uint64_t sumSquareAdc = 0;
  uint32_t numberOfSamples = 0;
  unsigned long tend = millis() + duration;

  for(;;) {
    _board->requestADC_Differential_0_1();
    int adc = _board->getValue();
    sumAdc += adc;
    sumSquareAdc += (adc * adc);
    numberOfSamples++;
  }

  double part1 = (double)(sumSquareAdc / numberOfSamples);
  double part2 = pow((double)sumAdc / numberOfSamples, 2);
  // float rmsVoltage = (_board->getMaxVoltage() / 32767) * sqrt(fabs(((double)(sumSquareAdc / numberOfSamples)) - pow((double)sumAdc / numberOfSamples, 2)));
  float rmsVoltage = (_board->getMaxVoltage() / 32767) * sqrt(fabs(part1 - part2));
  Serial.printf("%.0f ADC; %.0f ADC2; %.4f V\n", part1, part2, rmsVoltage);

  _board->reset();

  return rmsVoltage;
}

float Ads1115Board::readRmsVoltage(uint channel, uint duration) {

  _taskToNotifyFromISR = xTaskGetCurrentTaskHandle();
  pinMode(_alertInterruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_alertInterruptPin), ISR_RMScallback, FALLING);

  if(_board->begin() == false) {
    log_e("Connection to ADS1115 failed");
    return 0.0;
  }

  // range volts à définir (TODO)
  _board->setGain(2);

  // set data rate to maximum (860 samples per second)
  _board->setDataRate(7);

  // configure to have the RDY signal after one conversion
  _board->setComparatorThresholdHigh(0x8000);
  _board->setComparatorThresholdLow(0x0000);
  _board->setComparatorQueConvert(0);

  // start the continuous mode
  _board->setMode(0);  // FIXME

  // flush any pending notification (interrupt remains active)
  while (ulTaskNotifyTake(pdTRUE, 0) > 0);

  // trigger the first read
  // _board->requestADC(channel);
  _board->requestADC_Differential_0_1();

  int32_t sumAdc = 0;
  uint64_t sumSquareAdc = 0;
  uint32_t numberOfSamples = 0;
  float rmsVoltage = 0.0;

  unsigned long tend = millis() + duration;
  
  for(;;) {
    // block waiting for an interrupt event and leave in case of timeout
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)) == 0) {
      log_e("Ads1115: timeout");
      break;
    }

    int adc = _board->getValue();
    sumAdc += adc;
    sumSquareAdc += (adc * adc);
    numberOfSamples++;

    if (millis() > tend) {
      rmsVoltage = (_board->getMaxVoltage() / 32767) * sqrt(fabs(((double)(sumSquareAdc / numberOfSamples)) - pow((double)sumAdc / numberOfSamples, 2)));
      // Serial.printf("%d;%d;%ld;%f\n", numberOfSamples, sumAdc, sumSquareAdc, rmsVoltage);
      break;
    }
  }

  detachInterrupt(_alertInterruptPin);
  _board->reset();

  return rmsVoltage;
}

// The ISR routine called when the ADC conversion is ready
void Ads1115Board::ISR_RMScallback() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(_taskToNotifyFromISR, &xHigherPriorityTaskWoken);
  // FIXME xHigherPriorityTaskWoken removed arg
    #ifdef ESP32C3XIAO
      portYIELD_FROM_ISR();
    #else
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    #endif
}
