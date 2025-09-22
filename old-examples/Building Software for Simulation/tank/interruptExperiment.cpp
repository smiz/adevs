#include <iostream>
#include "InterruptHandler.h"
#include "adevs/adevs.h"

// using namespace adevs;

// Listener for recording the state and output of the interrupt handler
class InterruptListener : public EventListener<SimEvent> {
  public:
    InterruptListener() {}
    void outputEvent(Atomic<SimEvent>* model, SimEvent const &value, double t) {
        std::cout << "Output, t = " << t << ", ";
        if (value.getType() == SIM_INTERRUPT) {
            std::cout << "interrupt" << std::endl;
        } else if (value.getType() == SIM_MOTOR_VOLTAGE) {
            std::cout << "el = " << value.simMotorVoltage().el
                 << ", er = " << value.simMotorVoltage().er << std::endl;
        }
    }
    void stateChange(Atomic<SimEvent>* model, double t) {
        InterruptHandler* ih = dynamic_cast<InterruptHandler*>(model);
        std::cout << "State, t = " << t;
        std::cout << ", el = " << ih->getLeftOutput();
        std::cout << ", er = " << ih->getRightOutput();
        std::cout << ", e'l = " << ih->getLastLeftOutput();
        std::cout << ", e'r = " << ih->getLastRightOutput();
        std::cout << ",\n\tc = " << ih->getCounter();
        std::cout << ",ol = " << ih->getLeftOnTime();
        std::cout << ",or = " << ih->getRightOnTime();
        std::cout << ",rl = " << ih->getLeftReverse();
        std::cout << ",rr = " << ih->getRightReverse();
        std::cout << ",i = ";
        if (ih->getPhase() == InterruptHandler::WAIT) {
            std::cout << "WAIT" << std::endl;
        } else if (ih->getPhase() == InterruptHandler::EXEC) {
            std::cout << "EXEC" << std::endl;
        } else if (ih->getPhase() == InterruptHandler::OUTPUT) {
            std::cout << "OUTPUT" << std::endl;
        }
    }
};

int main(int argc, char** argv) {
    // Make sure that a frequency was given
    if (argc != 2) {
        std::cout << "Must provide a signal frequency" << std::endl;
        return 0;
    }
    // Set the output precision to make the small time advances apparent
    std::cout.precision(12);
    // Create the model, event listener, and simulator
    InterruptHandler* ih = new InterruptHandler(atof(argv[1]));
    InterruptListener* l = new InterruptListener();
    Simulator<SimEvent>* sim = new Simulator<SimEvent>(ih);
    sim->addEventListener(l);
    // Print the initial state of the model
    l->stateChange(ih, 0.0);
    // Run the simulation
    while (true) {

        std::list<SimEvent> input;
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
            std::cout << std::endl;
            sim->execNextEvent();
        }
        // Simulate the transient events
        for (int i = 0; i < c && sim->nextEventTime() == t; i++) {
            std::cout << std::endl;
            sim->execNextEvent();
        }
        // Inject the input
        input.push_back(SimEvent(motor_setting));
        std::cout << std::endl;
        sim->computeNextState(input, t);
    }
    // Clean up
    delete sim;
    delete l;
    delete ih;
    return 0;
}
