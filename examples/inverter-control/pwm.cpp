#include "adevs.h"
#include "pwm.h"
#include <cassert>

PWM::PWM(double freq):
	adevs::SimpleDigraph<event_t>()
{
	for (int i = 0; i < 3; i++)
	{
		phases[i] = new PWM_Phase(freq,i);
		add(phases[i]);
		couple(this,phases[i]);
		couple(phases[i],this);
	}
}

PWM_Phase::PWM_Phase(double freq, int phase):
	adevs::Atomic<event_t>(),
	phase(phase),
	period(1.0/freq),
	tswitch(period),
	cycle(0.0),
	q(false)	
{
}

double PWM_Phase::ta()
{
	return ::max(0.0,tswitch);
}

void PWM_Phase::delta_int()
{
	q = !q;
	tswitch = period*((!q) ? fabs(cycle) : (1.0-fabs(cycle)));
}

void PWM_Phase::delta_ext(double e, const adevs::Bag<event_t>& xb)
{
	// Account for interval that has elapsed
	tswitch =- e;
	for (auto x: xb)
		cycle = x.value.pwm[phase];
}

void PWM_Phase::delta_conf(const adevs::Bag<event_t>& xb)
{
	delta_int();
	delta_ext(0.0,xb);
}

/// Produces switch signals
void PWM_Phase::output_func(adevs::Bag<event_t>& yb)
{
	event_t y;
	y.type = SET_SWITCH;
	y.value.open_close.channel = phase;
	if (cycle == 0.0) y.value.open_close.polarity = 0.0;
	else if (!q) y.value.open_close.polarity = 0.0;
	else if (cycle > 0.0) y.value.open_close.polarity = 1.0;
	else y.value.open_close.polarity = -1.0;
	yb.insert(y);
}
