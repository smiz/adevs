#include <iostream>
#include "AssemblyLine_bottom_up.h"
using namespace std;
using namespace adevs;

// Listener for recording the state and output of the assembly line.
class AssemblyLineListener : public EventListener<int> {
  public:
    AssemblyLineListener(AssemblyLine* assembly_line)
        : assembly_line(assembly_line) {}
    void outputEvent(Event<int> y, double t) {
        // Output from the AssembyLine
        if (y.model == assembly_line) {
            cout << "Output, t = " << t << ", y = " << y.value << endl;
        }
    }
    void stateChange(Atomic<int>* model, double t) {
        // Get the model of the machine
        Machine* m = dynamic_cast<Machine*>(model);
        // Print the state of the machine
        cout << "State, t = " << t;
        if (model == assembly_line->getPress()) {
            cout << ", press = (";
        } else {
            cout << ", drill = (";
        }
        cout << m->getParts() << "," << m->getSigma() << ")" << endl;
    }

  private:
    AssemblyLine* assembly_line;
};

int main() {
    // Create the model, event listener, and simulator
    AssemblyLine* assembly_line = new AssemblyLine();
    AssemblyLineListener* l = new AssemblyLineListener(assembly_line);
    Simulator<int>* sim = new Simulator<int>(assembly_line);
    sim->addEventListener(l);
    // Print the initial state of the model
    l->stateChange(assembly_line->getDrill(), 0.0);
    l->stateChange(assembly_line->getPress(), 0.0);
    // Run the simulation
    while (true) {

        list<Event<int>> input;
        // The value to inject
        int blanks;
        // Time to inject the input
        double t;
        int c;
        // Read the time and input values
        cin >> t >> c >> blanks;
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
        Event<int> input_event(assembly_line, blanks);
        input.push_back(input_event);
        cout << endl;
        sim->computeNextState(input, t);
    }
    // Run until the simulation completes
    while (sim->nextEventTime() < DBL_MAX) {
        cout << endl;
        sim->execNextEvent();
    }
    // Clean up
    delete sim;
    delete l;
    delete assembly_line;
    return 0;
}
