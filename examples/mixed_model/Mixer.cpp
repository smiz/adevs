#include "Mixer.h"
#include <iostream>
using namespace std;
using namespace adevs;

// Assign identifiers to ports
const int Mixer::dough_ball = 0;
const int Mixer::knob = 1;
const int Mixer::dough = 2;

Mixer::Mixer():
rk4<PortValue<double> >(2,0.001), // One cont. var and step size = 0.01
Q(0.13) // Set the doughball size
{
	sample = 0.0;
	immediate = false;
	init(0,0.0); // Initialize the cookie dough produced
	init(1,0.0); // Initialize time
	ql = 0.0; // Last discrete state value
	active = true; // Machine begins producing dough
}
	
void Mixer::der_func(const double* q, double *dq)
{
	dq[1] = 1.0; // Time
	// If the machine is idle, then make no dough
	if (active == false)
	{
		dq[0] = 0.0;
	}
	// Production starts slow and approaches a steady rate of 2
	else
	{
		dq[0] = 2.0-exp(-0.25*q[0]);
	}
}

void Mixer::state_event_func(const double *q, double* z)
{
	// Produce a new dough ball whenver Q more units of dough are made
	z[0] = q[0]-ql-Q; // This changes sign when q[0]-ql = Q
}

double Mixer::time_event_func(const double* q)
{ 
	if (immediate) return 0.0;
	else return max(sample-q[1],0.0);
}

void Mixer::discrete_action(double* q, const Bag<PortValue<double> >& xb)
{
	if (sample-q[1] <= 0.0) sample = q[1] + 0.01;
	// If a dough ball is produced
	if (q[0]-ql >= Q) ql = q[0];
	// Look for changes in the machine switch
	Bag<PortValue<double> >::const_iterator iter;
	for (iter = xb.begin(); iter != xb.end(); iter++)
	{
		// Turn the machine on
		if ((*iter).port == knob && ((*iter).value == 1.0))
		{
			active = true;
		}
		// Turn the machine off
		else if ((*iter).port == knob && ((*iter).value == 0.0))
		{
			// Lose any excess dough
			cerr << "Lost " << q[0]-ql << endl;
			q[0] = ql; 
			active = false;
		}
	}
	if (immediate == false) immediate = true;
	else immediate = false;
}

void Mixer::discrete_output(const double* q, Bag<PortValue<double> >& yb)
{
	// Spit out a ball of dough
	if (q[0]-ql >= Q)
	{
		PortValue<double> dough_event(dough_ball,Q);
		yb.insert(dough_event);
	}
	// Output the quantity of dough produced
	PortValue<double> sample_event(dough,q[0]);
	yb.insert(sample_event);
}
