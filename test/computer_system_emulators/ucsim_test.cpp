#include "adevs_qemu.h"
#include <vector>
#include <string>
#include <iostream>
using namespace adevs;
using namespace std;

class SingleBoard_8052:
	public QemuComputer<int>
{
	public:
		SingleBoard_8052(const char* flash_img):
			QemuComputer(1E-5)
		{
			vector<string> args;
			create_8052(args,flash_img);
		}
		void gc_output(Bag<int>&){}
};

int main()
{
	SingleBoard_8052 *computer = new SingleBoard_8052("test.ihx");
	Simulator<int>* sim = new Simulator<int>(computer);
	while (sim->nextEventTime() < 1.0)
	{
		cout << sim->nextEventTime() << endl;
		sim->execNextEvent();
	}
	delete sim;
	delete computer;
}
