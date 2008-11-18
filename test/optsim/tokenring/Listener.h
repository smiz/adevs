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
			omp_init_lock(&lock);
		}
		~Listener()
		{
			omp_destroy_lock(&lock);
		}
		void outputEvent(adevs::Event<PortValue> x, double t){}
		void stateChange(adevs::Atomic<PortValue>* model, double t, void* data)
		{
			omp_set_lock(&lock);
			node* n = dynamic_cast<node*>(model);
			assert(t == n->getTime(data));
			std::cout << n->getMessage(data);
			std::cout.flush();
			omp_unset_lock(&lock);
		}
	private:
		omp_lock_t lock;
};

#endif
