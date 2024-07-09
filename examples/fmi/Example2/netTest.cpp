#include <cmath>
#include <iostream>
#include "Ethernet.h"
using namespace std;
using namespace adevs;

static int const numApps = 3;
static double const tend = 10.0;

class App : public AtomicModel {
  public:
    static int const data_out;
    static int const data_in;

    App(int addr, double freq)
        : AtomicModel(),
          addr(addr),
          t(0.0),
          outstanding(0),
          send(0),
          recv(0),
          freq(freq),
          timeToSend(1.0 / freq) {}
    double ta() {
        if (t < tend) {
            return timeToSend;
        } else {
            return adevs_inf<double>();
        }
    }
    void delta_int() {
        t += ta();
        timeToSend = 1.0 / freq;
        send++;
    }
    void delta_ext(double e, Bag<IO_Type> const &xb) {
        timeToSend -= e;
        t += e;
        assert(xb.size() == 1);
        recv++;
    }
    void delta_conf(Bag<IO_Type> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }
    void output_func(Bag<IO_Type> &yb) {
        IO_Type y;
        y.port = data_out;
        y.value = new NetworkData(NetworkData::APP_DATA, (addr + 1) % numApps,
                                  100000, new SimObject());
        yb.push_back(y);
    }
    int getSent() const { return send; }
    int getRecv() const { return recv; }

  private:
    int const addr;
    double t;
    int outstanding;
    int send, recv;
    double const freq;
    double timeToSend;
};

int const App::data_out = 0;
int const App::data_in = 1;

int main() {
    Digraph<SimObject*>* model = new Digraph<SimObject*>();
    NetworkCard* nic[numApps];
    App* app[numApps];
    Ethernet* net = new Ethernet();
    model->add(net);
    for (int i = 0; i < numApps; i++) {
        int to_app, from_app;
        nic[i] = new NetworkCard(i, 1000);
        app[i] = new App(i, 100 * (i + 1));
        model->add(app[i]);
        net->attach(nic[i], to_app, from_app);
        model->couple(app[i], app[i]->data_out, net, from_app);
        model->couple(net, to_app, app[i], app[i]->data_in);
    }
    // Create the simulator
    Simulator<IO_Type>* sim = new Simulator<IO_Type>(model);
    // Run the simulation, testing the solution as we go
    while (sim->nextEventTime() < adevs_inf<double>()) {
        sim->execNextEvent();
    }
    for (int i = 0; i < numApps; i++) {
        cout << i << "," << nic[i]->getCollisions() << "," << app[i]->getSent()
             << "," << app[i]->getRecv() << endl;
    }
    delete sim;
    delete net;
    return 0;
}
