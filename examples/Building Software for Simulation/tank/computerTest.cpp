#include "Computer.h"
#include "SimEvents.h"
#include <fstream>
using namespace std;
using namespace adevs;

class ComputerListener: public EventListener<SimEvent>
{
	public:
		ComputerListener(const Computer* computer):
			EventListener<SimEvent>(),computer(computer),
			vout("voltage.dat")
		{
			vout << 0 << " " << 0 << " " << 0 << endl; // Print volts @ t=0
		}
		void outputEvent(Event<SimEvent> y, double t)
		{
			if (y.model == computer) {
				SimMotorVoltage event = y.value.simMotorVoltage();
				vout << t << " " << event.el << " " << event.er << endl;
			}
		}
		void stateChange(Atomic<SimEvent>*,double){}
	private:
		const Computer* computer;
		ofstream vout;
};

int main(int argc, char** argv)
{
	// Get the parameters for the experiment from the command line
	if (argc != 4) {
		cout << "freq left_throttle right_throttle" << endl;
		return 0;
	}
	// Get the frequency of the voltage signal from the first argument
	double freq = atof(argv[1]);
	// Create a command from the driver that contains the duty ratios and
	// directions.
	SimPacket sim_command;
	sim_command.left_power = atof(argv[2]);
	sim_command.right_power = atof(argv[3]);
	// Create computer, simulator, and event listener. 
	Computer* computer = new Computer(freq);
	Simulator<SimEvent>* sim = new Simulator<SimEvent>(computer);
	ComputerListener* l = new ComputerListener(computer);
	// Add an event listener to plot the voltage signals
	sim->addEventListener(l);
	// Inject the driver command into the simulation at time 0
	Bag<Event<SimEvent> > input;
	SimEvent cmd(sim_command);
	Event<SimEvent> event(computer,cmd);
	input.insert(event);
	sim->computeNextState(input,0.0);
	// Run the simulation 
	while (sim->nextEventTime() <= 0.004)
		sim->execNextEvent();
	// Clean up and exit
	delete sim; delete computer; delete l;
	return 0;
}
