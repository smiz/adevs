#include "ElectricalData.h"
#include <iostream>
using namespace std;

void ElectricalData::buildAdmitMatrix(AdmittanceNetwork& Y)
{
	for (vector<line_t>::const_iterator iter = getLines().begin();
		iter != getLines().end(); iter++)
	{
		int from = (*iter).from;
		int to = (*iter).to;
		Y.add_line(from,to,(*iter).y);
	}
	for (vector<unsigned>::const_iterator iter = getGenrs().begin();
			iter != getGenrs().end(); iter++)
	{
		Y.add_self(*iter,1.0/getGenrParams(*iter).Xd);
	}
	for (unsigned i = 0; i < getNodeCount(); i++)
	{
		Y.add_self(i,getAdmittance(i));
	}
}

