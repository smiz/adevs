/*
 * Copyright (c) 2014, James Nutaro
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

#ifndef _adevs_fmi_h_
#define _adevs_fmi_h_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "adevs/solvers/hybrid.h"
#include "fmi2FunctionTypes.h"
#include "fmi2Functions.h"
#include "fmi2TypesPlatform.h"

// Functions for loading DLL and so files
#ifdef _WIN32
#include <windows.h>
#define OPEN_LIB(name)       LoadLibrary(name)
#define GET_FUNC(hndl, name) GetProcAddress(hndl, name)
#define CLOSE_LIB(hndl)      FreeLibrary(hndl)
#else
#include <dlfcn.h>
#define OPEN_LIB(name)       dlopen(name, RTLD_LAZY)
#define GET_FUNC(hndl, name) dlsym(hndl, name)
#define CLOSE_LIB(hndl)      dlclose(hndl)
#endif


namespace adevs {

/*
 * Load an FMI wrapped continuous system model for use in a
 * discrete event simulation. The FMI can then be attached
 * to any of the ODE solvers and event detectors in adevs
 * for simulation with the Hybrid class. This FMI loader
 * does not automatically extract model information from the
 * description XML, and so that information must be provided
 * explicitly by the end-user, but you probably need to know
 * this information regardless if you are using the FMI inside
 * of a larger discrete event simulation.
 */
template <typename OutputType>
class FMI : public ode_system<OutputType> {
  public:
    /*
     * This constructs a wrapper around an FMI. The constructor
     * must be provided with the FMI's GUID, the number of state variables,
     * number of event indicators, and the path to the .so file
     * that contains the FMI functions for this model.
     */
    FMI(char const* modelname, char const* guid, char const* resource_location,
        int num_state_variables, int num_event_indicators,
        char const* shared_lib_name, double const tolerance = 1E-8,
        int num_extra_event_indicators = 0, double start_time = 0.0,
        bool provides_jacobian = false);
    /// Destructor
    virtual ~FMI();
    /// Copy the initial state of the model to q
    virtual void init(double* q);
    /// Compute the derivative for state q and put it in dq
    virtual void der_func(double const* q, double* dq);
    /// Compute the state event functions for state q and put them in z
    virtual void state_event_func(double const* q, double* z);
    /// Compute the time event function using state q
    virtual double time_event_func(double const* q);
    /*
     * This method is invoked immediately following an update of the
     * continuous state variables and signal to the FMI the end
     * of an integration state.
     */
    virtual void postStep(double* q);
    /*
     * Like the postStep() method, but this is called at the end
     * of a trial step made by the numerical integrator. Trial steps
     * are used to locate state events and to estimate numerical
     * errors. The state vector q passed to this method may not be
     * the final vector assigned to the model at the end of
     * the current integration step. To get that value use
     * the postStep() method.
     */
    virtual void postTrialStep(double* q);
    /*
     * The internal transition function. This function will process all events
     * required by the FMI. Any derived class should call this method for the
     * parent class, then set or get any variables as appropriate, and then
     * call the base class method again to account for these changes.
     */
    virtual void internal_event(double* q, bool const* state_event);
    /*
     * The external transition See the notes on the internal_event function for
     * derived classes.
     */
    virtual void external_event(double* q, double e,
                                std::list<PinValue<OutputType>> const &xb);
    /*
     * The confluent transition function. See the notes on the internal_event function for
     * derived classes.
     */
    virtual void confluent_event(double* q, bool const* state_event,
                                 std::list<PinValue<OutputType>> const &xb);
    /*
     * The output function. This can read variables from the FMI, but should
     * not make any modifications to those variables.
     */
    virtual void output_func(double const* q, bool const* state_event,
                             std::list<PinValue<OutputType>> &yb);

