#include "des.h"


class Machine : public Partition {
  public:
    Machine(std::string name) : Partition(), name(name) {}
    void exec(std::vectorstd::shared_ptr<Event>> &imminent) {
        for (auto ii : imminent) {
            ii->exec();
        }
    }
    unsigned int jobsFinished = 0;
    unsigned int jobsPending = 0;
    unsigned int jobsReceived = 0;
    std::string const name;
};

std::shared_ptr<Machine> machine1;
std::shared_ptr<Machine> machine2;

class Leave : public Event {
  public:
    Leave(std::shared_ptr<Machine> m, double t) : Event(m, t), _machine(m) {}
    void exec();
    bool prep() { return true; }

  private:
    std::shared_ptr<Machine> _machine;
};

void Leave::exec() {
    std::cout << "Leave " << _machine->name << " @ " << _machine->now() << std::endl;
    assert(_machine->jobsPending > 0);
    _machine->jobsPending--;
    _machine->jobsFinished++;
    if (_machine->jobsPending > 0) {
        _machine->schedule(std::make_shared<Leave>(_machine, _machine->now() + 1));
    }
}

class Arrive : public Event {
  public:
    Arrive(std::shared_ptr<Machine> m, double t) : Event(m, t), _machine(m) {}
    void exec() {
        std::cout << "Arrive " << _machine->name << " @ " << _machine->now() << std::endl;
        _machine->jobsReceived++;
        _machine->jobsPending++;
        if (_machine->jobsPending == 1) {
            _machine->schedule(
                std::make_shared<Leave>(_machine, _machine->now() + 1));
        }
        _machine->schedule(std::make_shared<Arrive>(_machine, _machine->now() + 1));
    }
    bool prep() { return true; }

  private:
    std::shared_ptr<Machine> _machine;
};

class Transfer : public Event {
  public:
    Transfer(std::shared_ptr<Machine> const m1, std::shared_ptr<Machine> m2)
        : Event(m2), _machine1(m1), _machine2(m2) {}
    bool prep() { return (_machine2->jobsReceived < _machine1->jobsFinished); }
    void exec() {
        std::cout << "Transfer " << _machine2->name << " @ " << _machine2->now()
             << std::endl;
        if (_machine2->jobsPending == 0) {
            _machine2->schedule(
                std::make_shared<Leave>(_machine2, _machine2->now() + 1));
        }
        _machine2->jobsPending++;
        _machine2->jobsReceived++;
        _machine2->schedule(std::make_shared<Transfer>(_machine1, _machine2));
    }

  private:
    std::shared_ptr<Machine> const _machine1;
    std::shared_ptr<Machine> const _machine2;
};

int main() {
    machine1 = std::make_shared<Machine>("M1");
    machine2 = std::make_shared<Machine>("M2");
    machine1->schedule(std::make_shared<Arrive>(machine1, 0));
    machine2->schedule(std::make_shared<Transfer>(machine1, machine2));

    std::shared_ptr<World> world = std::make_shared<World>();
    world->add(machine1);
    world->add(machine2);
    world->couple(machine1, machine2);
    world->couple(machine2, machine1);

    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(world);
    sim->execUntil(10);

    std::cout << "Machine\tR\tP\tF" << std::endl;
    std::cout << "M1\t" << machine1->jobsReceived << "\t" << machine1->jobsPending
         << "\t" << machine1->jobsFinished << std::endl;
    std::cout << "M2\t" << machine2->jobsReceived << "\t" << machine2->jobsPending
         << "\t" << machine2->jobsFinished << std::endl;

    return 0;
}
