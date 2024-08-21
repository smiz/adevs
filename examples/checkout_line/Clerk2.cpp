#include "Clerk2.h"
#include <iostream>
using namespace std;
using namespace adevs;

// Assign locally unique port numbers
int const Clerk2::arrive = 0;
int const Clerk2::depart = 1;
// Define the size of a 'small' order
double const Clerk2::SMALL_ORDER = 1.0;
// Time that must separate a small order pre-emptions
double const Clerk2::PREEMPT_TIME = 5.0;

Clerk2::Clerk2() : Atomic<IO_Type>(), preempt(0.0), t(0.0) {}

void Clerk2::delta_ext(double e, Bag<IO_Type> const &xb) {
    /// Update the clock
    t += e;
    /// Update the time spent working on the current order
    if (!line.empty()) {
        line.front().t_left -= e;
    }
    /// Reduce the preempt time
    preempt -= e;
    /// Place new customers into the line
    Bag<IO_Type>::const_iterator iter = xb.begin();
    for (; iter != xb.end(); iter++) {
        cout << "Clerk: A new customer arrived at t = " << t << endl;
        /// Create a copy of the incoming customer and set the entry time
        customer_info_t c;
        c.customer = new Customer(*((*iter).value));
        c.t_left = c.customer->twait;
        /// Record the time at which the customer enters the line
        c.customer->tenter = t;
        /// If the customer has a small order
        if (preempt <= 0.0 && c.t_left <= SMALL_ORDER) {
            cout << "Clerk: The new customer has preempted the current one!"
                 << endl;
            /// We won't preempt another customer for at least this long
            preempt = PREEMPT_TIME;
            /// Put the new customer at the front of the line
            line.push_front(c);
        }
        /// otherwise just put the customer at the end of the line
        else {
            cout << "Clerk: The new customer is at the back of the line"
                 << endl;
            line.push_back(c);
        }
    }
}

void Clerk2::delta_int() {
    // Update the clerk's clock
    t += ta();
    // Update the preemption timer
    preempt -= ta();
    // Remove the departing customer from the front of the line.
    line.pop_front();
    // Check to see if any customers are waiting.
    if (line.empty()) {
        cout << "Clerk: The line is empty at t = " << t << endl;
        return;
    }
    // If the preemption time has passed, then look for a small
    // order that can be promoted to the front of the line.
    list<customer_info_t>::iterator i;
    for (i = line.begin(); i != line.end() && preempt <= 0.0; i++) {
        if ((*i).t_left <= SMALL_ORDER) {
            cout << "Clerk: A queued customer has a small order at time " << t
                 << endl;
            customer_info_t small_order = *i;
            line.erase(i);
            line.push_front(small_order);
            preempt = PREEMPT_TIME;
            break;
        }
    }
}

void Clerk2::delta_conf(Bag<IO_Type> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

double Clerk2::ta() {
    // If the line is empty, then there is nothing to do
    if (line.empty()) {
        return DBL_MAX;
    }
    // Otherwise, wait until the first customer will leave
    else {
        return line.front().t_left;
    }
}

void Clerk2::output_func(Bag<IO_Type> &yb) {
    /// Set the exit time of the departing customer
    line.front().customer->tleave = t + ta();
    /// Place the customer at the front of the line onto the depart port.
    IO_Type y(depart, line.front().customer);
    yb.push_back(y);
    // Report the departure
    cout << "Clerk: A customer departed at t = " << t + ta() << endl;
}