    /// Get the current time
    double get_time() const { return t_now; }
    /// Get the value of a real variable
    double get_real(int k);
    /// Set the value of a real variable
    void set_real(int k, double val);
    /// Get the value of an integer variable
    int get_int(int k);
    /// Set the value of an integer variable
    void set_int(int k, int val);
    /// Get the value of a boolean variable
    bool get_bool(int k);
    /// Set the value of a boolean variable
    void set_bool(int k, bool val);
    /// Get the value of a string variable
    std::string get_string(int k);
    /// Set the value of a string variable
    void set_string(int k, std::string &val);
    /// Get the jacobian if this is supported by the FMI
    bool get_jacobian(double const* q, double* J);

  private:
    // Reference to the FMI
    fmi2Component c;
    // Pointer to the FMI interface
    fmi2Component (*_fmi2Instantiate)(fmi2String, fmi2Type, fmi2String,
                                      fmi2String, fmi2CallbackFunctions const*,
                                      fmi2Boolean, fmi2Boolean);
    void (*_fmi2FreeInstance)(fmi2Component);
    fmi2Status (*_fmi2SetupExperiment)(fmi2Component, fmi2Boolean, fmi2Real,
                                       fmi2Real, fmi2Boolean, fmi2Real);
    fmi2Status (*_fmi2EnterInitializationMode)(fmi2Component);
    fmi2Status (*_fmi2ExitInitializationMode)(fmi2Component);
    fmi2Status (*_fmi2GetReal)(fmi2Component, fmi2ValueReference const*, size_t,
                               fmi2Real*);
    fmi2Status (*_fmi2GetInteger)(fmi2Component, fmi2ValueReference const*,
                                  size_t, fmi2Integer*);
    fmi2Status (*_fmi2GetBoolean)(fmi2Component, fmi2ValueReference const*,
                                  size_t, fmi2Boolean*);
    fmi2Status (*_fmi2GetString)(fmi2Component, fmi2ValueReference const*,
                                 size_t, fmi2String*);
    fmi2Status (*_fmi2SetReal)(fmi2Component, fmi2ValueReference const*, size_t,
                               fmi2Real const*);
    fmi2Status (*_fmi2SetInteger)(fmi2Component, fmi2ValueReference const*,
                                  size_t, fmi2Integer const*);
    fmi2Status (*_fmi2SetBoolean)(fmi2Component, fmi2ValueReference const*,
                                  size_t, fmi2Boolean const*);
    fmi2Status (*_fmi2SetString)(fmi2Component, fmi2ValueReference const*,
                                 size_t, fmi2String const*);
    fmi2Status (*_fmi2EnterEventMode)(fmi2Component);
    fmi2Status (*_fmi2NewDiscreteStates)(fmi2Component, fmi2EventInfo*);
    fmi2Status (*_fmi2EnterContinuousTimeMode)(fmi2Component);
    fmi2Status (*_fmi2CompletedIntegratorStep)(fmi2Component, fmi2Boolean,
                                               fmi2Boolean*, fmi2Boolean*);
    fmi2Status (*_fmi2SetTime)(fmi2Component, fmi2Real);
    fmi2Status (*_fmi2SetContinuousStates)(fmi2Component, fmi2Real const*,
                                           size_t);
    fmi2Status (*_fmi2GetDerivatives)(fmi2Component, fmi2Real*, size_t);
    fmi2Status (*_fmi2GetEventIndicators)(fmi2Component, fmi2Real*, size_t);
    fmi2Status (*_fmi2GetContinuousStates)(fmi2Component, fmi2Real*, size_t);
    fmi2Status (*_fmi2GetDirectionalDerivative)(
        fmi2Component, fmi2ValueReference const*, size_t,
        fmi2ValueReference const*, size_t, fmi2Real const*, fmi2Real*);
    // Instant of the next time event
    double next_time_event;
    // Current time
    double t_now;
// so library handle
#ifdef _WIN32
    HINSTANCE so_hndl;
#else
    void* so_hndl;
#endif
    // Are we in continuous time mode?
    bool cont_time_mode;
    // Number of event indicators that are not governed by the FMI
    int num_extra_event_indicators;
    // Start time of the simulation
    double start_time;

