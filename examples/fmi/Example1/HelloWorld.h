#ifndef HelloWorld_h_
#define HelloWorld_h_
#include "adevs.h"
#include "adevs_fmi.h"

class HelloWorld:
	// Derive model from the adevs FMI class
	public adevs::FMI<double>
{
	public:
		// Constructor loads the FMI
		HelloWorld():
			// Call FMI constructor
			FMI<double>
			(
			 	"HelloWorld", // model name from modelDescription.xml
		 		"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}", // GUID from modelDescription.xml
				1, // Number of derivative variables
				0, // numberOfEventIndicators from modelDescription.xml
				"binaries/linux64/HelloWorld.so" // Location of the shared object file produced by omc
			)
		{
		}
		// Get/set the real variable named x, which has index 0
		double get_x() { return get_real(0); }
		void set_x(double val) { set_real(0,val); }
		// Get/set the real variable named der(x), which has index 1
		double get_der_x() { return get_real(1); }
		void set_der_x(double val) { set_real(1,val); }
		// Get/set the real variable named a, which has index 2
		double get_a() { return get_real(2); }
		void set_a(double val) { set_real(2,val); }
};

#endif
