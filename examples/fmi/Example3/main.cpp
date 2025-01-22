#include <iostream>
#include "CherryBomb.h"
#include "adevs/adevs.h"
#include "adevs/fmi.h"
using namespace std;
using namespace adevs;

/**
 * This is an FMI + Modelica version of the bouncing bomb example
 * in the examples/cherry_bomb directory.
 */
class CherryBombExt :
    // Derive model from the adevs FMI class
    public CherryBomb {
  public:
    // Constructor loads the FMI
    CherryBombExt() : CherryBomb() {}
    // Internal transition function
    void internal_event(double* q, bool const* state_event) {
        // Call the method of the base class
        CherryBomb::internal_event(q, state_event);
    }
    // External transition function
    void external_event(double* q, double e, list<std::string> const &xb) {
        // Call the method of the base class
        CherryBomb::external_event(q, e, xb);
        // Drop the ball
        set_dropped(true);
        // Call the base class method again to
        // recalculate variables that depend on
        // the dropped variable
        CherryBomb::external_event(q, e, xb);
    }
    // Time remaining to the next sample
    double time_event_func(double const* q) {
        // Return the value of the base class
        return CherryBomb::time_event_func(q);
    }
};

/**
 * A miscreant drops the ball and reports the explosion.
 */
class Miscreant : public adevs::Atomic<std::string> {
  public:
    Miscreant() : adevs::Atomic<std::string>(), start(true), tstart(1.0) {}
    double ta() { return ((start) ? tstart : adevs_inf<double>()); }
    void delta_int() { start = false; }
    void delta_ext(double e, list<std::string> const &xb) {}
    void delta_conf(list<std::string> const &) {}
    void output_func(list<std::string> &yb) {
        if (start) {
            yb.push_back("light");
        }
    }


  private:
    bool start;
    double const tstart;
};

int main() {
    // Create our model of the bomb
    CherryBombExt* bomb = new CherryBombExt();
    // Wrap a set of solvers around it
    Hybrid<std::string>* hybrid_model = new Hybrid<std::string>(
        bomb,  // Model to simulate
        new corrected_euler<std::string>(bomb, 1E-5, 0.01),  // ODE solver
        new discontinuous_event_locator<std::string>(bomb,
                                                     1E-5)  // Event locator
        // You must use this event locator for OpenModelica because it does
        // not generate continuous zero crossing functions
    );
    // Couple the miscreant and the bomb
    SimpleDigraph<std::string>* model = new SimpleDigraph<std::string>();
    Miscreant* miscreant = new Miscreant();
    model->add(miscreant);
    model->add(hybrid_model);
    // Input from the miscreate lights the fuse and drops the bomb
    model->couple(miscreant, hybrid_model);
    // Create the simulator
    Simulator<std::string>* sim = new Simulator<std::string>(model);
    // Run the simulation for ten seconds
    while (!bomb->get_exploded()) {
        cout << sim->nextEventTime() << " ";
        sim->execNextEvent();
        cout << bomb->get_h() << " " << bomb->get_fuseTime() << " "
             << bomb->get_exploded() << endl;
    }
    // Cleanup
    delete sim;
    delete hybrid_model;
    // Done!
    return 0;
}
