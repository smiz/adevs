#include "des.h"
using namespace std;

class Machine : public Partition {
  public:
    Machine(string name) : Partition(), name(name) {}
    void exec(vector<Event*> &imm) {
        for (unsigned i = 0; i < imm.size(); i++) {
            imm[i]->exec();
        }
    }
    int jobsFinished, jobsPending, jobsReceived;
    string const name;
};

Machine *m1, *m2;

class PartLeave : public Event {
  public:
    PartLeave(Machine* m, double t) : Event(m, t), m(m) {}
    void exec();
    bool prep() { return true; }

  private:
    Machine* m;
};

class PartArrive : public Event {
  public:
    PartArrive(Machine* m, double t) : Event(m, t), m(m) {}
    void exec();
    bool prep() { return true; }

  private:
    Machine* m;
};

void PartLeave::exec() {
    cout << "Leave " << m->name << " @ " << m->now() << endl;
    assert(m->jobsPending > 0);
    m->jobsPending--;
    m->jobsFinished++;
    if (m->jobsPending > 0) {
        m->schedule(new PartLeave(m, m->now() + 1));
    }
    if (m == m1) {
        m->schedule(new PartArrive(m2, m->now()));
    }
}

void PartArrive::exec() {
    cout << "Arrive " << m->name << " @ " << m->now() << endl;
    m->jobsReceived++;
    m->jobsPending++;
    if (m->jobsPending == 1) {
        m->schedule(new PartLeave(m, m->now() + 1));
    }
    if (m == m1) {
        m->schedule(new PartArrive(m, m->now() + 1));
    }
}

int main() {
    m1 = new Machine("M1");
    m2 = new Machine("M2");
    m1->schedule(new PartArrive(m1, 0));
    World* world = new World();
    world->add(m1);
    world->add(m2);
    world->couple(m1, m2);
    Simulator* sim = new Simulator(world);
    sim->execUntil(5);
    cout << "Machine\tR\tP\tF" << endl;
    cout << "M1\t" << m1->jobsReceived << "\t" << m1->jobsPending << "\t"
         << m1->jobsFinished << endl;
    cout << "M2\t" << m2->jobsReceived << "\t" << m2->jobsPending << "\t"
         << m2->jobsFinished << endl;
    delete sim;
    delete world;
    return 0;
}
