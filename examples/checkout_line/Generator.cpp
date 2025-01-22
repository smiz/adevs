#include "Generator.h"
#include <fstream>
using namespace std;
using namespace adevs;

// Assign a locally unique number to the arrival port
int const Generator::arrive = 0;

Generator::Generator(char const* sched_file) : Atomic<IO_Type>() {
    // Open the file containing the schedule
    fstream input_strm(sched_file);
    // Store the arrivals in a list
    double next_arrival_time = 0.0;
    double last_arrival_time = 0.0;
    while (true) {
        Customer* customer = new Customer;
        input_strm >> next_arrival_time >> customer->twait;
        // Check for end of file
        if (input_strm.eof()) {
            delete customer;
            break;
        }
        // The entry time holds the inter arrival times, not the
        // absolute entry time.
        customer->tenter = next_arrival_time - last_arrival_time;
        // Put the customer at the back of the line
        arrivals.push_back(customer);
        last_arrival_time = next_arrival_time;
    }
}

double Generator::ta() {
    // If there are not more customers, next event time is infinity
    if (arrivals.empty()) {
        return DBL_MAX;
    }
    // Otherwise, wait until the next arrival
    return arrivals.front()->tenter;
}

void Generator::delta_int() {
    // Remove the first customer.
    arrivals.pop_front();
}

void Generator::delta_ext(double e, list<IO_Type> const &xb) {
    /// The generator is input free, and so it ignores external events.
}

void Generator::delta_conf(list<IO_Type> const &xb) {
    /// The generator is input free, and so it ignores input.
    delta_int();
}

void Generator::output_func(list<IO_Type> &yb) {
    // First customer in the list is produced as output
    IO_Type output(arrive, arrivals.front());
    yb.push_back(output);
}
