#include <list>
#include <iostream>
#include "adevs.h"
#include "SimpleAtomic.h"
using namespace adevs;
using namespace std;

class SimpleNetwork: public Network<SimpleIO>
{
	public:
		SimpleNetwork():
		Network<SimpleIO>()
		{
			for (int i = 0; i < 10; i++)
			{
				SimpleAtomic* model = new SimpleAtomic();
				model->setParent(this);
				models.push_back(model);
			}
		}
		void getComponents(Set<Devs<SimpleIO>*>& c)
		{
			list<Devs<SimpleIO>*>::iterator iter;
			for (iter = models.begin(); iter != models.end(); iter++)
			{
				c.insert(*iter);
			}
		}
		void route(const SimpleIO&, Devs<SimpleIO>*, Bag<Event<SimpleIO> >&){}
		bool model_transition()
		{
			models.pop_back();
			return false;
		}
		~SimpleNetwork()
		{
			list<Devs<SimpleIO>*>::iterator iter;
			for (iter = models.begin(); iter != models.end(); iter++)
			{
				delete *iter;
			}
		}
	private:
		list<Devs<SimpleIO>*> models;
};

int main()
{
	SimpleNetwork* model = new SimpleNetwork();
	Simulator<SimpleIO>* sim = new Simulator<SimpleIO>(model);
	while (sim->nextEventTime() < DBL_MAX && SimpleAtomic::atomic_number != 0)
	{
		sim->execNextEvent();
		assert(SimpleAtomic::internal_execs == SimpleAtomic::atomic_number+1);
		SimpleAtomic::internal_execs = 0;
	}
	return 0;
}
