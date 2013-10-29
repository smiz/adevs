/**
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */
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
		virtual void postStep(double* q){};
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
 * <p>This extension of the ode_system provides for modeling some semi-explicit
 * DAEs of index 1, specifically those in the form dx/dt = f(x,y), y = g(x,y).
 * The solution to y=g(x,y) is found by iteration on y.
 * See "The Numerical Solution of Differential-Algebraic Systems by Runge-Kutta
 * Methods" by Ernst Hairer, Michel Roche and Christian Lubich, published
 * by Springer as Lecture Notes in Mathematics, Volum 1409, c. 1989.
 * The section on Half-explicit methods (starting pg. 20 of my copy)
 * describes the procedure.</p>
 * <p>Only the methods that include the algebraic variables should be overriden.
 * Any explicit, single step ODE solver can be used to generate trajectories for
 * this object (e.g., the Runge-Kutta methods included with adevs will work).</p>
 */
template <typename X> class dae_se1_system:
	public ode_system<X>
{
	public:
		/**
		 * Make a system with N state variables, M state event functions
		 * and A algebraic variables. The error tolerance determines
		 * the acceptable accuracy for the algebraic solution to y=g(x,y).
		 * The max_iters argument determines how many iterations will
		 * be used to try and generate a solution.
		 */
		dae_se1_system(int N_vars, int M_event_funcs, int A_alg_vars,
				double err_tol = 1E-10, int max_iters = 30, double alpha = -1.0):
			ode_system<X>(N_vars,M_event_funcs),
			A(A_alg_vars),
			max_iters(max_iters),
			err_tol(err_tol),
			alpha(alpha)
			{
				failed = 0;
				max_err = 0.0;
				a = new double[A];
				atmp = new double[A];
				d = new double[A];
				f[1] = new double[A];
				f[0] = new double[A];
			}
		/// Get the number of algebraic variables
		int numAlgVars() const { return A; }
		/// Get the algebraic variables
		double getAlgVar(int i) const { return a[i]; }
		/**
		 * Write an intial solution for the state variables q
		 * and algebraic variables a.
		 */
		virtual void init(double* q, double* a) = 0;
		/**
		 * Calculate the algebraic function for the state vector
		 * q and algebraic variables a and store the result to af.
		 * A solution to alg_func(a,q) = a will be found by 
		 * iteration on this function.
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
		virtual void postStep(double* q, double* a) = 0;
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
			delete [] d;
			delete [] a;
			delete [] atmp;
			delete [] f[1];
			delete [] f[0];
		}
		/**
		 * Get the number of times that the error tolerance was not satisfied
		 * before the iteration limit was reached.
		 */
		int getIterFailCount() const { return failed; }
		/**
		 * Get the worst error in a case where the algebraic solver did
		 * not satisfy the error tolerance. This will be zero if
		 * there were no failures of the algebraic solver.
		 */
		double getWorseError() const { return max_err; }
		/// Do not override
		void init(double* q)
		{
			init(q,a);
		}
		/// Do not override
		void der_func(const double* q, double* dq)
		{
			solve(q);
			der_func(q,a,dq);
		}
		/// Override only if you have no state event functions.
		void state_event_func(const double* q, double* z)
		{
			solve(q);
			state_event_func(q,a,z);
		}
		/// Override only if you have no time events.
		double time_event_func(const double* q)
		{
			solve(q);
			return time_event_func(q,a);
		}
		/// Do not override
		void postStep(double* q)
		{
			solve(q);
			postStep(q,a);
		}
		/// Do not override
		void internal_event(double* q, const bool* state_event)
		{
			// The variable a was solved for in the post step
			internal_event(q,a,state_event);
			// Make sure the algebraic variables are consistent with q
			solve(q);
			postStep(q,a);
		}
		/// Do not override
		void external_event(double* q, double e, const Bag<X>& xb)
		{
			// The variable a was solved for in the post step
			external_event(q,a,e,xb);
			// Make sure the algebraic variables are consistent with q
			solve(q);
			postStep(q,a);
		}
		/// Do not override
		void confluent_event(double *q, const bool* state_event, const Bag<X>& xb)
		{
			// The variable a was solved for in the post step
			confluent_event(q,a,state_event,xb);
			// Make sure the algebraic variables are consistent with q
			solve(q);
			postStep(q,a);
		}
		/// Do not override
		void output_func(const double *q, const bool* state_event, Bag<X>& yb)
		{
			// The variable a was solved for in the post step
			output_func(q,a,state_event,yb);
		}
	protected:
		/**
		 * Solve the algebraic equations. This should
		 * not usually need to be called by the derived class. An exception might
		 * be where updated values for the algebraic variables are needed from
		 * within an event function due to some discrete change in q or the
		 * structure of the systems.
		 */
		void solve(const double* q);
	private:
		const int A, max_iters;
		const double err_tol, alpha;
		// Guess at the algebraic solution
		double *a, *atmp;
		// Guesses at g(y)-y
		double* f[2];
		// Direction
		double* d;
		// Maximum error in the wake of a failure
		double max_err;
		// Number of failures
		int failed;
};

