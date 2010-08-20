#include "IEEE_CDF_Data.h"
#include <fstream>
#include <iostream>
using namespace std;

#define DEGS_TO_RADS (3.14/180.0)
const int IEEE_CDF_Data::LINE_LEN = 1000;

void IEEE_CDF_Data::read_field(int start, int end)
{
	start--;
	end--;
	for (int i = start; i <= end; i++)
	{
		buffer[i-start] = line[i];
	}
	buffer[end+1] = '\0';
}

IEEE_CDF_Data::IEEE_CDF_Data(const char* data_file)
{
	line = new char[LINE_LEN];
	buffer = new char[LINE_LEN];
	ifstream fin(data_file);
	if (fin.bad() && fin == 0)
	{
		throw IEEE_CDF_FileException("Could not open file");
	}
	// Get the base power level
	fin.getline(line,LINE_LEN);
	read_field(32,37);
	double Sbase = atof(buffer);
	// Toss the next line
	fin.getline(line,LINE_LEN);
	// Read the bus information
	int flag;
	do
	{
		fin.getline(line,LINE_LEN);
		read_field(1,4);
		flag = atoi(buffer);
		bus_data_t bus_data;
		if (flag != -999)
		{
			bus_data.ID = flag-1;
			read_field(28,33); bus_data.v = atof(buffer);
			read_field(34,40); bus_data.theta = atof(buffer);
			bus_data.theta *= DEGS_TO_RADS;
			read_field(41,49); bus_data.load_mw = atof(buffer)/Sbase;
			read_field(50,59); bus_data.load_mvar = atof(buffer)/Sbase;
			read_field(60,67); bus_data.genr_mw = atof(buffer)/Sbase;
			read_field(68,75); bus_data.genr_mvar = atof(buffer)/Sbase;
			read_field(107,114); bus_data.G = atof(buffer);
			read_field(115,122); bus_data.B = atof(buffer);
			if (bus_data.genr_mw != 0.0)
				addGenr(bus_data);
			nodes.push_back(bus_data);
		}
	}
	while (flag != -999);
	// Strip the next line 
	fin.getline(buffer,LINE_LEN);
	// Get the transmission data
	do
	{
		fin.getline(line,LINE_LEN);
		read_field(1,4);
		flag = atoi(buffer);
		line_t line_data;
		if (flag != -999)
		{
			line_data.from = flag-1;
			read_field(6,9); line_data.to = atoi(buffer)-1; // End line
			read_field(20,29); real(line_data.y) = atof(buffer); // Real line impedence
			read_field(30,40); imag(line_data.y) = atof(buffer); // Complex line impendence
			line_data.y = 1.0/line_data.y;
			lines.push_back(line_data);
		}
	}
	while (flag != -999);
	// Done
	delete [] line;
	delete [] buffer;
	// Got all of the data, now set the initial conditions for the generators
	setInitialConditions();
}

void IEEE_CDF_Data::setInitialConditions()
{
	AdmittanceNetwork Y(getNodeCount());
	buildAdmitMatrix(Y);
	// Find the injected currents
	Complex *V = new Complex[getNodeCount()];
	Complex *I = new Complex[getNodeCount()]; // For power injected by the loads
	for (unsigned i = 0; i < nodes.size(); i++)
	{
		bus_data_t b = nodes[i];
		V[i] = polar(b.v,b.theta);
	}
	Y.solve_for_current(V,I);
	// Initialize the generators. This won't work without a good load flow, and
	// so the solution really should be run to equilibrium before continuing.
	for (unsigned i = 0; i < genr_nodes.size(); i++)
	{
		int node_id = genr_nodes[i];
		genr_t& g = genrs[node_id];
		g.w0 = 0.0;
		Complex E = I[node_id]*g.Xd;
		g.Ef0 = abs(E);
		g.T0 = arg(E);
		g.Pm0 = real(V[node_id]*conj((E-V[node_id])/g.Xd));
		g.C0 = 0.0; // The power will move if T0 != 0.0 for everyone
		I[node_id] -= V[i]*conj(E/g.Xd); // Subtract the power injected by the generator
	}
	// Cleanup
	delete [] I;
	delete [] V;
}

void IEEE_CDF_Data::addGenr(bus_data_t bus_data)
{
	// Set the model parameters; this does not set the initial
	// values of the state variables
	genr_t genr_data;
	genrs[bus_data.ID] = genr_data;
	genr_nodes.push_back(bus_data.ID);
}

ElectricalData::genr_t IEEE_CDF_Data::getGenrParams(unsigned node)
{
	return genrs[node];
}

void IEEE_CDF_Data::setGenrParams(unsigned node, genr_t params)
{
	genrs[node] = params;
}

Complex IEEE_CDF_Data::getCurrent(unsigned node)
{
	return Complex(0.0,0.0);
}
	
Complex IEEE_CDF_Data::getAdmittance(unsigned node)
{
	// Passive loads
	bus_data_t b = nodes[node];
	Complex S(b.load_mw,b.load_mvar);
	if (b.genr_mw == 0.0) imag(S) -= b.genr_mvar;
	Complex V = polar(b.v,b.theta);
	Complex Ii = conj(S)/conj(V);
	Complex Y = Ii/V;
	return Y;
}

IEEE_CDF_Data::~IEEE_CDF_Data()
{
}
