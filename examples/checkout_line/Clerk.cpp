#include "Clerk.h"
#include <iostream>
using namespace std;
using namespace adevs;

// Assign locally unique identifiers to the ports
int const Clerk::arrive = 0;
int const Clerk::depart = 1;

Clerk::Clerk()
    : Atomic<IO_Type>(),  // Initialize the parent Atomic model
      t(0.0),             // Set the clock to zero
      t_spent(0.0)        // No time spent on a customer so far
{}

void Clerk::delta_ext(double e, Bag<IO_Type> const &xb) {
    // Print a notice of the external transition
    cout << "Clerk: Computed the external transition function at t = " << t + e
         << endl;
    // Update the clock
    t += e;
    // Update the time spent on the customer at the front of the line
    if (!line.empty()) {
        t_spent += e;
    }
    // Add the new customers to the back of the line.
    Bag<IO_Type>::const_iterator i = xb.begin();
    for (; i != xb.end(); i++) {
        // Copy the incoming Customer and place it at the back of the line.
        line.push_back(new Customer(*((*i).value)));
        // Record the time at which the customer entered the line.
        line.back()->tenter = t;
    }
    // Summarize the model state
    cout << "Clerk: There are " << line.size() << " customers waiting." << endl;
    cout << "Clerk: The next customer will leave at t = " << t + ta() << "."
         << endl;
}

void Clerk::delta_int() {
    // Print a notice of the internal transition
    cout << "Clerk: Computed the internal transition function at t = "
         << t + ta() << endl;
    // Update the clock
    t += ta();
    // Reset the spent time
    t_spent = 0.0;
    // Remove the departing customer from the front of the line.
    line.pop_front();
    // Summarize the model state
    cout << "Clerk: There are " << line.size() << " customers waiting." << endl;
    cout << "Clerk: The next customer will leave at t = " << t + ta() << "."
         << endl;
}

void Clerk::delta_conf(Bag<IO_Type> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

void Clerk::output_func(Bag<IO_Type> &yb) {
    // Get the departing customer
    Customer* leaving = line.front();
    // Set the departure time
    leaving->tleave = t + ta();
    // Eject the customer
    IO_Type y(depart, leaving);
    yb.insert(y);
    // Print a notice of the departure
    cout << "Clerk: Computed the output function at t = " << t + ta() << endl;
    cout << "Clerk: A customer just departed!" << endl;
}

double Clerk::ta() {
    // If the list is empty, then next event is at inf
    if (line.empty()) {
        return DBL_MAX;
    }
    // Otherwise, return the time remaining to process the current customer
    return line.front()->twait - t_spent;
}

void Clerk::gc_output(Bag<IO_Type> &g) {
    // Delete the outgoing customer objects
    Bag<IO_Type>::iterator i;
    for (i = g.begin(); i != g.end(); i++) {
        delete (*i).value;
    }
}

Clerk::~Clerk() {
    // Delete anything remaining in the customer queue
    list<Customer*>::iterator i;
    for (i = line.begin(); i != line.end(); i++) {
        delete *i;
    }
}
