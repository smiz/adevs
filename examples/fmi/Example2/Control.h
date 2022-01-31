#ifndef Control_h_
#define Control_h_
#include "adevs.h"
#include "adevs_fmi.h"

class Control:
	public adevs::FMI<IO_Type>
{
	public:
		Control():
			adevs::FMI<IO_Type>
			(
				"Control",
				"{420b618a-2167-443c-a7fa-f31fc2a92746}",
				"file:///home/1qn/Code/adevs-code/examples/fmi/Example2/Control/resources",
				0,
				0,
				"Control/binaries/linux64/Control.so"
			)
		{
		}
		double get_qd1() { return get_real(6); }
		void set_qd1(double val) { set_real(6,val); }
		double get_qd2() { return get_real(7); }
		void set_qd2(double val) { set_real(7,val); }
		double get_xd() { return get_real(8); }
		void set_xd(double val) { set_real(8,val); }
		double get_zd() { return get_real(9); }
		void set_zd(double val) { set_real(9,val); }
		double get_L() { return get_real(10); }
		void set_L(double val) { set_real(10,val); }
		double get_l() { return get_real(11); }
		void set_l(double val) { set_real(11,val); }
		double get_pi() { return get_real(12); }
		void set_pi(double val) { set_real(12,val); }
		double get_xp() { return get_real(13); }
		void set_xp(double val) { set_real(13,val); }
};

#endif