    static void fmilogger(fmi2ComponentEnvironment componentEnvironment,
                          fmi2String instanceName, fmi2Status status,
                          fmi2String category, fmi2String message, ...) {
        if (message != NULL) {

            fprintf(stderr, message, "\n");
        }
    }

    fmi2CallbackFunctions* callbackFuncs;

    void iterate_events();
    /* Set to true if this FMI provides directional
          * directives for calculating a jacobian. The
          * default is false.
          */
    bool const provides_jacobian;
    /// Identifiers for variables used to calculate the
    /// jacobian column by column with a single FMI call
    /// per column
    fmi2ValueReference* jac_col;
    // FMI derivative gain term
    double* jac_v;
};


template <typename OutputType>
FMI<OutputType>::FMI(char const* modelname, char const* guid,
                     char const* resource_location, int num_state_variables,
                     int num_event_indicators, char const* so_file_name,
                     double const tolerance, int num_extra_event_indicators,
                     double start_time, bool provides_jacobian)
    :  // One extra variable at the end for time
      ode_system<OutputType>(num_state_variables + 1,
                             num_event_indicators + num_extra_event_indicators),
      next_time_event(adevs_inf<double>()),
      t_now(start_time),
      so_hndl(NULL),
      cont_time_mode(false),
      num_extra_event_indicators(num_extra_event_indicators),
      provides_jacobian(provides_jacobian),
      jac_col(NULL),
      jac_v(NULL) {
    // If we will get the jacobian, this holds everything except the time
    // derivative that has been appended to the FMI equation set
    if (provides_jacobian) {
        jac_col = new fmi2ValueReference[num_state_variables];
        jac_v = new double[num_state_variables];
        // References to each of our derivative functions f_1, f_2, etc.
        for (int i = 0; i < num_state_variables; i++) {
            jac_col[i] = num_state_variables + i;
            jac_v[i] = 1.0;
        }
    }
    // Get points to the FMI functions
    fmi2CallbackFunctions tmp = {adevs::FMI<OutputType>::fmilogger, calloc,
                                 free, NULL, NULL};
    callbackFuncs = new fmi2CallbackFunctions(tmp);
    so_hndl = OPEN_LIB(so_file_name);
    if (so_hndl == NULL) {
        throw adevs::exception("Could not load so file", this);
    }
    // This only works with a POSIX compliant compiler/system
    _fmi2Instantiate =
        (fmi2Component(*)(fmi2String, fmi2Type, fmi2String, fmi2String,
                          fmi2CallbackFunctions const*, fmi2Boolean,
                          fmi2Boolean))GET_FUNC(so_hndl, "fmi2Instantiate");
    assert(_fmi2Instantiate != NULL);
    _fmi2FreeInstance =
        (void (*)(fmi2Component))GET_FUNC(so_hndl, "fmi2FreeInstance");
    assert(_fmi2FreeInstance != NULL);
    _fmi2SetupExperiment = (fmi2Status(*)(
        fmi2Component, fmi2Boolean, fmi2Real, fmi2Real, fmi2Boolean,
        fmi2Real))GET_FUNC(so_hndl, "fmi2SetupExperiment");
    assert(_fmi2SetupExperiment != NULL);
    _fmi2EnterInitializationMode = (fmi2Status(*)(fmi2Component))GET_FUNC(
        so_hndl, "fmi2EnterInitializationMode");
    assert(_fmi2EnterInitializationMode != NULL);
    _fmi2ExitInitializationMode = (fmi2Status(*)(fmi2Component))GET_FUNC(
        so_hndl, "fmi2ExitInitializationMode");
    assert(_fmi2ExitInitializationMode != NULL);
    _fmi2GetReal =
        (fmi2Status(*)(fmi2Component, fmi2ValueReference const*, size_t,
                       fmi2Real*))GET_FUNC(so_hndl, "fmi2GetReal");
    assert(_fmi2GetReal != NULL);
    _fmi2GetInteger =
        (fmi2Status(*)(fmi2Component, fmi2ValueReference const*, size_t,
                       fmi2Integer*))GET_FUNC(so_hndl, "fmi2GetInteger");
    assert(_fmi2GetInteger != NULL);
    _fmi2GetBoolean =
        (fmi2Status(*)(fmi2Component, fmi2ValueReference const*, size_t,
                       fmi2Boolean*))GET_FUNC(so_hndl, "fmi2GetBoolean");
    assert(_fmi2GetBoolean != NULL);
    _fmi2GetString =
        (fmi2Status(*)(fmi2Component, fmi2ValueReference const*, size_t,
                       fmi2String*))GET_FUNC(so_hndl, "fmi2GetString");
    assert(_fmi2GetString != NULL);
    _fmi2SetReal =
        (fmi2Status(*)(fmi2Component, fmi2ValueReference const*, size_t,
                       fmi2Real const*))GET_FUNC(so_hndl, "fmi2SetReal");
    assert(_fmi2SetReal != NULL);
    _fmi2SetInteger =
        (fmi2Status(*)(fmi2Component, fmi2ValueReference const*, size_t,
                       fmi2Integer const*))GET_FUNC(so_hndl, "fmi2SetInteger");
    assert(_fmi2SetInteger != NULL);
    _fmi2SetBoolean =
        (fmi2Status(*)(fmi2Component, fmi2ValueReference const*, size_t,
                       fmi2Boolean const*))GET_FUNC(so_hndl, "fmi2SetBoolean");
    assert(_fmi2SetBoolean != NULL);
    _fmi2SetString =
        (fmi2Status(*)(fmi2Component, fmi2ValueReference const*, size_t,
                       fmi2String const*))GET_FUNC(so_hndl, "fmi2SetString");
    assert(_fmi2SetString != NULL);
    _fmi2EnterEventMode =
        (fmi2Status(*)(fmi2Component))GET_FUNC(so_hndl, "fmi2EnterEventMode");
    assert(_fmi2EnterEventMode != NULL);
    _fmi2NewDiscreteStates =
        (fmi2Status(*)(fmi2Component, fmi2EventInfo*))GET_FUNC(
            so_hndl, "fmi2NewDiscreteStates");
    assert(_fmi2NewDiscreteStates != NULL);
    _fmi2EnterContinuousTimeMode = (fmi2Status(*)(fmi2Component))GET_FUNC(
        so_hndl, "fmi2EnterContinuousTimeMode");
    assert(_fmi2EnterContinuousTimeMode != NULL);
    _fmi2CompletedIntegratorStep = (fmi2Status(*)(
        fmi2Component, fmi2Boolean, fmi2Boolean*,
        fmi2Boolean*))GET_FUNC(so_hndl, "fmi2CompletedIntegratorStep");
    assert(_fmi2CompletedIntegratorStep != NULL);
    _fmi2SetTime = (fmi2Status(*)(fmi2Component, fmi2Real))GET_FUNC(
        so_hndl, "fmi2SetTime");
    assert(_fmi2SetTime != NULL);
    _fmi2SetContinuousStates =
        (fmi2Status(*)(fmi2Component, fmi2Real const*, size_t))GET_FUNC(
            so_hndl, "fmi2SetContinuousStates");
    assert(_fmi2SetContinuousStates != NULL);
    _fmi2GetDerivatives =
        (fmi2Status(*)(fmi2Component, fmi2Real*, size_t))GET_FUNC(
            so_hndl, "fmi2GetDerivatives");
    assert(_fmi2GetDerivatives != NULL);
    _fmi2GetEventIndicators =
        (fmi2Status(*)(fmi2Component, fmi2Real*, size_t))GET_FUNC(
            so_hndl, "fmi2GetEventIndicators");
    assert(_fmi2GetEventIndicators != NULL);
    _fmi2GetContinuousStates =
        (fmi2Status(*)(fmi2Component, fmi2Real*, size_t))GET_FUNC(
            so_hndl, "fmi2GetContinuousStates");
    assert(_fmi2GetContinuousStates != NULL);
    _fmi2GetContinuousStates =
        (fmi2Status(*)(fmi2Component, fmi2Real*, size_t))GET_FUNC(
            so_hndl, "fmi2GetContinuousStates");
    assert(_fmi2GetContinuousStates != NULL);
    // If this is NULL then the function is not supported
    _fmi2GetDirectionalDerivative = (fmi2Status(*)(
        fmi2Component, fmi2ValueReference const*, size_t,
        fmi2ValueReference const*, size_t, fmi2Real const*,
        fmi2Real*))GET_FUNC(so_hndl, "fmi2GetDirectionalDerivative");
    // Create the FMI component
    c = _fmi2Instantiate(modelname, fmi2ModelExchange, guid, resource_location,
                         callbackFuncs, fmi2False, fmi2False);
    assert(c != NULL);
    _fmi2SetupExperiment(c, fmi2True, tolerance, -1.0, fmi2False, -1.0);
}

