#include <memory>

#include "des.h"

using namespace std;


class Machine : public Partition {
  public:
    Machine() : Partition() {}
    void exec(vector<shared_ptr<Event>> &imm) {
        for (unsigned i = 0; i < imm.size(); i++) {
            imm[i]->exec();
        }
    }

    unsigned int jobsFinished = 0;
    unsigned int jobsPending = 0;
    unsigned int jobsReceived = 0;
};

class PartLeave : public Event {
  public:
    PartLeave(shared_ptr<Machine> m, double t) : Event(m, t), m(m) {}
    void exec();
    bool prep() { return true; }

  private:
    shared_ptr<Machine> m;
};

void PartLeave::exec() {
    cout << "Leave @ " << m->now() << endl;
    assert(m->jobsPending > 0);
    m->jobsPending--;
    m->jobsFinished++;
    if (m->jobsPending > 0) {
        m->schedule(make_shared<PartLeave>(m, m->now() + 1));
    }
}

class PartArrive : public Event {
  public:
    PartArrive(shared_ptr<Machine> m, double t) : Event(m, t), m(m) {}
    void exec() {
        cout << "Arrive @ " << m->now() << endl;
        m->jobsReceived++;
        m->jobsPending++;
        if (m->jobsPending == 1) {
            m->schedule(make_shared<PartLeave>(m, m->now() + 1));
        }
        m->schedule(make_shared<PartArrive>(m, m->now() + 1));
    }
    bool prep() { return true; }

  private:
    shared_ptr<Machine> m;
};

int main() {
    shared_ptr<Machine> m = make_shared<Machine>();
    m->schedule(make_shared<PartArrive>(m, 0));
    World* world = new World();
    world->add(m.get());
    Simulator* sim = new Simulator(world);
    sim->execUntil(5);
    cout << "Machine\tR\tP\tF" << endl;
    cout << "M\t" << m->jobsReceived << "\t" << m->jobsPending << "\t"
         << m->jobsFinished << endl;

    return 0;
}
