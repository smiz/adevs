#include "adevs.h"
#include "adevs_rk4.h"
#include <iostream>
using namespace std;
using namespace adevs;

/**
This example simulates a ball bouncing on a flat surface.
The motion of the ball is governed by the second order
differential equation

d^2 h /dt^2 = -9.8

which can be written as two first order equations

dh/dt = v
dv/dt = -9.8

The velocity of the ball changes instantaneously when the
ball strikes the floor. The ball hits the floor when

h = 0

When this event occurs, the velocity of the ball changes
instantaneously from v to -v.

This particular model produces output whenever the ball changes
its height by 0.01, and, of course, when h = 0.
*/
class BouncingBall: public rk4<double>
{
	public:
		BouncingBall(double h0):
		// Base this on the rk4 solver using two state variables and
		// an integration time step of 0.01
		rk4<double>(3,0.01)
		{
			init(0,h0); // Initialize the height of the ball
			init(1,0.0); // Initialize the velocity of the ball
			init(2,0.0); // Time
			hl = h0; // value of h at the last state event
		}
		// Compute the derivative dq using the state q
		void der_func(const double* q, double *dq)
		{
			dq[0] = q[1]; // dh/dt = v
			dq[1] = -9.8; // dv/dt = -9.8
			dq[2] = 1.0; // dt/dt = 1.0
		}
		// A state event occurs at the time the state_event_func changes
		// sign. In this example, state events occur whenever 
		// |h-hl|-0.01 = 0
		void state_event_func(const double *q, double* z)
		{
			z[0] = fabs(q[0]-hl)-0.01; 
		}
		// This model does not have any time events
		double time_event_func(const double* q)
		{
			return DBL_MAX;
		}
		// The discrete action updates the last output value
		// and, if h = 0, change the velocity
		void discrete_action(double* q, const Bag<double>& xb)
		{
			hl = q[0];
			if (q[0] <= 0.0) q[1] = -q[1];
		}
		// Output the height of the ball
		void discrete_output(const double* q, Bag<double>& yb)
		{
			yb.insert(q[0]);
			cout << q[2] << " " << q[0] << endl;
		}
		void gc_output(Bag<double>& g){}

	private:
		double hl;
};

int main()
{
	// Create a ball with a height of 1
	BouncingBall* model = new BouncingBall(1.0);
	Simulator<double>* sim = new Simulator<double>(model);
	while (sim->nextEventTime() < 10.0)
	{
		sim->execNextEvent();
	}
	delete sim;
	delete model;
	return 0;
}
