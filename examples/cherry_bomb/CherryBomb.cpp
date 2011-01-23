#include "adevs.h"
#include <iostream>
using namespace std;
using namespace adevs;

// Array indices for the CherryBomb state variables
#define H 0
#define V 1
#define T 2
// Discrete variable enumeration for the CherryBomb
typedef enum { FUSE_LIT, DOUSE, EXPLODE } Phase;

class CherryBomb: public ode_system<string> {
	public:
		CherryBomb():ode_system<string>(
				3, // three state variables including time
				1 // 1 state event condition
				) {
			phase = FUSE_LIT; // Light the fuse!
		}
		void init(double *q) {
			q[H] = 1.0; // Initial height
			q[V] = 0.0; // Initial velocity
			q[T] = 0.0; // Start time at zero
		}
		void der_func(const double* q, double* dq) {
			dq[V] = -9.8; 
			dq[H] = q[V]; 
			dq[T] = 1.0; 
		}
		void state_event_func(const double* q, double *z) {
			// Test for hitting the ground. 
			if (q[V] < 0.0) return q[H];
			else z[0] = 1.0;
		}
		double time_event_func(const double* q) {
			if (q[T] < 2.0) return 2.0 - q[T]; // Explode at time 2
			else return DBL_MAX; // Don't do anything after that
		}
		void external_event(double* q, double e, const Bag<string>& xb) {
			phase = DOUSE; // Any input is a douse event
		}
		void internal_event(double* q, const bool* state_event) {
			if (state_event[0]) q[V] = -q[V]; // Bounce!
			if (state_event[1]) phase = EXPLODE;
		}
		void confluent_event(double* q, const Bag<string>& xb,
			const bool* state_event) {
			internal_event(q,state_event);
			external_event(q,0.0,xb);
		}
		void output_func(const double *q, const bool* state_event,
				Bag<string>& yb) {
			if (stat_event[1] && phase == FUSE_LIT)
				yb.insert("BOOM!"); // Explode!
		}
		void postStep(const double* q) {
			// Write the current state to std out
			cout << q[T] << " " << q[H] << " " << q[V] << " " << phase << endl;
		}
		// No garbage collection is needed
		void gc_output(Bag<string>&){}
		// Get the current value of the discrete variable
		Phase getPhase() { return phase; } 
	private:
		Phase phase;
};

int main() {
	CherryBomb* bomb = new CherryBomb();
	Simulator<string>* sim = new Simulator<string>(bomb);
	while (bomb->getPhase() == FUSE_LIT)
		sim->execNextEvent();
	delete sim; delete bomb;
	return 0;
}