template <typename OutputType>
bool FMI<OutputType>::get_jacobian(double const* q, double* J) {
    fmi2Status status;
    // Number of FMI variables. Does not include time.
    unsigned const N = this->numVars() - 1;
    // If we just checking for support then exit immediately
    if (!provides_jacobian || J == NULL) {
        return provides_jacobian;
    }
    // Enter continuous time mode to calculate Jacobian entries
    if (!cont_time_mode) {
        status = _fmi2EnterContinuousTimeMode(c);
        assert(status == fmi2OK);
        cont_time_mode = true;
    }
    // Set state variables to their current values
    status = _fmi2SetTime(c, q[N]);
    assert(status == fmi2OK);
    status = _fmi2SetContinuousStates(c, q, N);
    assert(status == fmi2OK);
    // Zero out J
    for (unsigned i = 0; i < (N + 1) * (N + 1); i++) {
        J[i] = 0.0;
    }
    // Calculate each entry of the Jacobian matrix. OpenModelica
    // appears to use 0..N-1 for the state variable reference
    // numbers and N..2N-1 for the corresponding derivative numbers.
    // For the moment, we assume this is the case.
    for (fmi2ValueReference state_var = 0; state_var < N; state_var++) {
        status = _fmi2GetDirectionalDerivative(c, jac_col, N, &state_var, 1,
                                               jac_v, J + state_var * N);
        assert(status == fmi2OK);
    }
    // Partial of time is 1 and nobody else depends on it. This, of course, may
    // be wrong but FMI does not appear to support partials of f with respect to
    // time. If you really need this, add dtau/dt = 1 to your equation set and use
    // tau as your time variable.
    J[(N + 1) * (N + 1) - 1] = 1.0;
    return true;
}

