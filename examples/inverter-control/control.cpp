#include "adevs.h"
#include "control.h"
#include "circuit.h"

/**
  * Model of a control law.
  */
Control::Control():
	adevs::Atomic<event_t>(),
	waiting(true)
{
}

void Control::delta_int()
{
	waiting = true;
}

void Control::dq_to_abc(const double* dq, double* abc, double angle)
{
	abc[0] = cos(angle+Circuit::phase_angle[0])*dq[0]-sin(angle+Circuit::phase_angle[0])*dq[1]+dq[2];
	abc[1] = cos(angle+Circuit::phase_angle[1])*dq[0]-sin(angle+Circuit::phase_angle[1])*dq[1]+dq[2];
	abc[2] = cos(angle+Circuit::phase_angle[2])*dq[0]-sin(angle+Circuit::phase_angle[2])*dq[1]+dq[2];
}

void Control::abc_to_dq(const double* abc, double* dq, double angle)
{
	dq[0] = (2.0/3.0)*
		(cos(angle)*abc[0]+cos(angle+Circuit::phase_angle[1])*abc[1]+cos(angle+Circuit::phase_angle[2])*abc[2]);
	dq[1] = (2.0/3.0)*
		(-sin(angle)*abc[0]-sin(angle+Circuit::phase_angle[1])*abc[1]-sin(angle+Circuit::phase_angle[2])*abc[2]);
	dq[2] = (2.0/3.0)*
		((1.0/2.0)*abc[0]+(1.0/2.0)*abc[1]+(1.0/2.0)*abc[2]);
}

void Control::delta_ext(double e, const adevs::Bag<event_t>& xb)
{
	control_law(e,xb[0].value.data.iabc,xb[0].value.data.vabc);
	waiting = false;
}

void Control::delta_conf(const adevs::Bag<event_t>& xb)
{
	delta_int();
	delta_ext(0.0,xb);
}

double Control::ta()
{
	return (waiting) ? adevs_inf<double>() : 0.0;
}

void Control::output_func(adevs::Bag<event_t>& yb)
{
	event_t y;
	y.type = PWM_DUTY_CYCLE;
	for (int i = 0; i < 3; i++)
	{
		y.value.pwm[i] = vout[i];
		assert(y.value.pwm[i] >= -1.0);
		assert(y.value.pwm[i] <= 1.0);
	}
	yb.insert(y);
}

