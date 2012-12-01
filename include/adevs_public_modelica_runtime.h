#ifndef _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
#define _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
#include <nvector/nvector_serial.h>

class AdevsSampleData
{
	public:
		AdevsSampleData(double tStart, double tInterval);
		bool atEvent(double tNow, double epsilon) const;
		double timeToEvent(double tNow) const;
		void update(double tNow, double epsilon);
	private:
		const double tStart, tInterval;
		int n;
};

#endif
