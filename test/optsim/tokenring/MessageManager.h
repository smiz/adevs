#ifndef _manager_h_
#define _manager_h_
#include "adevs.h"
#include "node.h"

class PortValueMessageManager:
	public adevs::MessageManager<PortValue>
{
	public:
		PortValue clone(PortValue& pv)
		{
		   PortValue copy(pv);
		   copy.value = new token_t(*(copy.value));
		   return copy;
		}
		void destroy(PortValue& pv)
		{
			delete pv.value;
		}
};

#endif

