/**
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
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include "adevs_hybrid.h"
#include "fmi2Functions.h"
#include "fmi2FunctionTypes.h"
#include "fmi2TypesPlatform.h"

// Functions for loading DLL and so files
#ifdef _WIN32
#include <windows.h>
#define OPEN_LIB(name) LoadLibrary(name)
#define GET_FUNC(hndl,name) GetProcAddress(hndl,name)
#define CLOSE_LIB(hndl) FreeLibrary(hndl)
#else
#include <dlfcn.h>
#define OPEN_LIB(name) dlopen(name,RTLD_LAZY)
#define GET_FUNC(hndl,name) dlsym(hndl,name)
#define CLOSE_LIB(hndl) dlclose(hndl)
#endif

namespace adevs
{

/**
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
template <typename X> class FMI:
	public ode_system<X>
{
	public:
		/**
		 * This constructs a wrapper around an FMI. The constructor
		 * must be provided with the FMI's GUID, the number of state variables,
		 * number of event indicators, and the path to the .so file
		 * that contains the FMI functions for this model.
		 */
		FMI(const char* modelname,
			const char* guid,
			const char* resource_location,
			int num_state_variables,
			int num_event_indicators,
			const char* shared_lib_name,
			const double tolerance = 1E-8,
			int num_extra_event_indicators = 0,
			double start_time = 0.0);
		/// Copy the initial state of the model to q
		virtual void init(double* q);
		/// Compute the derivative for state q and put it in dq
		virtual void der_func(const double* q, double* dq);
		/// Compute the state event functions for state q and put them in z
		virtual void state_event_func(const double* q, double* z);
		/// Compute the time event function using state q
		virtual double time_event_func(const double* q);
		/**
		 * This method is invoked immediately following an update of the
		 * continuous state variables and signal to the FMI the end
		 * of an integration state.
		 */
		virtual void postStep(double* q);
		/**
		 * Like the postStep() method, but this is called at the end
		 * of a trial step made by the numerical integrator. Trial steps
		 * are used to locate state events and to estimate numerical
		 * errors. The state vector q passed to this method may not be
		 * the final vector assigned to the model at the end of
		 * the current integration step. The get that value use
		 * the postStep() method.
		 */
		virtual void postTrialStep(double* q);
		/**
		 * The internal transition function. This function will process all events
		 * required by the FMI. Any derived class should call this method for the
		 * parent class, then set or get any variables as appropriate, and then
		 * call the base class method again to account for these changes. 
		 */
		virtual void internal_event(double* q,
				const bool* state_event);
		/**
		 * The external transition See the notes on the internal_event function for
		 * derived classes.
		 */
		virtual void external_event(double* q, double e,
				const Bag<X>& xb);
		/**
		 * The confluent transition function. See the notes on the internal_event function for
		 * derived classes.
		 */
		virtual void confluent_event(double *q, const bool* state_event,
				const Bag<X>& xb);
		/**
		 * The output function. This can read variables for the FMI, but should
		 * not make any modifications to those variables.
		 */
		virtual void output_func(const double *q, const bool* state_event,
				Bag<X>& yb);
		/**
		 * Garbage collection function. This works just like the Atomic gc_output method.
		 * The default implementation does nothing.
		 */
		virtual void gc_output(Bag<X>& gb);
		/// Destructor
		virtual ~FMI();
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
		void set_string(int k, std::string& val);
	private:
		// Reference to the FMI
		fmi2Component c;
		// Pointer to the FMI interface
		fmi2Component (*_fmi2Instantiate)(fmi2String, fmi2Type,
				fmi2String, fmi2String, const fmi2CallbackFunctions*,
				fmi2Boolean, fmi2Boolean);
		void (*_fmi2FreeInstance)(fmi2Component);
		fmi2Status (*_fmi2SetupExperiment)(fmi2Component, fmi2Boolean,
				fmi2Real, fmi2Real, fmi2Boolean, fmi2Real);
		fmi2Status (*_fmi2EnterInitializationMode)(fmi2Component);
		fmi2Status (*_fmi2ExitInitializationMode)(fmi2Component);
		fmi2Status (*_fmi2GetReal)(fmi2Component, const fmi2ValueReference*, size_t, fmi2Real*);
		fmi2Status (*_fmi2GetInteger)(fmi2Component, const fmi2ValueReference*, size_t, fmi2Integer*);
		fmi2Status (*_fmi2GetBoolean)(fmi2Component, const fmi2ValueReference*, size_t, fmi2Boolean*);
		fmi2Status (*_fmi2GetString)(fmi2Component, const fmi2ValueReference*, size_t, fmi2String*);
		fmi2Status (*_fmi2SetReal)(fmi2Component, const fmi2ValueReference*, size_t, const fmi2Real*);
		fmi2Status (*_fmi2SetInteger)(fmi2Component, const fmi2ValueReference*, size_t, const fmi2Integer*);
		fmi2Status (*_fmi2SetBoolean)(fmi2Component, const fmi2ValueReference*, size_t, const fmi2Boolean*);
		fmi2Status (*_fmi2SetString)(fmi2Component, const fmi2ValueReference*, size_t, const fmi2String*);
		fmi2Status (*_fmi2EnterEventMode)(fmi2Component);
		fmi2Status (*_fmi2NewDiscreteStates)(fmi2Component,fmi2EventInfo*);
		fmi2Status (*_fmi2EnterContinuousTimeMode)(fmi2Component);
		fmi2Status (*_fmi2CompletedIntegratorStep)(fmi2Component, fmi2Boolean, fmi2Boolean*, fmi2Boolean*);
		fmi2Status (*_fmi2SetTime)(fmi2Component, fmi2Real);
		fmi2Status (*_fmi2SetContinuousStates)(fmi2Component, const fmi2Real*, size_t);
		fmi2Status (*_fmi2GetDerivatives)(fmi2Component, fmi2Real*, size_t);
		fmi2Status (*_fmi2GetEventIndicators)(fmi2Component, fmi2Real*, size_t);
		fmi2Status (*_fmi2GetContinuousStates)(fmi2Component, fmi2Real*, size_t);	
		
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

		static void fmilogger(
			fmi2ComponentEnvironment componentEnvironment,
			fmi2String instanceName,
			fmi2Status status,
			fmi2String category,
			fmi2String message,...)
		{
			if (message != NULL){			
		
				fprintf(stderr, message,"\n");
			}
		}

		fmi2CallbackFunctions* callbackFuncs;

		void iterate_events();
};

