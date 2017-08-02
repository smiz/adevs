#include "des.h"
using namespace std;

Partition::~Partition()
{
	while (!fel.empty())
	{
		delete fel.front();
		fel.pop_front();
	}
	while (!cel.empty())
	{
		delete cel.front();
		cel.pop_front();
	}
	for (unsigned k = 0; k < imm.size(); k++)
		delete imm[k];
}

void Partition::schedule(Event* ev)
{
	// If this is not ours it must go to its owner
	if (ev->partition() != this)
		other.push_back(ev);
	// If this is a conditional event then
	// put it into the conditional event list
	else if (ev->conditional())
		cel.push_back(ev);
	// Otherwise put this into our schedule
	else
	{
		list<Event*>::iterator iter = fel.begin();
		for(; iter != fel.end(); iter++)
		{
			if ((*iter)->timestamp() > ev->timestamp())
			{
				fel.insert(iter,ev);
				return;
			}
		}
		fel.push_back(ev);
	}
}

void Partition::delta_int()
{
	if (!imm.empty())
	{
		exec(imm);
		mode = 'C';
		for (unsigned k = 0; k < imm.size(); k++)
			delete imm[k];
		imm.clear();
	}
	else mode = 'F';
}

void Partition::delta_ext(double e, const adevs::Bag<Event*>& xb)
{
	tNow += e;
	mode = 'C';
	adevs::Bag<Event*>::const_iterator iter = xb.begin();
	for (; iter != xb.end(); iter++)
	{
		if (*iter != NULL && (*iter)->partition() == this)
			schedule(*iter);
	}
}

void Partition::delta_conf(const adevs::Bag<Event*>& xb)
{
	delta_int();
	delta_ext(0.0,xb);
}

double Partition::ta()
{
	if (mode == 'C')
		return 0.0;
	else if (fel.empty())
		return adevs_inf<double>();
	else
		return fel.front()->timestamp() - tNow;
}

void Partition::output_func(adevs::Bag<Event*>& yb)
{
	// Notify others
	for (unsigned k = 0; k < other.size(); k++)
		yb.insert(other[k]);
	other.clear();
	// Find the set of events that we will execute
	// and gather the input for those events
	if (mode == 'F' && !fel.empty())
	{
		tNow = fel.front()->timestamp();
		do
		{
			fel.front()->prep();
			imm.push_back(fel.front());
			fel.pop_front();
		}
		while (!fel.empty() && fel.front()->timestamp() == tNow);
	}
	else if (mode == 'C')
	{
		list<Event*>::iterator iter = cel.begin();
		while (iter != cel.end())
		{
			if ((*iter)->prep())
			{
				imm.push_back(*iter);
				iter = cel.erase(iter);
			}
			else iter++;
		}
	}
	// If we are going to execute an event, inform everyone
	// to scan their conditional event list in the next iteration
	if (!imm.empty() && yb.size() == 0)
		yb.insert(NULL);
}

