#include "Cleaner.h"
using namespace adevs;

const int Cleaner::in = 0;
const int Cleaner::out = 1;
const int Cleaner::status = 2;

Cleaner::Cleaner(int cookie_limit, double burn_duration):
Atomic<PortValue<double> >(),
cookie_limit(cookie_limit),
clean_duration(burn_duration)
{
	q_size = 0;
	cookie_count = 0;
	phase = WORK;
}

void Cleaner::delta_int()
{
	if (phase == WORK && q_size > 0) phase = CLEAR;
	else if (phase == WORK && q_size == 0) 
	{
		phase = CLEAN;
		cookie_count = 0;
	}
	else if (phase == CLEAN) phase = WORK;
}

void Cleaner::delta_ext(double e, const Bag<PortValue<double> >& xb)
{
	Bag<PortValue<double> >::const_iterator iter = xb.begin();
	for (; iter != xb.end(); iter++)
	{
		if ((*iter).port == in) q_size++;
		else if ((*iter).port == out) { q_size--; cookie_count++; }
	}
	if (q_size == 0 && phase == CLEAR)
	{
		phase = CLEAN;
		cookie_count = 0;
	}
}

void Cleaner::delta_conf(const Bag<PortValue<double> >& xb)
{
	delta_int();
	delta_ext(0.0,xb);
}

void Cleaner::output_func(Bag<PortValue<double> >& yb)
{
	if (cookie_count >= cookie_limit && phase == WORK)
	{
		PortValue<double> event(status,0.0);
		yb.insert(event);
	}
	else
	{
		PortValue<double> event(status,1.0);
		yb.insert(event);
	}
}

double Cleaner::ta()
{
	if (phase == WORK && cookie_count >= cookie_limit)
		return 0.0;
	else if (q_size == 0 && phase == CLEAN)
		return clean_duration;
	else
		return DBL_MAX;
}
