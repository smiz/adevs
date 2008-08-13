#include "LengthMonitor.h"
using namespace adevs;

const int LengthMonitor::in = 0;
const int LengthMonitor::out = 1;
const int LengthMonitor::status = 2;

LengthMonitor::LengthMonitor(int queue_limit):
Atomic<PortValue<double> >(),
max_q_size(queue_limit)
{
	queue_size = 0;
	phase = NOTFULL;
}

void LengthMonitor::delta_int()
{
	if (queue_size == max_q_size) phase = FULL;
	else phase = NOTFULL;
}

void LengthMonitor::delta_ext(double e, const Bag<PortValue<double> >& xb)
{
	Bag<PortValue<double> >::const_iterator iter;
	for (iter = xb.begin(); iter != xb.end(); iter++)
	{
		if ((*iter).port == in) queue_size++;
		else if ((*iter).port == out) queue_size--; 
		assert(queue_size <= max_q_size);
	}
	if (queue_size == max_q_size && phase == NOTFULL) phase = SWITCH;
	else if (queue_size < max_q_size && phase == FULL) phase = SWITCH;
}

void LengthMonitor::delta_conf(const Bag<PortValue<double> >& xb)
{
	delta_int();
	delta_ext(0.0,xb);
}

void LengthMonitor::output_func(Bag<PortValue<double> >& yb)
{
	if (queue_size == max_q_size) 
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

double LengthMonitor::ta()
{
	if (phase == SWITCH) return 0.0;
	return DBL_MAX;
}
