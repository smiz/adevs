#ifndef _pwm_h_
#define _pwm_h_
#include "adevs.h"
#include "events.h"

/**
  * A PWM signal generator
  */
class PWM_Phase:
	public adevs::Atomic<event_t>
{
	public:
		/**
		  * Create a pwm with the supplied clock frequency.
		  * @param freq Switching frequency for the pwm
		  * @param phase The phase to which this leg is attached
		  */
		PWM_Phase(double freq, int phase);
		void delta_int();
		/// Accepts duty cycle signals
		void delta_ext(double e, const adevs::Bag<event_t>& xb);
		void delta_conf(const adevs::Bag<event_t>& xb);
		/// Produces switch signals
		void output_func(adevs::Bag<event_t>& yb);
		/// Time advance
		double ta();
		void gc_output(adevs::Bag<event_t>&){}
		~PWM_Phase(){}
	private:
		// Phase we are responsible for
		const int phase;
		// Switching period
		const double period;
		// Fraction of period to be on
		double tswitch, cycle;
		// Current state
		bool q;
};

/**
  * A three channel PWM controller.
  */
class PWM:
	public adevs::SimpleDigraph<event_t>
{
	public:
		PWM(double freq);
		~PWM(){}
	private:
		PWM_Phase* phases[3];
};

#endif
