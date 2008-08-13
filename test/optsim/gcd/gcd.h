#ifndef __gcd_h_
#define __gcd_h_
#include "genr.h"
#include "counter.h"
#include "delay.h"
#include "adevs.h"

class gcd: public adevs::Digraph<object*>
{
	public:
		static const int in;
		static const int out;
		static const int signal;
		static const int start;
		static const int stop;

		gcd(const std::vector<double>& pattern, double dt,
		int iterations, bool active = false):
		adevs::Digraph<object*>()
		{
			genr* g = new genr(pattern,iterations,active);
			delay* d = new delay(dt);
			counter* c = new counter;
			build(g,c,d);
		}
		gcd(double period, double dt, 
		int iterations, bool active = false):
		adevs::Digraph<object*>()
		{
			genr* g = new genr(period,iterations,active);
			delay* d = new delay(dt);
			counter* c = new counter;
			build(g,c,d);
		}
		~gcd(){}
	private:
		void build(genr* g, counter* c, delay* d) 
		{
			add(g);
			add(d);
			add(c);
			couple(this,in,d,d->in);
			couple(this,start,g,g->start);
			couple(this,stop,g,g->stop);
			couple(g,g->signal,this,signal);
			couple(d,d->out,this,out);
			couple(d,d->out,c,c->in);
		}
};

const int gcd::in(0);
const int gcd::out(1);
const int gcd::start(2);
const int gcd::stop(3);
const int gcd::signal(4);

#endif
