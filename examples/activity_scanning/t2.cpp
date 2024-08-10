#include "des.h"
using namespace std;

class Machine : public Partition {
  public:
    Machine(string name) : Partition(), name(name) {}
    void exec(vector<shared_ptr<Event>> &imm) {
        for (unsigned i = 0; i < imm.size(); i++) {
            imm[i]->exec();
        }
    }
    unsigned int jobsFinished = 0;
    unsigned int jobsPending = 0;
    unsigned int jobsReceived = 0;
    string const name;
};

shared_ptr<Machine> m1;
shared_ptr<Machine> m2;

class PartLeave : public Event {
  public:
    PartLeave(shared_ptr<Machine> m, double t) : Event(m, t), m(m) {}
    void exec();
    bool prep() { return true; }

  private:
    shared_ptr<Machine> m;
};

class PartArrive : public Event {
  public:
    PartArrive(shared_ptr<Machine> m, double t) : Event(m, t), m(m) {}
    void exec();
    bool prep() { return true; }

  private:
    shared_ptr<Machine> m;
};

void PartLeave::exec() {
    cout << "Leave " << m->name << " @ " << m->now() << endl;
    assert(m->jobsPending > 0);
    m->jobsPending--;
    m->jobsFinished++;
    if (m->jobsPending > 0) {
        m->schedule(make_shared<PartLeave>(m, m->now() + 1));
    }
    if (m == m1) {
        m->schedule(make_shared<PartArrive>(m2, m->now()));
    }
}

void PartArrive::exec() {
    cout << "Arrive " << m->name << " @ " << m->now() << endl;
    m->jobsReceived++;
    m->jobsPending++;
    if (m->jobsPending == 1) {
        m->schedule(make_shared<PartLeave>(m, m->now() + 1));
    }
    if (m == m1) {
        m->schedule(make_shared<PartArrive>(m, m->now() + 1));
    }
}

int main() {
    m1 = make_shared<Machine>("M1");
    m2 = make_shared<Machine>("M2");
    m1->schedule(make_shared<PartArrive>(m1, 0));
    World* world = new World();
    world->add(m1.get());
    world->add(m2.get());
    world->couple(m1.get(), m2.get());
    Simulator* sim = new Simulator(world);
    sim->execUntil(5);
    cout << "Machine\tR\tP\tF" << endl;
    cout << "M1\t" << m1->jobsReceived << "\t" << m1->jobsPending << "\t"
         << m1->jobsFinished << endl;
    cout << "M2\t" << m2->jobsReceived << "\t" << m2->jobsPending << "\t"
         << m2->jobsFinished << endl;

    return 0;
}
