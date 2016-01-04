#include "adevs_qemu.h"
#include <vector>
#include <string>
#include <iostream>
#include <string>
using namespace adevs;
using namespace std;

typedef unsigned char IO_Type;

class Echo:
	public Atomic<IO_Type>
{
	public:
		Echo():
			Atomic<IO_Type>(),
			waiting(false),
			t(0.0),
			count(0)
		{
		}
		double ta() { if (waiting) return adevs_inf<double>(); else return 0.1; }
		void delta_int()
		{
			count++;
			t += ta();
			waiting = true;
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			t += e;
			printf("Got %x @ %f\n",(*(xb.begin())),t);
			waiting = false;
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
		bool waiting;
		double t;
		unsigned char count;
};

class SingleBoard_8052:
	public QemuComputer<IO_Type>
{
	public:
		SingleBoard_8052(const char* flash_img):
			QemuComputer<IO_Type>(1E-4)
		{
			vector<string> args;
			serial_port = new uCsimSerialPort();
			serial_port->append_qemu_arguments(args);
			create_8052(args,flash_img,NULL);
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			unsigned char data = *(xb.begin());
			serial_port->write_bytes(&data,1);
			QemuComputer<IO_Type>::delta_ext(e,xb);
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			unsigned char data = *(xb.begin());
			serial_port->write_bytes(&data,1);
			QemuComputer<IO_Type>::delta_conf(xb);
		}
		void output_func(Bag<IO_Type>& yb)
		{
			QemuComputer<IO_Type>::output_func(yb);
			int num_to_read = serial_port->num_bytes_to_read();
			if (num_to_read > 0)
			{
				assert(num_to_read == 1);
				unsigned char data;
				serial_port->read_bytes(&data);
				yb.insert(data);
			}
		}
		void gc_output(Bag<IO_Type>&){}
		~SingleBoard_8052()
		{
			delete serial_port;
		}
	private:
		uCsimSerialPort* serial_port;
};

int main()
{
	SimpleDigraph<IO_Type>* model = new SimpleDigraph<IO_Type>();
	SingleBoard_8052 *computer = new SingleBoard_8052("serial.ihx");
	Echo* mem = new Echo();
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
	delete model;
}
