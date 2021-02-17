#include <iostream>
#include <cassert>
#include "adevs.h"
using namespace std;
using namespace adevs;

class Model: public Atomic<int,sd_time<int> >
{
	public:
		Model(string name):
			Atomic<int,sd_time<int> >(),
			name(name),
			count(0){}
		sd_time<int> ta()
		{
			if (count%2 == 0)
				return sd_time<int>(1,0);
			return sd_time<int>(0,0);
		}
		void delta_int()
		{
			count++;
		}
		void delta_ext(sd_time<int> e, const Bag<int>&){}
		void delta_conf(const Bag<int>&){ delta_int(); }
		void output_func(Bag<int>& y)
		{
			y.insert(count);
		}
		void gc_output(Bag<int>&){}
		string get_name() const { return name; }
	private:
		const string name;
		int count;
};

class Passive: public Atomic<int,sd_time<int> >
{
	public:
		Passive(string name):
			Atomic<int,sd_time<int> >(),
			name(name){}
		sd_time<int> ta()
		{
			return adevs_inf<sd_time<int> >();
		}
		void delta_int(){}
		void delta_ext(sd_time<int> e, const Bag<int>&)
		{
			cout << name << " " << e << endl;
		}
		void delta_conf(const Bag<int>&){}
		void output_func(Bag<int>&){}
		void gc_output(Bag<int>&){}
		string get_name() const { return name; }
	private:
		const string name;
};

class MyEventListener: public EventListener<int,sd_time<int> >
{
	public:
		MyEventListener():EventListener<int,sd_time<int> >(){}
		void outputEvent(Event<int,sd_time<int> > x, sd_time<int> t)
		{
			Model* model = dynamic_cast<Model*>(x.model);
			cout << model->get_name() << " t = " << t <<  " , y = " << x.value << endl;
		}
		void stateChange(Atomic<int,sd_time<int> >* model, sd_time<int> t)
		{
			Model* cast_model = dynamic_cast<Model*>(model);
			if (cast_model != NULL)
				cout << cast_model->get_name() << " t = " << t <<  " , state changed " << endl;
			Passive* passive = dynamic_cast<Passive*>(model);
			if (passive != NULL)
				cout << passive->get_name() << " t = " << t <<  " , state changed " << endl;
		}
		void inputEvent(Event<int,sd_time<int> > x, sd_time<int> t)
		{
			Model* model = dynamic_cast<Model*>(x.model);
			if (model != NULL)
				cout << model->get_name() << " t = " << t <<  " , x = " << x.value << endl;
			Passive* passive = dynamic_cast<Passive*>(x.model);
			if (passive != NULL)
				cout << passive->get_name() << " t = " << t <<  " , x = " << x.value << endl;
		}
};

void test1()
{
	cout << "TEST 1" << endl;
	Model* model = new Model("model");
	Simulator<int,sd_time<int> >* sim = new Simulator<int,sd_time<int> >(model);
	MyEventListener* listener = new MyEventListener();
	sim->addEventListener(listener);
	while (sim->nextEventTime() < sd_time<int>(10,0))
	{
		sd_time<int> tnew = sim->nextEventTime()+adevs_epsilon<sd_time<int> >();
		assert(sim->execNextEvent() == tnew);
	}
	delete sim;
	delete model;
	delete listener;
}

void test2()
{
	cout << "TEST 2" << endl;
	Model* A = new Model("A");
	Passive* B = new Passive("B");
	SimpleDigraph<int,sd_time<int> >* model = new SimpleDigraph<int,sd_time<int> >();
	model->add(A);
	model->add(B);
	model->couple(A,B);
	Simulator<int,sd_time<int> >* sim = new Simulator<int,sd_time<int> >(model);
	MyEventListener* listener = new MyEventListener();
	sim->addEventListener(listener);
	while (sim->nextEventTime() < sd_time<int>(10,0))
	{
		sd_time<int> tnew = sim->nextEventTime()+adevs_epsilon<sd_time<int> >();
		assert(sim->execNextEvent() == tnew);
	}
	delete sim;
	delete model;
	delete listener;
}

int main()
{
	test1();
	test2();
	return 0;
}
