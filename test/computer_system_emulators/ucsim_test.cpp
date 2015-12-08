#include "adevs_qemu.h"
#include <vector>
#include <string>
#include <iostream>
using namespace adevs;
using namespace std;

typedef unsigned char IO_Type;

class MemSetter:
	public Atomic<IO_Type>
{
	public:
		MemSetter():
			Atomic<IO_Type>(),
			count(0),
			t(0.0)
		{
		}
		double ta() { return 1.0; }
		void delta_int()
		{
			t += ta();
			count = count+1;
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			t += e;
			printf("Got %x @ %f\n",(*(xb.begin())),t);
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(Bag<IO_Type>& yb)
		{
			printf("Sent %x @ %f\n",count,t+ta());
			yb.insert(count);
		}
		void gc_output(Bag<IO_Type>&){}
	private:
		unsigned char count;
		double t;
};

class SingleBoard_8052:
	public QemuComputer<IO_Type>
{
	public:
		SingleBoard_8052(const char* flash_img):
			QemuComputer<IO_Type>(1E-1),
			mem(NULL)
		{
			vector<string> args;
			create_8052(args,flash_img,&mem);
			prev_value = mem->read_mem(0x90);
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			QemuComputer<IO_Type>::delta_ext(e,xb);
			mem->write_mem(0xb0,*(xb.begin()));
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			QemuComputer<IO_Type>::delta_conf(xb);
			mem->write_mem(0xb0,*(xb.begin()));
		}
		void output_func(Bag<IO_Type>& yb)
		{
			QemuComputer<IO_Type>::output_func(yb);
			unsigned char val = mem->read_mem(0x90);
			if (val != prev_value)
			{
				prev_value = val;
				yb.insert(val);
			}
		}
		void gc_output(Bag<IO_Type>&){}
	private:
		ComputerMemoryAccess* mem;
		unsigned char prev_value;
};

int main()
{
	SimpleDigraph<IO_Type>* model = new SimpleDigraph<IO_Type>();
	SingleBoard_8052 *computer = new SingleBoard_8052("test.ihx");
	MemSetter* mem = new MemSetter();
	model->add(computer);
	model->add(mem);
	model->couple(computer,mem);
	model->couple(mem,computer);
	Simulator<IO_Type>* sim = new Simulator<IO_Type>(model);
	while (sim->nextEventTime() < 10.0)
	{
		sim->execNextEvent();
	}
	delete sim;
	delete computer;
}
