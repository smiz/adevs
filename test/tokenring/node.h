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

using PortValue = adevs::PortValue<shared_ptr<token_t>>;
using Network = adevs::Digraph<shared_ptr<token_t>>;
using Model = adevs::Devs<PortValue>;
using Simulator = adevs::Simulator<PortValue>;
using Devs = adevs::Devs<PortValue>;


class Node : public adevs::Atomic<PortValue> {
  public:
    static int const in;
    static int const out;
    //enum class Port { IN = 0, OUT = 1 };

    Node(int ID, int holdtime, shared_ptr<token_t> token)
        : adevs::Atomic<PortValue>(),
          ID(ID),
          token(token),
          out_token(token),
          holdtime(holdtime),
          sigma(DBL_MAX),
          t(0.0) {
        if (token != NULL) {
            sigma = holdtime;
        }
    }

    double ta() { return sigma; }

    void delta_int() {
        assert(token != NULL);
        t += ta();
        printf("%d sent %d @ t = %.0f\n", ID, out_token->value, t);
        out_token = token;
        sigma = DBL_MAX;
    }

    void delta_ext(double e, list<PortValue> const &x) {
        t += e;
        assert(x.size() == 1);
        token = static_cast<shared_ptr<token_t>>((*(x.begin())).value);
        if (out_token == NULL) {
            out_token = token;
        }
        sigma = holdtime;
        printf("%d got %d @ t = %.0f\n", ID, token->value, t);
    }

    void delta_conf(list<PortValue> const &x) {
        t += ta();
        assert(x.size() == 1);
        printf("%d sent %d @ t = %.0f\n", ID, token->value, t);
        out_token = token =
            static_cast<shared_ptr<token_t>>((*(x.begin())).value);
        sigma = holdtime;
        printf("%d got %d @ t = %.0f\n", ID, token->value, t);
    }

    void output_func(list<PortValue> &y) {
        PortValue pv(out, out_token);
        y.push_back(pv);
    }


  private:
    int ID;
    shared_ptr<token_t> token;
    shared_ptr<token_t> out_token;

    double holdtime, sigma, t;
};

int const Node::in(0);
int const Node::out(1);

#endif
