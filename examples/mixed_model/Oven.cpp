#include "Oven.h"
using namespace std;
using namespace adevs;

const int Oven::in = 0;
const int Oven::out = 1;

Oven::Oven():
Atomic<PortValue<double> >()
{
}

void Oven::delta_int()
{
	if (!q.empty()) q.pop_front();
}

void Oven::delta_ext(double e, const Bag<PortValue<double> >& xb)
{
	if (!q.empty()) q.front() = q.front()-e;
	q.push_back(0.1);
}

void Oven::delta_conf(const Bag<PortValue<double> >& xb)
{
	delta_int();
	delta_ext(0.0,xb);
}

void Oven::output_func(Bag<PortValue<double> >& yb)
{
	PortValue<double> eject_event(out,1.0);
	yb.insert(eject_event);
}
		
double Oven::ta()
{
	if (!q.empty()) return q.front();
	else return DBL_MAX;
}
