#ifndef _control_h_
#define _control_h_
#include "adevs.h"
#include "events.h"

/**
  * Model of a control law.
  */
class Control:
	public adevs::Atomic<event_t>
{
	public:
		Control();
		void delta_int();
		void delta_ext(double e, const adevs::Bag<event_t>& xb);
		void delta_conf(const adevs::Bag<event_t>& xb);
		double ta();
		void output_func(adevs::Bag<event_t>& yb);
		void gc_output(adevs::Bag<event_t>&){}
		~Control(){}
	protected:
		/// Modeler's control law
		virtual void control_law(double h, const double* iabc, const double* vabc) = 0;
		static void dq_to_abc(const double* dq, double* abc, double angle);
		static void abc_to_dq(const double* abc, double* dq, double angle);
		/// Output duty cycles
		double vout[3];
	private:
		bool waiting;
};

#endif
