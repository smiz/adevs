#include "ElectricalModelEqns.h"

using namespace adevs;

static double const pi = 3.1415926535897931;
// Macros for accessing generator state variables
#define TIME                0
#define STATE_VARS_PER_GENR 5
#define EVENT_COND_PER_GENR 6
#define OMEGA(i)            STATE_VARS_PER_GENR* i + 1  // Angular speed
#define THETA(i)            STATE_VARS_PER_GENR* i + 2  // Power angle
#define C(i) \
    STATE_VARS_PER_GENR* i + 3  // Freqeuncy (mech. power) control signal
#define Pm(i) STATE_VARS_PER_GENR* i + 4  // Mechanical power
#define EF(i) STATE_VARS_PER_GENR* i + 5  // Exciter voltage

// Inidices for the state event functions
#define OVER_FREQ_EVENT(i)          (EVENT_COND_PER_GENR * i)
#define UNDER_FREQ_EVENT(i)         (EVENT_COND_PER_GENR * i + 1)
#define MECH_POWER_SAMPLE_EVENT(i)  (EVENT_COND_PER_GENR * i + 2)
#define SPEED_SAMPLE_EVENT(i)       (EVENT_COND_PER_GENR * i + 3)
#define VOLTAGE_SATURATE_EVENT(i)   (EVENT_COND_PER_GENR * i + 4)
#define VOLTAGE_UNSATURATE_EVENT(i) (EVENT_COND_PER_GENR * i + 5)

// Hysteresis for state events which need it
static double const StateEventEps = 1E-4;
// Port for changing the network loads
int const ElectricalModelEqns::SetLoad = -2;
// Port for setting generator sampling conditions
int const ElectricalModelEqns::SetGenrSample = -1;
// Port for setting control information
int const ElectricalModelEqns::Control = -3;
// Port for tripping a generator offline
int const ElectricalModelEqns::GenrTrip = -4;
// Port for making fractional adjustments to load
int const ElectricalModelEqns::LoadAdj = -5;

// Constructor
ElectricalModelEqns::ElectricalModelEqns(ElectricalData* data, bool no_events)
    : ode_system<PortValue<BasicEvent*>>(
          data->getGenrCount() * STATE_VARS_PER_GENR +
              1,  // Number of state variables
          EVENT_COND_PER_GENR * data->getGenrCount() *
              (!no_events)  // Number of state event functions
          ),
      // Save a pointer to the electrical data
      data(data),
      no_events(no_events),
      load_adj(0.0),  // Load starts at its base value
      Y(data->getNodeCount()) {
    Ef = new Complex[data->getGenrCount()];
    // Breaker status vector
    breaker_closed = new bool[data->getGenrCount()];
    // Excitation status vector
    excite_status = new ExcitationState[data->getGenrCount()];
    // Allocate the transmission network admittance matrix
    data->buildAdmitMatrix(Y);
    // Allocate the diagonal node admittance matrix
    load_Yt = new Complex[data->getNodeCount()];
    genr_Yt = new Complex[data->getNodeCount()];
    // Allocate the node voltage and current vectors
    voltage = new Complex[data->getNodeCount()];
    current = new Complex[data->getNodeCount()];
    // Allocate the injected current vector
    load_inj_current = new Complex[data->getNodeCount()];
    genr_inj_current = new Complex[data->getNodeCount()];
    // Allocate mechanical power set point vector
    PS = new double[data->getGenrCount()];
    // The diagonal Yt matrix contains admittance information for generators and lines
    for (std::vectorunsigned>::const_iterator iter = data->getGenrs().begin();
         iter != data->getGenrs().end(); iter++) {
        genr_Yt[*iter] = 1.0 / data->getGenrParams(*iter).Xd;
    }
    for (unsigned i = 0; i < data->getNodeCount(); i++) {
        // Add the load admittance (zero for no load)
        load_Yt[i] = data->getAdmittance(i);
        // Get the initial Norton equiv. load current
        load_inj_current[i] = data->getCurrent(i);
    }
    // Generator output ports
    GenrSampleOutput = new int[data->getGenrCount()];
    unsigned i;
    std::vectorunsigned>::const_iterator iter;
    for (i = 0, iter = data->getGenrs().begin(); iter != data->getGenrs().end();
         i++, iter++) {
        // Generator number -> BusID lookup table
        genr_bus_lookup[*iter] = i;
        // Initialize the sample output port number
        GenrSampleOutput[i] = *iter;
        // Get the generator parameters
        ElectricalData::genr_t param = data->getGenrParams(*iter);
        if (param.fix_at_Pm0) {
            PS[i] = param.Pm0;
        } else {
            PS[i] = param.Ps;
        }
        // Norton equiv. current is just EF/Xd
        genr_inj_current[*iter] = polar(param.Ef0, param.T0) / param.Xd;
        // Set the breaker status to closed
        breaker_closed[i] = true;
        // Set the exciter status
        excite_status[i] = NOT_SAT;
        // Check that this is really true
        assert(fabs(param.Ef0) <= param.Ef_max);
    }
    // Compute the initial bus voltages
    solve_for_voltage();
}

