#include <list>
#include <iostream>
#include "adevs.h"
#include "SimpleAtomic.h"
using namespace adevs;
using namespace std;

class SimpleNetwork: public Network<PortValue<char> >
{
	public:
		SimpleNetwork(SimpleNetwork** networks, int id):
		Network<PortValue<char> >(),
		networks(networks),
		id(id)
		{
			networks[id] = this;
			if (id == 0) 
			{
				model = new SimpleAtomic();
				model->setParent(this);
			}
			else model = NULL;
		}
		void getComponents(Set<Devs<PortValue<char> >*>& c)
		{
			if (model != NULL) c.insert(model);
		}
		void route(const PortValue<char>&, Devs<PortValue<char> >*, Bag<Event<PortValue<char> > >&)
		{
		}
		bool model_transition()
		{
			if (model != NULL)
			{
				int next = (id+1)%2;
				networks[next]->model = model;
				model->setParent(networks[next]);
				model = NULL;
				return true;
			}
			else return false;
		}
		~SimpleNetwork()
		{
			if (model != NULL) delete model;
		}
	private:
		Devs<PortValue<char> >* model;
		SimpleNetwork** networks;
		int id;
};

int main()
{
	Digraph<char> top_model;
	SimpleNetwork* networks[2];
	SimpleNetwork* model = new SimpleNetwork(networks,0);
	top_model.add(model);
	model = new SimpleNetwork(networks,1);
	top_model.add(model);
	Simulator<PortValue<char> >* sim = new Simulator<PortValue<char> >(&top_model);
	while (sim->nextEventTime() < 10.0)
	{
		assert(SimpleAtomic::atomic_number == 1);
		sim->execNextEvent();
		assert(SimpleAtomic::internal_execs == 1);
		SimpleAtomic::internal_execs = 0;
	}
	return 0;
}
