#include <cmath>
#include <iostream>
#include <random>
#include "Control.h"
#include "Ethernet.h"
#include "Robot.h"
#include "adevs/adevs.h"
#include "adevs/fmi.h"
#include "io.h"


// using namespace adevs;

/**
  * Demonstrates control of a robotic arm when sensor measurements and
  * actuator commands pass through a shared communication network, such
  * as an Ethernet.
  */

/// Labels for input and output ports
static int const controlAddr = 1;
static int const robotAddr = 2;
/// Number of sources of background traffic
static int numApps = 0;

/// This atomic model generates traffic for the network
/// using a Poisson process.
class App : public AtomicModel {
  public:
    static int const data_out;

    App(double frequency, int bytes)
        : AtomicModel(), bytes(bytes), random(std ::exponential_distribution<>(1.0 / frequency)) {}

    /// Calculate the time to next packet
    double ta() { return random(generator); }

    void delta_int() {}

    void delta_ext(double, std::list<IO_Type> const &) {}

    void delta_conf(std::list<IO_Type> const &) {}


    /// Put a packet on the network
    void output_func(std::list<IO_Type> &yb) {
        IO_Type y;
        y.port = data_out;
        y.value = new NetworkData(NetworkData::APP_DATA, 999, bytes, new SimObject());
        yb.push_back(y);
    }

  private:
    int const bytes;
    std::random_device rd = std::random_device();
    std::mt19937 generator = std::mt19937(rd());
    std::exponential_distribution<> random;
};

int const App::data_out = 0;

/// Extension of the Control modelica model that implements
/// the control strategy.
class ControlExt : public Control {
  public:
    static int const sample;
    static int const command;

    ControlExt() : Control(), doCmd(false) {
        for (int i = 0; i < 2; i++) {
            T[i] = err[i] = ierr[i] = 0.0;
        }
    }
    double time_event_func(double const* q) {
        double tSup = Control::time_event_func(q);
        // Control model has no events of its own
        assert(tSup == adevs_inf<double>());
        if (doCmd) {
            return 0.0;
        } else {
            return tSup;
        }
    }
    void internal_event(double* q, bool const* state_event) {
        Control::internal_event(q, state_event);
        doCmd = false;
    }
    void external_event(double* q, double e, std::list<IO_Type> const &xb) {
        // We generate commands in response to new data
        Control::external_event(q, e, xb);
        // Nothing to do if no time has elapsed
        if (e == 0.0) {
            return;
        }
        process_input_data(e, xb);
        Control::external_event(q, e, xb);
    }
    void confluent_event(double* q, bool const* state_event, std::list<IO_Type> const &xb) {
        double h = time_event_func(q);
        // We generate commands in response to new data
        Control::confluent_event(q, state_event, xb);
        // Nothing to do if no time has elapsed
        if (h == 0.0) {
            return;
        }
        process_input_data(h, xb);
        Control::confluent_event(q, state_event, xb);
    }
    void output_func(double const* q, bool const* state_event, std::list<IO_Type> &yb) {
        Control::output_func(q, state_event, yb);
        CommandSig* sig = new CommandSig(T[0], T[1]);
        IO_Type msg;
        msg.port = command;
        msg.value = new NetworkData(NetworkData::APP_DATA, robotAddr, 100, sig);
        yb.push_back(msg);
    }

  private:
    bool doCmd;
    double err[2], ierr[2], T[2];
    void process_input_data(double h, std::list<IO_Type> const &xb) {
        for (std::list<IO_Type>::const_iterator iter = xb.begin(); iter != xb.end(); iter++) {
            NetworkData* pkt = dynamic_cast<NetworkData*>((*iter).value);
            SampleSig* sig = dynamic_cast<SampleSig*>(pkt->getPayload());
            process_info(sig->getQ1(), sig->getQ2(), h);
        }
    }
    void process_info(double q1, double q2, double h) {
        double oldErr[2], derr[2];
        oldErr[0] = err[0];
        oldErr[1] = err[1];
        err[0] = get_qd1() - q1;
        err[1] = get_qd2() - q2;
        for (int i = 0; i < 2; i++) {
            derr[i] = (err[i] - oldErr[i]) / h;
            ierr[i] += h * err[i];
            T[i] = 2000.0 * (err[i] + 0.05 * derr[i] + ierr[i] / 300.0);
        }
        doCmd = true;
    }
};

int const ControlExt::sample = 0;
int const ControlExt::command = 1;

/// Extends the Robot modelica model
class RobotExt : public Robot {
  public:
    static int const sample;
    static int const command;

    RobotExt() : Robot(), doSample(true) {
        set_T_1_(0.0);
        set_T_2_(0.0);
    }
    double time_event_func(double const* q) {
        // The sampled robot will have time events.
        double tSup = Robot::time_event_func(q);
        if (doSample) {
            return 0.0;
        } else {
            return tSup;
        }
    }
    void internal_event(double* q, bool const* state_event) {
        // Give the FMI an opportunity to process its own events
        Robot::internal_event(q, state_event);
        // Do we want to generate a sample for the controller?
        test_for_sample();
        // Give the FMI an opportunity to process its own events
        // after we have changed any state variables due to our
        // own event.
        Robot::internal_event(q, state_event);
    }
    void external_event(double* q, double e, std::list<IO_Type> const &xb) {
        Robot::external_event(q, e, xb);
        process_input_data(xb);
        Robot::external_event(q, e, xb);
    }
    void confluent_event(double* q, bool const* state_event, std::list<IO_Type> const &xb) {
        Robot::confluent_event(q, state_event, xb);
        test_for_sample();
        process_input_data(xb);
        Robot::confluent_event(q, state_event, xb);
    }
    void output_func(double const* q, bool const* state_event, std::list<IO_Type> &yb) {
        if (doSample) {
            SampleSig* sig = new SampleSig(get_q1(), get_q2());
            IO_Type msg;
            msg.port = sample;
            msg.value = new NetworkData(NetworkData::APP_DATA, controlAddr, 100, sig);
            yb.push_back(msg);
        }
    }

