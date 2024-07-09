#include "Decision.h"
#include <iostream>
using namespace std;
using namespace adevs;

// Assign identifiers to ports.  Assumes NUM_LINES = 2.
// The numbers are selected to allow indexing into the
// line length and port number arrays.
int const Decision::departures[NUM_LINES] = {0, 1};
int const Decision::arrive[NUM_LINES] = {0, 1};
// Inport port for arriving customer that need to make a decision
int const Decision::decide = NUM_LINES;

Decision::Decision() : Atomic<IO_Type>() {
    // Set the initial line lengths to zero
    for (int i = 0; i < NUM_LINES; i++) {
        line_length[i] = 0;
    }
}

void Decision::delta_int() {
    // Move out all of the deciders
    deciding.clear();
}

void Decision::delta_ext(double e, Bag<IO_Type> const &x) {
    // Assign new arrivals to a line and update the line length
    Bag<IO_Type>::const_iterator iter = x.begin();
    for (; iter != x.end(); iter++) {
        if ((*iter).port == decide) {
            int line_choice = find_shortest_line();
            Customer* customer = new Customer(*((*iter).value));
            pair<int, Customer*> p(line_choice, customer);
            deciding.push_back(p);
            line_length[p.first]++;
        }
    }
    // Decrement the length of lines that had customers leave
    for (int i = 0; i < NUM_LINES; i++) {
        iter = x.begin();
        for (; iter != x.end(); iter++) {
            if ((*iter).port < NUM_LINES) {
                line_length[(*iter).port]--;
            }
        }
    }
}

void Decision::delta_conf(Bag<IO_Type> const &x) {
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

void Decision::output_func(Bag<IO_Type> &y) {
    // Send all customers to their lines
    list<pair<int, Customer*>>::iterator i = deciding.begin();
    for (; i != deciding.end(); i++) {
        IO_Type event((*i).first, (*i).second);
        y.insert(event);
    }
}

void Decision::gc_output(Bag<IO_Type> &g) {
    Bag<IO_Type>::iterator iter = g.begin();
    for (; iter != g.end(); iter++) {
        delete (*iter).value;
    }
}

Decision::~Decision() {
    clear_deciders();
}

void Decision::clear_deciders() {
    list<pair<int, Customer*>>::iterator i = deciding.begin();
    for (; i != deciding.end(); i++) {
        delete (*i).second;
    }
    deciding.clear();
}

int Decision::find_shortest_line() {
    int shortest = 0;
    for (int i = 0; i < NUM_LINES; i++) {
        if (line_length[shortest] > line_length[i]) {
            shortest = i;
        }
    }
    return shortest;
}
