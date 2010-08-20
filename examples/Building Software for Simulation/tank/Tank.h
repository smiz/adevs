#ifndef Tank_h_
#define Tank_h_
#include "Computer.h"
#include "TankPhysics.h"

// This is the complete model of the tank with the TankPhysics and Computer
class Tank:
	public adevs::Network<SimEvent>
{
	public:
		// Tank has an interrupt handler with the specified frequency,
		// starts at the specified position, and generates events for
		// the display at the given interval
		Tank(double freq, double x0, double y0, double theta0, double cint);
		// Get the components of the computer
		void getComponents(adevs::Set<adevs::Devs<SimEvent>* > &c);
		// Route events within the computer
		void route(const SimEvent& value, adevs::Devs<SimEvent>* model,
				adevs::Bag<adevs::Event<SimEvent> > &r);
		// Get the physics model
		const TankPhysics* getPhysics() const { return &physics; }
		// Get the computer
		const Computer* getComputer() const { return &computer; }
	private:
		Computer computer;
		TankPhysics physics;
};

#endif
