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

class CherryBomb: public rk45<string> {
	public:
		CherryBomb():rk45<string>(
				3, // three state variables including time
				0.01, // maximum time step
				0.001, // error tolerance for one integration step
				1 // 1 state event condition
				) {
			init(H,1.0); // Initial height
			init(V,0.0); // Initial velocity
			init(T,0.0); // Start time at zero
			phase = FUSE_LIT; // Light the fuse!
		}
		void der_func(const double* q, double* dq) {
			dq[V] = -9.8; // Equation 5.1
			dq[H] = q[V]; // Equation 5.2
			dq[T] = 1.0; // Equation 5.3
		}
		void state_event_func(const double* q, double *z) {
			// Test condition 5.4. The test uses h <= 0 instead of h = 0 to avoid
			// a problem if h, which is a floating point number, is not exactly 0.
			// For instance, it might be computed at 1E-32 which is close enough.
			if (q[H] <= 0.0 && q[V] < 0.0) z[0] = 1.0;
			else z[0] = -1.0;
		}
		double time_event_func(const double* q) {
			if (q[T] < 2.0) return 2.0 - q[T]; // Explode at time 2
			else return DBL_MAX; // Don't do anything after that
		}
		void discrete_action(double* q, const Bag<string>& xb) {
			if (xb.size() > 0 && phase == FUSE_LIT) phase = DOUSE; // Any input is a douse event
			else if (q[T] >= 2.0 && phase == FUSE_LIT) phase = EXPLODE; // Explode at time 2
			if (q[H] <= 0.0) q[V] = -q[V]; // Bounce
		}
		void discrete_output(const double *q, Bag<string>& yb) {
			if (q[T] >= 2.0 && phase == FUSE_LIT) yb.insert("BOOM!"); // Explode!
		}
		void state_changed(const double* q) {
			// Write the current state to std out
			cout << q[T] << " " << q[H] << " " << q[V] << " " << phase << endl;
		}
		void gc_output(Bag<string>&){} // No garbage collection is needed
		Phase getPhase() { return phase; } // Get the current value of the discrete variable
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



