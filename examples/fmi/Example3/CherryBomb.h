#ifndef CherryBomb_h_
#define CherryBomb_h_
#include "adevs.h"
#include "adevs_fmi.h"

class CherryBomb:
	public adevs::FMI<std::string>
{
	public:
		CherryBomb():
			adevs::FMI<std::string>
			(
				"CherryBomb",
				"{596c9dd4-1bc5-41de-9388-6a5be2ec421d}",
				"file:///home/rotten/Code/adevs-code/examples/fmi/Example3/CherryBomb/resources",
				3,
				3,
				"CherryBomb/binaries/linux64/CherryBomb.so"
			)
		{
		}
		double get_fuseTime() { return get_real(0); }
		void set_fuseTime(double val) { set_real(0,val); }
		double get_h() { return get_real(1); }
		void set_h(double val) { set_real(1,val); }
		double get_v() { return get_real(2); }
		void set_v(double val) { set_real(2,val); }
		double get_der_fuseTime_() { return get_real(3); }
		void set_der_fuseTime_(double val) { set_real(3,val); }
		double get_der_h_() { return get_real(4); }
		void set_der_h_(double val) { set_real(4,val); }
		double get_der_v_() { return get_real(5); }
		void set_der_v_(double val) { set_real(5,val); }
		double get_g() { return get_real(6); }
		void set_g(double val) { set_real(6,val); }
		bool get_dropped() { return get_bool(3); }
		void set_dropped(bool val) { set_bool(3,val); }
		bool get_exploded() { return get_bool(4); }
		void set_exploded(bool val) { set_bool(4,val); }
};

#endif