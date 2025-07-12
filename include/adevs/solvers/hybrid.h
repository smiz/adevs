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

#include <any>
#include <algorithm>
#include <cmath>
#include "adevs/models.h"


namespace adevs {

/** 
 * @brief Simulate piece wise continuous models that produce and react to discrete events.
 * 
 * This is the base class for all hybrid dynamic models. You should derive from this class
 * to define a component that is described by a piecewise continuous system of differential
 * equations.
 */
template <typename ValueType = std::any>
class ode_system {
  public:
    /**
      *  @brief Create a system with N state variables and M state event functions
      * 
      * The state variables are the variables comprising x in the derivative function
      * dx/dt = f(x). The state event functions are the number of elements in the z
      * array returned by the state_event_func() method. This interface is used to
      * define the system of equations that will be solved by the Hybrid class.
      * 
      * @param N_vars The number of state variables in the system
      * @param M_event_funcs The number of state events in the system
      */
    ode_system(int N_vars, int M_event_funcs) : N(N_vars), M(M_event_funcs) {}

    /// @brief Get the number of state variables
    int numVars() const { return N; }

    /// @brief Get the number of state events
    int numEvents() const { return M; }

    /**
     * @brief Set the initial state of the model to q
     * 
     * The array q must have at least numVars() elements. Write the
     * initial state of the model to q.
     *
     * @param q The array into which you must write the initial state
     */ 
    virtual void init(double* q) = 0;

    /**
     * @brief Compute the derivative for state q and put it in dq
     * 
     * Use the provided state q to compute the derivative dq/dt
     * and store it in the array dq. This is the function f in
     * dx/dt = f(x).
     * 
     * @param q The state of the model
     * @param dq The array into which you must write the derivative
     */
    virtual void der_func(double const* q, double* dq) = 0;

    /**
     * @brief Compute the state event functions for state q and put them in z.
     * 
     * The ode_system will undergo an internal transition at any point in time
     * when any of the state event functions set z[i] = 0.
     * 
     * @param q The continuous state of the model
     * @param z The array into which you must write the values of the
     * state event functions
     */
    virtual void state_event_func(double const* q, double* z) = 0;

    /** 
     * @brief Compute a time advance value for the continuous state q
     * 
     * This is the equivalent of a time advance for an Atomic but calculated 
     * using the continuous state q. The return value must be the time remaining
     * to the next internal event from the instant that the state entered
     * the continuous state q. If you need to compute the current time
     * to find the time to the next event, you can use the dx/dt = 1
     * in your set of differential equations (because the solution is
     * x(t) = t).
     * 
     * @param q The continuous state of the model
     * @return The time remaining to the next internal event
     */
    virtual double time_event_func(double const* q) = 0;

    /**
     * @brief This method is invoked each time a new continuous state is
     * selected by the numerical integration algorithm.
     * 
     * This method is invoked immediately following an update of the
     * continuous state variables during numerical integration. The main use of this
     * callback is to update algebraic variables that must satisfy a constraint
     * equation g(x,y) = 0, where x are your continuous state variables and y
     * is some set of algebraic variables. The default implementation does nothing.
     * 
     * @param q The continuous state of the model
     */
    virtual void postStep(double* q) {}

    /**
     * @brief This method is called after a trial step of the numerical integration.
     * 
     * This method is invoked for every step attempted by the numerical
     * integration method while estimating errors, looking for state events,
     * and performing other numerical tasks. The default implementation does
     * nothing.
     * 
     * @param q The continuous state of the model
     */
    virtual void postTrialStep(double* q) {}

    /**
     * @brief The internal transition function.
     * 
     * This is called when the time returned by the time_event_func() returns
     * zero or a value in the z array of the state_event_func() method is set to
     * zero. An entry in state_event array will contain true if the corresponding
     * element in the z array of the state_event_func() was zero. 
     * If this event is dues to a time event, then entry numEvents() in the
     * state_event array will be true. This acts like the internal transition function
     * of an Atomic model.
     * 
     * @param q The continuous state of the model at the event. This can be modified
     * by the internal state transition by writing values to the entries in q.
     * @param state_event Entries in this array indicate the cause of the internal
     * event.
     */
    virtual void internal_event(double* q, bool const* state_event) = 0;

