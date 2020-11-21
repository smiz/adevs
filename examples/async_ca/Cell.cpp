#include "Cell.h"
using namespace std;
using namespace adevs;

bool* Cell::vis;
unsigned Cell::rule;
int Cell::length;

void Cell::setParams(bool* vis, unsigned rule, int length)
{
	Cell::vis = vis;
	Cell::rule = rule;
	Cell::length = length;
}

Cell::Cell(int position, const bool* const state, double h):
	adevs::Atomic<CellEvent<value_t> >(),
	pos(position),
	left((pos == 0) ? length-1 : pos-1),
	right((pos == length-1) ? 0 : pos+1),
	h(h),
	q(state[pos]),
	dt(h)
{
	n[0] = state[left];
	n[1] = state[right];
}

double Cell::ta() { return dt; }

void Cell::delta_int() 
{
	q = vis[pos];
	dt = h;
}

void Cell::delta_ext(double e, const adevs::Bag<CellEvent<value_t> >& xb) 
{
	dt -= e;
	for (auto iter = xb.begin(); iter != xb.end(); iter++)
	{
		if ((*iter).value.pos == left) n[0] = (*iter).value.value;
		else if ((*iter).value.pos == right) n[1] = (*iter).value.value;
	}
}

void Cell::delta_conf(const adevs::Bag<CellEvent<value_t> >& xb) 
{
	delta_int();
	delta_ext(0.0,xb);
}

void Cell::output_func(adevs::Bag<CellEvent<value_t> >& yb) 
{
	CellEvent<value_t> y;
	y.y = y.z = 0;
	y.value.pos = pos;
	char bits = (n[0]) | (q << 1) | (n[1] << 2);
	vis[pos] = y.value.value = (rule >> bits) & 0x01;
	y.x = left;
	yb.insert(y);
	y.x = right;
	yb.insert(y);
}
