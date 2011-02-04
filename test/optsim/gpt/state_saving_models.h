#ifndef _state_saving_h_
#include "transd.h"
#include "proc.h"
#include "genr.h"

class genrWithStateSaving:
	public genr 
{
	public:

		genrWithStateSaving(double period): 
			genr(period){}
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

class procWithStateSaving:
	public proc
{
	public:
		procWithStateSaving(double proc_time):
			proc(proc_time){}
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

class transdWithStateSaving:
	public transd
{
	public:
		transdWithStateSaving(double observ_time):
			transd(observ_time){}
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

#endif
