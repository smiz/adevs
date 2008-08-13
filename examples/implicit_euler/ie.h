#ifndef _ie_h_
#define _ie_h_
#include "adevs_dess.h"
#include <cmath>

template <class X> class ie: public adevs::DESS<X> {
	public:
		/**
		 * The constructor requires an initial value q0 for the continuous
		 * state variable and a maximum step size h_max for the implicit Euler
		 * integration scheme.
		 */
		ie(double q0, double h_max):adevs::DESS<X>(),h_max(h_max),q(q0){}
		// Get the current value of the continuous state variable.
		double getStateVars() const { return q; }
		// Compute the derivative function using the supplied state variable value.
		virtual double der_func(double q) = 0;
		// Compute the value the zero crossing function.
		virtual double state_event_func(double q) = 0;
		// The discrete action function can set the value of q by writing to its reference.
		virtual void discrete_action(double& q, const adevs::Bag<X>& xb) = 0;
		// The discrete output function should place output values in yb.
		virtual void discrete_output(double q, adevs::Bag<X>& yb) = 0;
		virtual void state_changed(double q){};
		// Implementation of the DESS evolve_func method
		void evolve_func(double h);
		// Implementation of the DESS next_event_func method
		double next_event_func(bool& is_event);
		// Implementation of the DESS discrete_action_func method
		void discrete_action_func(const adevs::Bag<X>& xb);
		// Implementation of the DESS dscrete_output_func method
		void discrete_output_func(adevs::Bag<X>& yb);
		// Implementation of the DESS state_changed method
		void state_changed();
		/// Destructor
		~ie(){}
	private:
		const double h_max;
		double q;
		double integ(double qq, double h);
		// Return the sign of x
		static int sgn(double x) {
			if (x < 0.0) return -1;
			else if (x > 0.0) return 1;
			else return 0;
		}
};

template <class X>
double ie<X>::integ(double qq, double h) {
	double q1 = qq;
	double q2 = qq + h*der_func(q1);
	while (fabs(q1-q2) > 1E-12) {
		q1 = q2;
		q2 = qq + h*der_func(q1);
	}
	return q2;
}

template <class X>
void ie<X>::evolve_func(double h) {
	q = integ(q,h);
}

template <class X>
double ie<X>::next_event_func(bool& is_event) {
	double h = h_max;
	double z1 = state_event_func(q);
	double z2 = state_event_func(integ(q,h));
	while (true) {
		if (sgn(z1) == sgn(z2)) {
			is_event = false;
			break;
		}
		else if (sgn(z1) != sgn(z2) && fabs(z2) < 1E-12) {
			is_event = true;
			break;
		}
		h = (h*z1)/(z1-z2);
		z2 = state_event_func(integ(q,h));
	}
	return h;
}

template <class X>	
void ie<X>::discrete_action_func(const adevs::Bag<X>& xb) {
	discrete_action(q,xb);
}

template <class X>	
void ie<X>::discrete_output_func(adevs::Bag<X>& yb) {
	discrete_output(q,yb);
}

template <class X>	
void ie<X>::state_changed() {
	state_changed(q);
}

#endif