template <typename OutputType>
void FMI<OutputType>::iterate_events() {
    fmi2Status status;
    // Put into consistent initial state
    fmi2EventInfo eventInfo;
    do {
        status = _fmi2NewDiscreteStates(c, &eventInfo);
        assert(status == fmi2OK);
    } while (eventInfo.newDiscreteStatesNeeded == fmi2True);
    if (eventInfo.nextEventTimeDefined == fmi2True) {
        next_time_event = eventInfo.nextEventTime;
    } else {
        next_time_event = adevs_inf<double>();
    }
    assert(status == fmi2OK);
}

template <typename OutputType>
void FMI<OutputType>::init(double* q) {
    fmi2Status status;
    // Initialize all variables
    status = _fmi2EnterInitializationMode(c);
    assert(status == fmi2OK);
    // Done with initialization
    status = _fmi2ExitInitializationMode(c);
    assert(status == fmi2OK);
    // Put into consistent initial state
    iterate_events();
    // Enter continuous time mode to start integration
    status = _fmi2EnterContinuousTimeMode(c);
    assert(status == fmi2OK);
    // Set initial value for time
    status = _fmi2SetTime(c, t_now);
    assert(status == fmi2OK);
    // Get starting state variable values
    status = _fmi2GetContinuousStates(c, q, this->numVars() - 1);
    assert(status == fmi2OK);
    q[this->numVars() - 1] = t_now;
    cont_time_mode = true;
}

