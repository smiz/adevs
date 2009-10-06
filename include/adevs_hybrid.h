#ifndef _adevs_hybrid_h_
#define _adevs_hybrid_h_
#include <algorithm>
#include "adevs_models.h"

namespace adevs
{

/**
 * This is the base class for all hybrid systems. The init and der_func methods
 * are used to implement the model's continuous dynamics. The other functions
 * are for discrete event dynamics.
 */
template <typename X> class ode_system
{
	public:
		/// Make a system with N state variables and M state event functions
		ode_system(int N_vars, int M_event_funcs):
			N(N_vars),M(M_event_funcs){}
		/// Get the number of state variables
		int numVars() const { return N; }
		/// Get the number of state events
		int numEvents() const { return M; }
		/// Copy the initial state of the model to q
		virtual void init(double* q) = 0;
		/// Compute the derivative for state q and put it in dq
		virtual void der_func(const double* q, double* dq) = 0;
		/// Compute the state event functions for state q and put them in z
		virtual void state_event_func(const double* q, double* z) = 0;
		/// Compute the time event function using state q
		virtual double time_event_func(const double* q) = 0;
		/// The internal transition function
		virtual void internal_event(double* q,
				const bool* state_event) = 0;
		/// The external transition function
		virtual void external_event(double* q, double e,
				const Bag<X>& xb) = 0;
		/// The confluent transition function
		virtual void confluent_event(double *q, const bool* state_event,
				const Bag<X>& xb) = 0;
		/// The output function
		virtual void output_func(const double *q, const bool* state_event,
				Bag<X>& yb) = 0;
		/// Garbage collection function. This works just like the Atomic gc_output method.
		virtual void gc_output(Bag<X>& gb) = 0;
		/// Destructor
		virtual ~ode_system(){}
	private:
		const int N, M;
};

/**
 * This is the interface for numerical integrators that are to be used with the
 * Hybrid class.
 */
template <typename X> class ode_solver
{
	public:
		/**
		 * Create and ode_solver that will integrate the der_func method of the
		 * supplied ode_system.
		 */
		ode_solver(ode_system<X>* sys):sys(sys){}
		/**
		 * Take an integration step from state q of at most size h_lim and
		 * return the step size that was actually used. Copy the result of
		 * the integration step to q.
		 */
		virtual double integrate(double* q, double h_lim) = 0;
		/**
		 * Advance the system through exactly h units of time.
		 */
		virtual void advance(double* q, double h) = 0;
		/// Destructor
		virtual ~ode_solver(){}
	protected:
		ode_system<X>* sys;
};

/**
 * This is the interface for algorithms that detect state events in the trajectory
 * of an ode_system. The ode_solver provided to this class is used to compute
 * intermediate states during the detection process.
 */
template <typename X> class event_locator
{
	public:
		/**
		 * The locator will use the der_func and state_event_func of the supplied
		 * ode_system object.
		 */
		event_locator(ode_system<X>* sys):sys(sys){}
		/**
		 * Find the first state event in the interval [0,h] starting from
		 * state qstart. The method returns true if an event is found,
		 * setting the events flags to true if the corresponding z entry in
		 * the state_event_func above triggered the event. The value of
		 * h is overwritten with the event time, and the state of the model
		 * at that time is copied to qend.
		 */
		virtual bool find_events(bool* events, const double *qstart, 
				double* qend, ode_solver<X>* solver, double& h) = 0;
		/// Destructor
		virtual ~event_locator(){}
	protected:
		ode_system<X>* sys;
};

/**
 * This Atomic model encapsulates an ode_system and numerical solvers for it.
 * Output from the Hybrid model is produced by the output_func method of the
 * ode_system whenever a state event or time event occurs. Internal, external,
 * and confluent events for the Hybrid model are computed with the corresponding
 * methods of the ode_system. The time advance of the Hybrid class ensures that
 * its internal events coincide with state and time events in the ode_system.
 */
template <typename X> class Hybrid:
	public Atomic<X>
{
	public:
		/**
		 * Create and initialize a simulator for the system. All objects 
		 * are adopted by the Hybrid object and are deleted when it is.
		 */
		Hybrid(ode_system<X>* sys, ode_solver<X>* solver,
				event_locator<X>* event_finder):
			sys(sys),solver(solver),event_finder(event_finder)
		{
			q = new double[sys->numVars()];
			q_trial = new double[sys->numVars()];
			event = new bool[sys->numEvents()+1];
			event_exists = false;
			sys->init(q_trial); // Get the initial state of the model
			for (int i = 0; i < sys->numVars(); i++) q[i] = q_trial[i];
			tentative_step(); // Take the first tentative step
		}
		/// Get the value of the kth continuous state variable
		double getState(int k) const { return q[k]; }
		/// Get the array of state variables
		const double* getState() const { return q; }
		/// Get the system that this solver is operating on
		ode_system<X>* getSystem() { return sys; }
		/**
		 * Do not override this method. It performs numerical integration and
		 * invokes the ode_system method for internal events as needed.
		 */
		void delta_int()
		{
			if (event_exists) // Execute the internal event
				sys->internal_event(q_trial,event); 
			// Copy the new state vector to q
			for (int i = 0; i < sys->numVars(); i++) q[i] = q_trial[i];
			tentative_step(); // Take a tentative step
		}
		/**
		 * Do not override this method. It performs numerical integration and
		 * invokes the ode_system for external events as needed.
		 */
		void delta_ext(double e, const Bag<X>& xb)
		{
			solver->advance(q,e); // Advance the state q by e
			sys->external_event(q,e,xb); // Compute the external event
			// Copy the new state to the trial solution 
			for (int i = 0; i < sys->numVars(); i++) q_trial[i] = q[i];
			tentative_step(); // Take a tentative step
		}
		/**
		 * Do not override. This method invokes the ode_system method
		 * for confluent events as needed.
		 */
		void delta_conf(const Bag<X>& xb)
		{
			if (event_exists) // Execute the confluent or external event
				sys->confluent_event(q_trial,event,xb); 
			else sys->external_event(q_trial,ta(),xb);
			// Copy the new state vector to q
			for (int i = 0; i < sys->numVars(); i++) q[i] = q_trial[i];
			tentative_step(); // Take a tentative step 
		}
		/// Do not override.
		double ta() { return sigma; }
		/// Do not override. Invokes the ode_system output function as needed.
		void output_func(Bag<X>& yb)
		{
			if (event_exists) sys->output_func(q_trial,event,yb);
		}
		/// Do not override. Invokes the ode_system gc_output method as needed.
		void gc_output(Bag<X>& gb) { sys->gc_output(gb); }
		/// Destructor deletes everything.
		virtual ~Hybrid()
		{
			delete [] q; delete [] q_trial; delete [] event;
			delete event_finder; delete solver; delete sys;
		}
	private:
		ode_system<X>* sys; // The ODE system
		ode_solver<X>* solver; // Integrator for the ode set
		event_locator<X>* event_finder; // Event locator
		double sigma; // Time to the next internal event
		double *q, *q_trial; // Current and tentative states
		bool* event; // Flags indicating the encountered event surfaces
		bool event_exists; // True if there is at least one event
		// Execute a tentative step and calculate the time advance function
		void tentative_step()
		{
			// Check for a time event
			double time_event = sys->time_event_func(q);
			// Integrate up to that time at most
			double step_size = solver->integrate(q_trial,time_event);
			// Look for state events inside of the interval [0,step_size]
			bool state_event_exists =
				event_finder->find_events(event,q,q_trial,solver,step_size);
			// Find the time advance and set the time event flag
			sigma = std::min(step_size,time_event);
			event[sys->numEvents()] = time_event <= sigma;
			event_exists = event[sys->numEvents()] || state_event_exists;
		}
};

} // end of namespace

#endif
