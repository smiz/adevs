#ifndef HelloWorld_h_
#define HelloWorld_h_
#include "adevs.h"
#include "adevs_fmi.h"

class HelloWorld:
	public adevs::FMI<double>
{
	public:
		HelloWorld():
			adevs::FMI<double>
			(
				"HelloWorld",
				"{2f7d0bd5-b0c8-4093-91ea-700cd3254ebd}",
				"file:///home/rotten/Code/adevs-code/examples/fmi/Example1/HelloWorld/resources",
				1,
				0,
				"HelloWorld/binaries/linux64/HelloWorld.so"
			)
		{
		}
		double get_x() { return get_real(0); }
		void set_x(double val) { set_real(0,val); }
		double get_der_x_() { return get_real(1); }
		void set_der_x_(double val) { set_real(1,val); }
		double get_a() { return get_real(2); }
		void set_a(double val) { set_real(2,val); }
};

#endif