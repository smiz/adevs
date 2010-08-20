#include "SimControl.h"
#include <iostream>
using namespace std;

int main()
{
	try
	{
		SimControl control;
		control.run();	
	}
	catch(DisplayException err)
	{
		cerr << err.what() << endl;
	}
	return 0;
}