void ElectricalModelEqns::init(double* q) {
    // Initialize the time variable to zero
    q[TIME] = 0.0;
    // Get the initial state of the generators from the electrical data
    unsigned i;
    std::vectorunsigned>::const_iterator iter;
    for (i = 0, iter = data->getGenrs().begin(); iter != data->getGenrs().end();
         i++, iter++) {
        // Get the generator parameters
        ElectricalData::genr_t param = data->getGenrParams(*iter);
        // Initial frequency
        q[OMEGA(i)] = param.w0;
        // Initial Ef phasor amplitude
        q[EF(i)] = param.Ef0;
        // Initial mechanical power output and set point
        q[Pm(i)] = param.Pm0;
        // Initial power control signal value
        q[C(i)] = param.C0;
        // Initial power angle
        q[THETA(i)] = param.T0;
    }
}

void ElectricalModelEqns::updateVoltageAndInjCurrent(double const* q) {
    // Variables for iterating over all of the generators
    unsigned i;
    std::vectorunsigned>::const_iterator iter;
    // Find the Norton equiv. current for each machine
    for (i = 0, iter = data->getGenrs().begin(); iter != data->getGenrs().end();
         i++, iter++) {
        // Convert the excitator voltage phasor to a complex number
        Ef[i] = polar(q[EF(i)], q[THETA(i)]);
        // Put the Norton equiv. current into the injected current matrix.
        // The current is zero if the breaker is open!
        if (breaker_closed[i]) {
            genr_inj_current[*iter] = Ef[i] / data->getGenrParams(*iter).Xd;
        } else {
            genr_inj_current[*iter] = Complex(0.0, 0.0);
        }
    }
    // Compute the complex voltage at each bus (loads and generator terminals)
    solve_for_voltage();
}

void ElectricalModelEqns::getBusFreqs(double const* q, double* freq) {
    unsigned i;
    std::vectorunsigned>::const_iterator iter;
    double* dq = new double[numVars()];
    Complex* dI = new Complex[data->getNodeCount()];
    Complex* dV = new Complex[data->getNodeCount()];
    der_func(q, dq);
    for (i = 0; i < data->getNodeCount(); i++) {
        dI[i] = Complex(0.0,
                        0.0);  // Assumes constant injected current at the loads
    }
    for (i = 0, iter = data->getGenrs().begin(); iter != data->getGenrs().end();
         i++, iter++) {
        if (breaker_closed[i]) {
            Complex V1 = polar(dq[EF(i)], q[THETA(i)]);
            Complex V2 = polar(q[EF(i)] * dq[THETA(i)], q[THETA(i)] - pi / 2.0);
            dI[*iter] = (V1 - V2) / data->getGenrParams(*iter).Xd;
        }
    }
    Y.solve_for_voltage(dI, dV);
    for (i = 0; i < data->getNodeCount(); i++) {
        double angle = arg(getVoltage(i));
        double mag = abs(getVoltage(i));
        double A[2][2], B[2];
        A[0][0] = cos(angle);
        A[1][0] = -mag * cos(angle - pi / 2.0);
        A[0][1] = sin(angle);
        A[1][1] = -mag * sin(angle - pi / 2.0);
        B[0] = abs(dV[i]) * cos(arg(dV[i]));
        B[1] = abs(dV[i]) * sin(arg(dV[i]));
        double det = A[0][0] * A[1][1] - A[1][0] * A[0][1];
        freq[i] = (-A[0][1] * B[0] + A[0][0] * B[1]) / det;
    }
    delete[] dq;
    delete[] dI;
    delete[] dV;
}

