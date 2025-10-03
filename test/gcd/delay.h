#ifndef _delay_h_
#define _delay_h_
#include <cassert>
#include <cstdio>
#include <list>
#include "adevs/adevs.h"
#include "object.h"

using pin_t = adevs::pin_t;
using Atomic = adevs::Atomic<ObjectPtr>;

typedef std::list<std::pair<double, ObjectPtr>> delay_q;

class delay : public Atomic {
  public:
    pin_t in;
    pin_t out;

    delay(double t) : Atomic(), dt(t), sigma(adevs_inf<double>()) {}
    void delta_int() {
        delay_q::iterator i;
        for (i = q.begin(); i != q.end(); i++) {
            (*i).first -= ta();
        }
        while (!q.empty()) {
            if (q.front().first <= 1E-14) {
                q.pop_front();
            } else {
                assert(q.front().first >= 0.0);
                sigma = q.front().first;
                return;
            }
        }
        sigma = adevs_inf<double>();
    }
    void delta_ext(double e, std::list<PinValue> const &x) {
        delay_q::iterator i;
        for (i = q.begin(); i != q.end(); i++) {
            (*i).first -= e;
        }
        std::list<PinValue>::const_iterator xi;
        for (xi = x.begin(); xi != x.end(); xi++) {
            assert((*xi).pin == in);
            q.push_back(std::pair<double, ObjectPtr>(dt, ObjectPtr(new object(*((*xi).value)))));
        }
        assert(q.front().first >= 0.0);
        sigma = q.front().first;
    }
    void delta_conf(std::list<PinValue> const &x) {
        delta_int();
        delta_ext(0.0, x);
    }
    void output_func(std::list<PinValue> &y) {
        delay_q::iterator i;
        for (i = q.begin(); i != q.end(); i++) {
            if ((*i).first <= ta()) {
                PinValue pv;
                pv.pin = out;
                pv.value = ObjectPtr(new object(*((*i).second)));
                y.push_back(pv);
            }
        }
    }
    double ta() { return sigma; }

  private:
    double dt, sigma;
    delay_q q;
};

#endif
