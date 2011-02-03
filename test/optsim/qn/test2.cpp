#include "Qn.h"
#include "adevs.h"
#include <iostream>
#include <fstream>
using namespace adevs;
using namespace std;

int Sq::seed = 0;

class EndListener: public EventListener<int>
{
	public:
		EndListener():gout("g.dat"),dout("c.dat"),in(0),out(0)
		{
			gout << 0 << " " << 0 << endl;
			dout << 0 << " " << 0 << endl;
		}
		void outputEvent(Event<int> x, double t)
		{
			if (dynamic_cast<Genr*>(x.model) != NULL)
			{
				gout << t << " " << in++ << endl;
			}
		}
		void stateChange(Atomic<int>* model, double t)
		{
			if (dynamic_cast<Collector*>(model) != NULL)
			{
				dout << t << " " << out++ << endl;
			}
		}
		~EndListener() { gout.close(); dout.close(); }
	private:
		ofstream gout, dout;
		unsigned int in, out;
};

int main(int argc, char** argv)
{
	Ql* ql = new Ql(10);
	Ql* qlb = new Ql(10);
	Genr* g = new Genr(1.0);
	Collector* c = new Collector();
	SimpleDigraph<int>* model = new SimpleDigraph<int>();
	LpGraph lpg;
	g->setProc(0);
	ql->setProc(1);
	c->setProc(2);
	qlb->setProc(3);
	model->setProc(-1);
	model->add(ql);
	model->add(g);
	model->add(c);
	model->add(qlb);
	model->couple(g,ql);
	model->couple(g,qlb);
	model->couple(ql,c);
	model->couple(qlb,c);
	lpg.addEdge(0,1);
	lpg.addEdge(0,3);
	lpg.addEdge(1,2);
	lpg.addEdge(3,2);
	AbstractSimulator<int>* sim;
	if (argc == 2) sim = new ParSimulator<int>(model,lpg);
	else sim = new Simulator<int>(model);
	EndListener* l = new EndListener();
	sim->addEventListener(l);
	sim->execUntil(50);
	delete sim;
	delete model;
	delete l;
	return 0;
}