template <typename X>
FMI<X>::FMI(const char* modelname,
			const char* guid,
			const char* resource_location,
			int num_state_variables,
			int num_event_indicators,
			const char* so_file_name,
			const double tolerance,
			int num_extra_event_indicators,
			double start_time):
	// One extra variable at the end for time
	ode_system<X>(num_state_variables+1,num_event_indicators+num_extra_event_indicators),
	next_time_event(adevs_inf<double>()),
	t_now(start_time),
	so_hndl(NULL),
	cont_time_mode(false),
	num_extra_event_indicators(num_extra_event_indicators)	
{
	fmi2CallbackFunctions tmp = {adevs::FMI<X>::fmilogger,calloc,free,NULL,NULL};
	callbackFuncs = new fmi2CallbackFunctions(tmp);
	so_hndl = OPEN_LIB(so_file_name);
	if (so_hndl == NULL)
	{
		throw adevs::exception("Could not load so file",this);
    }
	// This only works with a POSIX compliant compiler/system
	_fmi2Instantiate = (fmi2Component (*)(fmi2String, fmi2Type,
		fmi2String, fmi2String, const fmi2CallbackFunctions*,
		fmi2Boolean, fmi2Boolean))GET_FUNC(so_hndl,"fmi2Instantiate");
	assert(_fmi2Instantiate != NULL);
	_fmi2FreeInstance = (void (*)(fmi2Component))GET_FUNC(so_hndl,"fmi2FreeInstance");
	assert(_fmi2FreeInstance != NULL);
	_fmi2SetupExperiment = (fmi2Status (*)(fmi2Component, fmi2Boolean,
		fmi2Real, fmi2Real, fmi2Boolean, fmi2Real))GET_FUNC(so_hndl,"fmi2SetupExperiment");
	assert(_fmi2SetupExperiment != NULL);
	_fmi2EnterInitializationMode = (fmi2Status (*)(fmi2Component))GET_FUNC(so_hndl,"fmi2EnterInitializationMode");
	assert(_fmi2EnterInitializationMode != NULL);
	_fmi2ExitInitializationMode = (fmi2Status (*)(fmi2Component))GET_FUNC(so_hndl,"fmi2ExitInitializationMode");
	assert(_fmi2ExitInitializationMode != NULL);
	_fmi2GetReal = (fmi2Status (*)(fmi2Component, const fmi2ValueReference*, size_t, fmi2Real*))
		GET_FUNC(so_hndl,"fmi2GetReal");
	assert(_fmi2GetReal != NULL);
	_fmi2GetInteger = (fmi2Status (*)(fmi2Component, const fmi2ValueReference*, size_t, fmi2Integer*)) 
		GET_FUNC(so_hndl,"fmi2GetInteger");
	assert(_fmi2GetInteger != NULL);
	_fmi2GetBoolean = (fmi2Status (*)(fmi2Component, const fmi2ValueReference*, size_t, fmi2Boolean*))
		GET_FUNC(so_hndl,"fmi2GetBoolean");
	assert(_fmi2GetBoolean != NULL);
	_fmi2GetString = (fmi2Status (*)(fmi2Component, const fmi2ValueReference*, size_t, fmi2String*))
		GET_FUNC(so_hndl,"fmi2GetString");
	assert(_fmi2GetString != NULL);
	_fmi2SetReal = (fmi2Status (*)(fmi2Component, const fmi2ValueReference*, size_t, const fmi2Real*))
		GET_FUNC(so_hndl,"fmi2SetReal");
	assert(_fmi2SetReal != NULL);
	_fmi2SetInteger = (fmi2Status (*)(fmi2Component, const fmi2ValueReference*, size_t, const fmi2Integer*))
		GET_FUNC(so_hndl,"fmi2SetInteger");
	assert(_fmi2SetInteger != NULL);
	_fmi2SetBoolean = (fmi2Status (*)(fmi2Component, const fmi2ValueReference*, size_t, const fmi2Boolean*))
		GET_FUNC(so_hndl,"fmi2SetBoolean");
	assert(_fmi2SetBoolean != NULL);
	_fmi2SetString = (fmi2Status (*)(fmi2Component, const fmi2ValueReference*, size_t, const fmi2String*))
		GET_FUNC(so_hndl,"fmi2SetString");
	assert(_fmi2SetString != NULL);
	_fmi2EnterEventMode = (fmi2Status (*)(fmi2Component))GET_FUNC(so_hndl,"fmi2EnterEventMode");
	assert(_fmi2EnterEventMode != NULL);
	_fmi2NewDiscreteStates = (fmi2Status (*)(fmi2Component,fmi2EventInfo*))GET_FUNC(so_hndl,"fmi2NewDiscreteStates");
	assert(_fmi2NewDiscreteStates != NULL);
	_fmi2EnterContinuousTimeMode = (fmi2Status (*)(fmi2Component))GET_FUNC(so_hndl,"fmi2EnterContinuousTimeMode");
	assert(_fmi2EnterContinuousTimeMode != NULL);
	_fmi2CompletedIntegratorStep = (fmi2Status (*)(fmi2Component, fmi2Boolean, fmi2Boolean*, fmi2Boolean*))
		GET_FUNC(so_hndl,"fmi2CompletedIntegratorStep");
	assert(_fmi2CompletedIntegratorStep != NULL);
	_fmi2SetTime = (fmi2Status (*)(fmi2Component, fmi2Real))GET_FUNC(so_hndl,"fmi2SetTime");
	assert(_fmi2SetTime != NULL);
	_fmi2SetContinuousStates = (fmi2Status (*)(fmi2Component, const fmi2Real*, size_t))
		GET_FUNC(so_hndl,"fmi2SetContinuousStates");
	assert(_fmi2SetContinuousStates != NULL);
	_fmi2GetDerivatives = (fmi2Status (*)(fmi2Component, fmi2Real*, size_t))GET_FUNC(so_hndl,"fmi2GetDerivatives");
	assert(_fmi2GetDerivatives != NULL);
	_fmi2GetEventIndicators = (fmi2Status (*)(fmi2Component, fmi2Real*, size_t))GET_FUNC(so_hndl,"fmi2GetEventIndicators");
	assert(_fmi2GetEventIndicators != NULL);
	_fmi2GetContinuousStates = (fmi2Status (*)(fmi2Component, fmi2Real*, size_t))GET_FUNC(so_hndl,"fmi2GetContinuousStates");
	assert(_fmi2GetContinuousStates != NULL);
	// Create the FMI component
	c = _fmi2Instantiate(modelname,fmi2ModelExchange,guid,resource_location,callbackFuncs,fmi2False,fmi2False);
	assert(c != NULL);
	_fmi2SetupExperiment(c,fmi2True,tolerance,-1.0,fmi2False,-1.0);
}

