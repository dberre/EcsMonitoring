#include <Arduino.h>

#include "AcquisitionTimer.h"

AcquisitionTimer::AcquisitionTimer(esp_timer_cb_t callback) {
  _timer = NULL;
  _timerConfig.arg = NULL;
  _timerConfig.callback = callback;
  _timerConfig.dispatch_method = ESP_TIMER_TASK;
  _timerConfig.name = "AcquisitionTimer";
}

void AcquisitionTimer::start(uint64_t duration) {
  duration_ = duration  * 1000000ULL;
  if (_timer == NULL) {
    esp_timer_create(&_timerConfig, &_timer);
  }
  esp_timer_start_once(_timer, duration_);
  startTimeStamp_ = esp_timer_get_time();
}

void AcquisitionTimer::suspend() {
  esp_timer_stop(_timer);
  suspendTimestamp_ = esp_timer_get_time();
}

void AcquisitionTimer::resume() {
  uint64_t remaining = duration_ - (suspendTimestamp_ - startTimeStamp_);
  esp_timer_start_once(_timer, remaining);
}

void AcquisitionTimer::stop() {
  if (_timer != NULL) {
    esp_timer_stop(_timer);
    esp_timer_delete(_timer);
    _timer = NULL;
  }
}