#ifndef _genr_h_
#define _genr_h_
#include "adevs.h"

/**
 * This model produces periodic output until it receives an
 * input. At that time it stops.
 */
class Genr:
	public adevs::Atomic<char>
{
	public:
		Genr():
			adevs::Atomic<char>(),
			running(true),
			letter(0),
			letter_count(26)
			{
				letter_array = new char[letter_count];
				letter_array[0] = 'A';
				for (int i = 1; i < letter_count; i++)
					letter_array[i] = letter_array[i-1]+1;
			}
		void delta_int() { letter = (letter+1) % letter_count; }
		void delta_ext(double e, const adevs::Bag<char>& xb)
		{
			running = false;
		}
		void delta_conf(const adevs::Bag<char>& xb)
		{
			running = false;
		}
		void output_func(adevs::Bag<char>& yb)
		{
			yb.insert(letter_array[letter]);
		}
		double ta() { if (running) return 0.5; else return DBL_MAX; }
		void gc_output(adevs::Bag<char>&){}
		void beginLookahead()
		{
			// Save the state
			chkpt.letter = letter;
			chkpt.running = running;
		} 
		void endLookahead()
		{
			// Restore the state
			letter = chkpt.letter;
			running = chkpt.running;
		} 
		char getNextOutput() const { return letter_array[letter]; }
		bool isRunning() const { return running; }
		~Genr() { delete [] letter_array; }
	private:
		bool running;
		int letter;
		const int letter_count;
		char* letter_array;
		struct checkpoint_t{ bool running; int letter; };
		checkpoint_t chkpt;
};

#endif
			

