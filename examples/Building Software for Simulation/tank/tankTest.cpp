#include <fstream>
#include "SimEvents.h"
#include "Tank.h"
using namespace std;
using namespace adevs;

class TankListener : public EventListener<SimEvent> {
  public:
    TankListener(Tank const* tank)
        : EventListener<SimEvent>(),
          tank(tank),
          foutc("computer.dat"),
          foutt("tank.dat") {
        foutc << "#t el er" << endl;
        foutc << "0 0 0" << endl;
        foutt << "#t T v w turning theta wl wr" << endl;
        foutt << "0 0 0 0 0 0 0 0" << endl;
    }
    void outputEvent(Event<SimEvent> y, double t) {
        // Output from the computer
        if (y.model == tank->getComputer()) {
            SimMotorVoltage event = y.value.simMotorVoltage();
            foutc << t << " " << event.el << " " << event.er << endl;
        }
    }
    void stateChange(Atomic<SimEvent>* model, double t) {
        // Change in the state of the tank
        if (model == tank->getPhysics()) {
            foutt << t << " " << tank->getPhysics()->getTorque() << " "
                  << tank->getPhysics()->getSpeed() << " "
                  << tank->getPhysics()->getTurnSpeed() << " "
                  << tank->getPhysics()->isTurning() << " "
                  << tank->getPhysics()->getHeading() << " "
                  << tank->getPhysics()->leftMotorSpeed() << " "
                  << tank->getPhysics()->rightMotorSpeed() << endl;
        }
    }
    ~TankListener() {
        foutt.close();
        foutc.close();
    }

  private:
    Tank const* tank;
    ofstream foutc, foutt;
};

int main(int argc, char** argv) {
    // Get the parameters for the experiment from the command line
    if (argc != 5) {
        cout << "freq left_throttle right_throttle tend" << endl;
        return 0;
    }
    // Get the frequency of the voltage signal from the first argument
    double freq = atof(argv[1]);
    // Create a command from the driver that contains the duty ratios and
    // directions.
    SimPacket sim_command;
    sim_command.left_power = atof(argv[2]);
    sim_command.right_power = atof(argv[3]);
    double tend = atof(argv[4]);
    // Create tank, simulator, and event listener.
    Tank* tank = new Tank(freq, 0, 0, 0, 0.1);
    Simulator<SimEvent>* sim = new Simulator<SimEvent>(tank);
    TankListener* l = new TankListener(tank);
    // Add an event listener to plot the trajectory
    sim->addEventListener(l);
    // Inject the driver command into the simulation at time 0
    list<Event<SimEvent>> input;
    SimEvent cmd(sim_command);
    Event<SimEvent> event(tank, cmd);
    input.push_back(event);
    sim->computeNextState(input, 0.0);
    // Run the simulation
    while (sim->nextEventTime() <= 1.0) {
        sim->execNextEvent();
    }
    // Stop the tank
    sim_command.left_power = 0.0;
    sim_command.right_power = 0.0;
    event.value = sim_command;
    input.clear();
    input.push_back(event);
    sim->computeNextState(input, 1.0);
    // Run a little longer
    while (sim->nextEventTime() <= tend) {
        sim->execNextEvent();
    }
    // Clean up and exit
    delete sim;
    delete tank;
    delete l;
    return 0;
}