template <typename OutputType>
void FMI<OutputType>::der_func(double const* q, double* dq) {
    fmi2Status status;
    if (!cont_time_mode) {
        status = _fmi2EnterContinuousTimeMode(c);
        assert(status == fmi2OK);
        cont_time_mode = true;
    }
    status = _fmi2SetTime(c, q[this->numVars() - 1]);
    assert(status == fmi2OK);
    status = _fmi2SetContinuousStates(c, q, this->numVars() - 1);
    assert(status == fmi2OK);
    status = _fmi2GetDerivatives(c, dq, this->numVars() - 1);
    assert(status == fmi2OK);
    dq[this->numVars() - 1] = 1.0;
}

template <typename OutputType>
void FMI<OutputType>::state_event_func(double const* q, double* z) {
    fmi2Status status;
    if (!cont_time_mode) {
        status = _fmi2EnterContinuousTimeMode(c);
        assert(status == fmi2OK);
        cont_time_mode = true;
    }
    status = _fmi2SetTime(c, q[this->numVars() - 1]);
    assert(status == fmi2OK);
    status = _fmi2SetContinuousStates(c, q, this->numVars() - 1);
    assert(status == fmi2OK);
    status = _fmi2GetEventIndicators(
        c, z, this->numEvents() - num_extra_event_indicators);
    assert(status == fmi2OK);
}

template <typename OutputType>
double FMI<OutputType>::time_event_func(double const* q) {
    return next_time_event - q[this->numVars() - 1];
}

template <typename OutputType>
void FMI<OutputType>::postStep(double* q) {
    assert(cont_time_mode);
    // Don't advance the FMI state by zero units of time
    // when in continuous mode
    if (q[this->numVars() - 1] <= t_now) {
        return;
    }
    // Try to complete the integration step
    fmi2Status status;
    fmi2Boolean enterEventMode;
    fmi2Boolean terminateSimulation;
    t_now = q[this->numVars() - 1];
    status = _fmi2SetTime(c, t_now);
    assert(status == fmi2OK);
    status = _fmi2SetContinuousStates(c, q, this->numVars() - 1);
    assert(status == fmi2OK);
    status = _fmi2CompletedIntegratorStep(c, fmi2True, &enterEventMode,
                                          &terminateSimulation);
    assert(status == fmi2OK);
    // Force an event if one is indicated
    if (enterEventMode == fmi2True) {
        next_time_event = t_now;
    }
}

template <typename OutputType>
void FMI<OutputType>::postTrialStep(double* q) {
    assert(cont_time_mode);
    // Restore values changed by der_func and state_event_func
    fmi2Status status;
    status = _fmi2SetTime(c, q[this->numVars() - 1]);
    assert(status == fmi2OK);
    status = _fmi2SetContinuousStates(c, q, this->numVars() - 1);
    assert(status == fmi2OK);
}

template <typename OutputType>
void FMI<OutputType>::internal_event(double* q, bool const* state_event) {
    fmi2Status status;
    // postStep will have updated the continuous variables, so
    // we just process discrete events here.
    if (cont_time_mode) {
        status = _fmi2EnterEventMode(c);
        assert(status == fmi2OK);
        cont_time_mode = false;
    }
    // Process events
    iterate_events();
    // Update the state variable array
    status = _fmi2GetContinuousStates(c, q, this->numVars() - 1);
    assert(status == fmi2OK);
}

