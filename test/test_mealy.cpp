#include "adevs.h"
#include <iostream>
#include <cmath>
#include <cassert>
using namespace std;
using namespace adevs;

class Listener:
	public EventListener<int>
{
	public:
		Listener():EventListener<int>(),c(0),s(0),value(-1){}
		void outputEvent(Event<int> x, double t)
		{
			c++;
			assert(value == -1 || x.value == value); 
			value = x.value;
		}
		void stateChange(Atomic<int>* model, double t)
		{
			#pragma omp critical
			s++;
		}
		int c, s, value;
};

class Periodic:
	public Atomic<int>
{
	public:
		Periodic(double p):Atomic<int>(),p(p),c(0){}
		double ta()
		{
			if (c > 10) return adevs_inf<double>();
			return p;
		}
		void delta_int() { c++; }
		void delta_ext(double, const Bag<int>&){}
		void delta_conf(const Bag<int>&){}
		void output_func(Bag<int>& yb)
		{
			yb.insert(1);
		}
		void gc_output(Bag<int>&){}
	private:
		const double p;
		int c;
};

class Receiver:
	public Atomic<int>
{
	public:
		Receiver():Atomic<int>(),c(0){}
		double ta()
		{
			return adevs_inf<double>();
		}
		void delta_int(){}
		void delta_ext(double e, const Bag<int>& xb)
		{
			c += xb.size();
		}
		void delta_conf(const Bag<int>& xb)
		{
			delta_ext(0.0,xb);
		}
		int get_c() const { return c; }
		void output_func(Bag<int>&){}
		void gc_output(Bag<int>&){}
	private:
		int c;
};

class Trigger:
	public MealyAtomic<int>
{
	public:
		Trigger():MealyAtomic<int>(),ttg(adevs_inf<double>()){}
		double ta()
		{
			return ttg;
		}
		void delta_int()
		{
			ttg = adevs_inf<double>();
		}
		void delta_ext(double e, const Bag<int>& xb)
		{
			assert(ee == e);
			ttg = 1.0;
		}
		void delta_conf(const Bag<int>& xb)
		{
			ttg = 1.0;
		}
		void output_func(Bag<int>& yb)
		{
			// Turn off
			yb.insert(0);
		}
		void output_func(const Bag<int>& xb, Bag<int>& yb)
		{
			// Turn on
			yb.insert(*(xb.begin()));
		}
		void output_func(double e, const Bag<int>& xb, Bag<int>& yb)
		{
			ee = e;
			output_func(xb,yb);
		}
		void gc_output(Bag<int>& gb){}
	private:
		double ttg, ee;
};

void test1()
{
	SimpleDigraph<int>* model = new SimpleDigraph<int>();
	Trigger *trigger = new Trigger();
	Periodic *periodic = new Periodic(sqrt(2.0));
	model->add(trigger);
	model->add(periodic);
	model->couple(periodic,trigger);
	Listener* l = new Listener();
	Simulator<int>* sim = new Simulator<int>(model);
	sim->addEventListener(l);
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		sim->execNextEvent();
		assert(
			(l->c == 1 && l->value == 0) ||  
			(l->c == 2 && l->value == 1) 
		);
		assert(l->c == l->s);
		l->c = l->s = 0;
		l->value = -1;
	}
	delete sim;
	delete l;
	delete model;
}

void test2()
{
	SimpleDigraph<int>* model = new SimpleDigraph<int>();
	Trigger *triggera = new Trigger();
	Trigger *triggerb = new Trigger();
	Periodic *periodic = new Periodic(sqrt(2.0));
	model->add(triggera);
	model->add(triggerb);
	model->add(periodic);
	model->couple(periodic,triggera);
	model->couple(periodic,triggerb);
	Listener* l = new Listener();
	Simulator<int>* sim = new Simulator<int>(model);
	sim->addEventListener(l);
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		sim->execNextEvent();
		assert(
			(l->c == 2 && l->value == 0) ||  
			(l->c == 3 && l->value == 1) 
		);
		assert(l->c == l->s);
		l->c = l->s = 0;
		l->value = -1;
	}
	delete sim;
	delete l;
	delete model;
}

void test3()
{
	SimpleDigraph<int>* model = new SimpleDigraph<int>();
	Trigger *triggera = new Trigger();
	Trigger *triggerb = new Trigger();
	Periodic *periodic = new Periodic(sqrt(2.0));
	Receiver *rx = new Receiver();
	model->add(triggera);
	model->add(triggerb);
	model->add(periodic);
	model->add(rx);
	model->couple(periodic,triggera);
	model->couple(periodic,triggerb);
	model->couple(triggera,rx);
	model->couple(triggerb,rx);
	Listener* l = new Listener();
	Simulator<int>* sim = new Simulator<int>(model);
	sim->addEventListener(l);
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		int c = rx->get_c();
		sim->execNextEvent();
		assert(
			(l->c == 2 && l->value == 0) ||  
			(l->c == 3 && l->value == 1) 
		);
		assert(l->c == l->s-1);
		l->c = l->s = 0;
		l->value = -1;
		assert(rx->get_c() == c+2);
	}
	delete sim;
	delete l;
	delete model;
}

void test4()
{
	bool except = false;
	SimpleDigraph<int>* model = new SimpleDigraph<int>();
	Trigger *triggera = new Trigger();
	Trigger *triggerb = new Trigger();
	Periodic *periodic = new Periodic(sqrt(2.0));
	model->add(triggera);
	model->add(triggerb);
	model->add(periodic);
	model->couple(periodic,triggera);
	model->couple(triggera,triggerb);
	model->couple(triggerb,triggera);
	Simulator<int>* sim = new Simulator<int>(model);
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		try
		{
			sim->execNextEvent();
		}
		catch(...)
		{
			except = true;
			break;
		}
	}
	assert(except);
	delete sim;
	delete model;
}

void test5()
{
	cout << "TEST 5" << endl;
	SimpleDigraph<int>* model = new SimpleDigraph<int>();
	Trigger *triggera = new Trigger();
	Trigger *triggerb = new Trigger();
	Periodic *periodic = new Periodic(1.0);
	Receiver *rx = new Receiver();
	model->add(triggera);
	model->add(triggerb);
	model->add(periodic);
	model->add(rx);
	model->couple(periodic,triggera);
	model->couple(periodic,triggerb);
	model->couple(triggera,rx);
	model->couple(triggerb,rx);
	Listener* l = new Listener();
	Simulator<int>* sim = new Simulator<int>(model);
	sim->addEventListener(l);
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		int c = rx->get_c();
		sim->execNextEvent();
		assert(periodic->ta() == adevs_inf<double>() ||
			(l->c == 3 && l->value == 1));
		assert(l->c == l->s-1);
		l->c = l->s = 0;
		l->value = -1;
		assert(rx->get_c() == c+2);
	}
	delete sim;
	delete l;
	delete model;
	cout << "TEST 5 PASSED" << endl;
}

int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
	return 0;
}

