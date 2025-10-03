
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

#ifndef _adevs_cvode_h_
#define _adevs_cvode_h_
#include <cvode/cvode.h>
#include <any>
#include "adevs/adevs.h"

namespace adevs {

/**
  * \page cvode.cpp
  * \include cvode.cpp
  */

/**
 * @brief Use CVode from the SUNDIALS package to simulate a piecewise continuous ODE model
 * that produces and responds to discrete events.
 * 
 * You will need to install and be familiar with the
 * <a href="https://computing.llnl.gov/projects/sundials">SUNDIALS</a>
 * CVode package to use this class. Your CVODE
 * derived Atomic model can be integrated directly into a larger adevs discrete
 * event simulation. An example of using the CVODE class is in the
 * test case \subpage cvode.cpp
 */

template <typename ValueType = std::any>
class CVODE : public Atomic<ValueType> {
  public:
    /**
   * @brief Constructor
   * 
   * The constructor should be used to initialize CVode for your model
   * and assign the initial state vector.
   */
    CVODE();
    /**
     * @brief Destructor
     * 
     * The destructor should be used to clean up the CVode data structures
     * that you initialized in the constructor.
     */
    virtual ~CVODE();
    /**
     * @brief Do not override!
     * 
     * Do not override the internal state transition function! Use cvode_delta_int()
     * instead to change state in response to internal events.
     */
    void delta_int();
    /**
     * @brief Do not override!
     * 
     * Do not override the external state transition function! Use cvode_delta_ext()
     * instead to change state in response to external events.
     */
    void delta_ext(double e, std::list<PinValue<ValueType>> const &xb);
    /**
     * @brief Do not override!
     * 
     * Do not override the confluent state transition function! Use cvode_delta_conf()
     * instead to change state in response to simultaneous internal and external
     * events.
     */
    void delta_conf(std::list<PinValue<ValueType>> const &xb);
    /**
     * @brief Do not override!
     * 
     * Do not override the output function! Use cvode_output_func() to produce
     * output at internal events.
     */
    void output_func(std::list<PinValue<ValueType>> &yb);
    /**
     * @brief Do not override!
     * 
     * Do not override the time advance function! See cvode_integrate() for how to
     * schedule time events.
     */
    double ta();

  protected:
    /**
   * @brief Override this method to implement your internal transition function.
   * 
   * This is called when the simulation clock reaches the time returned in the
   * tf parameter passed to the cvode_integrate method if the event parameter
   * was set to true. Use this method to implement your model's internal transition 
   * function.
   */
    virtual void cvode_delta_int() = 0;
    /**
     * @brief Override this method to implement your external transition function.
     * 
     * This is called when a discrete event arrives at the model prior to the next
     * internal event. Use this method to implement your model's external transition
     * function.
     * @param t  This is the current time as seen by the model and not the elapsed time.
     * @param xb The list of input that arrived at the model.
     */
    virtual void cvode_delta_ext(double t, std::list<PinValue<ValueType>> const &xb) = 0;
    /**
     * @brief Override this method to implement your confluent transition function.
     * 
     * This is your CVode model's confluent transition function. It is called only
     * if the event parameter passed to cvode_integrate() was set to true. Otherwise,
     * the model experiences an external event instead.
     * 
     * @param xb The list of input that arrived at the model.
     */
    virtual void cvode_delta_conf(std::list<PinValue<ValueType>> const &xb) = 0;
    /**
     * @brief Override this method to implement your output function.
     * 
     * This where your CVode model can produce events to be consumed by other models
     * in the larger discrete event simulation. This is called when the simulation
     * time matches the time value returned by cvode_integrate().
     * 
     * @param yb A list of fill with output events.
     */
    virtual void cvode_output_func(std::list<PinValue<ValueType>> &yb) = 0;
    /**
     * @brief Override this method so that it returns the current state of the model.
     * 
     * Return the current state of the model. This is the state at the time
     * value returned by cvode_integrate().
     * 
     * @return The current state of the model.
     */
    virtual N_Vector cvode_get_state() = 0;
    /**
     * @brief Override this method to advance the continuous state of the model
     * up to the next state event or some appropriate limit for your model.
     * 
     * Use the CVode integrator to advance your continuous state. The time that you
     * advanced to must be returned in tf. If you want to treat this time as an
     * internal event, then set the event parameter to true.
     * 
     * @param tf The time at which the current state is valid.
     * @param event Set to true if an internal event should occur at this time.
     */
    virtual void cvode_integrate(double &tf, bool &event) = 0;
    /**
     * @brief Override this method to advance the continuous state of the model
     * up to the next state event or exactly the limit prescribed by the caller.
     * 
     * Use the CVode integrator to advance the continuous state exactly to time tf.
     * If you want to treat this time as an internal event, then set the event
     * event parameter to true.
     * @param tf The time to which the state must be advanced.
     * @param event Set to true if an internal event should occur at this time.
     */
    virtual void cvode_integrate_until(double tf, bool &event) = 0;
    /**
     * @brief This is called when the model needs to be reinitialized following
     * a state event.
     * 
     * Use the CVode reinitialize functions to restart the CVode solver
     * with the state y and time t.
     * 
     * @param y The new initial state following the event.
     * @param t The time at which this state is valid.
     */
    virtual void cvode_reinit(N_Vector y, double t) = 0;

  private:
    N_Vector y_checkpoint;
    bool has_event;
    double t, t_projected;
};

template <typename ValueType>
CVODE<ValueType>::CVODE()
    : Atomic<ValueType, double>(), y_checkpoint(nullptr), has_event(false), t(0.0) {}

template <typename ValueType>
CVODE<ValueType>::~CVODE() {
    if (y_checkpoint != nullptr) {
        N_VDestroy(y_checkpoint);
    }
}

template <typename ValueType>
double CVODE<ValueType>::ta() {
    if (y_checkpoint == nullptr) {
        y_checkpoint = N_VClone(this->cvode_get_state());
        N_VAddConst(this->cvode_get_state(), 0.0, y_checkpoint);
    }
    this->cvode_integrate(t_projected, has_event);
    return t_projected - t;
}


template <typename ValueType>
void CVODE<ValueType>::delta_int() {
    if (has_event) {
        this->cvode_delta_int();
    }
    t = t_projected;
    N_VAddConst(this->cvode_get_state(), 0.0, y_checkpoint);
}

template <typename ValueType>
void CVODE<ValueType>::delta_conf(std::list<PinValue<ValueType>> const &xb) {
    if (has_event) {
        this->cvode_delta_conf(xb);
    } else {
        this->cvode_delta_ext(t_projected, xb);
    }
    t = t_projected;
    N_VAddConst(this->cvode_get_state(), 0.0, y_checkpoint);
}

template <typename ValueType>
void CVODE<ValueType>::delta_ext(double e, std::list<PinValue<ValueType>> const &xb) {
    this->cvode_reinit(y_checkpoint, t);
    t += e;
    this->cvode_integrate_until(t, has_event);
    if (has_event) {
        this->cvode_delta_conf(xb);
    } else {
        this->cvode_delta_ext(t, xb);
    }
    N_VAddConst(this->cvode_get_state(), 0.0, y_checkpoint);
}

template <typename ValueType>
void CVODE<ValueType>::output_func(std::list<PinValue<ValueType>> &yb) {
    if (has_event) {
        this->cvode_output_func(yb);
    }
}

}  // namespace adevs

#endif
