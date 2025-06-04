/*
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
#include "adevs/models.h"


namespace adevs {

/*
 * This is the base class for all hybrid systems. The init and der_func methods
 * are used to implement the model's continuous dynamics. The other functions
 * are for discrete event dynamics.
 */
template <typename ValueType>
class ode_system {
  public:
    /// Make a system with N state variables and M state event functions
    ode_system(int N_vars, int M_event_funcs) : N(N_vars), M(M_event_funcs) {}

    /// Get the number of state variables
    int numVars() const { return N; }

    /// Get the number of state events
    int numEvents() const { return M; }

    /// Copy the initial state of the model to q
    virtual void init(double* q) = 0;

    /// Compute the derivative for state q and put it in dq
    virtual void der_func(double const* q, double* dq) = 0;

    /// Compute the state event functions for state q and put them in z
    virtual void state_event_func(double const* q, double* z) = 0;

    /*
     * Compute the time event function using state q. The time to next
     * is measured as an interval from the present time. Therefore you
     * will need the current time, the time of your next event, and
     * the return value is the difference. An easy way to track the
     * current time is a state variable tnow with dtnow/dt = 1.
     */
    virtual double time_event_func(double const* q) = 0;

    /*
     * This method is invoked immediately following an update of the
     * continuous state variables. The main use of this callback is to
     * update algberaic variables. The default implementation does nothing.
     */
    virtual void postStep(double* q) {}

    /*
     * This is called after a trial step. It can be used to restore the values
     * of any variables that might have been changed by der_func or state_event_func
     * while calculating the trial step. By default this method does nothing.
     */
    virtual void postTrialStep(double* q) {}

    /*
     * The internal transition function. The state_event array will contain
     * true if the corresponding level crossing function z triggered the
     * this internal event. If this is a time event, then entry numEvents()
     * will be true.
     */
    virtual void internal_event(double* q, bool const* state_event) = 0;

    /// The external transition function
    virtual void external_event(double* q, double e,
                                std::list<adevs::PinValue<ValueType>> const &xb) = 0;

    /// The confluent transition function
    virtual void confluent_event(double* q, bool const* state_event,
                                 std::list<adevs::PinValue<ValueType>> const &xb) = 0;

    /// The output function
    virtual void output_func(double const* q, bool const* state_event,
                             std::list<adevs::PinValue<ValueType>> &yb) = 0;

    /// Get the N x N Jacobian matrix. The supplied array must be filled with the Jacobian
    /// in column major ordering to make it compatible with LAPACK and similar
    /// linear algebra packages. The default implementation is empty. The method
    /// must return false if it is not supported and true if it is supported. If the
    /// argument J is NULL then the call is just to test for support.
    virtual bool get_jacobian(double const* q, double* J) { return false; }

    /// Destructor
    virtual ~ode_system() {}

  private:
    int const N, M;
};

// Clang complains about the postTrialStep declaration.
// Because what we wrote is what we intended, the
// warning is disable just for this class definition.
#ifdef _clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

/*
 * This is the interface for numerical integrators that are to be used with the
 * Hybrid class.
 */
template <typename ValueType>
class ode_solver {
  public:
    /*
     * Create and ode_solver that will integrate the der_func method of the
     * supplied ode_system.
     */
    ode_solver(ode_system<ValueType>* sys) : sys(sys) {}

    /*
     * Take an integration step from state q of at most size h_lim and
     * return the step size that was actually used. Copy the result of
     * the integration step to q.
     */
    virtual double integrate(double* q, double h_lim) = 0;

    /*
     * Advance the system through exactly h units of time.
     */
    virtual void advance(double* q, double h) = 0;

    /// Destructor
    virtual ~ode_solver() {}

  protected:
    /// The system of odes to be acted upon by the solver
    ode_system<ValueType>* sys;
};

/*
 * This is the interface for algorithms that detect state events in the trajectory
 * of an ode_system. The ode_solver provided to this class is used to compute
 * intermediate states during the detection process.
 */
template <typename ValueType>
class event_locator {
  public:
    /*
     * The locator will use the der_func and state_event_func of the supplied
     * ode_system object.
     */
    event_locator(ode_system<ValueType>* sys) : sys(sys) {}

    /*
     * Find the first state event in the interval [0,h] starting from
     * state qstart. The method returns true if an event is found,
     * setting the events flags to true if the corresponding z entry in
     * the state_event_func above triggered the event. The value of
     * h is overwritten with the event time, and the state of the model
     * at that time is copied to qend. The event finding method should
     * select an instant of time when the zero crossing function is zero or
     * has changed sign to trigger an event.
     */
    virtual bool find_events(bool* events, double const* qstart, double* qend,
                             ode_solver<ValueType>* solver, double &h) = 0;

    /// Destructor
    virtual ~event_locator() {}

  protected:
    /// The system of odes to be acted upon by the event locator
    ode_system<ValueType>* sys;
};

/*
 * This Atomic model encapsulates an ode_system and numerical solvers for it.
 * Output from the Hybrid model is produced by the output_func method of the
 * ode_system whenever a state event or time event occurs. Internal, external,
 * and confluent events for the Hybrid model are computed with the corresponding
 * methods of the ode_system. The time advance of the Hybrid class ensures that
 * its internal events coincide with state and time events in the ode_system.
 */
