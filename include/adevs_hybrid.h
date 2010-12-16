#ifndef _adevs_hybrid_h_
#define _adevs_hybrid_h_
#include <algorithm>
#include <cmath>
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
		/**
		 * This method is invoked immediately following an update of the
		 * continuous state variables. The main use of this callback is to
		 * update algberaic variables. The default implementation does nothing.
		 */
		virtual void postStep(const double* q){};
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
 * This extension of the ode_system provides for modeling some semi-explicit
 * DAEs of index 1. These have the form dx/dt = f(x,y), y = g(x,y).
 * The solution to y=g(x,y) is found by fixed point iteration. 
 * Only the methods that include the algebraic variables should be overriden.
 * Any ODE solver can be used to generate trajectories for this object.
 */
template <typename X> class dae_se1_system:
	public ode_system<X>
{
	public:
		/**
		 * Make a system with N state variables, M state event functions
		 * and A algebraic variables.
		 */
		dae_se1_system(int N_vars, int M_event_funcs, int A_alg_vars,
				double err_tol = 1E-8):
			ode_system<X>(N_vars,M_event_funcs),
			A(A_alg_vars),
			err_tol(err_tol),
			good(0)
			{
				a[0] = new double[A];
				a[1] = new double[A];
			}
		/// Get the number of algebraic variables
		int numAlgVars() const { return A; }
		/// Get the algebraic variables
		double getAlgVar(int i) const { return a[good][i]; }
		/**
		 * Write an intial solution for the state variables q
		 * and algebraic variables a.
		 */
		virtual void init(double* q, double* a) = 0;
		/**
		 * Calculate the algebraic function for the state vector
		 * q and algebraic variables a and store the result to af.
		 * A solution to alg_func(a,q) = a will be found by fixed
		 * point iteration on this function; i.e., by
		 * a(i+1) = af(a(i),q) until a(i+1)-af(q(i),q) ~ 0.
		 */
		virtual void alg_func(const double* q, const double* a, double* af) = 0;
		/**
		 * Calculate the derivative of the state variables and store
		 * the result in dq.
		 */
		virtual void der_func(const double* q, const double* a, double* dq) = 0;
		/**
		 * Calculate the state event functions and store the result in z.
		 */
		virtual void state_event_func(const double* q, const double* a, double* z) = 0;
		/// Compute the time event function using state q and algebraic variables a
		virtual double time_event_func(const double* q, const double* a) = 0;
		/**
		 * Update any variables that need updating at then end of a simulation step.
		 * This is called both when postStep(q) would be called and also immediately
		 * after the execution a discrete state transition.
		 */
		virtual void postStep(const double* q, const double* a) = 0;
		/// The internal transition function
		virtual void internal_event(double* q, double* a,
				const bool* state_event) = 0;
		/// The external transition function
		virtual void external_event(double* q, double* a,
				double e, const Bag<X>& xb) = 0;
		/// The confluent transition function
		virtual void confluent_event(double *q, double* a, 
				const bool* state_event, const Bag<X>& xb) = 0;
		/// The output function
		virtual void output_func(const double *q, const double* a,
				const bool* state_event, Bag<X>& yb) = 0;
		/// Destructor
		virtual ~dae_se1_system()
		{
			delete [] a[0];
			delete [] a[1];
		}

		// Do not override
		void init(double* q)
		{
			init(q,a[good]);
		}
		// Do not override
		void der_func(const double* q, double* dq)
		{
			solve(q);
			der_func(q,a[good],dq);
		}
		// Override only if you have no state event functions.
		void state_event_func(const double* q, double* z)
		{
			solve(q);
			state_event_func(q,a[good],z);
		}
		// Override only if you have no time events.
		double time_event_func(const double* q)
		{
			solve(q);
			return time_event_func(q,a[good]);
		}
		// Do not override
		void postStep(const double* q)
		{
			solve(q);
			postStep(q,a[good]);
		}
		// Do not override
		void internal_event(double* q, const bool* state_event)
		{
			// The variable a was solved for in the post step
			internal_event(q,a[good],state_event);
			// Make sure the algebraic variables are consistent with q
			solve(q);
			postStep(q,a[good]);
		}
		// Do not override
		void external_event(double* q, double e, const Bag<X>& xb)
		{
			// The variable a was solved for in the post step
			external_event(q,a[good],e,xb);
			// Make sure the algebraic variables are consistent with q
			solve(q);
			postStep(q,a[good]);
		}
		// Do not override
		void confluent_event(double *q, const bool* state_event, const Bag<X>& xb)
		{
			// The variable a was solved for in the post step
			confluent_event(q,a[good],state_event,xb);
			// Make sure the algebraic variables are consistent with q
			solve(q);
			postStep(q,a[good]);
		}
		// Do not override
		void output_func(const double *q, const bool* state_event, Bag<X>& yb)
		{
			// The variable a was solved for in the post step
			output_func(q,a[good],state_event,yb);
		}
	protected:
		/**
		 * Solve the algebraic equations by fixed point iteration. This should
		 * not usually need to be called by the derived class. An exception might
		 * be where updated values for the algebraic variables are needed from
		 * within an event function due to some discrete change in q or the
		 * structure of the systems.
		 */
		void solve(const double* q);
	private:
		const int A;
		const double err_tol;
		int good;
		double* a[2];
};

template <typename X>
void dae_se1_system<X>::solve(const double* q)
{
	double err;
	// Iterate unsubsequent guesses and hope that they
	// converge to a solution
	int alt = (good+1)%2;
	do
	{
		err = 0.0;
		alg_func(q,a[good],a[alt]);
		for (int i = 0; i < A; i++)
		{
			double ee = fabs(a[good][i]-a[alt][i]);
			if (ee > err) err = ee;
		}
		good = alt;
		alt = (good+1)%2;
	}
	while (err > err_tol);
}

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
		/// Did a discrete event occur at the last state transition?
		bool eventHappened() const { return event_happened; }
		/**
		 * Do not override this method. It performs numerical integration and
		 * invokes the ode_system method for internal events as needed.
		 */
		void delta_int()
		{
			// Execute any discrete events
			event_happened = event_exists;
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
			event_happened = true;
			solver->advance(q,e); // Advance the state q by e
			// Let the model adjust algebraic variables, etc. for the new state
			sys->postStep(q);
			// Process the discrete input
			sys->external_event(q,e,xb); 
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
			// Execute any discrete events
			event_happened = true;
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
			// Let the model adjust algebraic variables, etc. for the new state
			sys->postStep(q_trial);
			if (event_exists)
				sys->output_func(q_trial,event,yb);
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
		bool event_happened; // True if a discrete event in the ode_system took place
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
