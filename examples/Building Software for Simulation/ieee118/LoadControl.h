#ifndef LoadControl_h_
#define LoadControl_h_
#include "adevs.h"
#include "ElectricalData.h"
#include <map>

class LoadControl:
	public adevs::Atomic<adevs::PortValue<BasicEvent*> >
{
	public:
		static const int load_change;
		static const int sample_arrive;
		static const int sample_setting;

		LoadControl(ElectricalData* data, int freq_steps, double K = 5.0);
		void delta_int();
		void output_func(adevs::Bag<adevs::PortValue<BasicEvent*> >& yb);
		double ta();
		void delta_ext(double,const adevs::Bag<adevs::PortValue<BasicEvent*> >&);
		void delta_conf(const adevs::Bag<adevs::PortValue<BasicEvent*> >&);
		void gc_output(adevs::Bag<adevs::PortValue<BasicEvent*> >&);
	private:
		const double FreqTol, freq_threshold, K;
		std::map<unsigned,double> fdata;
		double adjustment;
		const double max_adjust;
		bool init, signal;
};

#endif
