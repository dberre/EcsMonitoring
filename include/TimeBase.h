#ifndef _TIME_BASE_H
#define _TIME_BASE_H

#include <time.h>

class TimeBase {
    public:
    TimeBase();
    void init(time_t);
    time_t getNextTick();
    time_t getCurrentTick();

    private:
    time_t current;
};

#endif /* _TIME_BASE_H */