template <typename X>
void FMI<X>::iterate_events()
{
	fmi2Status status;
	// Put into consistent initial state
	fmi2EventInfo eventInfo;
	do
	{
		status = _fmi2NewDiscreteStates(c,&eventInfo);
		assert(status == fmi2OK);
	}
	while (eventInfo.newDiscreteStatesNeeded == fmi2True);
	if (eventInfo.nextEventTimeDefined == fmi2True)
		next_time_event = eventInfo.nextEventTime;
	else
		next_time_event = adevs_inf<double>();
	assert(status == fmi2OK);
}

template <typename X>
void FMI<X>::init(double* q)
{
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
	status = _fmi2SetTime(c,t_now);
	assert(status == fmi2OK);
	// Get starting state variable values
	status = _fmi2GetContinuousStates(c,q,this->numVars()-1);
	assert(status == fmi2OK);
	q[this->numVars()-1] = t_now;
	cont_time_mode = true;
}

template <typename X>
void FMI<X>::der_func(const double* q, double* dq)
{
	fmi2Status status;
	if (!cont_time_mode)
	{
		status = _fmi2EnterContinuousTimeMode(c);
		assert(status == fmi2OK);
		cont_time_mode = true;
	}
	status =_fmi2SetTime(c,q[this->numVars()-1]);
	assert(status == fmi2OK);
	status = _fmi2SetContinuousStates(c,q,this->numVars()-1);
	assert(status == fmi2OK);
	status = _fmi2GetDerivatives(c,dq,this->numVars()-1);
	assert(status == fmi2OK);
	dq[this->numVars()-1] = 1.0;
}

