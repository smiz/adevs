#ifndef _TankPhysicsEqns_h_
#define _TankPhysicsEqns_h_
#include "adevs.h"
#include "adevs_hybrid.h"
#include "SimEvents.h"
#include <fstream>

class TankPhysicsEqns: public adevs::ode_system<SimEvent>
{
	public:
		// Create a tank at an intial position
		TankPhysicsEqns(double x0, double y0, double theta0,
				double cint, bool simple = false);
		// Initialize the continuous state variables
		void init(double* q);
		// Differential functions
		void der_func (const double *q, double *dq);
		// State event function that detects turning movements
		void state_event_func (const double *q, double *z);
		// Sample x, y, and theta periodically
		double time_event_func (const double *q);
		// Expiration of the timer and collision events are internal
		void internal_event(double* q, const bool* events);
		// Change in the motor voltage is an external event
		void external_event(double *q, double e,
				const adevs::Bag<SimEvent> &xb);
		// Confluent events
		void confluent_event(double* q, const bool* events,
				const adevs::Bag<SimEvent> &xb);
		// Output position events for the display when the timer expires
		void output_func(const double *q, const bool* events,
				adevs::Bag<SimEvent> &yb);
		// Garbage collection
		void gc_output(adevs::Bag<SimEvent>&){}
		// Get the resistance of the motor (left and right are the same)
		double getMotorOhms() const { return Rm; }
		// Is the tank turning?
		bool isTurning() const { return turning; }
		// Get the current in the motor
		double getLeftCurrent(const double* q) const;
		double getRightCurrent(const double* q) const;
		// Index of the turning state events and timer time event
		static const int TURNL, TURNR, TIMER_EXPIRE;
		// Indices of the state variables
		static const int WL, WR, FL, FR, IL, IR, X, Y, THETA, W, V, TIMER;
		// Model parameters
		const double mt, Jt, B, Br, Bs, Bl, Sl, Lm,
			  Rm, Jg, Bg, g, alpha, r, Kt;
	private:
		// Initial conditions
		const double x0, y0, theta0;
		// Hysteresis value for stopping turns
		const double Hs;
		// Communication interval for the numerical integration algorithm
		const double cint;
		// Motor voltages
		double el, er;
		// Is the tank turning?
		bool turning; 
		// Use simplified dynamics?
		const bool simple;
};

#endif
