#include <iostream>
#include "adevs/adevs.h"

using Atomic = adevs::Atomic<>;
using PinValue = adevs::PinValue<>;
using Simulator = adevs::Simulator<>;

/**
 * This example shows how to simulate a simple system that
 * changes its state and produces output at a fixed rate.
 * Here are console messages printed by this simulation:
 *
 * Next event is @ time 1
 * Output: 0
 * State: 1
 * Next event is @ time 2
 * Output: 1
 * State: 2
 * 
 * This offers some insight into how the simulator works. The
 * Simulator calculates the time of next event by using the
 * model's time advance function. The simulation clock is
 * advanced to that time and then the model's output function
 * is called followed by the internal transition function.
 */

/**
  * Our Periodic model is created by deriving it from the Atomic
  * class and providing implementation of each virtual method.
  * In this simple example, only the time advance function, 
  * output function, and internal transition function play are
  * important.
  */
class Periodic : public Atomic {
  public:
    /// Our constructor calls the default constructor and sets the
    /// initial value of a counter member variable, which is the
    /// state of our model.
    Periodic() : Atomic(), state(0) {}
    /// The time advance function is used by the Simulator class
    /// to schedule our next event. We return 1.0 to tell the simulator
    /// that our events occur at times 1, 2, 3, and so forth.
    double ta() { return 1.0; }
    /// We produce our current state as the output value before changing
    /// the state in the internal transition function.
    void output_func(std::list<PinValue> &) { std::cout << "Output: " << state << std::endl; }
    /// We change our state by incrementing it in our internal transition function.
    void delta_int() {
        state++;
        std::cout << "State: " << state << std::endl;
    }
    /// The external transition function is not used in this example.
    /// It is never called by the Simulator.
    void delta_ext(double, std::list<PinValue> const &) {}
    /// The confluent transition function is not used in this example,
    /// It is never called by the Simulator.
    void delta_conf(std::list<PinValue> const &) {}

  private:
    /// The state of our model is a single integer
    int state;
};

/**
 * The main function creates a Simulator for our Periodic model and runs
 * the simulation for 2 units of time.
 */
int main() {
    /// Create an instance of our Periodic model in a shared pointer
    auto model = std::make_shared<Periodic>();
    /// Create a simulator for our Periodic model
    Simulator simulator(model);
    /// Run the simulator for 2 units of time
    while (simulator.nextEventTime() <= 2.0) {
        /// Report when the next event will occur
        std::cout << "Next event is @ time " << simulator.nextEventTime() << std::endl;
        /// Execute the next event at that time
        simulator.execNextEvent();
    }
    /// Done!
    return 0;
}
