#ifndef Computer_h_
#define Computer_h_
#include "PacketProcessing.h"
#include "InterruptHandler.h"

// This is a model of the computer. It contains an InterruptHandler
// and a PacketProcessing model.
class Computer:
	public adevs::Network<SimEvent>
{
	public:
		// Create a computer whose interrupt handler has the
		// specified frequency.
		Computer(double freq);
		// Get the components of the computer
		void getComponents(adevs::Set<adevs::Devs<SimEvent>* > &c);
		// Route events within the computer
		void route(const SimEvent& value, adevs::Devs<SimEvent>* model,
				adevs::Bag<adevs::Event<SimEvent> > &r);
	private:
		PacketProcessing p;
		InterruptHandler i;
};

#endif