void ElectricalModelEqns::der_func(double const* q, double* dq) {
    // Update all of the voltage and current values for q
    updateVoltageAndInjCurrent(q);
    // Compute the new derivative values for each generator
    unsigned i;
    std::vectorunsigned>::const_iterator iter;
    for (i = 0, iter = data->getGenrs().begin(); iter != data->getGenrs().end();
         i++, iter++) {
        // Find the terminal current phasors
        Complex I = (Ef[i] - voltage[*iter]) / data->getGenrParams(*iter).Xd;
        // Compute the real electrical power demand on the generator
        double Pe = real(voltage[*iter] * conj(I));
        // Swing equation
        dq[OMEGA(i)] = ((q[Pm(i)] - Pe) / data->getGenrParams(*iter).M) *
                       breaker_closed[i];
        dq[THETA(i)] = q[OMEGA(i)] * breaker_closed[i];
        // Mechanical power control
        double uc = data->getGenrParams(*iter).Agc * q[THETA(i)];
        dq[C(i)] = -data->getGenrParams(*iter).Tspd1 *
                   (q[C(i)] + data->getGenrParams(*iter).R * q[OMEGA(i)] + uc) *
                   breaker_closed[i];
        dq[Pm(i)] = data->getGenrParams(*iter).Tspd2 *
                    (q[C(i)] - q[Pm(i)] + PS[i]) * breaker_closed[i];
        // Voltage control. This is a very simple voltage regulator
        // model based on the standard IEEE type 1 and 2 but with
        // most parameters defaulted to unity, zero, or very large
        // values.
        double Vt = abs(voltage[*iter]);
        // Do not allow Ef to leave its valid range
        if (excite_status[i] == NOT_SAT || excite_status[i] == FALLING) {
            dq[EF(i)] = ((data->getGenrParams(*iter).Vref - Vt) /
                         data->getGenrParams(*iter).Te) *
                        breaker_closed[i];
        } else {
            dq[EF(i)] = 0.0;
        }
    }
    // Time
    dq[TIME] = 1.0;
}

void ElectricalModelEqns::output_func(double const* q, bool const* state_event,
                                      std::list<PortValue<BasicEvent*>> &yb) {
    // Output a sample event is any of them are primed to fire
    map<unsigned, GenrSampleEvent>::iterator siter = sample_conds.begin();
    for (siter = sample_conds.begin(); siter != sample_conds.end(); siter++) {
        if ((*siter).second.outputImmediately()) {
            unsigned genr_index = genr_bus_lookup[(*siter).first];
            PortValue<BasicEvent*> e;
            GenrSampleEvent* value = new GenrSampleEvent((*siter).second);
            e.value = value;
            e.port = value->getBusID();
            // Assign the current state variable values
            value->setMechPower(q[Pm(genr_index)] * breaker_closed[genr_index]);
            value->setRotorSpeed(q[OMEGA(genr_index)] *
                                 breaker_closed[genr_index]);
            value->setSampleTime(q[TIME]);
            value->setMechPowerSetPoint(PS[genr_index] *
                                        breaker_closed[genr_index]);
            value->freqBreakerOpen(!breaker_closed[genr_index]);
            // Find the terminal load
            Complex exciter_Ef = polar(q[EF(genr_index)], q[THETA(genr_index)]);
            Complex I = (exciter_Ef - voltage[(*siter).first]) /
                        data->getGenrParams((*siter).first).Xd;
            value->setLoad(voltage[(*siter).first] * conj(I));
            // Send the output event
            yb.push_back(e);
        }
    }
}

void ElectricalModelEqns::internal_event(double* q, bool const* z) {
    // Don't do anything if event are turned off
    if (no_events) {
        return;
    }
    // Test for event conditions on electrical variables
    unsigned i;
    std::vectorunsigned>::const_iterator iter;
    for (i = 0, iter = data->getGenrs().begin(); iter != data->getGenrs().end();
         i++, iter++) {
        // Handle the sampling conditions
        if (sample_conds.find(*iter) != sample_conds.end()) {
            // Remove a sampling condition if it fired
            if (sample_conds[*iter].outputImmediately()) {
                if (sample_conds[*iter].oneShot()) {
                    sample_conds.erase(*iter);
                } else {
                    sample_conds[*iter].outputImmediately(false);
                    sample_conds[*iter].setRotorSpeed(q[OMEGA(i)]);
                    sample_conds[*iter].setMechPower(q[Pm(i)]);
                }
            }
            // Otherwise test sampling conditions and prime the event if they are satisfied
            else {
                bool trigger = z[SPEED_SAMPLE_EVENT(i)] ||
                               z[MECH_POWER_SAMPLE_EVENT(i)] ||
                               z[OVER_FREQ_EVENT(i)] || z[UNDER_FREQ_EVENT(i)];
                sample_conds[*iter].outputImmediately(trigger);
            }
        }
        // Test the frequency variables for a breaker trip
        if (z[OVER_FREQ_EVENT(i)] || z[UNDER_FREQ_EVENT(i)]) {
            // Open the breaker
            breaker_closed[i] = false;
            // Set the admittance to zero (open circuit)
            Y.remove_self(*iter, genr_Yt[*iter]);
            genr_Yt[*iter] = Complex(0.0, 0.0);
        }
        // Test for voltage out of range
        if (z[VOLTAGE_SATURATE_EVENT(i)]) {
            excite_status[i] = SAT;
        } else if (z[VOLTAGE_UNSATURATE_EVENT(i)]) {
            if (excite_status[i] == SAT) {
                excite_status[i] = FALLING;
            } else {
                excite_status[i] = NOT_SAT;
            }
        }
    }
}

