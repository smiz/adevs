#include <iostream>
#include "HelloWorld.h"
#include "adevs/adevs.h"
#include "adevs/fmi.h"

// using namespace adevs;

/**
 * This model will sample its own continuous state and generate an output
 * with a frequency f in Hertz.
 */
class HelloWorldExt : public HelloWorld {
  public:
    // Constructor loads the FMI and sets the output frequency
    HelloWorldExt()
        : HelloWorld(),
          // Set frequency of output to 10 Hz
          f(10.0),
          // Time of first output
          tnext(1.0 / f) {}
    // Internal transition function
    void internal_event(double* q, bool const* state_event) {
        HelloWorld::internal_event(q, state_event);
        assert(get_time() == q[1]);  // These should match
        // Update the time to the next event
        tnext += 1.0 / f;
    }
    // Time remaining to the next sample
    double time_event_func(double const* q) {
        assert(get_time() == q[1]);  // These should match
        return tnext - get_time();
    }
    // Print state at each output event
    void output_func(double const* q, bool const* state_event,
                     std::list<double> &yb) {
        HelloWorld::output_func(q, state_event, yb);
        // Get the model state. This is real variable 0 according to modelDescription.xml
        double x = get_x();
        // Get the model parameter. This is real variable 2 according to modelDescription.xml
        double a = get_a();
        assert(q[0] == x);  // These should match
        // Output our state
        yb.push_back(x);
        // Print the time, computed state, and exact state
        std::cout << get_time() << " " << x << " " << exp(a * get_time()) << std::endl;
    }

  private:
    double const f;
    double tnext;
};

int main() {
    // Create our model
    FMI<double>* hello = new HelloWorldExt();
    // Wrap a set of solvers around it
    Hybrid* hybrid_model = new Hybrid(
        hello,                                           // Model to simulate
        new corrected_euler<double>(hello, 1E-5, 0.01),  // ODE solver
        new discontinuous_event_locator<double>(hello, 1E-5)  // Event locator
        // You must use this event locator for OpenModelica because it does
        // not generate continuous zero crossing functions
    );
    // Create the simulator
    Simulator* sim = new Simulator(hybrid_model);
    // Run the simulation for ten seconds
    while (sim->nextEventTime() <= 10.0) {
        sim->execNextEvent();
    }
    // Cleanup
    delete sim;
    delete hybrid_model;
    // Done!
    return 0;
}