    /**
     * @brief The external transition function
     * 
     * This is called when an input arrives prior to the expiration of the time
     * returned by the time_event_func() method and before any of the z elements
     * supplied by the state_event_func() method are zero. In all ways, it functions
     * like the external transition function of an Atomic model.
     * 
     * @param q The continuous state of the model at the event. This can be modified
     * by the internal state transition by writing values to the entries in q.
     * @param e The time elapsed since the last internal, external, or confluent event.
     * @param xb The input arriving at the model.
     */
    virtual void external_event(double* q, double e,
                                std::list<adevs::PinValue<ValueType>> const &xb) = 0;

    /**
     * @brief The confluent transition function
     * 
     * This is called when an input arrives at the same time that an internal event
     * would occur.
     * 
     * @param q The continuous state of the model at the event. This can be modified
     * by writing values to the entries in q.
     * @param state_event Entries in this array indicate the cause of the internal
     * event.
     * @param xb The input arriving at the model.
     */
    virtual void confluent_event(double* q, bool const* state_event,
                                 std::list<adevs::PinValue<ValueType>> const &xb) = 0;

    /**
     * @brief The output function
     * 
     * The ode_system version of the Atomic output_func() method. See the
     * internal_state_transition() of the ode_system for a description of the
     * state variable array q and state_event array. The model's output should
     * be inserted into the supplied list.
     * 
     * @param q The continuous state of the model at the output
     * @param state_event Entries in this array indicate the cause of the internal event.
     * @param yb A list to be filled with output from the component.
     */
    virtual void output_func(double const* q, bool const* state_event,
                             std::list<adevs::PinValue<ValueType>> &yb) = 0;

    /**
     *  @brief Get the Jacobian matrix for this component
     * 
     * The supplied array must be filled with the Jacobian
     * in column major ordering to make it compatible with LAPACK and similar
     * linear algebra packages. The default implementation is empty. The method
     * must return false if it is not supported and true if it is supported. If the
     * argument J is nullptr then the call is just to test for support.
     * 
     * @param q The state of the model
     * @param J column major Jacobian matrix to be filled with the Jacobian.
     * @return True if the method fills the Jacobian and false otherwise.
     */
    virtual bool get_jacobian(double const* q, double* J) { return false; }

    /// @brief Destructor
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

/** 
 * @brief This is the interface for numerical integrators that are to be used with the
 * Hybrid class.
 * 
 * If you want to create a new ODE solver for simulating your model with the Hybrid
 * class, then you will need that new solver to implement this interface.
 */
template <typename ValueType = std::any>
class ode_solver {
  public:
    /**
     * @brief Constructor that accepts as its argument the ode_system to simulation.
     * 
     * Create and ode_solver that will integrate the der_func() method of the
     * supplied ode_system.
     * 
     * @param sys The ode_system to integrate numerically
     */
    ode_solver(ode_system<ValueType>* sys) : sys(sys) {}

    /**
     * @brief Advance the solution by numerical integration up to a given
     * amount of time.
     * 
     * Take an integration step from state q through a step size of
     * most size h_lim and return the step size that was actually used. 
     * Copy the result of the integration step to q.
     * 
     * @param q The state to begin integration from and the array by which
     * the new state should be returned.
     * @param h_lim The upper limit of the step size
     * @return The step size that was used.
     */
    virtual double integrate(double* q, double h_lim) = 0;

    /**
     * @brief Advance the solution by exactly the given amount of time.
     * 
     * Take an integration step from state q through a step size of
     * exactly h. Copy the result of the integration step to q. 
     * 
     * @param q The state to begin integration from and the array by which
     * the new state should be returned.
     * @param h The step size
     */
    virtual void advance(double* q, double h) = 0;

    /**
     * @brief Destructor
     * 
     * This does not destroy the system passed to the ode_solver
     * in its constructor.
     */
    virtual ~ode_solver() {}

  protected:
    /// @brief The system of odes to be acted upon by the solver
    ode_system<ValueType>* sys;
};

/**
 * @brief This is the interface used by the Hybrid class to find state events
 * in the trajectory of an ode_system.
 * 
 * The ode_solver provided to this class is used to compute intermediate states
 * during the detection process. If you want to implement a new algorithm for
 * detecting state events, then you will want your new algorithm to implement
 * this interface.
 */
template <typename ValueType = std::any>
class event_locator {
  public:
    /**
     * @brief Constructor that is provided with the ode_system to simulate.
     * 
     * The locator algorithm will use the der_func() and state_event_func() 
     * of the supplied ode_system object.
     */
    event_locator(ode_system<ValueType>* sys) : sys(sys) {}

