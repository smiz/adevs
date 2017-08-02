#include "des.h"
using namespace std;

class Machine:
	public Partition
{
	public:
		Machine(string name):Partition(),name(name){}
		void exec(vector<Event*>& imm)
		{
			for (unsigned i = 0; i < imm.size(); i++)
			{
				imm[i]->exec();
			}
		}
		int jobsFinished, jobsPending, jobsReceived;
		const string name;
};

Machine *m1, *m2;

class PartLeave:
	public Event
{
	public:
		PartLeave(Machine* m, double t):Event(m,t),m(m){}
		void exec();
		bool prep() { return true; }
	private:
		Machine* m;
};

void PartLeave::exec()
{
	cout << "Leave " << m->name << " @ " << m->now() << endl;
	assert(m->jobsPending > 0);
	m->jobsPending--;
	m->jobsFinished++;
	if (m->jobsPending > 0)
		m->schedule(new PartLeave(m,m->now()+1));
}

class PartArrive:
	public Event
{
	public:
		PartArrive(Machine* m, double t):Event(m,t),m(m){}
		void exec()
		{
			cout << "Arrive " << m->name << " @ " << m->now() << endl;
			m->jobsReceived++;
			m->jobsPending++;
			if (m->jobsPending == 1)
				m->schedule(new PartLeave(m,m->now()+1));
			m->schedule(new PartArrive(m,m->now()+1));
		}
		bool prep() { return true; }
	private:
		Machine* m;
};

class PartTransfer:
	public Event
{
	public:
		PartTransfer(const Machine* m1, Machine *m2):
			Event(m2),m1(m1),m2(m2){}
		bool prep()
		{
			return (m2->jobsReceived < m1->jobsFinished);
		}
		void exec()
		{
			cout << "Transfer " << m2->name << " @ " << m2->now() << endl;
			if (m2->jobsPending == 0)
				m2->schedule(new PartLeave(m2,m2->now()+1));
			m2->jobsPending++;
			m2->jobsReceived++;
			m2->schedule(new PartTransfer(m1,m2));
		}
	private:
		const Machine* m1;
		Machine* m2;
};

int main()
{
	m1 = new Machine("M1");
	m2 = new Machine("M2");
	m1->schedule(new PartArrive(m1,0));
	m2->schedule(new PartTransfer(m1,m2));
	World* world = new World();
	world->add(m1);
	world->add(m2);
	world->couple(m1,m2);
	world->couple(m2,m1);
	Simulator* sim = new Simulator(world);
	sim->execUntil(10);
	cout << "Machine\tR\tP\tF" << endl;
	cout << "M1\t" << m1->jobsReceived << "\t" << m1->jobsPending << "\t" << m1->jobsFinished << endl;
	cout << "M2\t" << m2->jobsReceived << "\t" << m2->jobsPending << "\t" << m2->jobsFinished << endl;
	delete sim;
	delete world;
	return 0;
}