template <typename OutputType>
void FMI<OutputType>::external_event(double* q, double e,
                                     std::list<PinValue<OutputType>> const &xb) {
    fmi2Status status;
    // Go to event mode if we have not yet done so
    if (cont_time_mode) {
        status = _fmi2EnterEventMode(c);
        assert(status == fmi2OK);
        cont_time_mode = false;
    }
    // process any events that need processing
    iterate_events();
    status = _fmi2GetContinuousStates(c, q, this->numVars() - 1);
    assert(status == fmi2OK);
}

template <typename OutputType>
void FMI<OutputType>::confluent_event(double* q, bool const* state_event,
                                      std::list<PinValue<OutputType>> const &xb) {
    fmi2Status status;
    // postStep will have updated the continuous variables, so
    // we just process discrete events here.
    if (cont_time_mode) {
        status = _fmi2EnterEventMode(c);
        assert(status == fmi2OK);
        cont_time_mode = false;
    }
    iterate_events();
    status = _fmi2GetContinuousStates(c, q, this->numVars() - 1);
    assert(status == fmi2OK);
}

template <typename OutputType>
void FMI<OutputType>::output_func(double const* q, bool const* state_event,
                                  std::list<PinValue<OutputType>> &yb) {}

template <typename OutputType>
FMI<OutputType>::~FMI() {
    _fmi2FreeInstance(c);
    delete callbackFuncs;
    if (jac_col != NULL) {
        delete[] jac_col;
        delete[] jac_v;
    }
    CLOSE_LIB(so_hndl);
}

template <typename OutputType>
double FMI<OutputType>::get_real(int k) {
    fmi2ValueReference const ref = k;
    fmi2Real val;
    fmi2Status status = _fmi2GetReal(c, &ref, 1, &val);
    assert(status == fmi2OK);
    return val;
}

template <typename OutputType>
void FMI<OutputType>::set_real(int k, double val) {
    fmi2ValueReference const ref = k;
    fmi2Real fmi_val = val;
    fmi2Status status = _fmi2SetReal(c, &ref, 1, &fmi_val);
    assert(status == fmi2OK);
}

template <typename OutputType>
int FMI<OutputType>::get_int(int k) {
    fmi2ValueReference const ref = k;
    fmi2Integer val;
    fmi2Status status = _fmi2GetInteger(c, &ref, 1, &val);
    assert(status == fmi2OK);
    return val;
}

template <typename OutputType>
void FMI<OutputType>::set_int(int k, int val) {
    fmi2ValueReference const ref = k;
    fmi2Integer fmi_val = val;
    fmi2Status status = _fmi2SetInteger(c, &ref, 1, &fmi_val);
    assert(status == fmi2OK);
}

template <typename OutputType>
bool FMI<OutputType>::get_bool(int k) {
    fmi2ValueReference const ref = k;
    fmi2Boolean val;
    fmi2Status status = _fmi2GetBoolean(c, &ref, 1, &val);
    assert(status == fmi2OK);
    return (val == fmi2True);
}

template <typename OutputType>
void FMI<OutputType>::set_bool(int k, bool val) {
    fmi2ValueReference const ref = k;
    fmi2Boolean fmi_val = fmi2False;
    if (val) {
        fmi_val = fmi2True;
    }
    fmi2Status status = _fmi2SetBoolean(c, &ref, 1, &fmi_val);
    assert(status == fmi2OK);
}

template <typename OutputType>
std::string FMI<OutputType>::get_string(int k) {
    fmi2ValueReference const ref = k;
    fmi2String val;
    fmi2Status status = _fmi2GetString(c, &ref, 1, &val);
    assert(status == fmi2OK);
    return val;
}

template <typename OutputType>
void FMI<OutputType>::set_string(int k, std::string &val) {
    fmi2ValueReference const ref = k;
    fmi2String fmi_val = fmi2False;
    fmi2Status status = _fmi2SetString(c, &ref, 1, &fmi_val);
    assert(status == fmi2OK);
}

}  // namespace adevs

#endif
