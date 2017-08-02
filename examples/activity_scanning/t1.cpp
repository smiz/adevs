#include "des.h"
using namespace std;

class Machine:
	public Partition
{
	public:
		Machine():Partition(){}
		void exec(vector<Event*>& imm)
		{
			for (unsigned i = 0; i < imm.size(); i++)
			{
				imm[i]->exec();
			}
		}
		int jobsFinished, jobsPending, jobsReceived;
};

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
	cout << "Leave @ " << m->now() << endl;
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
			cout << "Arrive @ " << m->now() << endl;
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

int main()
{
	Machine* m = new Machine();
	m->schedule(new PartArrive(m,0));
	World* world = new World();
	world->add(m);
	Simulator* sim = new Simulator(world);
	sim->execUntil(5);
	cout << "Machine\tR\tP\tF" << endl;
	cout << "M\t" << m->jobsReceived << "\t" << m->jobsPending << "\t" << m->jobsFinished << endl;
	delete sim;
	delete world;
	return 0;
}
