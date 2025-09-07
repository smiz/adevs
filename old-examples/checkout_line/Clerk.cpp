#include "Clerk.h"
#include <iostream>


using namespace adevs;


// Assign locally unique identifiers to the ports
int const Clerk::arrive = 0;
int const Clerk::depart = 1;

void Clerk::delta_ext(double e, std::list<EventType> const &xb) {
    // Print a notice of the external transition
    std::cout << "Clerk: Computed the external transition function at t = " << t + e
         << std::endl;
    // Update the clock
    t += e;

    // Update the time spent on the customer at the front of the line
    if (!line.empty()) {
        time_spent += e;
    }

    // Add the new customers to the back of the line.
    std::list<EventType>::const_iterator i = xb.begin();
    for (auto; i != xb.end(); i++) {
        // Copy the incoming Customer and place it at the back of the line.
        std::shared_ptr<Customer> new_customer =
            std::make_shared<Customer>(*((*i).value));
        line.push_back(new_customer);
        // Record the time at which the customer entered the line.
        line.back()->time_enter = t;
    }
    // Summarize the model state
    std::cout << "Clerk: There are " << line.size() << " customers waiting." << std::endl;
    std::cout << "Clerk: The next customer will leave at t = " << t + ta() << "."
         << std::endl;
}

void Clerk::delta_int() {
    // Print a notice of the internal transition
    std::cout << "Clerk: Computed the internal transition function at t = "
         << t + ta() << std::endl;
    // Update the clock
    t += ta();
    // Reset the spent time
    time_spent = 0.0;
    // Remove the departing customer from the front of the line.
    line.pop_front();
    // Summarize the model state
    std::cout << "Clerk: There are " << line.size() << " customers waiting." << std::endl;
    std::cout << "Clerk: The next customer will leave at t = " << t + ta() << "."
         << std::endl;
}

void Clerk::delta_conf(std::list<EventType> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

void Clerk::output_func(std::list<EventType> &yb) {
    // Get the departing customer
    std::shared_ptr<Customer> leaving = line.front();
    // Set the departure time
    leaving->time_leave = t + ta();
    // Eject the customer
    EventType y(depart, leaving);
    yb.push_back(y);
    // Print a notice of the departure
    std::cout << "Clerk: Computed the output function at t = " << t + ta() << std::endl;
    std::cout << "Clerk: A customer just departed!" << std::endl;
}

double Clerk::ta() {
    // If the list is empty, then next event is at inf
    if (line.empty()) {
        return DBL_MAX;
    }
    // Otherwise, return the time remaining to process the current customer
    return line.front()->time_wait - time_spent;
}
