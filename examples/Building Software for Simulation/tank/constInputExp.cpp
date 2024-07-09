#include "SimEvents.h"
#include "Tank.h"
#include "TankEventListener.h"
using namespace std;

int main(int argc, char** argv) {
    // Get the parameters for the experiment from the command line
    if (argc != 4) {
        cout << "freq left_throttle right_throttle" << endl;
        return 0;
    }
    // Get the frequency of the voltage signal from the first argument
    double freq = atof(argv[1]);
    // Create a command from the driver that contains the duty ratios from
    // the second and third arguments.
    SimPacket sim_command;
    sim_command.left_power = atof(argv[2]);
    sim_command.right_power = atof(argv[3]);
    // Create the tank, simulator, and event listener. The arguments to the
    // tank are its initial position (x = y = 0), heading (theta = 0), and
    // the smallest interval of time that will separate any two reports of
    // the tank's state (0.02 seconds).
    Tank* tank = new Tank(freq, 0.0, 0.0, 0.0, 0.02);
    Simulator* sim = new Simulator(tank);
    TankEventListener* l = new TankEventListener(tank);
    // Add an event listener to compute the power dissipated in the motors
    sim->addEventListener(l);
    // Inject the driver command into the simulation at time zero
    ModelInputBag input;
    SimEvent cmd(sim_command);
    ModelInput event(tank, cmd);
    input.insert(event);
    sim->computeNextState(input, 0.0);
    // Run the simulation for 3 seconds
    while (sim->nextEventTime() <= 3.0) {
        sim->execNextEvent();
    }
    // Write the result to the console
    cout << freq << " " << l->getPowerLost() << endl;
    // Clean up and exit
    delete sim;
    delete tank;
    delete l;
    return 0;
}
