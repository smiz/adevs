#ifndef _state_saving_h_
#include "delay.h"
#include "counter.h"
#include "genr.h"
#include "gcd.h"

class genrWithStateSaving:
	public genr 
{
	public:

		genrWithStateSaving(const std::vector<double>& pattern, 
				int iterations, bool active = true):
			genr(pattern,iterations,active){}
		genrWithStateSaving(double period, int iterations, bool active = true):
			genr(period,iterations,active){}
		void beginLookahead()
		{
			chkpt = save_state();
		}
		void endLookahead()
		{
			restore_state(chkpt);
			gc_state(chkpt);
		}
	private:
		void* chkpt;	
};

class delayWithStateSaving:
	public delay
{
	public:
		delayWithStateSaving(double t):
			delay(t){}
		void beginLookahead()
		{
			chkpt = save_state();
		}
		void endLookahead()
		{
			restore_state(chkpt);
			gc_state(chkpt);
		}
	private:
		void* chkpt;
};

class counterWithStateSaving:
	public counter
{
	public:
		counterWithStateSaving():
			counter(){}
		void beginLookahead()
		{
			chkpt = save_state();
		}
		void endLookahead()
		{
			restore_state(chkpt);
			gc_state(chkpt);
		}	
	private:
		void* chkpt;	
};

class gcdWithStateSaving:
	public gcd
{
	public:
		gcdWithStateSaving(const std::vector<double>& pattern, double dt,
			int iterations, bool active = false):
		gcd(pattern,dt,iterations,active){}

		gcdWithStateSaving(double period, double dt, 
			int iterations, bool active = false):
		gcd(period,dt,iterations,active){}
};

#endif
