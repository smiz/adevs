#ifndef __transd_h_
#define __transd_h_
#include <vector>
#include <iostream>
#include <cstdlib>
#include "adevs.h"
#include "job.h"
/*
The transducer computes various statistics about the
performance of the queuing system.  It receives new jobs
on its ariv port, finished jobs on its solved port,
and generates and output on its out port when the observation
interval has elapsed.
*/
class transd: public adevs::Atomic<PortValue>
{
	public:
		/// Constructor
		transd(double observ_time):
		adevs::Atomic<PortValue>(),
		observation_time(observ_time),
		sigma(observation_time),
		total_ta(0.0),
		t(0.0),
		finished(false)
		{
		}
		/// Internal transition function
		void delta_int() 
		{
			// Keep track of the simulation time
			t += sigma;
			// Passivate once the data collection period has ended
			finished = true;
			sigma = DBL_MAX; 
		}
		/// External transition function
		void delta_ext(double e, const adevs::Bag<PortValue>& x)
		{
			// Keep track of the simulation time
			t += e;
			// Save new jobs in order to compute statistics when they are
			// completed.
			adevs::Bag<PortValue>::iterator iter;
			for (iter = x.begin(); iter != x.end(); iter++) 
			{
				if ((*iter).port == ariv)
				{
					job j((*iter).value);
					j.t = t;
					jobs_arrived.push_back(j);
				}
			}
			// Compute time required to process completed jobs
			for (iter = x.begin(); iter != x.end(); iter++) 
			{
				if ((*iter).port == solved)
				{
					job j((*iter).value);
					std::vector<job>::iterator i = jobs_arrived.begin();
					for (; i != jobs_arrived.end(); i++)
					{
						if ((*i).id == j.id)
						{
							total_ta += t-(*i).t;
							break;
						}
					}
					j.t = t;
					jobs_solved.push_back(j);
				}
			}
			// Continue with next event time unchanged
			sigma -= e;
		}
		/// Confluent transition function
		void delta_conf(const adevs::Bag<PortValue>& x)
		{
			delta_int();
			delta_ext(0.0,x);
		}
		/// Output function
		void output_func(adevs::Bag<PortValue>& y)
		{
			/// Generate an output event to stop the generator
			job j;
			PortValue pv(out,j);
			y.insert(pv);
		}
		/// Time advance function
		double ta() { return sigma; }
		/// Garbage collection. No heap allocation in output, so do nothing
		void gc_output(adevs::Bag<PortValue>&){}
		/// Print summary data using the saved state
		void printSummary(void* state)
		{
			state_t* s;
			if (state == NULL) s = (state_t*)save_state();
			else s = (state_t*)state;
			/// Print the number of jobs if we are not done
			if (!s->finished)
			{
				std::cout << "Working @ " << s->t << std::endl;
				std::cout << "jobs arrived : " << s->jobs_arrived.size () << std::endl;
				std::cout << "jobs solved : " << s->jobs_solved.size() << std::endl;
				return;
			}
			/// Dump interesting statistics to the console
			double throughput;
			double avg_ta_time;
			if (!s->jobs_solved.empty()) 
			{
				avg_ta_time = s->total_ta / s->jobs_solved.size();
				if (s->t > 0.0)
				{
					throughput = s->jobs_solved.size() / s->t;
				}
				else 
				{
					throughput = 0.0;
				}
			}
			else 
			{
				avg_ta_time = 0.0;
				throughput = 0.0;
			}
			std::cout << "End time: " << s->t << std::endl;
			std::cout << "jobs arrived : " << s->jobs_arrived.size () << std::endl;
			std::cout << "jobs solved : " << s->jobs_solved.size() << std::endl;
			std::cout << "AVERAGE TA = " << avg_ta_time << std::endl;
			std::cout << "THROUGHPUT = " << throughput << std::endl;
			if (state == NULL) gc_state(s);
		}
		/// Destructor
		~transd()
		{
			jobs_arrived.clear();
			jobs_solved.clear();
		}
		/// Model input port
		static const int ariv;
		static const int solved;
		/// Model output port
		static const int out;

		void* save_state()
		{
			state_t* state = new state_t();
			state->jobs_arrived = jobs_arrived;
			state->jobs_solved = jobs_solved;
			state->observation_time = observation_time;
			state->sigma = sigma;
			state->total_ta = total_ta;
			state->t = t;
			state->finished = finished;
			return state;
		}
		void restore_state(void* data)
		{
			state_t* state = (state_t*)data;
			jobs_arrived = state->jobs_arrived;
			jobs_solved = state->jobs_solved;
			observation_time = state->observation_time;
			sigma = state->sigma;
			total_ta = state->total_ta;
			t = state->t;
			finished = state->finished;
		}
		void gc_state(void* data)
		{
			delete (state_t*)data;
		}

	private:	
		/// Model state variables
		std::vector<job> jobs_arrived;
		std::vector<job> jobs_solved; 
		double observation_time, sigma, total_ta, t;
		bool finished;
		struct state_t
		{
			std::vector<job> jobs_arrived;
			std::vector<job> jobs_solved; 
			double observation_time, sigma, total_ta, t;
			bool finished;
		};
};

/// Assign unique 'names' to ports
const int transd::ariv(0);
const int transd::solved(1);
const int transd::out(2);

#endif