  private:
    double q1_sample_value, q2_sample_value;
    bool doSample;

    void process_input_data(std::list<IO_Type> const &xb) {
        for (std::list<IO_Type>::const_iterator iter = xb.begin(); iter != xb.end(); iter++) {
            assert((*iter).port == command);
            assert((*iter).value != NULL);
            NetworkData* pkt = dynamic_cast<NetworkData*>((*iter).value);
            CommandSig* sig = dynamic_cast<CommandSig*>(pkt->getPayload());
            process_command(sig->getT1(), sig->getT2());
        }
    }
    void process_command(double T1, double T2) {
        set_T_1_(T1);
        set_T_2_(T2);
    }
    void test_for_sample() {
        doSample = (q1_sample_value != get_q1_sample() || q2_sample_value != get_q2_sample());
        if (doSample) {
            q1_sample_value = get_q1_sample();
            q2_sample_value = get_q2_sample();
        }
    }
};

int const RobotExt::sample = 1;
int const RobotExt::command = 2;

// Connect control and robot directly by using this function
void makeDirectNetwork(Digraph<SimObject*>* model, Devs<IO_Type>* hybrid_ctrl,
                       Devs<IO_Type>* hybrid_arm) {
    model->couple(hybrid_arm, RobotExt::sample, hybrid_ctrl, ControlExt::sample);
    model->couple(hybrid_ctrl, ControlExt::command, hybrid_arm, RobotExt::command);
}

// Or connect them through a shared network with this function
void makeEtherNetwork(Digraph<SimObject*>* model, Devs<IO_Type>* hybrid_ctrl,
                      Devs<IO_Type>* hybrid_arm) {
    Ethernet* net = new Ethernet();
    model->add(net);
    NetworkCard* robotApp = new NetworkCard(robotAddr);
    NetworkCard* controlApp = new NetworkCard(controlAddr);
    int to_app, from_app;
    net->attach(robotApp, to_app, from_app);
    model->couple(hybrid_arm, RobotExt::sample, net, from_app);
    model->couple(net, to_app, hybrid_arm, RobotExt::command);
    net->attach(controlApp, to_app, from_app);
    model->couple(hybrid_ctrl, ControlExt::command, net, from_app);
    model->couple(net, to_app, hybrid_ctrl, ControlExt::sample);

    for (int i = 0; i < numApps; i++) {
        App* app = new App(125.0, 100);
        model->add(app);
        NetworkCard* nicApp = new NetworkCard(-9999);
        net->attach(nicApp, to_app, from_app);
        model->couple(app, app->data_out, net, from_app);
    }
}

void print_report(RobotExt* arm, double t) {}

int main(int argc, char** argv) {
    if (argc == 2) {
        numApps = atoi(argv[1]);
    }
    RobotExt* arm = new RobotExt();
    Hybrid<IO_Type>* hybrid_arm =
        new Hybrid<IO_Type>(arm, new rk_45<IO_Type>(arm, 1E-5, 0.001),
                            new discontinuous_event_locator<IO_Type>(arm, 1E-5));
    ControlExt* ctrl = new ControlExt();
    Hybrid<IO_Type>* hybrid_ctrl =
        new Hybrid<IO_Type>(ctrl, new rk_45<IO_Type>(ctrl, 1E-5, 0.001),
                            new discontinuous_event_locator<IO_Type>(ctrl, 1E-5));
    Digraph<SimObject*>* model = new Digraph<SimObject*>();
    model->add(hybrid_ctrl);
    model->add(hybrid_arm);
    // makeDirectNetwork(model,hybrid_ctrl,hybrid_arm);
    makeEtherNetwork(model, hybrid_ctrl, hybrid_arm);
    // Create the simulator
    Simulator<IO_Type>* sim = new Simulator<IO_Type>(model);
    // Run the simulation, testing the solution as we go
    double tReport = 0.0;
    double maxError = 0.0;
    while (sim->nextEventTime() <= 20.0) {
        if (maxError < arm->get_error()) {
            maxError = arm->get_error();
        }
        double t = sim->nextEventTime();
        sim->execNextEvent();
        if (t != tReport) {
            tReport = t;
            std::cout << tReport << " " << arm->get_x() << " " << arm->get_z() << " "
                      << ctrl->get_xd() << " " << ctrl->get_zd() << " " << arm->get_q1() << " "
                      << arm->get_q2() << " " << arm->get_error() << " " << maxError << std::endl;
        }
    }
    if (maxError < arm->get_error()) {
        maxError = arm->get_error();
    }
    std::cerr << "L1 err = " << maxError << std::endl;
    delete sim;
    delete model;
    return 0;
}
