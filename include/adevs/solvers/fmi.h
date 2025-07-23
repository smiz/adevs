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

#include <filesystem>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "adevs/solvers/hybrid.h"
#include "fmilib.h"

namespace adevs {

/**
 * @brief Wrap an FMI for Model Exchange object in and ode_system
 *  
 * Load an FMU wrapped continuous system model for use in a
 * discrete event simulation. The FMU can then be attached
 * to any of the ODE solvers and event detectors in adevs
 * for simulation with the Hybrid class. You need to have the
 * <a href="https://github.com/modelon-community/fmi-library">
 * FMI Library</a> installed to used this class. At this time,
 * the ModelExchange class supports FMI 2.0
 * 
 * @see ode_system
 */
template <typename ValueType = std::any>
class ModelExchange : public ode_system<ValueType> {
  public:
    /**
     * @brief Load and fmu for use in a simulation
     * 
     * This loads the FMU from a .fmu file.
     * 
     * @param fmu Path to the fmu file
     * @param tolerance Tolerance for the algebraic loop solvers
     * inside the FMU
     */
    ModelExchange(const char* const fmu, double tolerance);
    /// @brief Destructor
    virtual ~ModelExchange();
    /// @brief Copy the initial state of the model to q
    ///
    /// Do no overload
    virtual void init(double* q);
    /// @brief Computes the derivative for state q and put it in dq
    ///
    /// Do no overload
    virtual void der_func(double const* q, double* dq);
    /// @compute Compute the state event functions for state q and put them in z
    ///
    /// Do no overload
    virtual void state_event_func(double const* q, double* z);
    /// @brief Compute the time event function using state q
    ///
    /// Do no overload
    virtual double time_event_func(double const* q);
    /** 
     * @brief This method is invoked immediately following an update of the
     * continuous state variables.
     * 
     * Signal the FMU the end to end an integration step. Do not
     * overload.
     */
    virtual void postStep(double* q);
    /**
     * @brief Like the postStep() method, but this is called at the end
     * of a trial step made by the numerical integrator.
     * 
     * Do not overload. Trial steps
     * are used to locate state events and to estimate numerical
     * errors. The state vector q passed to this method may not be
     * the final vector assigned to the model at the end of
     * the current integration step. To get that value use
     * the postStep() method.
     */
    virtual void postTrialStep(double* q);
    /**
     * @brief The internal transition function
     *
     * This function will process all events
     * required by the FMU. Any derived class should call this method for the
     * parent class, then set or get any variables as appropriate, and then
     * call the base class method again to account for these changes. If you
     * overload the internal_event() method you must call the parent method
     * at the start of your method and call it again if you use the set_variable()
     * method.
     */
    virtual void internal_event(double* q, bool const* state_event);
    /**
     * @brief The external transition
     * 
     * See the notes on the internal_event function for
     * derived classes.
     */
    virtual void external_event(double* q, double e,
                                std::list<PinValue<ValueType>> const &xb);
    /**
     * @brief The confluent transition function.
     * 
     * See the notes on the internal_event function for
     * derived classes.
     */
    virtual void confluent_event(double* q, bool const* state_event,
                                 std::list<PinValue<ValueType>> const &xb);
    /**
     * @brief The output function.
     * 
     * This can read variables from the FMU, but should
     * not make any modifications to those variables.
     */
    virtual void output_func(double const* q, bool const* state_event,
                             std::list<PinValue<ValueType>> &yb);

    /// @brief Get the current time
    ///
    /// @return The simulation time at the last instant this object
    /// had an integration step of an event
    double get_time() const { return t_now; }
    /// @brief Get the value of a variable
    ///
    /// The argument must be the variable name as it appears in the
    /// xml file that comes with the FMU. You can unzip the fmu file
    /// and look in the modelDescription.xml to get this information.
    ///
    /// @param name The name of the variable whose value will be returned
    std::any get_variable(std::string name);
    /// @brief Set the value of a variable
    ///
    /// See get_variable()
    /// @param name The name of the variable whose value will be returned
    /// @param value The value to assign
    void set_variable(std::string name, std::any value);
    /// @brief Get the jacobian if this is supported by the FMU
    ///
    /// Do not overload
    bool get_jacobian(double const* q, double* J);

