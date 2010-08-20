#include "SimControl.h"
#include <iostream>
#include <sys/time.h>
using namespace std;

// Return time in milliseconds
static unsigned long getTime()
{
	timeval t;
	gettimeofday(&t,NULL);
	return t.tv_sec*1000+ldiv(t.tv_usec,1000).quot;
}

SimControl::SimControl():
	has_quit(false),
	display(),
	freq(3000.0),
	// Open a socket to receive data from the operator
	addr("localhost"),
	sock(addr,10000)
{
	tank = NULL;
	sim = NULL;
	resetTank();
	display.addEventListener(this);
}

void SimControl::run()
{
	// Create a buffer to hold the command data
	char* buffer = new char[CommandPacket::getPacketSize()];
	// Input events for the simulator
	ModelInputBag input;
	// Run until the user quits
	while (!has_quit)
	{
		// Draw the current world state
		display.draw();
		// Process any pending window events
		display.pollEvents();
		// Look for input on the socket
		if (sock.isPending(ost::Socket::pendingInput,0))
		{
			// Get the packet
			sock.receive(buffer,CommandPacket::getPacketSize());
			CommandPacket command(buffer);
			SimPacket sim_command;
			sim_command.left_power = command.getLeftThrottle();
			sim_command.right_power = command.getRightThrottle();
			SimEvent cmd(sim_command);
			ModelInput event(tank,cmd);
			input.insert(event);
		}
		// Advance the simulator to the current time
		unsigned long t_start = getTime();
		unsigned long t = getTime();
		while (sim->nextEventTime()*1000.0 <= t-tzero)
		{
			sim->execNextEvent();
			t = getTime();
			if (t-t_start > 1000)
			{
				cerr << "Can not run fast enough" << endl;
				break;
			}
		}
		// Apply any input
		if (!input.empty()) 
		{
			double tin = (double)(t-tzero)/1000.0;
			if (tin > sim->nextEventTime()) tin = sim->nextEventTime();
			sim->computeNextState(input,tin);
			input.clear();
		}
	}
	// Clean up and return
	delete [] buffer;
}

void SimControl::resetTank()
{
	double x = 0.0;
	double y = 0.0;
	double theta = 0.0;
	if (sim != NULL) delete sim;
	if (tank != NULL) delete tank;
	tank = new Tank(freq,x,y,theta,0.02);
	sim = new Simulator(tank);
	sim->addEventListener(this);
	display.clearStoredTraj();
	display.setTankState(x,y,theta);
	// Get the simulation start time
	tzero = getTime();
}

SimControl::~SimControl()
{
}

void SimControl::reset()
{
	resetTank();
}

void SimControl::quit()
{
	has_quit = true;
}

void SimControl::increaseFreq()
{
	freq += 100.0;
	cout << "Freq = " << freq << endl;
}

void SimControl::decreaseFreq()
{
	freq -= 100.0;
	cout << "Freq = " << freq << endl;
}

void SimControl::outputEvent(ModelOutput event, double t)
{
	if (event.model == tank && event.value.getType() == SIM_TANK_POSITION)
	{
		double x = event.value.simTankPosition().x;
		double y = event.value.simTankPosition().y;
		double theta = event.value.simTankPosition().theta;
		display.setTankState(x,y,theta);
	}
}
