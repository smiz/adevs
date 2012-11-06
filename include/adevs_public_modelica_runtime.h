#ifndef _ADEVS_PUBLIC_SIMULATION_RUNTIME_H
#define _ADEVS_PUBLIC_SIMULATION_RUNTIME_H

struct adevs_omc_local_data
{
	adevs_omc_local_data():
		timeValue(-1.0){}
	double timeValue;
};

struct adevs_omc_data
{
	adevs_omc_data()
	{
		localData = new adevs_omc_local_data*[1];
		localData[0] = new adevs_omc_local_data;
	}
	~adevs_omc_data()
	{
		delete localData[0];
		delete [] localData;
	}
	adevs_omc_local_data** localData;
	bool found_solution;
};

#endif
