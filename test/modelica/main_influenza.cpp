#include "adevs.h"
#include "Influenza.h"
#include <cmath>
#include <iostream>
#include <fstream>
using namespace std;
using namespace adevs;

class InfluenzaExt:
	public Influenza
{
	public:
		InfluenzaExt():
			Influenza(),
			soln("delayed_sir.txt")
		{
		}
	   void printState()
	   {
			int totalPop =
				::ceil(get_Susceptible() +
				get_Infectious() +
				get_Recovered() +
				get_Deceased());
			soln << (int)::floor(get_time()) << " "
				<< (int)::ceil(get_Susceptible())
				<< " " << (int)::ceil(get_Infectious())
				<< " " << (int)::ceil(get_Recovered())
				<< " " << (int)::ceil(get_Deceased())
				<< " " << totalPop
				<< endl;
			soln.flush();
		}
		~InfluenzaExt()
		{
			soln.close();
		}
	private:
	   ofstream soln;
};

void compare()
{
	ifstream fin[2];
	fin[0].open("delayed_sir.ok");
	fin[1].open("delayed_sir.txt");
	int t = 0;
	do
	{
		int v[2][6];
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				fin[i] >> v[i][j]; 
			}
		}
		if (fin[0].eof() || fin[1].eof())
			break;
		for (int j = 0; j < 6; j++)
		{
			int diff = abs(v[0][j]-v[1][j]);
			if (diff > 1)
			{
				cerr << "j diff is " << diff << " @ t = " << t << endl;
			}
			assert(diff <= 1);
		}
		t++;
	}
	while (true);
	fin[0].close();
	fin[1].close();
}

int main()
{
	InfluenzaExt* influenza = new InfluenzaExt();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		influenza,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(influenza,1E-5,0.1),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(influenza,1E-5));
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
	double tL = 0.0;
	influenza->printState();
	while (sim->nextEventTime() <= 90.0)
	{
		double tN = sim->nextEventTime();
		sim->execNextEvent();
		if (tN - tL >= 1.5)
		{
			tL = floor(tN);
			influenza->printState();
		}
	}
	delete sim;
	delete influenza;
	compare();
	return 0;
}
