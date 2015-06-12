#ifndef CherryBomb_h_
#define CherryBomb_h_
#include "adevs.h"
#include "adevs_fmi.h"

class CherryBomb:
	// Derive model from the adevs FMI class
	public adevs::FMI<std::string>
{
	public:
		// Constructor loads the FMI
		CherryBomb():
			// Call FMI constructor
			adevs::FMI<std::string>
			(
			 	"CherryBomb", // model name from modelDescription.xml
		 		"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}", // GUID from modelDescription.xml
				3, // Number of derivative variables
				3, // numberOfEventIndicators from modelDescription.xml
				"binaries/linux64/CherryBomb.so" // Location of the shared object file produced by omc
			)
		{
		}
		double get_fuseTime() { return get_real(0); }
		void set_fuseTime(double val) { set_real(0,val); }
		double get_h() { return get_real(1); }
		void set_h(double val) { set_real(1,val); }
		double get_v() { return get_real(2); }
		void set_v(double val) { set_real(2,val); }
		double get_der_fuseTime() { return get_real(3); }
		void set_der_fuseTime(double val) { set_real(3,val); }
		double get_der_h() { return get_real(4); }
		void set_der_h(double val) { set_real(4,val); }
		double get_der_v() { return get_real(5); }
		void set_der_v(double val) { set_real(5,val); }
		double get_g() { return get_real(6); }
		void set_g(double val) { set_real(6,val); }
		bool get__D_whenCondition1() { return get_bool(0); }
		void set__D_whenCondition1(bool val) { set_bool(0,val); }
		bool get__D_whenCondition2() { return get_bool(1); }
		void set__D_whenCondition2(bool val) { set_bool(1,val); }
		bool get_dropped() { return get_bool(2); }
		void set_dropped(bool val) { set_bool(2,val); }
		bool get_exploded() { return get_bool(3); }
		void set_exploded(bool val) { set_bool(3,val); }
};

#endif
