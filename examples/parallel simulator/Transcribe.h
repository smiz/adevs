#ifndef _transcribe_h_
#define _transcribe_h_
#include "adevs.h"

/**
 * This model copies its input to its output following a
 * delay.
 */
class Transcribe:
	public adevs::Atomic<char>
{
	public:
		Transcribe():adevs::Atomic<char>(),ttg(DBL_MAX),to_transcribe(' '){}
		void delta_int() { ttg = DBL_MAX; }
		void delta_ext(double e, const adevs::Bag<char>& xb)
		{
			if (ttg == DBL_MAX)
			{
				ttg = 1.0;
				// Find the largest input value
				adevs::Bag<char>::const_iterator iter = xb.begin();
				to_transcribe = *iter;
				for (; iter != xb.end(); iter++)
				{
					if (to_transcribe < *iter)  to_transcribe = *iter;
				}
			}
			else ttg -= e;
		}
		void delta_conf(const adevs::Bag<char>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(adevs::Bag<char>& yb)
		{
			yb.insert(to_transcribe);
		}
		double ta() { return ttg; }
		void gc_output(adevs::Bag<char>&){}
		double lookahead() { return 1.0; }
		void beginLookahead()
		{
			// Save the state
			chkpt.ttg = ttg;
			chkpt.to_transcribe = to_transcribe;
		} 
		void endLookahead()
		{
			// Restore the state
			ttg = chkpt.ttg;
			to_transcribe = chkpt.to_transcribe;
		} 
		char getMemory() const { return to_transcribe; }
	private:
		double ttg;
		char to_transcribe;
		struct checkpoint_t { double ttg; char to_transcribe; };
		checkpoint_t chkpt;
};

#endif
			

