#include "TimeBase.h"

TimeBase::TimeBase() {
    localtime(&current);
}

void TimeBase::init(time_t origin) {
    current = origin;
}

time_t TimeBase::getCurrentTick() {
    return current;
}

time_t TimeBase::getNextTick() {
    current += 60;
    return current;
}