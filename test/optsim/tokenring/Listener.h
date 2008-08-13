#ifndef _listener_h_
#define _listener_h_
#include "adevs.h"
#include "node.h"

class Listener:
	public adevs::EventListener<PortValue>
{
	public:
		Listener():adevs::EventListener<PortValue>(){}
		void outputEvent(adevs::Event<PortValue> x, double t){}
		void stateChange(adevs::Atomic<PortValue>* model, double t, void* data)
		{
			assert(t == node::getTime(data));
			std::cout << node::getMessage(data);
			std::cout.flush();
		}
};

#endif
