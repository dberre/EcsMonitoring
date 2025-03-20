#ifndef _WATCHDOG_TIMER_H
#define _WATCHDOG_TIMER_H

#include <esp_timer.h>

class WatchdogTimer {
    public:
    WatchdogTimer(esp_timer_cb_t callback, uint64_t duration);
    void suspend();
    void restart();
    void stop();

    private:
    esp_timer_create_args_t _timerConfig;
    esp_timer_handle_t _timer;
    uint64_t duration_;
};

#endif /* _WATCHDOG_TIMER_H */