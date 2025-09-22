#include <memory>

#include "des.h"


class Machine : public Partition {
  public:
    Machine() : Partition() {}
    void exec(std::vectorstd::shared_ptr < Event >> &imminent) {
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
    Leave(std::shared_ptr<Machine> m, double t) : Event(m, t), _machine(m) {}
    void exec();
    bool prep() { return true; }

  private:
    std::shared_ptr<Machine> _machine;
};

void Leave::exec() {
    std::cout << "Leave @ " << _machine->now() << std::endl;
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
        std::cout << "Arrive @ " << _machine->now() << std::endl;
        _machine->jobsReceived++;
        _machine->jobsPending++;
        if (_machine->jobsPending == 1) {
            _machine->schedule(std::make_shared<Leave>(_machine, _machine->now() + 1));
        }
        _machine->schedule(std::make_shared<Arrive>(_machine, _machine->now() + 1));
    }
    bool prep() { return true; }

  private:
    std::shared_ptr<Machine> _machine;
};

int main() {
    std::shared_ptr<Machine> machine = std::make_shared<Machine>();
    machine->schedule(std::make_shared<Arrive>(machine, 0));

    std::shared_ptr<World> world = std::make_shared<World>();
    world->add(machine);

    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(world);
    sim->execUntil(5);

    std::cout << "Machine\tR\tP\tF" << std::endl;
    std::cout << "M\t" << machine->jobsReceived << "\t" << machine->jobsPending << "\t"
              << machine->jobsFinished << std::endl;

    return 0;
}
