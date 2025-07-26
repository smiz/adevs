#include <memory>

#include "des.h"

using namespace std;


class Machine : public Partition {
  public:
    Machine() : Partition() {}
    void exec(vector<shared_ptr<Event>> &imminent) {
        for (auto ii : imminent) {
            ii->exec();
        }
    }

    unsigned int jobsFinished = 0;
    unsigned int jobsPending = 0;
    unsigned int jobsReceived = 0;
};

class Leave : public Event {
  public:
    Leave(shared_ptr<Machine> m, double t) : Event(m, t), _machine(m) {}
    void exec();
    bool prep() { return true; }

  private:
    shared_ptr<Machine> _machine;
};

void Leave::exec() {
    cout << "Leave @ " << _machine->now() << endl;
    assert(_machine->jobsPending > 0);
    _machine->jobsPending--;
    _machine->jobsFinished++;
    if (_machine->jobsPending > 0) {
        _machine->schedule(make_shared<Leave>(_machine, _machine->now() + 1));
    }
}

class Arrive : public Event {
  public:
    Arrive(shared_ptr<Machine> m, double t) : Event(m, t), _machine(m) {}
    void exec() {
        cout << "Arrive @ " << _machine->now() << endl;
        _machine->jobsReceived++;
        _machine->jobsPending++;
        if (_machine->jobsPending == 1) {
            _machine->schedule(
                make_shared<Leave>(_machine, _machine->now() + 1));
        }
        _machine->schedule(make_shared<Arrive>(_machine, _machine->now() + 1));
    }
    bool prep() { return true; }

  private:
    shared_ptr<Machine> _machine;
};

int main() {
    shared_ptr<Machine> machine = make_shared<Machine>();
    machine->schedule(make_shared<Arrive>(machine, 0));

    shared_ptr<World> world = make_shared<World>();
    world->add(machine);

    shared_ptr<Simulator> sim = make_shared<Simulator>(world);
    sim->execUntil(5);

    cout << "Machine\tR\tP\tF" << endl;
    cout << "M\t" << machine->jobsReceived << "\t" << machine->jobsPending
         << "\t" << machine->jobsFinished << endl;

    return 0;
}
