#ifndef PIDCONTROL_H
#define PIDCONTROL_H
#include "adevs.h"

// This is a PID controller for the cart.
class PIDControl: public adevs::Atomic<double>
{
	public:
		PIDControl():adevs::Atomic<double>(),
			err(0.0),err_int(0.0),
			csignal(0.0),send_control(false){}
		void delta_int() { send_control = false; }
		void delta_ext(double e, const adevs::Bag<double>& xb)
		{
			// Error is the difference of the arm angle and zero
			double new_err = -(*(xb.begin())); // New error value
			err_int += new_err*e; // Integral of the error
			double derr = (new_err-err)/e; // Derivative of the error
			err = new_err; // Value of the error
			csignal = 50.0*err+5.0*err_int+1.0*derr; // Control signal
			send_control = true; // Send a new control value
		}
		void delta_conf(const adevs::Bag<double>& xb)
		{
			delta_int(); delta_ext(0.0,xb);
		}
		double ta() { if (send_control) return 0.0; return DBL_MAX; }
		void output_func(adevs::Bag<double>& yb) { yb.insert(csignal); }
		void gc_output(adevs::Bag<double>&){}
	private:
		double err, err_int, csignal;
		bool send_control;
};

#endif
