#include "des.h"
using namespace std;

class Machine : public Partition {
  public:
    Machine(string name) : Partition(), name(name) {}
    void exec(vector<shared_ptr<Event>> &imminent) {
        for (auto ii : imminent) {
            ii->exec();
        }
    }
    unsigned int jobsFinished = 0;
    unsigned int jobsPending = 0;
    unsigned int jobsReceived = 0;
    string const name;
};

shared_ptr<Machine> machine1;
shared_ptr<Machine> machine2;

class Leave : public Event {
  public:
    Leave(shared_ptr<Machine> m, double t) : Event(m, t), _machine(m) {}
    void exec();
    bool prep() { return true; }

  private:
    shared_ptr<Machine> _machine;
};

class Arrive : public Event {
  public:
    Arrive(shared_ptr<Machine> m, double t) : Event(m, t), _machine(m) {}
    void exec();
    bool prep() { return true; }

  private:
    shared_ptr<Machine> _machine;
};

void Leave::exec() {
    cout << "Leave " << _machine->name << " @ " << _machine->now() << endl;
    assert(_machine->jobsPending > 0);
    _machine->jobsPending--;
    _machine->jobsFinished++;
    if (_machine->jobsPending > 0) {
        _machine->schedule(make_shared<Leave>(_machine, _machine->now() + 1));
    }
    if (_machine == machine1) {
        _machine->schedule(make_shared<Arrive>(machine2, _machine->now()));
    }
}

void Arrive::exec() {
    cout << "Arrive " << _machine->name << " @ " << _machine->now() << endl;
    _machine->jobsReceived++;
    _machine->jobsPending++;
    if (_machine->jobsPending == 1) {
        _machine->schedule(make_shared<Leave>(_machine, _machine->now() + 1));
    }
    if (_machine == machine1) {
        _machine->schedule(make_shared<Arrive>(_machine, _machine->now() + 1));
    }
}

int main() {
    machine1 = make_shared<Machine>("M1");
    machine2 = make_shared<Machine>("M2");
    machine1->schedule(make_shared<Arrive>(machine1, 0));
    World* world = new World();
    world->add(machine1.get());
    world->add(machine2.get());
    world->couple(machine1.get(), machine2.get());
    Simulator* sim = new Simulator(world);
    sim->execUntil(5);
    cout << "Machine\tR\tP\tF" << endl;
    cout << "M1\t" << machine1->jobsReceived << "\t" << machine1->jobsPending
         << "\t" << machine1->jobsFinished << endl;
    cout << "M2\t" << machine2->jobsReceived << "\t" << machine2->jobsPending
         << "\t" << machine2->jobsFinished << endl;

    return 0;
}