void ElectricalModelEqns::external_event(
    double* q, double e, std::list<PortValue<BasicEvent*>> const &xb) {
    // Don't do anything if event are turned off
    if (no_events) {
        return;
    }
    // Iterate through the input event list
    for (std::list<PortValue<BasicEvent*>>::const_iterator iter = xb.begin();
         iter != xb.end(); iter++) {
        // Apply a uniform load adjustment
        if ((*iter).port == LoadAdj) {
            LoadAdjustEvent* load_event =
                dynamic_cast<LoadAdjustEvent*>((*iter).value);
            assert(load_event != NULL);
            for (unsigned bus = 0; bus < data->getNodeCount(); bus++) {
                // Subtract the old load value from the admittance matrix
                Y.remove_self(bus, (1.0 + load_adj) * load_Yt[bus]);
                // Add the new load
                Y.add_self(bus,
                           (1.0 + load_event->getAdjustment()) * load_Yt[bus]);
            }
            load_adj = load_event->getAdjustment();
        }
        // A load change event alters the node current and/or admittance
        if ((*iter).port == SetLoad) {
            LoadEvent* load_event = dynamic_cast<LoadEvent*>((*iter).value);
            assert(load_event != NULL);
            unsigned bus = load_event->getBusID();
            // Subtract the old load value from the admittance matrix
            Y.remove_self(bus, (1.0 + load_adj) * load_Yt[bus]);
            // Add the new load
            load_Yt[bus] = load_event->getAdmittance();
            Y.add_self(bus, (1.0 + load_adj) * load_Yt[bus]);
            // Get the new current
            load_inj_current[bus] = load_event->getCurrent();
        }
        // Store generator sample events
        else if ((*iter).port == SetGenrSample) {
            // Get a copy of the input event
            GenrSampleEvent sample_cond =
                *(dynamic_cast<GenrSampleEvent*>((*iter).value));
            // Remember the state variable values that can trigger samples
            unsigned genr_index = genr_bus_lookup[sample_cond.getBusID()];
            sample_cond.setRotorSpeed(q[OMEGA(genr_index)]);
            sample_cond.setMechPower(q[Pm(genr_index)]);
            // Save the condition, replace and previous condition
            sample_conds[sample_cond.getBusID()] = sample_cond;
        }
        // Set the mechanical power control
        else if ((*iter).port == Control) {
            // Get the control input
            SetPointEvent* set_point =
                dynamic_cast<SetPointEvent*>((*iter).value);
            unsigned genr_index = genr_bus_lookup[set_point->getBusID()];
            PS[genr_index] = set_point->getMechPowerSetPoint();
        }
        // Trip a generator offline
        else if ((*iter).port == GenrTrip) {
            GenrFailEvent* genr_fail =
                dynamic_cast<GenrFailEvent*>((*iter).value);
            unsigned genr_index = genr_bus_lookup[genr_fail->getBusID()];
            breaker_closed[genr_index] = false;
            if (sample_conds.find(genr_fail->getBusID()) !=
                sample_conds.end()) {
                sample_conds[genr_fail->getBusID()].outputImmediately(true);
            }
            // Set the admittance to zero (open circuit)
            Y.remove_self(genr_fail->getBusID(),
                          genr_Yt[genr_fail->getBusID()]);
            genr_Yt[genr_fail->getBusID()] = Complex(0.0, 0.0);
        }
    }
}

void ElectricalModelEqns::confluent_event(
    double* q, bool const* z, std::list<PortValue<BasicEvent*>> const &xb) {
    internal_event(q, z);
    external_event(q, 0, xb);
}

