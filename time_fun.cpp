// Marcin Bieganek 
#include "router.hpp"

void update_time_left(struct timeval *start, struct timeval *end, struct timeval *tv) {
    struct timeval time_used;
    timersub(end, start, &time_used);
    if (tv->tv_sec > time_used.tv_sec)
        timersub(tv, &time_used, tv);
    else if (tv->tv_sec < time_used.tv_sec)
        timersub(&time_used, tv, tv);
    else {
        if (tv->tv_usec >= time_used.tv_usec)
            timersub(tv, &time_used, tv);
        else
            timersub(&time_used, tv, tv);
    }
}