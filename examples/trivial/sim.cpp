/**
 * This models two nodes in a network. Node A sends 
 * a message to B, and then waits 60 seconds for
 * a reply. If it gets not reply, it reports failure.
 * If it gets the reply, it reports success. A
 * late reply is simple dropped.
 */
#include "adevs.h"
#include <iostream>
#include <cstdlib>
using namespace adevs;
using namespace std;

/**
 * The sending node.
 */
class A:
	public Atomic<string>
{
	public:
		A():
			Atomic<string>(),
			mode(SEND) // Send immediately
		{
		}
		double ta()
		{
			if (mode == SEND) return 0.0;
			else if (mode == WAIT) return 60.0;
			else return DBL_MAX;
		}
		void delta_int()
		{
			// If we send a message, then wait
			// for a replay
			if (mode == SEND)
				mode = WAIT;
			// Otherwise the replay is late
			else
			{
				cout << "No reply" << endl;
				mode = IDLE;
			}
		}
		void delta_ext(double e, const Bag<string>& xb)
		{
			cout << "Recvd " << *(xb.begin()) << endl;
			if (mode == IDLE)
				cout << "Late reply" << endl;
			else
				cout << "Reply on time" << endl;
			mode = IDLE;
		}
		void delta_conf(const Bag<string>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(Bag<string>& xb)
		{
			xb.insert("Hello?");
		}
		void gc_output(Bag<string>&){}
	private:
		enum Mode { SEND, WAIT, IDLE };
		Mode mode;
};

/**
 * The receving node.
 */
class B:
	public Atomic<string>
{
	public:
		B():
			Atomic<string>(),
			mode(IDLE)
		{
		}
		void delta_int() { mode = IDLE; }
		void delta_ext(double, const Bag<string>&)
		{
			mode = REPLY;
		}
		void delta_conf(const Bag<string>&)
		{
			mode = REPLY;
		}
		void output_func(Bag<string>& yb)
		{
			yb.insert("Hi!");
		}
		double ta()
		{
			if (mode == REPLY)
				return rand()%100;
			else
				return DBL_MAX;
		}
		void gc_output(Bag<string>&){}
	private:
		enum Mode { REPLY, IDLE };
		Mode mode;
};

int main(int argc, char** argv)
{
	if (argc > 1)
		srand(atoi(argv[1]));
	SimpleDigraph<string>* dig = new SimpleDigraph<string>();
	A* a = new A();
	B* b = new B();
	dig->add(a);
	dig->add(b);
	dig->couple(a,b);
	dig->couple(b,a);
	Simulator<string>* sim = new Simulator<string>(dig);
	while (sim->nextEventTime() < 200.0)
		sim->execNextEvent();
	delete sim;
	delete dig;
	return 0;
}
