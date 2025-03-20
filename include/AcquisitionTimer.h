#ifndef _ACQUISITION_TIMER_H
#define _ACQUISITION_TIMER_H

#include <esp_timer.h>

class AcquisitionTimer {
    public:
    AcquisitionTimer(esp_timer_cb_t callback, uint64_t duration);
    void setDuration(uint64_t duration);
    void start();
    void stop();
    void suspend();
    void resume();

    private:
    esp_timer_create_args_t _timerConfig;
    esp_timer_handle_t _timer;
    uint64_t duration_;
    uint64_t suspendTimestamp_;
    uint64_t startTimeStamp_;
};

#endif /* _ACQUISITION_TIMER_H */