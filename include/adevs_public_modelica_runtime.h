/**
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */
#ifndef _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
#define _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
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
		~AdevsDelayData();
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
		bool isEnabled() const { return num_els != 0; }
	private:
		struct point_t { double t, v; };
		const double maxDelay;
		point_t* traj;
		int start, num_els, size;

		int get_index(int i) const { return (start+i)%size; }
};

class AdevsMathEventFunc
{
	public:
		AdevsMathEventFunc(double eps):init(true),eps(eps){}
		virtual double calcValue(double expr) = 0;
		/**
		 * Zero crossing functions are positive if
		 * their is no event and negative of expr
		 * has crossed the event threshold.
		 */
		virtual double getZUp(double expr) = 0;
		virtual double getZDown(double expr) = 0;
		void setInit(bool inInit) { init = inInit; }
		bool isInInit() const { return init; }
		virtual ~AdevsMathEventFunc(){}
	private:
		bool init;
	protected:
		const double eps;
};

#endif
