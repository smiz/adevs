#ifndef CART_MODEL_H
#define CART_MODEL_H
#include "adevs.h"

#define PI 3.1415926535897931
#define RAD_TO_DEG (180.0/PI)
// This is the dynamic model of the pendulum. The only input
// is the force applied by the cart's motor and the only output
// is the arm angle measurement.
class CartModel: public adevs::ode_system<double>
{
	public:
		CartModel();
		// Arm angle in degrees; zero degrees is straight down
		// and rotation is clockwise.
		double angle(const double* q) const { return RAD_TO_DEG*q[theta]; }
		void init(double* q);
		void der_func(const double *q, double *dq);
		void state_event_func(const double *q, double *z);
		void internal_event(double* q, const bool* state_event);
		void external_event(double* q, double e,
				const adevs::Bag<double>& xb);
		void confluent_event(double *q, const bool* state_event,
				const adevs::Bag<double>& xb);
		void output_func(const double *q, const bool* state_event,
				adevs::Bag<double>& yb);
		double time_event_func(const double*){ return DBL_MAX; }
		void gc_output(adevs::Bag<double>&){}
	private:
		// State variable indices
		const int x, theta, dx, dtheta, t;
		// Model parameters
		const double armMass, armFric, cartMass, cartFric,
			  armLen, g, mAngle;
		int k; // Last output level for the quantized sensor
		// Constraint matrix and vector for A [ddx ddtheta]^T = B
		double A[4][4], B[2];
		double F; // Motor force
};

#endif
