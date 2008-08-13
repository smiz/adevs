#include "OvenControl.h"
#include "adevs.h"
using namespace adevs;

const int OvenControl::q_status = 0;
const int OvenControl::cleaner_status = 1;
const int OvenControl::status = 2;
		
OvenControl::OvenControl():
Atomic<PortValue<double> >()
{
	oven[0] = oven[1] = WORK;
	q[0] = q[1] = NOTFULL;
}

void OvenControl::delta_int()
{
	q[0] = q[1];
	oven[0] = oven[1];
}

void OvenControl::delta_ext(double e, const Bag<PortValue<double> >& xb)
{
	q[0] = q[1];
	oven[0] = oven[1];
	Bag<PortValue<double> >::const_iterator iter;
	for (iter = xb.begin(); iter != xb.end(); iter++)
	{
		if ((*iter).port == q_status && (*iter).value == 0.0) q[1] = FULL;
		else if ((*iter).port == q_status && (*iter).value == 1.0) q[1] = NOTFULL;
		else if ((*iter).port == cleaner_status && (*iter).value == 0.0) oven[1] = CLEAN;
		else if ((*iter).port == cleaner_status && (*iter).value == 1.0) oven[1] = WORK;
	}
}

void OvenControl::delta_conf(const Bag<PortValue<double> >& xb)
{
	delta_int();
	delta_ext(0.0,xb);
}

void OvenControl::output_func(Bag<PortValue<double> >& yb)
{
	if (
		(oven[1] == CLEAN && oven[0] == WORK && q[0] == NOTFULL) ||
		(q[1] == FULL && q[0] == NOTFULL && oven[0] == WORK)
		)
	{
		PortValue<double> event(status,0.0);
		yb.insert(event);
		return;
	}
	else if (
		(oven[1] == WORK && oven[0] == CLEAN) ||
		(q[1] == NOTFULL && q[0] == FULL && oven[1] == WORK)
		)
	{
		PortValue<double> event(status,1.0);
		yb.insert(event);
		return;
	}
}	

double OvenControl::ta()
{
	if (oven[1] != oven[0] || q[1] != q[0]) return 0.0;
	return DBL_MAX;
}
