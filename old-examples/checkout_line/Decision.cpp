#include "Decision.h"
#include <iostream>


// using namespace adevs;


// Assign identifiers to ports.  Assumes NUM_LINES = 2.
// The numbers are selected to allow indexing into the
// line length and port number arrays.
int const Decision::departures[NUM_LINES] = {0, 1};
int const Decision::arrive[NUM_LINES] = {0, 1};
// Inport port for arriving customer that need to make a decision
int const Decision::decide = NUM_LINES;

Decision::Decision() : Atomic<EventType>() {
    // Set the initial line lengths to zero
    for (int ii = 0; ii < NUM_LINES; ii++) {
        line_length[ii] = 0;
    }
}

void Decision::delta_int() {
    // Move out all of the deciders
    deciding.clear();
}

void Decision::delta_ext(double e, std::list<EventType> const &x) {
    // Assign new arrivals to a line and update the line length
    std::list<EventType>::const_iterator iter = x.begin();
    for (; iter != x.end(); iter++) {
        if ((*iter).port == decide) {
            int line_choice = find_shortest_line();
            std::shared_ptr<Customer> customer = std::make_shared<Customer>(*((*iter).value));
            pair<int, std::shared_ptr<Customer>> p(line_choice, customer);
            deciding.push_back(p);
            line_length[p.first]++;
        }
    }
    // Decrement the length of lines that had customers leave
    for (int ii = 0; ii < NUM_LINES; ii++) {
        iter = x.begin();
        for (; iter != x.end(); iter++) {
            if ((*iter).port < NUM_LINES) {
                line_length[(*iter).port]--;
            }
        }
    }
}

void Decision::delta_conf(std::list<EventType> const &x) {
    delta_int();
    delta_ext(0.0, x);
}

double Decision::ta() {
    // If there are customers getting into line, then produce output
    // immediately.
    if (!deciding.empty()) {
        return 0.0;
    }
    // Otherwise, wait for another customer
    else {
        return DBL_MAX;
    }
}

void Decision::output_func(std::list<EventType> &y) {
    // Send all customers to their lines
    std::list<pair<int, std::shared_ptr<Customer>>>::iterator ii = deciding.begin();
    for (; ii != deciding.end(); ii++) {
        EventType event((*ii).first, (*ii).second);
        y.push_back(event);
    }
}

int Decision::find_shortest_line() {
    int shortest = 0;
    for (int ii = 0; ii < NUM_LINES; ii++) {
        if (line_length[shortest] > line_length[ii]) {
            shortest = ii;
        }
    }
    return shortest;
}
