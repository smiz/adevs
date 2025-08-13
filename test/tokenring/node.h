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

using Bag = std::list<adevs::PinValue<shared_ptr<token_t>>>;

class Node : public adevs::Atomic<shared_ptr<token_t>> {
  public:
    const adevs::pin_t in;
    const adevs::pin_t out;

    Node(int ID, int holdtime, shared_ptr<token_t> token)
        : adevs::Atomic<shared_ptr<token_t>>(),
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
        adevs::PinValue<shared_ptr<token_t>> pv(out, out_token);
        y.push_back(pv);
    }


  private:
    int ID;
    shared_ptr<token_t> token;
    shared_ptr<token_t> out_token;

    double holdtime, sigma, t;
};

#endif
