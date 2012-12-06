#ifndef _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
#define _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
#include <nvector/nvector_serial.h>
#include <list>

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

class AdevsDelayData
{
	public:
		/**
		 * Store a trajectory of at most length
		 * maxDelay.
		 */
		AdevsDelayData(double maxDelay);
		/**
		 * Sample the trajectory at the point t.
		 * A point must be added to the trajectory before
		 * this method is called.
		 */
		double sample(double t);
		/**
		 * Add a point to the trajectory.
		 */
		void insert(double t, double v);
		/**
		 * Get the maximum delay for this trajectory.
		 */
		double getMaxDelay() const { return maxDelay; } 
		/**
		 * Does this data have enough points to calculate
		 * a value.
		 */
		bool isEnabled() const { return !traj.empty(); }
	private:
		struct point_t { double t, v; };
		const double maxDelay;
		std::list<point_t> traj;
};
#endif
