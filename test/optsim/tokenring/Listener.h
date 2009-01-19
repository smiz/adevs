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
		void stateChange(adevs::Atomic<PortValue>* model, double t)
		{
			node* n = dynamic_cast<node*>(model);
			assert(t == n->getTime());
			#pragma omp critical
			{
				std::cout << n->getMessage();
				std::cout.flush();
			}
		}
};

#endif
