#ifndef _events_h_
#define _events_h_
#include "adevs/adevs.h"

/**
 * These two classes are the internal and external event types that
 * are used in the test.
 */

typedef enum { FAST, SLOW, STOP } Speed;

struct External {
    Speed speed;
    int value;
    static int num_existing;

    External(Speed spd = STOP, int value = 1) : speed(spd), value(value) {
        num_existing++;
    }
    ~External() { num_existing--; }
};

struct Internal {
    int value;
    double period;
    static int num_existing;

    Internal(double period = DBL_MAX, int value = 0)
        : value(value), period(period) {
        num_existing++;
    }
    Internal(Internal const &src) : value(src.value), period(src.period) {
        num_existing++;
    }
    ~Internal() { num_existing--; }
};

#endif