void ElectricalModelEqns::state_event_func(double const* q, double* z) {
    // Don't do anything if there are no events to look for
    if (no_events) {
        return;
    }
    // Iteration variables for the generators
    unsigned i;
    std::vectorunsigned>::const_iterator iter;
    // Check the freq. and voltage limit conditions
    for (i = 0, iter = data->getGenrs().begin(); iter != data->getGenrs().end();
         i++, iter++) {
        // If the generator is connected to the network, then check its speed
        if (breaker_closed[i]) {
            // Over frequency check
            z[OVER_FREQ_EVENT(i)] =
                q[OMEGA(i)] - 2.0 * pi * data->getGenrParams(*iter).FreqTol;
            // Over frequency check
            z[UNDER_FREQ_EVENT(i)] =
                2.0 * pi * data->getGenrParams(*iter).FreqTol + q[OMEGA(i)];
        } else {
            z[OVER_FREQ_EVENT(i)] = z[UNDER_FREQ_EVENT(i)] = 1.0;
        }
        // Voltage under limit, saturates at max + epsilon to avoid illegitimacy
        if (excite_status[i] == NOT_SAT) {
            z[VOLTAGE_SATURATE_EVENT(i)] =
                q[EF(i)] - (data->getGenrParams(*iter).Ef_max + StateEventEps);
            z[VOLTAGE_UNSATURATE_EVENT(i)] = 1.0;
        }
        // Unsaturate at slightly less than 1 volt pu
        else if (excite_status[i] == SAT) {
            z[VOLTAGE_SATURATE_EVENT(i)] = 1.0;
            z[VOLTAGE_UNSATURATE_EVENT(i)] =
                abs(voltage[*iter]) - (1.0 + StateEventEps);
        } else if (excite_status[i] == FALLING) {
            z[VOLTAGE_SATURATE_EVENT(i)] = abs(voltage[*iter]) - 1.0;
            z[VOLTAGE_UNSATURATE_EVENT(i)] =
                q[EF(i)] - data->getGenrParams(*iter).Ef_max;
        }
        // Sampling condition satisfied
        if (sample_conds.find(*iter) == sample_conds.end()) {
            z[MECH_POWER_SAMPLE_EVENT(i)] = DBL_MAX;
            z[SPEED_SAMPLE_EVENT(i)] = DBL_MAX;
        } else {
            z[MECH_POWER_SAMPLE_EVENT(i)] =
                sample_conds[*iter].getMechPowerSensitivity() -
                fabs(sample_conds[*iter].getMechPower() - q[Pm(i)]);
            z[SPEED_SAMPLE_EVENT(i)] =
                sample_conds[*iter].getRotorSpeedSensitivity() -
                fabs(sample_conds[*iter].getRotorSpeed() - q[OMEGA(i)]);
        }
    }
}

double ElectricalModelEqns::time_event_func(double const* q) {
    // Output a sample event is any of them are primed to fire
    map<unsigned int, GenrSampleEvent>::iterator siter = sample_conds.begin();
    for (siter = sample_conds.begin(); siter != sample_conds.end(); siter++) {
        if ((*siter).second.outputImmediately()) {
            return 0.0;
        }
    }
    return DBL_MAX;
}

void ElectricalModelEqns::solve_for_voltage() {
    for (unsigned i = 0; i < data->getNodeCount(); i++) {
        current[i] = load_inj_current[i] + genr_inj_current[i];
    }
    Y.solve_for_voltage(current, voltage);
}

Complex ElectricalModelEqns::getVoltage(unsigned bus) {
    return voltage[bus];
}

Complex ElectricalModelEqns::getInjCurrent(unsigned bus) {
    return load_inj_current[bus] + genr_inj_current[bus];
}

double ElectricalModelEqns::getGenrFreq(unsigned genr_number, double const* q) {
    return q[OMEGA(genr_number)];
}

double ElectricalModelEqns::getMechPower(unsigned genr_number,
                                         double const* q) {
    return q[Pm(genr_number)];
}

bool ElectricalModelEqns::genrOffLine(unsigned genr_number) {
    return !breaker_closed[genr_number];
}

ElectricalModelEqns::~ElectricalModelEqns() {
    delete[] breaker_closed;
    delete[] load_inj_current;
    delete[] genr_inj_current;
    delete[] voltage;
    delete[] load_Yt;
    delete[] genr_Yt;
    delete[] PS;
    delete[] current;
    delete[] GenrSampleOutput;
    delete[] Ef;
    delete data;
}
