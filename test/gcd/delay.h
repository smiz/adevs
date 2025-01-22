#ifndef _delay_h_
#define _delay_h_
#include <cassert>
#include <cstdio>
#include <list>
#include "adevs/adevs.h"
#include "object.h"

typedef std::list<std::pair<double, object*>> delay_q;

class delay : public adevs::Atomic<PortValue> {
  public:
    static int const in;
    static int const out;

    delay(double t) : adevs::Atomic<PortValue>(), dt(t), sigma(DBL_MAX) {}
    void delta_int() {
        delay_q::iterator i;
        for (i = q.begin(); i != q.end(); i++) {
            (*i).first -= ta();
        }
        while (!q.empty()) {
            if (q.front().first <= 1E-14) {
                delete q.front().second;
                q.pop_front();
            } else {
                assert(q.front().first >= 0.0);
                sigma = q.front().first;
                return;
            }
        }
        sigma = DBL_MAX;
    }
    void delta_ext(double e, list<PortValue> const &x) {
        delay_q::iterator i;
        for (i = q.begin(); i != q.end(); i++) {
            (*i).first -= e;
        }
        list<PortValue>::const_iterator xi;
        for (xi = x.begin(); xi != x.end(); xi++) {
            assert((*xi).port == in);
            q.push_back(
                std::pair<double, object*>(dt, new object(*((*xi).value))));
        }
        assert(q.front().first >= 0.0);
        sigma = q.front().first;
    }
    void delta_conf(list<PortValue> const &x) {
        delta_int();
        delta_ext(0.0, x);
    }
    void output_func(list<PortValue> &y) {
        delay_q::iterator i;
        for (i = q.begin(); i != q.end(); i++) {
            if ((*i).first <= ta()) {
                PortValue pv;
                pv.port = out;
                pv.value = new object(*((*i).second));
                y.push_back(pv);
            }
        }
    }
    double ta() { return sigma; }

  private:
    double dt, sigma;
    delay_q q;
};

int const delay::in = 0;
int const delay::out = 1;

#endif
