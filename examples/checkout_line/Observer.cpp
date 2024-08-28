#include "Observer.h"
using namespace std;
using namespace adevs;

// Assign a locally unique number to the input port
int const Observer::departed = 0;

Observer::Observer(char const* output_file)
    : Atomic<IO_Type>(), output_strm(output_file) {
    // Write a header describing the data fields
    output_strm << "# Col 1: Time customer enters the line" << endl;
    output_strm << "# Col 2: Time required for customer checkout" << endl;
    output_strm << "# Col 3: Time customer leaves the store" << endl;
    output_strm << "# Col 4: Time spent waiting in line" << endl;
}

double Observer::ta() {
    // The Observer has no autonomous behavior, so its next event
    // time is always infinity.
    return DBL_MAX;
}

void Observer::delta_int() {
    // The Observer has no autonomous behavior, so do nothing
}

void Observer::delta_ext(double e, list<IO_Type> const &xb) {
    // Record the times at which the customer left the line and the
    // time spent in it.
    list<IO_Type>::const_iterator i;
    for (i = xb.begin(); i != xb.end(); i++) {
        Customer const* c = (*i).value;
        // Compute the time spent waiting in line
        double waiting_time = (c->tleave - c->tenter) - c->twait;
        // Dump stats to a file
        output_strm << c->tenter << " " << c->twait << " " << c->tleave << " "
                    << waiting_time << endl;
    }
}

void Observer::delta_conf(list<IO_Type> const &xb) {
    // The Observer has no autonomous behavior, so do nothing
}

void Observer::output_func(list<IO_Type> &yb) {
    // The Observer produces no output, so do nothing
}

Observer::~Observer() {
    // Close the statistics file
    output_strm.close();
}
