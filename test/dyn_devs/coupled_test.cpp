#include <iostream>
#include <list>
#include <cassert>
#include "adevs/adevs.h"

/**
 * This test case consists of an Atomic model and the
 * coupled model that is spawns and removes. The Atomic
 * model creates a Coupled model and connects the 
 * coupled model to itself. The Coupled model is a simple
 * type of activity diagram. When the activities have
 * run to completion, the Coupled model informs the Atomic
 * model that its execution is complete. At that time
 * the Atomic model removes the Coupled model from the
 * simulation.
 */

/**
 * This is a simple action block in our trivial
 * SysML like activity diagram. It implements the
 * token flow model of execution.
 */
class Action: public adevs::Atomic<> {
    public:

    Action(unsigned inputs) : adevs::Atomic<>(), inputs(inputs), tokens(0) { population++; }
    ~Action() { population--; }
    double ta() { return (inputs == tokens) ? 1.0 : adevs_inf<double>(); }
    void delta_int() { tokens = 0; }
    void delta_ext(double, const std::list<adevs::PinValue<>>& xb) { tokens += xb.size(); }
    void delta_conf(const std::list<adevs::PinValue<>>& xb) { tokens = xb.size(); }
    void output_func(std::list<adevs::PinValue<>>& yb) {
        adevs::PinValue<> output(out,0);
        yb.push_back(output);
    }

    const adevs::pin_t in, out;
    static int population;

    private:

    unsigned inputs, tokens;
};

int Action::population = 0;

/**
 * This is an activity diagram. We create identical
 * diagrams that all terminate eventually but
 * they will be of randomly selected length. The
 * length is controlled by the depth of the diagram.
 */
class Activity: public adevs::Coupled<> {
    public:

    Activity() : adevs::Coupled<>() {
        // Create an action that an initial input
        // to the diagram
        auto initial = std::make_shared<Action>(1);
        add_atomic(initial);
        create_coupling(in,initial->in);
        create_coupling(initial->in,initial);
        // Create a terminal action that needs two input
        // and produces an output from the diagram
        auto terminal = std::make_shared<Action>(2);
        add_atomic(terminal);
        create_coupling(terminal->out,out);
        create_coupling(initial->out,terminal->in);
        create_coupling(terminal->in,terminal);
        // Create an intermediate action and hook everything up
        if (rand()%2 == 0) {
            // Deepen the diagram by adding an Activity to
            // this Activity
            auto intermediate = std::make_shared<Activity>();
            add_coupled_model(intermediate);
            create_coupling(initial->out,intermediate->in);
            create_coupling(intermediate->out,terminal->in);
        } else {
            // Otherwise create an atomic action
            auto intermediate = std::make_shared<Action>(1);
            add_atomic(intermediate);
            create_coupling(initial->out,intermediate->in);
            create_coupling(intermediate->out,terminal->in);
            create_coupling(intermediate->in,intermediate);
        }
    }
    adevs::pin_t in, out;
};

/**
 * This atomic model creates and destroys diagrams.
 */
class Manager: public adevs::Atomic<> {
    public:

    Manager(std::shared_ptr<adevs::Coupled<>>& parent) :
        adevs::Atomic<>(), max_diagrams(5), finished(0),
        time_out(5.0), num_diagrams(0), parent(parent) {
            make_new_diagram();
    }
    double ta() { return time_to_next_event; }
    void delta_int() {
        /// Just produced an output. Now wait to see if
        /// the diagram finishes
        if (time_to_next_event == 0.0) {
            time_to_next_event = time_out;
            return;
        }
        // Destroy the active model and create a new one
        num_diagrams++;
        /// Remove the diagram and our coupling to it
        /// from the parent
        parent->remove_coupled_model(active);
        /// The Actions still existing because they are
        /// not actually removed until all state transitions
        /// have been calculated.
        assert(Action::population > 0);
        /// Create a new diagram or quit
        if (num_diagrams < max_diagrams) {
            make_new_diagram();
        } else {
            active = nullptr;
            time_to_next_event = adevs_inf<double>();
        }
    }
    void delta_ext(double, const std::list<adevs::PinValue<>>&) {
        delta_int();
        finished++;
    }
    void delta_conf(const std::list<adevs::PinValue<>>&) {
        delta_int();
        finished++;
    }
    void output_func(std::list<adevs::PinValue<>>& yb) {
        adevs::PinValue<> output(start,0);
        yb.push_back(output);
    }

    adevs::pin_t start, finish;
    const int max_diagrams;
    int finished;

    private:

    void make_new_diagram() {
        /// Add the new diagram to our parent and
        /// hook it up to our start and finish pins
        active = std::make_shared<Activity>();
        parent->add_coupled_model(active);
        parent->create_coupling(start,active->in);
        parent->create_coupling(active->out,finish);
        /// Set the time advance to zero so that we
        /// can generate an output on our start pin
        time_to_next_event = 0.0;
    }

    const double time_out;
    int num_diagrams;
    double time_to_next_event;
    std::shared_ptr<adevs::Coupled<>> parent;
    std::shared_ptr<Activity> active;

};

int main() {
    int total_finished = 0, total_created = 0;
    /// Run a bunch of trials of the test
    for (int i = 0; i < 100; i++) {
        auto parent = std::make_shared<adevs::Coupled<>>();
        auto manager = std::make_shared<Manager>(parent);
        parent->add_atomic(manager);
        /// Manager is always notified when it gets input on
        /// its finish pin.
        parent->create_coupling(manager->finish,manager);
        auto sim = std::make_shared<adevs::Simulator<>>(parent);
        while (sim->nextEventTime() < adevs_inf<double>()) {
            sim->execNextEvent();
        }
        /// There should be no Action items left becaused we
        /// destroyed them when we removed the Activity to
        /// which they belonged from the parent Coupled model.
        assert(Action::population == 0);
        total_finished += manager->finished;
        total_created += manager->max_diagrams;
    }
    /// These are true with very high probability
    assert(total_finished < total_created);
    assert(total_created > 0);
    assert(total_finished > 0);
    std::cout << total_finished << " of " << total_created << " completed" << std::endl;

    return 0;
}