  private:
    // Reference to the FMU
    fmi2_import_t* fmu;
    // Instant of the next time event
    double next_time_event;
    // Current time
    double t_now;
    // Are we in continuous time mode?
    bool cont_time_mode;

    void iterate_events();
    /// Identifiers for variables used to calculate the
    /// jacobian column by column with a single FMU call
    /// per column
    fmi2_value_reference_t* jac_col;
    // FMU derivative gain term
    double* jac_v;
    // FMI library callbacks
    jm_callbacks callbacks;

    struct variable_info_t {
        fmi2_value_reference_t ref;
        fmi2_base_type_enu_t var_type;
    };
    std::map<std::string,variable_info_t> name_to_variable_map;
};

template <typename ValueType>
std::any ModelExchange<ValueType>::get_variable(std::string name) {
    fmi2_status_t status;
    variable_info_t var_info = name_to_variable_map[name];
    if (var_info.var_type == fmi2_base_type_real) {
        fmi2_real_t val;
        status = fmi2_import_get_real(fmu,&var_info.ref,1,&val);
        assert(status == fmi2_status_ok);
        return val;
    }
    if (var_info.var_type == fmi2_base_type_int) {
        fmi2_integer_t val;
        status = fmi2_import_get_integer(fmu,&var_info.ref,1,&val);
        assert(status == fmi2_status_ok);
        return val;
    }
    if (var_info.var_type == fmi2_base_type_bool) {
        fmi2_boolean_t val;
        status = fmi2_import_get_boolean(fmu,&var_info.ref,1,&val);
        assert(status == fmi2_status_ok);
        return val;
    }
    return 0; 
}

template <typename ValueType>
void ModelExchange<ValueType>::set_variable(std::string name, std::any value) {
    fmi2_status_t status;
    variable_info_t var_info = name_to_variable_map[name];
    if (var_info.var_type == fmi2_base_type_real) {
        fmi2_real_t val = std::any_cast<fmi2_real_t>(value);
        status = fmi2_import_set_real(fmu,&var_info.ref,1,&val);
        assert(status == fmi2_status_ok);
        return;
    }
    if (var_info.var_type == fmi2_base_type_int) {
        fmi2_integer_t val = std::any_cast<fmi2_integer_t>(value);
        status = fmi2_import_set_integer(fmu,&var_info.ref,1,&val);
        assert(status == fmi2_status_ok);
        return;
    }
    if (var_info.var_type == fmi2_base_type_bool) {
        fmi2_boolean_t val= std::any_cast<fmi2_boolean_t>(value);
        status = fmi2_import_set_boolean(fmu,&var_info.ref,1,&val);
        assert(status == fmi2_status_ok);
    }
}

template <typename ValueType>
ModelExchange<ValueType>::ModelExchange(const char* const fmu_file, double tolerance) :
        // One extra variable at the end for time
        ode_system<ValueType>(),
        next_time_event(adevs_inf<double>()),
        t_now(0.0),
        cont_time_mode(false),
        jac_col(nullptr),
        jac_v(nullptr) {

    jm_status_enu_t status_enum;

    callbacks.malloc = malloc;
    callbacks.calloc = calloc;
    callbacks.realloc = realloc;
    callbacks.free = free;
    callbacks.logger = jm_default_logger;
    callbacks.log_level = jm_log_level_fatal;
    callbacks.context = 0;

    char* tmpPath = fmi_import_mk_temp_dir(&callbacks,std::filesystem::temp_directory_path().c_str(),nullptr);
    assert(tmpPath);
  
    fmi_import_context_t* context = fmi_import_allocate_context(&callbacks);
    fmi_version_enu_t version = fmi_import_get_fmi_version(context, fmu_file, tmpPath);
  
    if(version != fmi_version_2_0_enu) {
        throw adevs::exception("Only version FMI 2.0 is supported");
    }
  
    fmu = fmi2_import_parse_xml(context,tmpPath,0);
  
    if(!fmu) {
        throw adevs::exception("Error parsing FMI XML");
    }    
  
    if(fmi2_import_get_fmu_kind(fmu) == fmi2_fmu_kind_cs) {
        throw adevs::exception("Only ME 2.0 is supported\n");
    }

    // Load the dll
    status_enum = fmi2_import_create_dllfmu(fmu, fmi2_fmu_kind_me, nullptr);
    assert(status_enum != jm_status_error);
    status_enum = fmi2_import_instantiate(fmu,"adevs::ModelExchange",fmi2_model_exchange,nullptr,false);
    assert(status_enum != jm_status_error);

    // Done with the context and temporary directory
    fmi_import_free_context(context);
    fmi_import_rmdir(nullptr,tmpPath);
    free(tmpPath);

    this->set_num_state_variables(fmi2_import_get_number_of_continuous_states(fmu)+1);
    this->set_num_event_functions(fmi2_import_get_number_of_event_indicators(fmu));

    /// Load the variable list
    fmi2_import_variable_list_t* variable_list = fmi2_import_get_variable_list(fmu,0);
    size_t num_variables = fmi2_import_get_variable_list_size(variable_list);
    for (size_t i = 0; i < num_variables; i++) {
        fmi2_import_variable_t* var = fmi2_import_get_variable(variable_list,i);
        std::string name = fmi2_import_get_variable_name(var);
        variable_info_t info;
        info.ref = fmi2_import_get_variable_vr(var);
        info.var_type = fmi2_import_get_variable_base_type(var);
        name_to_variable_map[name] = info;
    }
    fmi2_import_free_variable_list(variable_list);
    /// Setup the FMU for simulation
    fmi2_import_set_debug_logging(fmu,false,0,nullptr);
    fmi2_import_setup_experiment(fmu, true, tolerance, 0.0, false, -1.0);
    /// Prep for jacobian calculations
    if (fmi2_import_get_capability(fmu,fmi2_me_providesDirectionalDerivatives)) {
        jac_col = new fmi2_value_reference_t[this->numVars()-1];
        jac_v = new double[this->numVars()-1];
        // References to each of our derivative functions f_1, f_2, etc.
        for (int i = 0; i < this->numVars()-1; i++) {
            jac_col[i] = this->numVars()-1 + i;
            jac_v[i] = 1.0;
        }
    }
}

template <typename ValueType>
bool ModelExchange<ValueType>::get_jacobian(double const* q, double* J) {
    fmi2_status_t status;
    // Number of FMU variables. Does not include time.
    unsigned const N = this->numVars()-1;
    // Do we support Jacobians?
    if (J == nullptr || jac_col == nullptr) {
        return jac_col != nullptr;
    }
    // Enter continuous time mode to calculate Jacobian entries
    if (!cont_time_mode) {
        status = fmi2_import_enter_continuous_time_mode(fmu);
        assert(status == fmi2_status_ok);
        cont_time_mode = true;
    }
    // Set state variables to their current values
    status = fmi2_import_set_time(fmu, q[N]);
    assert(status == fmi2_status_ok);
    status = fmi2_import_set_continuous_states(fmu, q, N);
    assert(status == fmi2_status_ok);
    // Zero out J
    for (unsigned i = 0; i < (N + 1) * (N + 1); i++) {
        J[i] = 0.0;
    }
    // Calculate each entry of the Jacobian matrix. OpenModelica
    // appears to use 0..N-1 for the state variable reference
    // numbers and N..2N-1 for the corresponding derivative numbers.
    // For the moment, we assume this is the case.
    // BEWARE!!!! fmi2_import_get_directional_derivative swaps the
    // reference lists under the hood. In a direct call to the FMI
    // API state_var and jac_col would be in reversed positions.
    for (fmi2_value_reference_t state_var = 0; state_var < N; state_var++) {
        status = fmi2_import_get_directional_derivative(fmu, &state_var, 1, jac_col, N, 
                                               jac_v, J + state_var * N);
        assert(status == fmi2_status_ok);
    }
    // Partial of time is 1 and nobody else depends on it. This, of course, may
    // be wrong but FMU does not appear to support partials of f with respect to
    // time. If you really need this, add dtau/dt = 1 to your equation set and use
    // tau as your time variable.
    J[(N + 1) * (N + 1) - 1] = 1.0;
    return true;
}

template <typename ValueType>
void ModelExchange<ValueType>::iterate_events() {
    fmi2_status_t status;
    // Put into consistent initial state
    fmi2_event_info_t eventInfo;
    // For some reason, the call to new_discrete_states() does
    // not always set these if an event does not happen. We
    // initialize them here to avoid a false alarm.
    eventInfo.newDiscreteStatesNeeded = false;
    eventInfo.nextEventTimeDefined = false;
    do {
        status = fmi2_import_new_discrete_states(fmu,&eventInfo);
        assert(status == fmi2_status_ok);
    } while (eventInfo.newDiscreteStatesNeeded);
    if (eventInfo.nextEventTimeDefined) {
        next_time_event = eventInfo.nextEventTime;
    } else {
        next_time_event = adevs_inf<double>();
    }
    assert(status == fmi2_status_ok);
}

template <typename ValueType>
void ModelExchange<ValueType>::init(double* q) {
    fmi2_status_t status;
    // Initialize all variables
    status = fmi2_import_enter_initialization_mode(fmu);
    assert(status == fmi2_status_ok);
    // Done with initialization
    status = fmi2_import_exit_initialization_mode(fmu);
    assert(status == fmi2_status_ok);
    // Put into consistent initial state
    iterate_events();
    // Enter continuous time mode to start integration
    status = fmi2_import_enter_continuous_time_mode(fmu);
    assert(status == fmi2_status_ok);
    // Set initial value for time
    status = fmi2_import_set_time(fmu,t_now);
    assert(status == fmi2_status_ok);
    // Get starting state variable values
    status = fmi2_import_get_continuous_states(fmu,q,this->numVars()-1);
    assert(status == fmi2_status_ok);
    q[this->numVars()-1] = t_now;
    cont_time_mode = true;
}

template <typename ValueType>
void ModelExchange<ValueType>::der_func(double const* q, double* dq) {
    fmi2_status_t status;
    if (!cont_time_mode) {
        status = fmi2_import_enter_continuous_time_mode(fmu);
        assert(status == fmi2_status_ok);
        cont_time_mode = true;
    }
    status = fmi2_import_set_time(fmu,q[this->numVars()-1]);
    assert(status == fmi2_status_ok);
    status = fmi2_import_set_continuous_states(fmu,q,this->numVars()-1);
    assert(status == fmi2_status_ok);
    status = fmi2_import_get_derivatives(fmu,dq,this->numVars()-1);
    assert(status == fmi2_status_ok);
    dq[this->numVars()-1] = 1.0;
}

template <typename ValueType>
void ModelExchange<ValueType>::state_event_func(double const* q, double* z) {
    fmi2_status_t status;
    // Don't do anything if we don't have any state events
    if (this->numEvents() == 0) return;
    if (!cont_time_mode) {
        status = fmi2_import_enter_continuous_time_mode(fmu);
        assert(status == fmi2_status_ok);
        cont_time_mode = true;
    }
    status = fmi2_import_set_time(fmu,q[this->numVars()-1]);
    assert(status == fmi2_status_ok);
    status = fmi2_import_set_continuous_states(fmu,q,this->numVars()-1);
    assert(status == fmi2_status_ok);
    status = fmi2_import_get_event_indicators(fmu,z,this->numEvents());
    assert(status == fmi2_status_ok);
}

template <typename ValueType>
double ModelExchange<ValueType>::time_event_func(double const* q) {
    if (next_time_event < adevs_inf<double>()) {
        return next_time_event - q[this->numVars()-1];
    }
    return adevs_inf<double>();
}

template <typename ValueType>
void ModelExchange<ValueType>::postStep(double* q) {
    assert(cont_time_mode);
    // Don't advance the FMU state by zero units of time
    // when in continuous mode
    if (q[this->numVars()-1] <= t_now) {
        return;
    }
    // Try to complete the integration step
    fmi2_status_t status;
    fmi2_boolean_t enterEventMode;
    fmi2_boolean_t terminateSimulation;
    t_now = q[this->numVars() - 1];
    status = fmi2_import_set_time(fmu,t_now);
    assert(status == fmi2_status_ok);
    status = fmi2_import_set_continuous_states(fmu,q,this->numVars()-1);
    assert(status == fmi2_status_ok);
    status = fmi2_import_completed_integrator_step(
        fmu,true, &enterEventMode,&terminateSimulation);
    assert(status == fmi2_status_ok);
    // Force an event if one is indicated
    if (enterEventMode) {
        next_time_event = t_now;
    }
}

template <typename ValueType>
void ModelExchange<ValueType>::postTrialStep(double* q) {
    assert(cont_time_mode);
    // Restore values changed by der_func and state_event_func
    fmi2_status_t status;
    status = fmi2_import_set_time(fmu,q[this->numVars()-1]);
    assert(status == fmi2_status_ok);
    status = fmi2_import_set_continuous_states(fmu,q,this->numVars()-1);
    assert(status == fmi2_status_ok);
}

template <typename ValueType>
void ModelExchange<ValueType>::internal_event(double* q, bool const* state_event) {
    fmi2_status_t status;
    // postStep will have updated the continuous variables, so
    // we just process discrete events here.
    if (cont_time_mode) {
        status = fmi2_import_enter_event_mode(fmu);
        assert(status == fmi2_status_ok);
        cont_time_mode = false;
    }
    // Process events
    iterate_events();
    // Update the state variable array
    status = fmi2_import_get_continuous_states(fmu,q,this->numVars()-1);
    assert(status == fmi2_status_ok);
}

template <typename ValueType>
void ModelExchange<ValueType>::external_event(double* q, double e,
                                     std::list<PinValue<ValueType>> const &xb) {
    fmi2_status_t status;
    // Go to event mode if we have not yet done so
    if (cont_time_mode) {
        status = fmi2_import_enter_event_mode(fmu);
        assert(status == fmi2_status_ok);
        cont_time_mode = false;
    }
    // process any events that need processing
    iterate_events();
    status = fmi2_import_get_continuous_states(fmu,q,this->numVars()-1);
    assert(status == fmi2_status_ok);
}

template <typename ValueType>
void ModelExchange<ValueType>::confluent_event(double* q, bool const* state_event,
                                      std::list<PinValue<ValueType>> const &xb) {
    fmi2_status_t status;
    // postStep will have updated the continuous variables, so
    // we just process discrete events here.
    if (cont_time_mode) {
        status = fmi2_import_enter_event_mode(fmu);
        assert(status == fmi2_status_ok);
        cont_time_mode = false;
    }
    iterate_events();
    status = fmi2_import_get_continuous_states(fmu,q,this->numVars()-1);
    assert(status == fmi2_status_ok);
}

template <typename ValueType>
void ModelExchange<ValueType>::output_func(double const* q, bool const* state_event,
                                  std::list<PinValue<ValueType>> &yb) {}

template <typename ValueType>
ModelExchange<ValueType>::~ModelExchange() {
    fmi2_import_terminate(fmu);
    fmi2_import_free_instance(fmu);
    fmi2_import_destroy_dllfmu(fmu);
    fmi2_import_free(fmu);
    if (jac_col != nullptr) {
        delete [] jac_col;
        delete [] jac_v;
    }
}

}  // namespace adevs

#endif