template <typename X>
void dae_se1_system<X>::solve(const double* q)
{
	int iter_count = 0, alt, good;
	double prev_err, err = 0.0, ee, beta, g2, alpha_tmp = alpha;
	/**
	 * Solve y=g(x,y) by the conjugate gradient method.
	 * Method iterates on f(x,y)=g(x,y)-y to find
	 * f(x,y)=0.
	 */
	_adevs_dae_se_1_system_solve_try_it_again:
	alt = 0;
	good = 1;
	prev_err = DBL_MAX;
	// First step by steepest descent
	alg_func(q,a,f[alt]);
	for (int i = 0; i < A; i++)
	{
		// Calculate f(x,y)
		f[alt][i] -= a[i];
		// First direction
		d[i] = -f[alt][i];
		// Make the move
		atmp[i] = a[i];
		a[i] += alpha_tmp*d[i];
	}
	// Otherwise, first guess by steepest descent
	// Finish search by conjugate gradiant
	while (iter_count < max_iters)
	{
		iter_count++;
		err = 0.0;
		// Calculate y = g(x,y) 
		alg_func(q,a,f[good]);
		// Check the quality of the solution
		for (int i = 0; i < A; i++)
		{
			// Calculate f(x,y) 
			f[good][i] -= a[i];
			// Get the largest error 
			ee = fabs(f[good][i]);
			if (ee > err) err = ee;
		}
		// If the solution is good enough then return
		if (err < err_tol) return;
		// If the solution is not converging...
		if (err > prev_err)
		{
			// Restore previous solution
			for (int i = 0; i < A; i++)
				a[i] = atmp[i];
			// Restart with a new value for alpha
			if (alpha_tmp < 0.0) alpha_tmp = -alpha_tmp;
			else alpha_tmp *= -0.5;
			goto _adevs_dae_se_1_system_solve_try_it_again;
		}
		prev_err = err;
		// Calculate beta. See Strang's "Intro. to Applied Mathematics",
		// pg. 379.
		beta = g2 = 0.0;
		for (int i = 0; i < A; i++)
			g2 += f[alt][i]*f[alt][i];
		for (int i = 0; i < A; i++)
			beta += f[good][i]*(f[good][i]-f[alt][i]);
		beta /= g2;
		// Calculate a new guess at the solution
		for (int i = 0; i < A; i++)
		{
			d[i] = beta*d[i]-f[good][i];
			atmp[i] = a[i];
			a[i] += alpha_tmp*d[i];
		}
		// Swap buffers
		good = alt;
		alt = (good+1)%2;
	}
	// Increment the failed count and worse case error if an 
	// acceptible solution was not found.
	failed++;
	if (err > max_err)
		max_err = err;
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
		 * at that time is copied to qend. The event finding method should
		 * select an instant of time when the zero crossing function is zero or
		 * has changed sign to trigger an event.
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
template <typename X, class T = double> class Hybrid:
	public Atomic<X,T>
{
	public:
		/**
		 * Create and initialize a simulator for the system. All objects 
		 * are adopted by the Hybrid object and are deleted when it is.
		 */
		Hybrid(ode_system<X>* sys, ode_solver<X>* solver,
				event_locator<X>* event_finder):
			sys(sys),solver(solver),event_finder(event_finder),
			e_accum(0.0)
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
			if (!missedOutput.empty())
			{
				missedOutput.clear();
				return;
			}
			e_accum += ta();
			// Execute any discrete events
			event_happened = event_exists;
			if (event_exists) // Execute the internal event
			{
				sys->internal_event(q_trial,event); 
				e_accum = 0.0;
			}
			// Copy the new state vector to q
			for (int i = 0; i < sys->numVars(); i++) q[i] = q_trial[i];
			tentative_step(); // Take a tentative step
		}
		/**
		 * Do not override this method. It performs numerical integration and
		 * invokes the ode_system for external events as needed.
		 */
		void delta_ext(T e, const Bag<X>& xb)
		{
			bool state_event_exists = false;
			event_happened = true;
			// Check that we have not missed a state event
			if (event_exists)
			{
				for (int i = 0; i < sys->numVars(); i++)
					q_trial[i] = q[i];
				solver->advance(q_trial,e);
				state_event_exists =
					event_finder->find_events(event,q,q_trial,solver,e);
				// We missed an event
				if (state_event_exists)
				{
					output_func(missedOutput);
					sys->confluent_event(q_trial,event,xb); 
					for (int i = 0; i < sys->numVars(); i++)
						q[i] = q_trial[i];
				}
			}
			if (!state_event_exists)// We didn't miss an event
			{
				solver->advance(q,e); // Advance the state q by e
				// Let the model adjust algebraic variables, etc. for the new state
				sys->postStep(q);
				// Process the discrete input
				sys->external_event(q,e+e_accum,xb);
			}
			e_accum = 0.0;
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
			if (!missedOutput.empty())
			{
				missedOutput.clear();
				if (sigma > 0.0) event_exists = false;
			}
			// Execute any discrete events
			event_happened = true;
			if (event_exists) 
				sys->confluent_event(q_trial,event,xb); 
			else sys->external_event(q_trial,e_accum+ta(),xb);
			e_accum = 0.0;
			// Copy the new state vector to q
			for (int i = 0; i < sys->numVars(); i++) q[i] = q_trial[i];
			tentative_step(); // Take a tentative step 
		}
		/// Do not override.
		T ta()
		{
			if (missedOutput.empty()) return sigma;
			else return 0.0;
		}
		/// Do not override. Invokes the ode_system output function as needed.
		void output_func(Bag<X>& yb)
		{
			if (!missedOutput.empty())
			{
				typename Bag<X>::iterator iter = missedOutput.begin();
				for (; iter != missedOutput.end(); iter++)
					yb.insert(*iter);
				if (sigma == 0.0) // Confluent event
					sys->output_func(q_trial,event,yb);
			}
			else
			{
				// Let the model adjust algebraic variables, etc. for the new state
				sys->postStep(q_trial);
				if (event_exists)
					sys->output_func(q_trial,event,yb);
			}
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
		double e_accum; // Accumlated time between discrete events
		Bag<X> missedOutput; // Output missed at an external event
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