template <typename X>
void FMI<X>::state_event_func(const double* q, double* z)
{
	fmi2Status status;
	if (!cont_time_mode)
	{
		status = _fmi2EnterContinuousTimeMode(c);
		assert(status == fmi2OK);
		cont_time_mode = true;
	}
	status = _fmi2SetTime(c,q[this->numVars()-1]);
	assert(status == fmi2OK);
	status = _fmi2SetContinuousStates(c,q,this->numVars()-1);
	assert(status == fmi2OK);
	status = _fmi2GetEventIndicators(c,z,this->numEvents()-num_extra_event_indicators);
	assert(status == fmi2OK);
}

template <typename X>
double FMI<X>::time_event_func(const double* q)
{
	return next_time_event-q[this->numVars()-1];
}

template <typename X>
void FMI<X>::postStep(double* q)
{
	assert(cont_time_mode);
	// Don't advance the FMI state by zero units of time
	// when in continuous mode
	if (q[this->numVars()-1] <= t_now)
		return;
	// Try to complete the integration step
	fmi2Status status;
	fmi2Boolean enterEventMode;
	fmi2Boolean terminateSimulation;
	t_now = q[this->numVars()-1];
	status = _fmi2SetTime(c,t_now);
	assert(status == fmi2OK);
	status = _fmi2SetContinuousStates(c,q,this->numVars()-1);
	assert(status == fmi2OK);
	status = _fmi2CompletedIntegratorStep(c,fmi2True,&enterEventMode,&terminateSimulation);
	assert(status == fmi2OK);
	// Force an event if one is indicated
	if (enterEventMode == fmi2True)
		next_time_event = t_now;
}

template <typename X>
void FMI<X>::postTrialStep(double* q)
{
	assert(cont_time_mode);
	// Restore values changed by der_func and state_event_func
	fmi2Status status;
	status = _fmi2SetTime(c,q[this->numVars()]-1);
	assert(status == fmi2OK);
	status = _fmi2SetContinuousStates(c,q,this->numVars()-1);
	assert(status == fmi2OK);
}

