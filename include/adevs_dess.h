/***************
Copyright (C) 2000-2006 by James Nutaro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Bugs, comments, and questions can be sent to nutaro@gmail.com
***************/
#ifndef _adevs_dess_h_
#define _adevs_dess_h_
#include "adevs_models.h"

namespace adevs
{

/**
This is the base class for building ODE and DAE solvers that can
be used inside of a discrete event simulation. A description of the
integration technique can be found in ????. The template argument
specifies the model input and output type. The DEVS state transition functions
and output function should not be overridden by derived classes.
The DEVS dynamic structure and output garbage collection functions can
be used by derived classes.
*/
template <class X> class DESS: public Atomic<X>
{
	public:
		/// Constructor. Time is assumed to start at 0.0.
		DESS();
		/**
		This virtual function is the continuous evolution function.
		Its purpose is to advance the continuous state variables for h units of time.
		A derived class should implement its integration scheme in this function.
		*/
		virtual void evolve_func(double h) = 0;
		/**
		This function should return the time until the next state or time
		event in the continuous system. This method can also return the time until the next
		integration step, in which case the variable is_event should
		be set to false. If the is_event flag is true, then next state change
		will be treated as an internal event of the DEVS model.
		*/
		virtual double next_event_func(bool& is_event) = 0;
		/**
		This is the discrete action function. The input bag contains the
		discrete events that are available at this time (if any).
		*/
		virtual void discrete_action_func(const Bag<X>& xb) = 0;
		/**
		This is the output function. The output function is evaluated
		when the next_event_func() elapses. The bag should be filled
		with output events.
		*/
		virtual void discrete_output_func(Bag<X>& yb) = 0;
		/**
		This method is executed initially (i.e., at simulation start or 
	    just following creation if added dynamically), after every
	    evaluation of the evolution function, and after every evaluation
	    of the discrete_action_func method. This method is intended to
	    facilitate writing the continuous system trajectory to an output
	    file. The overridden method should call some integrator specific
	    method that the user can implement for that purpose.
	    */
		virtual void state_changed() = 0;	
		/// The internal transition function should not be overridden
		void delta_int();
		/// The external transition function should not be overridden
		void delta_ext(double e, const Bag<X>& xb);
		/// The confluent transition function should not be overridden
		void delta_conf(const Bag<X>& xb);
		/// The output function should not be overridden
		void output_func(Bag<X>& yb);
		/// The time advance function should not be overridden
		double ta();
		/// Destructor
		virtual ~DESS(){}

	private:
		// Time until the next event
		double sigma;
		// The phase can be step, event, or output
		enum { DESS_STEP, DESS_EVENT, DESS_OUTPUT } phase;
		// Empty bag for internal events
		Bag<X> empty_bag;
		// Temporary bag for discrete output values
		Bag<X> ytmp;
};

template <class X>
DESS<X>::DESS()
{
	sigma = 0.0;
	phase = DESS_STEP;
}

template <class X>
void DESS<X>::delta_int()
{
	if (phase == DESS_OUTPUT)
	{
		ytmp.clear();
		bool event = false;
		sigma = next_event_func(event);
		if (event) phase = DESS_EVENT;
		else phase = DESS_STEP;
	}
	else
	{
		// Evolve the continuous variables
		evolve_func(sigma);
		// Notify derived class of state change
		state_changed();
		// If this is an internal discrete event
		if (phase == DESS_EVENT)
		{
			// Get the output
			discrete_output_func(ytmp);
			// Change the state 
			discrete_action_func(empty_bag);
			// Notify derived class of the event
			state_changed();
			phase = DESS_OUTPUT;
			sigma = 0.0;
		}
		// Otherwise, just schedule the next event or 
		// integration step
		else
		{
			bool event = false;
			sigma = next_event_func(event);
			if (event) phase = DESS_EVENT;
			else phase = DESS_STEP;
		}
	}
}

template <class X>
void DESS<X>::delta_ext(double e, const Bag<X>& xb)
{
	// Update the continuous variables
	evolve_func(e);
	// Notify the derived class
	state_changed();
	// Apply the discrete state change function
	discrete_action_func(xb);
	// Notify the derived class again
	state_changed();
	// Schedule the next step or event
	bool event = false;
	sigma = next_event_func(event);
	if (event) phase = DESS_EVENT;
	else phase = DESS_STEP;
}

template <class X>
void DESS<X>::delta_conf(const Bag<X>& xb)
{
	// If it is not a state event, then this is just an input coinciding with
	// some book keeping operations
	if (phase == DESS_OUTPUT || phase == DESS_STEP)
	{
		ytmp.clear();
		delta_ext(sigma,xb);
	}
	// Confluence of a state/time event an external input requires special
	// handling
	else if (phase == DESS_EVENT)
	{
		// Update the continuous variables
		evolve_func(sigma);
		// Notify the derived class
		state_changed();
		// Compute the output
		discrete_output_func(ytmp);
		// Compute the discrete state change
		discrete_action_func(xb);
		// Notify the derived class
		state_changed();
		// Send the output
		phase = DESS_OUTPUT;
		sigma = 0.0;
	}
}

template <class X>
void DESS<X>::output_func(Bag<X>& yb)
{
	typename Bag<X>::iterator iter = ytmp.begin();
	for (; iter != ytmp.end(); iter++)
	{
		yb.insert(*iter);
	}
}

template <class X>
double DESS<X>::ta()
{
	return sigma;
}

} // end of namespace

#endif
