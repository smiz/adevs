#include "Observer.h"
using namespace std;
using namespace adevs;

// Assign a locally unique number to the input port
int const Observer::departed = 0;

Observer::Observer(char const* output_file)
    : Atomic<EventType>(), output(output_file) {
    // Write a header describing the data fields
    output << "# Col 1: Time customer enters the line" << endl;
    output << "# Col 2: Time required for customer checkout" << endl;
    output << "# Col 3: Time customer leaves the store" << endl;
    output << "# Col 4: Time spent waiting in line" << endl;
}

double Observer::ta() {
    // The Observer has no autonomous behavior, so its next event
    // time is always infinity.
    return DBL_MAX;
}

void Observer::delta_int() {
    // The Observer has no autonomous behavior, so do nothing
}

void Observer::delta_ext(double e, list<EventType> const &xb) {
    // Record the times at which the customer left the line and the time spent in line.
    for (auto event : xb) {
        shared_ptr<Customer> customer = event.value;
        // Compute the time spent waiting in line and dump stats to the output
        double waiting_time =
            (customer->time_leave - customer->time_enter) - customer->time_wait;
        output << customer->time_enter << " " << customer->time_wait << " "
               << customer->time_leave << " " << waiting_time << endl;
    }
}

void Observer::delta_conf(list<EventType> const &xb) {
    // The Observer has no autonomous behavior, so do nothing
}

void Observer::output_func(list<EventType> &yb) {
    // The Observer produces no output, so do nothing
}

Observer::~Observer() {
    // Close the statistics file
    output.close();
}
