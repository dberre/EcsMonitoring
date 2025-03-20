#include <Arduino.h>
#include <esp_timer.h>

#include "WatchdogTimer.h"

WatchdogTimer::WatchdogTimer(esp_timer_cb_t callback, uint64_t duration) {
    duration_ = duration;
    _timerConfig.arg = NULL;
    _timerConfig.callback = callback;
    _timerConfig.dispatch_method = ESP_TIMER_TASK;
    _timerConfig.name = "WatchdogTimer";
    esp_timer_create(&_timerConfig, &_timer);
    esp_timer_start_once(_timer, duration_);
}

void WatchdogTimer::suspend() {
  if (_timer) {
    if(esp_timer_is_active(_timer)) {
      esp_timer_stop(_timer);
    }
  }
}  

void WatchdogTimer::restart() {
  if (_timer != NULL) {
    if(esp_timer_is_active(_timer)) {
      esp_timer_stop(_timer);
    }
    esp_timer_start_once(_timer, duration_);
  }
}

void WatchdogTimer::stop() {
  if (_timer != NULL) {
    if(esp_timer_is_active(_timer)) {
      esp_timer_stop(_timer);
    }
    esp_timer_delete(_timer);
    _timer = NULL;
  }
}