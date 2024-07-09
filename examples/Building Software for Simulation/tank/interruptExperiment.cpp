#include <iostream>
#include "InterruptHandler.h"
#include "adevs.h"
using namespace std;
using namespace adevs;

// Listener for recording the state and output of the interrupt handler
class InterruptListener : public EventListener<SimEvent> {
  public:
    InterruptListener() {}
    void outputEvent(Atomic<SimEvent>* model, SimEvent const &value, double t) {
        cout << "Output, t = " << t << ", ";
        if (value.getType() == SIM_INTERRUPT) {
            cout << "interrupt" << endl;
        } else if (value.getType() == SIM_MOTOR_VOLTAGE) {
            cout << "el = " << value.simMotorVoltage().el
                 << ", er = " << value.simMotorVoltage().er << endl;
        }
    }
    void stateChange(Atomic<SimEvent>* model, double t) {
        InterruptHandler* ih = dynamic_cast<InterruptHandler*>(model);
        cout << "State, t = " << t;
        cout << ", el = " << ih->getLeftOutput();
        cout << ", er = " << ih->getRightOutput();
        cout << ", e'l = " << ih->getLastLeftOutput();
        cout << ", e'r = " << ih->getLastRightOutput();
        cout << ",\n\tc = " << ih->getCounter();
        cout << ",ol = " << ih->getLeftOnTime();
        cout << ",or = " << ih->getRightOnTime();
        cout << ",rl = " << ih->getLeftReverse();
        cout << ",rr = " << ih->getRightReverse();
        cout << ",i = ";
        if (ih->getPhase() == InterruptHandler::WAIT) {
            cout << "WAIT" << endl;
        } else if (ih->getPhase() == InterruptHandler::EXEC) {
            cout << "EXEC" << endl;
        } else if (ih->getPhase() == InterruptHandler::OUTPUT) {
            cout << "OUTPUT" << endl;
        }
    }
};

int main(int argc, char** argv) {
    // Make sure that a frequency was given
    if (argc != 2) {
        cout << "Must provide a signal frequency" << endl;
        return 0;
    }
    // Set the output precision to make the small time advances apparent
    cout.precision(12);
    // Create the model, event listener, and simulator
    InterruptHandler* ih = new InterruptHandler(atof(argv[1]));
    InterruptListener* l = new InterruptListener();
    Simulator<SimEvent>* sim = new Simulator<SimEvent>(ih);
    sim->addEventListener(l);
    // Print the initial state of the model
    l->stateChange(ih, 0.0);
    // Run the simulation
    while (true) {
        // Bag for injecting the input
        Bag<SimEvent> input;
        // The value to inject
        SimMotorOnTime motor_setting;
        // Time to inject the input
        double t;
        int c;
        // Read the time and input values
        unsigned int o_l, o_r;
        cin >> t >> c >> o_l >> motor_setting.reverse_left >> o_r >>
            motor_setting.reverse_right;
        motor_setting.left = (unsigned char)o_l;
        motor_setting.right = (unsigned char)o_r;
        // If this is the end of the input, then quit
        if (cin.eof()) {
            break;
        }
        // Simulate until time t and then inject the input
        while (sim->nextEventTime() < t) {
            cout << endl;
            sim->execNextEvent();
        }
        // Simulate the transient events
        for (int i = 0; i < c && sim->nextEventTime() == t; i++) {
            cout << endl;
            sim->execNextEvent();
        }
        // Inject the input
        input.insert(SimEvent(motor_setting));
        cout << endl;
        sim->computeNextState(input, t);
    }
    // Clean up
    delete sim;
    delete l;
    delete ih;
    return 0;
}
