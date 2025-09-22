#ifndef _node_h_
#define _node_h_

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"

struct token_t {
    int value;
    token_t(int value = 0) : value(value) {}
};

using Bag = std::list<adevs::PinValue<std::shared_ptr<token_t>>>;
using Atomic = adevs::Atomic<std::shared_ptr<token_t>>;
using PinValue = adevs::PinValue<std::shared_ptr<token_t>>;
using pin_t = adevs::pin_t;

class Node : public Atomic {
  public:
    pin_t const in;
    pin_t const out;

    Node(int ID, int holdtime, std::shared_ptr<token_t> token)
        : Atomic(),
          ID(ID),
          token(token),
          out_token(token),
          holdtime(holdtime),
          sigma(adevs_inf<double>()),
          t(0.0) {
        if (token != nullptr) {
            sigma = holdtime;
        }
    }

    double ta() { return sigma; }

    void delta_int() {
        assert(token != nullptr);
        t += ta();
        printf("%d sent %d @ t = %.0f\n", ID, out_token->value, t);
        out_token = token;
        sigma = adevs_inf<double>();
    }

    void delta_ext(double e, Bag const &x) {
        t += e;
        assert(x.size() == 1);
        token = (*(x.begin())).value;
        if (out_token == nullptr) {
            out_token = token;
        }
        sigma = holdtime;
        printf("%d got %d @ t = %.0f\n", ID, token->value, t);
    }

    void delta_conf(Bag const &x) {
        t += ta();
        assert(x.size() == 1);
        printf("%d sent %d @ t = %.0f\n", ID, token->value, t);
        out_token = token = (*(x.begin())).value;
        sigma = holdtime;
        printf("%d got %d @ t = %.0f\n", ID, token->value, t);
    }

    void output_func(Bag &y) {
        PinValue pv(out, out_token);
        y.push_back(pv);
    }


  private:
    int ID;
    std::shared_ptr<token_t> token;
    std::shared_ptr<token_t> out_token;

    double holdtime, sigma, t;
};

#endif
