#include "adevs.h"
#include "Influenza.h"
#include <cmath>
#include <iostream>

using namespace std;
using namespace adevs;

class InfluenzaExt:
	public Influenza
{
	public:
		InfluenzaExt():
			Influenza(),
			firstOutput(true)
		{
		}
	   void printState()
	   {
			if (firstOutput)
			{
				cout << "# time, not sick, sick, immune, dead, total" << endl;
				firstOutput = false;
			}
			int totalPop =
				::ceil(get_Susceptible() +
				get_Infectious() +
				get_Recovered() +
				get_Deceased());
			cout << (int)::floor(get_time()) << " "
				<< (int)::ceil(get_Susceptible())
				<< " " << (int)::ceil(get_Infectious())
				<< " " << (int)::ceil(get_Recovered())
				<< " " << (int)::ceil(get_Deceased())
				<< " " << totalPop
				<< endl;
		}
	private:
	   bool firstOutput;
};

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
	return 0;
}