    /**
     * @brief Find the first state event in the interval [0,h] starting from
     * state qstart.
     * 
     * The method returns true if an event is found.
     * It must set set the events array entries to true for the corresponding z
     * entry from the state_event_func() that is zero. The value of
     * h is overwritten with the event time, and the state of the model
     * at that time is copied to qend. The event finding method should
     * select an instant of time when the zero crossing function is zero or
     * has changed sign to trigger an event.
     * 
     * @param events The entries in this array must be set to indicate which state
     * events were activated. A true entry indicates an event and false no event.
     * @param qstart The continuous state at the left side of the interval to search
     * for state events.
     * @param qend The continuous state at the right side of the interval to search for
     * state events. The state at the time of the event must be copied into this array
     * or it can be left alone if no event is found.
     * @param solver The ode_solver that can be used to calculate new states in the interval.
     * @param h Write to this value the time of the event relative to the start of the interval,
     * which is taken to be zero. Leave it alone if no event is found.
     * @return true if an event was found and false if no event was found.
     */
    virtual bool find_events(bool* events, double const* qstart, double* qend,
                             ode_solver<ValueType>* solver, double &h) = 0;

    /// @brief Destructor
    virtual ~event_locator() {}

  protected:
    /// The system of odes to be acted upon by the event locator
    ode_system<ValueType>* sys;
};

/**
 * @brief This Atomic model encapsulates an ode_system and numerical solvers
 * simulating that system.
 * 
 * Output from the Hybrid model is produced by the output_func() method of the
 * ode_system whenever a state event or time event occurs. Internal, external,
 * and confluent events for the Hybrid model are computed with the corresponding
 * methods of the ode_system. The time advance of the Hybrid class ensures that
 * its internal events coincide with state and time events in the ode_system.
 */
template <typename ValueType = std::any, class TimeType = double>
class Hybrid : public Atomic<ValueType, TimeType> {
  public:
    /**
     * @brief Create and initialize solvers for the ode_system.
     * 
     * All objects are adopted by the Hybrid object and are deleted when it is.
     * 
     * @param sys The system of equations to solve
     * @param solver The numerical integrator for taking steps in time
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

    /**
     * @brief Get the value of the kth continuous state variable
     *
     * This returns the kth element of the array returned by
     * the getState() method
     *  
     * @param k The index of the state variable
     * @return The value of the state variable
     */

    double getState(int k) const { return q[k]; }

    /**
     * @brief Get the array of state variables
     * 
     * @return The array of state variable values
     */
    double const* getState() const { return q; }

    /**
     * @brief Get the ode_system that this Hybrid model
     * is simulating
     * 
     * @return the ode_system passed to the constructor
     */
    ode_system<ValueType>* getSystem() { return sys; }

    /**
     * @brief Did a discrete event occur at the last state transition?
     *
     * This indicates if the previous internal, external, or confluent
     * event of the Hybrid object was caused by a state or time event
     * induced by the ode_system. The other possibility is that the
     * Hybrid object changed to advance the numerical solution of the
     * ode_system without a discrete event within the ode_system.
     * 
     * @return true if the ode_system caused a state or time event
     */
    bool eventHappened() const { return event_happened; }

    /**
     * @brief Do not override this method.
     * 
     * The Hybrid object uses its delta_int() method for numerical integration
     * and to invoke the ode_system method for internal events as needed to
     * handle state and time events.
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

    /**
     * @brief Do not override this method.
     * 
     * This method is used for numerical integration and
     * invokes the ode_system to handle for external events as needed.
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

    /**
     * @brief Do not override this method.
     * 
     * This method invokes the ode_system to handle
     * confluent events as needed.
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

    /// @brief Do not override.
    TimeType ta() {
        if (missedOutput.empty()) {
            return sigma;
        } else {
            return 0.0;
        }
    }

    /**
     * @brief Do not override.
     * 
     * Invokes the ode_system output function as needed.
     */
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
    double e_accum;      // Accumulated time between discrete events
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
