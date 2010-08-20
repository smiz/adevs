#ifndef GenrFail_h_
#define GenrFail_h_
#include "adevs.h"
#include "ElectricalData.h"

class GenrFail:
	public adevs::Atomic<adevs::PortValue<BasicEvent*> >
{
	public:
		static const int genr_fail;

		GenrFail(unsigned which, double time_to_drop, ElectricalData* data);
		void delta_int();
		void output_func(adevs::Bag<adevs::PortValue<BasicEvent*> >& yb);
		double ta();
		void delta_ext(double,const adevs::Bag<adevs::PortValue<BasicEvent*> >&){}
		void delta_conf(const adevs::Bag<adevs::PortValue<BasicEvent*> >&){}
		void gc_output(adevs::Bag<adevs::PortValue<BasicEvent*> >&);
	private:
		unsigned which;
		double time_to_drop;
		ElectricalData* data;
};

#endif
