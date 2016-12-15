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
				"{f40c3fc8-01ad-4616-bb33-d7310350db66}",
				0,
				0,
				"Control\\binaries\\win64\\Control.dll"
			)
		{
		}
		double get_qd1() { return get_real(0); }
		void set_qd1(double val) { set_real(0,val); }
		double get_qd2() { return get_real(1); }
		void set_qd2(double val) { set_real(1,val); }
		double get_xd() { return get_real(2); }
		void set_xd(double val) { set_real(2,val); }
		double get_zd() { return get_real(3); }
		void set_zd(double val) { set_real(3,val); }
		double get_L() { return get_real(4); }
		void set_L(double val) { set_real(4,val); }
		double get_l() { return get_real(5); }
		void set_l(double val) { set_real(5,val); }
		double get_pi() { return get_real(6); }
		void set_pi(double val) { set_real(6,val); }
		double get_xp() { return get_real(7); }
		void set_xp(double val) { set_real(7,val); }
};

#endif