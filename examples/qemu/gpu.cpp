/**
 * This is an example of a single computer that communicates
 * with a simulated serial port device. The simulated device
 * merely echoes each received character after a delay.
 */
#include "adevs_qemu.h"
#include <unistd.h>
#include <iostream>
#include <cassert>
#include <cstring>
using namespace std;
using namespace adevs;

typedef char IO_Type;

/**
 * Create an x86 with a passthrough gpu
 */
class x86:
	public QemuComputer<IO_Type>
{
	public:
		x86():QemuComputer<IO_Type>(1E-3)
		{
			string cdrom;
			vector<string> qemu_args;
			vector<string> disks;
			vector<string> disk_formats;
			// This is the disk image we want to boot
			disks.push_back("/home/1qn/Code/adevs-code/examples/qemu/ubuntu.qcow2");
			// This is the disk image format
			disk_formats.push_back("qcow2");
			// Add the pci-passthrough arguments for the gpu
			qemu_args.push_back("-device");
			qemu_args.push_back("vfio-pci,host=0000:17:00.0");
			// Launch the emulator using KVM for the microprocessor emulator
			create_x86(qemu_args,disks,disk_formats,cdrom,false,8028,FAST);
		}
		void gc_output(Bag<IO_Type>& gb){}
		~x86(){}
		double ta()
		{
			return QemuComputer<IO_Type>::ta();
		}
};

int main()
{
	srand(time(NULL));
	x86* computer = new x86();
	Simulator<IO_Type>* sim = new Simulator<IO_Type>(computer);
	int count = 0;
	const double treal = omp_get_wtime();
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		if ((++count) % 1000 == 0)
		{
			double real = omp_get_wtime()-treal;
			double tsim = sim->nextEventTime();
			double speedup = tsim/real;
			cout << "speedup=" << speedup << endl;
			count = 0;
		} 
		sim->execNextEvent();
	}
	delete sim;
	delete computer;
	return 0;
}
