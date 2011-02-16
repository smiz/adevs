#include "Qn.h"
#include "adevs_par_simulator.h"
#include <iostream>
#include <fstream>
#include <cstring>
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
	bool use_par_sim = false;
	bool recycle = false;
	if (argc < 4)
	{
		cerr << "Need # queues, # lines, and end time" << endl;
		return 0;
	}
	for (int i = 4; i < argc; i++)
	{
		if (strcmp(argv[i],"-par") == 0)
			use_par_sim = true;
		if (strcmp(argv[i],"-recycle") == 0)
			recycle = true;
	}
	if (use_par_sim)
		std::cerr << "Using " << omp_get_max_threads() << " threads" << endl;
	if (recycle)
		std::cerr << "Recycling" << std::endl;
	int queues = atoi(argv[1]);
	int lines = atoi(argv[2]);
	double tend = atof(argv[3]);
	cerr << "Q=" << queues << ", L=" << lines << ", Tend=" << tend << endl;
	LpGraph lpg;
	Qn* model = new Qn(queues,lines,lpg,recycle);
	AbstractSimulator<int>* sim;
	if (use_par_sim) sim = new ParSimulator<int>(model,lpg);
	else sim = new Simulator<int>(model);
	EndListener* l = new EndListener();
	sim->addEventListener(l);
	sim->execUntil(tend);
	delete sim;
	delete model;
	delete l;
	return 0;
}