template <typename X>
void FMI<X>::internal_event(double* q, const bool* state_event)
{
	fmi2Status status;
	// postStep will have updated the continuous variables, so 
	// we just process discrete events here.
	if (cont_time_mode)
	{
		status = _fmi2EnterEventMode(c);
		assert(status == fmi2OK);
		cont_time_mode = false;
	}
	// Process events
	iterate_events();
	// Update the state variable array
	status = _fmi2GetContinuousStates(c,q,this->numVars()-1);
	assert(status == fmi2OK);
}
				
template <typename X>
void FMI<X>::external_event(double* q, double e, const Bag<X>& xb)
{
	fmi2Status status;
	// Go to event mode if we have not yet done so
	if (cont_time_mode)
	{
		status = _fmi2EnterEventMode(c);
		assert(status == fmi2OK);
		cont_time_mode = false;
	}
	// process any events that need processing
	iterate_events();
	status = _fmi2GetContinuousStates(c,q,this->numVars()-1);
	assert(status == fmi2OK);
}
				
template <typename X>
void FMI<X>::confluent_event(double *q, const bool* state_event, const Bag<X>& xb)
{
	fmi2Status status;
	// postStep will have updated the continuous variables, so 
	// we just process discrete events here.
	if (cont_time_mode)
	{
		status = _fmi2EnterEventMode(c);
		assert(status == fmi2OK);
		cont_time_mode = false;
	}
	iterate_events();
	status = _fmi2GetContinuousStates(c,q,this->numVars()-1);
	assert(status == fmi2OK);
}
				
template <typename X>
void FMI<X>::output_func(const double *q, const bool* state_event, Bag<X>& yb)
{
}

template <typename X>
void FMI<X>::gc_output(Bag<X>& gb)
{
}

template <typename X>
FMI<X>::~FMI()
{
	_fmi2FreeInstance(c);
	delete callbackFuncs;
	CLOSE_LIB(so_hndl);
}

template <typename X>
double FMI<X>::get_real(int k)
{
	const fmi2ValueReference ref = k;
	fmi2Real val;
	fmi2Status status = _fmi2GetReal(c,&ref,1,&val);
	assert(status == fmi2OK);
	return val;
}

template <typename X>
void FMI<X>::set_real(int k, double val)
{
	const fmi2ValueReference ref = k;
	fmi2Real fmi_val = val;
	fmi2Status status = _fmi2SetReal(c,&ref,1,&fmi_val);
	assert(status == fmi2OK);
}

template <typename X>
int FMI<X>::get_int(int k)
{
	const fmi2ValueReference ref = k;
	fmi2Integer val;
	fmi2Status status = _fmi2GetInteger(c,&ref,1,&val);
	assert(status == fmi2OK);
	return val;
}

template <typename X>
void FMI<X>::set_int(int k, int val)
{
	const fmi2ValueReference ref = k;
	fmi2Integer fmi_val = val;
	fmi2Status status = _fmi2SetInteger(c,&ref,1,&fmi_val);
	assert(status == fmi2OK);
}

template <typename X>
bool FMI<X>::get_bool(int k)
{
	const fmi2ValueReference ref = k;
	fmi2Boolean val;
	fmi2Status status = _fmi2GetBoolean(c,&ref,1,&val);
	assert(status == fmi2OK);
	return (val == fmi2True);
}

template <typename X>
void FMI<X>::set_bool(int k, bool val)
{
	const fmi2ValueReference ref = k;
	fmi2Boolean fmi_val = fmi2False;
	if (val) fmi_val = fmi2True;
	fmi2Status status = _fmi2SetBoolean(c,&ref,1,&fmi_val);
	assert(status == fmi2OK);
}

template <typename X>
std::string FMI<X>::get_string(int k)
{
	const fmi2ValueReference ref = k;
	fmi2String val;
	fmi2Status status = _fmi2GetString(c,&ref,1,&val);
	assert(status == fmi2OK);
	return val;
}

template <typename X>
void FMI<X>::set_string(int k, std::string& val)
{
	const fmi2ValueReference ref = k;
	fmi2String fmi_val = fmi2False;
	fmi2Status status = _fmi2SetString(c,&ref,1,&fmi_val);
	assert(status == fmi2OK);
}

} // end of namespace

#endif
