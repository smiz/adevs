#include "Generator.h"
#include <fstream>


// using namespace adevs;


// Assign a locally unique number to the arrival port
int const Generator::arrive = 0;

Generator::Generator(std::string sched_file) : Atomic<EventType>() {
    // Open the file containing the schedule
    fstream input_strm(sched_file);
    // Store the arrivals in a list
    double next_arrival_time = 0.0;
    double last_arrival_time = 0.0;
    while (true) {
        std::shared_ptr<Customer> customer = std::make_shared<Customer>();
        input_strm >> next_arrival_time >> customer->time_wait;
        // Check for end of file
        if (input_strm.eof()) {
            break;
        }
        // The entry time holds the inter arrival times, not the
        // absolute entry time.
        customer->time_enter = next_arrival_time - last_arrival_time;
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
    return arrivals.front()->time_enter;
}

void Generator::delta_int() {
    // Remove the first customer.
    arrivals.pop_front();
}

void Generator::delta_ext(double e, std::list<EventType> const &xb) {
    // The generator is input free, so it ignores external events.
}

void Generator::delta_conf(std::list<EventType> const &xb) {
    delta_int();
}

void Generator::output_func(std::list<EventType> &yb) {
    // First customer in the list is produced as output
    EventType output(arrive, arrivals.front());
    yb.push_back(output);
}
