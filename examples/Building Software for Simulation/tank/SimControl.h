#ifndef _Sim_Control_h_
#define _Sim_Control_h_
#include "Display.h"
#include "adevs.h"
#include "Tank.h"
#include "CommandPacket.h"
#include "SimEventListener.h"
#include "cc++/socket.h"
/**
 * This is the main simulation control class.
 * It coordinates all of the other activities
 * in the simulation.
 */
class SimControl:
	public DisplayEventListener,
	public SimEventListener
{
	public:
		/**
		 * The constructor will throw any exceptions thrown
		 * by the Display or Simulator when it builds them.
		 */
		SimControl();
		/**
		 * Run until the user exits. This will return when
		 * the simulation is over.
		 */
		void run();
		/**
		 * Destructor.
		 */
		~SimControl();
		/// Listen for request from the user to reset the simulation
		void reset();
		/// Listen for a request from the user to quit the simulation
		void quit();
		/// Set the frequency of the PWM signal
		void increaseFreq();
		void decreaseFreq();
		/// Listen to output events from the tank model
		void outputEvent(ModelOutput x, double t);
		/// Listen to state changes in the tank model
		void stateChange(AtomicModel* model, double t){}
	private:
		bool has_quit;
		Display display;
		double freq;
		Tank* tank;
		Simulator* sim;
		unsigned int tzero;
		ost::IPV4Address addr;
		ost::UDPSocket sock;

		void resetTank();
};

#endif
