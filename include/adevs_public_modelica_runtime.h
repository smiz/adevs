#ifndef _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
#define _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
#include <nvector/nvector_serial.h>

class AdevsSampleData
{
	public:
		AdevsSampleData(double tStart, double tInterval);
		/**
		 * Returns true if next sample incident is within
		 * epsilon of the current time and the sampler
		 * is enabled. Returns false otherwise.
		 */
		bool atEvent(double tNow, double epsilon) const;
		// Time remaining to the next sample incident
		double timeToEvent(double tNow) const;
		/**
		 * Advance to the next sample instant if sampling
		 * is enabled.
		 */
		void update(double tNow, double epsilon);
		// Enable or disable sampling
		void setEnabled(bool enable) { enabled = enable; }
	private:
		const double tStart, tInterval;
		int n;
		bool enabled;
};

#endif
