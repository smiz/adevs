/**
 * This models two nodes in a network. Node A sends
 * a message to B, and then waits 60 seconds for
 * a reply. If it gets not reply, it reports failure.
 * If it gets the reply, it reports success. A
 * late reply is simple dropped.
 */
#include <cstdlib>
#include <iostream>
#include "adevs/adevs.h"
using namespace adevs;
using namespace std;


class Sender : public Atomic<string> {
  public:
    Sender() : Atomic<string>(), mode(Sender::Mode::SEND) {}

    double ta() {
        if (mode == Sender::Mode::SEND) {
            return 0.0;
        } else if (mode == Sender::Mode::WAIT) {
            return 60.0;
        } else {
            return DBL_MAX;
        }
    }

    void delta_int() {
        // If we send a message, then wait for a replay
        // Otherwise the replay is late
        if (mode == Sender::Mode::SEND) {
            mode = Sender::Mode::WAIT;
        } else {
            cout << "No reply" << endl;
            mode = Sender::Mode::IDLE;
        }
    }

    void delta_ext(double e, Bag<string> const &xb) {
        cout << "Recvd " << *(xb.begin()) << endl;
        if (mode == Sender::Mode::IDLE) {
            cout << "Late reply" << endl;
        } else {
            cout << "Reply on time" << endl;
        }
        mode = Sender::Mode::IDLE;
    }

    void delta_conf(Bag<string> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }

    void output_func(Bag<string> &xb) { xb.push_back("Hello?"); }

  private:
    enum class Mode { SEND, WAIT, IDLE };
    Mode mode;
};

class Receiver : public Atomic<string> {
  public:
    Receiver() : Atomic<string>(), mode(Receiver::Mode::IDLE) {}

    void delta_int() { mode = Receiver::Mode::IDLE; }

    void delta_ext(double, Bag<string> const &) {
        mode = Receiver::Mode::REPLY;
    }

    void delta_conf(Bag<string> const &) { mode = Receiver::Mode::REPLY; }

    void output_func(Bag<string> &yb) { yb.push_back("Hi!"); }

    double ta() {
        if (mode == Receiver::Mode::REPLY) {
            return rand() % 100;
        } else {
            return DBL_MAX;
        }
    }

  private:
    enum class Mode { REPLY, IDLE };
    Mode mode;
};

int main(int argc, char** argv) {
    if (argc > 1) {
        srand(atoi(argv[1]));
    }

    shared_ptr<SimpleDigraph<string>> digraph =
        make_shared<SimpleDigraph<string>>();
    shared_ptr<Sender> sender = make_shared<Sender>();
    shared_ptr<Receiver> receiver = make_shared<Receiver>();

    digraph->add(sender);
    digraph->add(receiver);
    digraph->couple(sender, receiver);
    digraph->couple(receiver, sender);

    shared_ptr<Simulator<string>> sim = make_shared<Simulator<string>>(digraph);

    while (sim->nextEventTime() < 200.0) {
        sim->execNextEvent();
    }

    return 0;
}
