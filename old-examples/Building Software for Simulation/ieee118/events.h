#ifndef _events_h_
#define _events_h_
#include <cfloat>
#include <complex>
#include <cstdlib>

// Complex data type
typedef std::complex<double> Complex;

/**
 * Base event type used as I/O throughout the model.
 */
class BasicEvent {
  public:
    BasicEvent() {}
    virtual BasicEvent* clone() const = 0;
    virtual ~BasicEvent() {}
};

/**
 * Load change events are used to indicate a change at a load bus.
 */
class LoadEvent : public BasicEvent {
  public:
    /**
		 * Create a load event for the specified bus. Set the
		 * Norton equiv. load current and admittance values as specified.
		 */
    LoadEvent(unsigned bus, Complex current, Complex admittance)
        : BasicEvent(), bus(bus), i(current), y(admittance) {}
    /// Get the bus number
    unsigned getBusID() const { return bus; }
    Complex getAdmittance() const { return y; }
    /// Get the Norton equiv. current at the bus
    Complex getCurrent() const { return i; }
    /// Destructor
    ~LoadEvent() {}
    BasicEvent* clone() const { return new LoadEvent(*this); }

  private:
    unsigned bus;
    Complex i, y;
};

/**
 * Event for making uniform, fractional load changes. 
 */
class LoadAdjustEvent : public BasicEvent {
  public:
    /**
		 * Adjusts the load uniformly by specified percentage of
		 * its base value. The new load will be the base load
		 * times 1+adjustment.
		 */
    LoadAdjustEvent(double adjustment) : BasicEvent(), adjustment(adjustment) {}
    /// Get the load adjustment
    double getAdjustment() const { return adjustment; }
    BasicEvent* clone() const { return new LoadAdjustEvent(*this); }

  private:
    double const adjustment;
};

/**
 * These events are used to set the sampling conditions on a generator.
 * They are provided to the ElectricalModel through its SetGenrSample
 * port.
 */
class GenrSampleEvent : public BasicEvent {
  public:
    /**
		 * Create an event for a particular generator. Generator's are identified
		 * by their bus number. The default sample condition is an immediate
		 * sample of the state variables.
		 */
    GenrSampleEvent(unsigned genr_bus_ID = 0)
        : BasicEvent(),
          bus(genr_bus_ID),
          dw(DBL_MAX),
          dP(DBL_MAX),
          immediate_output(true),
          freq_br_open(false),
          one_shot(true) {}
    /// Get the generation bus that this event applies to
    unsigned getBusID() const { return bus; }
    /// Get the generator angular velocity.
    double getRotorSpeed() const { return w; }
    /// Get the load at the generator terminals
    Complex getLoad() const { return load; }
    /// Get the mechanical power output
    double getMechPower() const { return mech_power; }
    /// Get the sample time
    double getSampleTime() const { return t; }
    /// Get the mechanical power set point
    double getMechPowerSetPoint() const { return PS; }
    /// Set the mechanical power set point
    void setMechPowerSetPoint(double PS) { this->PS = PS; }
    /// Set the rotor speed
    void setRotorSpeed(double w) { this->w = w; }
    /// Set the mechanical power
    void setMechPower(double mech_power) { this->mech_power = mech_power; }
    /// Set the terminal load
    void setLoad(Complex load) { this->load = load; }
    /// Set the sample time
    void setSampleTime(double t) { this->t = t; }
    /// Set or get the frequency breaker status
    void freqBreakerOpen(bool br_open) { this->freq_br_open = br_open; }
    bool freqBreakerOpen() const { return freq_br_open; }
    /**
		 * If the immediate flag is true, then the model will produce
		 * an output event immediately. Otherwise, it will wait until
		 * one of the sensitivity conditions is satisfied.
		 */
    void outputImmediately(bool flag) { this->immediate_output = flag; }
    /// Return true if there should be a sample now
    bool outputImmediately() const { return immediate_output; }
    /// Set the rotor speed change that will generate a new sample
    void setRotorSpeedSensitivity(double dw) { this->dw = dw; }
    /// Set the mechanical power change sensivity that will generate a new sample
    void setMechPowerSensitivity(double dP) { this->dP = dP; }
    /// Get the rotor speed change sensitivty
    double getRotorSpeedSensitivity() const { return dw; }
    /// Get the mechanical power change sensivity
    double getMechPowerSensitivity() const { return dP; }
    /// Is this a one shot sample, or should it persist?
    void oneShot(bool flag) { one_shot = flag; }
    bool oneShot() const { return one_shot; }
    BasicEvent* clone() const { return new GenrSampleEvent(*this); }
    /// Destructor
    ~GenrSampleEvent() {}

  private:
    unsigned bus;
    double dw, dP, t, w, mech_power, PS;
    bool immediate_output, freq_br_open, one_shot;
    Complex load;
};

/**
 * This class is for setting the mechanical power set point for a generator.
 * These events can be set to the ElectricalModel through its Control input port.
 */
class SetPointEvent : public BasicEvent {
  public:
    /**
		 * Create a set point for the generator mechanical power. The generator
		 * is identified by its bus number.
		 */
    SetPointEvent(unsigned bus, double Pm) : BasicEvent(), bus(bus), Pm(Pm) {}
    /// Get the bus number that is the target of this event
    unsigned getBusID() const { return bus; }
    /// Get the power set point
    double getMechPowerSetPoint() const { return Pm; }
    /// Destructor
    ~SetPointEvent() {}
    BasicEvent* clone() const { return new SetPointEvent(*this); }

  private:
    unsigned bus;
    double Pm;
};

/**
 * This class is used to cause a generator failure.
 */
class GenrFailEvent : public BasicEvent {
  public:
    /**
		 * Create an event that will cause the generator attached to the
		 * specified bus to trip offline.
		 */
    GenrFailEvent(unsigned bus) : BasicEvent(), bus(bus) {}
    /**
		 * Get the bus number that is the target of this event.
		 */
    unsigned getBusID() const { return bus; }
    /// Destructor
    ~GenrFailEvent() {}
    BasicEvent* clone() const { return new GenrFailEvent(*this); }

  private:
    unsigned bus;
};

#endif