template <typename ValueType, class TimeType = double>
class Hybrid : public Atomic<ValueType, TimeType> {
  public:
    /*
     * Create and initialize a simulator for the system. All objects
     * are adopted by the Hybrid object and are deleted when it is.
     * @param sys The system of equations to solver
     * @param solver The numerical solver for taking steps in time
     * @param event_finder The state event detection algorithm
     */
    Hybrid(ode_system<ValueType>* sys, ode_solver<ValueType>* solver,
           event_locator<ValueType>* event_finder)
        : sys(sys), solver(solver), event_finder(event_finder), e_accum(0.0) {
        q = new double[sys->numVars()];
        q_trial = new double[sys->numVars()];
        event = new bool[sys->numEvents() + 1];
        event_exists = false;
        sys->init(q_trial);  // Get the initial state of the model
        for (int i = 0; i < sys->numVars(); i++) {
            q[i] = q_trial[i];
        }
        tentative_step();  // Take the first tentative step
    }

    /// Get the value of the kth continuous state variable
    double getState(int k) const { return q[k]; }

    /// Get the array of state variables
    double const* getState() const { return q; }

    /// Get the system that this solver is operating on
    ode_system<ValueType>* getSystem() { return sys; }

    /// Did a discrete event occur at the last state transition?
    bool eventHappened() const { return event_happened; }

    /*
     * Do not override this method. It performs numerical integration and
     * invokes the ode_system method for internal events as needed.
     */
    void delta_int() {
        if (!missedOutput.empty()) {
            missedOutput.clear();
            return;
        }
        e_accum += ta();
        // Execute any discrete events
        event_happened = event_exists;
        if (event_exists)  // Execute the internal event
        {
            sys->internal_event(q_trial, event);
            e_accum = 0.0;
        }
        // Copy the new state vector to q
        for (int i = 0; i < sys->numVars(); i++) {
            q[i] = q_trial[i];
        }
        tentative_step();  // Take a tentative step
    }

    /*
     * Do not override this method. It performs numerical integration and
     * invokes the ode_system for external events as needed.
     */
    void delta_ext(TimeType e, std::list<adevs::PinValue<ValueType>> const &xb) {
        bool state_event_exists = false;
        event_happened = true;
        // Check that we have not missed a state event
        if (event_exists) {
            for (int i = 0; i < sys->numVars(); i++) {
                q_trial[i] = q[i];
            }
            solver->advance(q_trial, e);
            state_event_exists =
                event_finder->find_events(event, q, q_trial, solver, e);
            // We missed an event
            if (state_event_exists) {
                output_func(missedOutput);
                sys->confluent_event(q_trial, event, xb);
                for (int i = 0; i < sys->numVars(); i++) {
                    q[i] = q_trial[i];
                }
            }
        }
        if (!state_event_exists)  // We didn't miss an event
        {
            solver->advance(q, e);  // Advance the state q by e
            // Let the model adjust algebraic variables, etc. for the new state
            sys->postStep(q);
            // Process the discrete input
            sys->external_event(q, e + e_accum, xb);
        }
        e_accum = 0.0;
        // Copy the new state to the trial solution
        for (int i = 0; i < sys->numVars(); i++) {
            q_trial[i] = q[i];
        }
        tentative_step();  // Take a tentative step
    }

    /*
     * Do not override. This method invokes the ode_system method
     * for confluent events as needed.
     */
    void delta_conf(std::list<adevs::PinValue<ValueType>> const &xb) {
        if (!missedOutput.empty()) {
            missedOutput.clear();
            if (sigma > 0.0) {
                event_exists = false;
            }
        }
        // Execute any discrete events
        event_happened = true;
        if (event_exists) {
            sys->confluent_event(q_trial, event, xb);
        } else {
            sys->external_event(q_trial, e_accum + ta(), xb);
        }
        e_accum = 0.0;
        // Copy the new state vector to q
        for (int i = 0; i < sys->numVars(); i++) {
            q[i] = q_trial[i];
        }
        tentative_step();  // Take a tentative step
    }

    /// Do not override.
    TimeType ta() {
        if (missedOutput.empty()) {
            return sigma;
        } else {
            return 0.0;
        }
    }

    /// Do not override. Invokes the ode_system output function as needed.
    void output_func(std::list<adevs::PinValue<ValueType>> &yb) {
        if (!missedOutput.empty()) {
            for (auto iter : missedOutput) {
                yb.push_back(iter);
            }
            if (sigma == 0.0) {  // Confluent event
                sys->output_func(q_trial, event, yb);
            }
        } else {
            // Let the model adjust algebraic variables, etc. for the new state
            sys->postStep(q_trial);
            if (event_exists) {
                sys->output_func(q_trial, event, yb);
            }
        }
    }

  private:
    ode_system<ValueType>* sys;              // The ODE system
    ode_solver<ValueType>* solver;           // Integrator for the ode set
    event_locator<ValueType>* event_finder;  // Event locator
    double sigma;                            // Time to the next internal event
    double *q, *q_trial;                     // Current and tentative states
    bool* event;        // Flags indicating the encountered event surfaces
    bool event_exists;  // True if there is at least one event
    bool
        event_happened;  // True if a discrete event in the ode_system took place
    double e_accum;      // Accumlated time between discrete events
    std::list<adevs::PinValue<ValueType>> missedOutput;  // Output missed at an external event
    // Execute a tentative step and calculate the time advance function
    void tentative_step() {
        // Check for a time event
        double time_event = sys->time_event_func(q);
        // Integrate up to that time at most
        double step_size = solver->integrate(q_trial, time_event);
        // Look for state events inside of the interval [0,step_size]
        bool state_event_exists =
            event_finder->find_events(event, q, q_trial, solver, step_size);
        // Find the time advance and set the time event flag
        sigma = std::min<double>(step_size, time_event);
        event[sys->numEvents()] = time_event <= sigma;
        event_exists = event[sys->numEvents()] || state_event_exists;
        sys->postTrialStep(q);
    }
};

}  // namespace adevs

#endif
