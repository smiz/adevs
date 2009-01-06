#ifndef _listener_h_
#define _listener_h_
#include "adevs.h"
#include "node.h"

class Listener:
	public adevs::EventListener<PortValue>
{
	public:
		Listener():
			adevs::EventListener<PortValue>()
		{
		}
		~Listener()
		{
		}
		void outputEvent(adevs::Event<PortValue> x, double t){}
		void stateChange(adevs::Atomic<PortValue>* model, double t, void* data)
		{
			node* n = dynamic_cast<node*>(model);
			assert(t == n->getTime(data));
			std::cout << n->getMessage(data);
			std::cout.flush();
		}
};

#endif
