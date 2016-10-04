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

/**
 * I/O between the models are strings of characters
 * going to and from the serial port.
 */
struct computer_io_type
{
	char* buf;
	unsigned size;
	computer_io_type(unsigned size):
		buf(new char[size]),
		size(size)
	{
	}
	computer_io_type(char* buf, unsigned size):
		buf(buf),
		size(size)
	{
	}
	computer_io_type(const computer_io_type& other):
		buf(new char[other.size]),
		size(other.size)
	{
		memcpy(buf,other.buf,size);
	}
	~computer_io_type()
	{
		delete [] buf;
	}
};

typedef computer_io_type* IO_Type;

/**
 * An echo server is modeled as a simple server with infinite queue
 * and a fixed service time.
 */
class SerialEcho:
	public Atomic<IO_Type>
{
	public:
		SerialEcho():
			Atomic<IO_Type>(),
			// One second per character
			proc_time(1.0),
			ttg(proc_time)
		{
		}
		void delta_int()
		{
			ttg = proc_time;
			q.pop_front();
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			if (!q.empty()) ttg -= e;
			for (auto x: xb)
				for (unsigned i = 0; i < x->size; i++)
				{
					cout << "x: " << x->buf[i] << endl;
					q.push_back(x->buf[i]); 
				}
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		double ta()
		{
			if (q.empty())
				return adevs_inf<double>();
			else
				return ttg;
		}
		void output_func(Bag<IO_Type>& yb)
		{
			char* buf = new char[1];
			*buf = q.front();
			cout << "y: " << buf[0] << endl;
			yb.insert(new computer_io_type(buf,1));
		}
		void gc_output(Bag<IO_Type>& gb)
		{
			for (auto x: gb)
				delete x;
		}
	private:
		// Time to process a single character
		const double proc_time;
		// Time remaining on the current character
		double ttg;
		// Queue of characters to process
		std::list<char> q;
};

/**
 * Create an x86 with a serial port and state transition/output
 * functions to get data into and out of the serial port.
 */
class x86:
	public QemuComputer<IO_Type>
{
	public:
		x86():QemuComputer<IO_Type>(1E-3)
		{
			// Create a computer with a serial port
			vector<string> qemu_args;
			serial_port = new QemuSerialPort();
			serial_port->append_qemu_arguments(qemu_args);
			/**
			 * Use can get this disk image from
			 *
			 * http://www.ornl.gov/~1qn/qemu-images/jack.img
			 *
			 */
			create_x86(qemu_args,"jack.img",2048,PRECISE);
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			// Receive incoming data at the serial port
			QemuComputer<IO_Type>::delta_ext(e,xb);
			for (auto x: xb)
				serial_port->write_bytes(x->buf,x->size);
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			// Receive incoming data at the serial port
			QemuComputer<IO_Type>::delta_conf(xb);
			for (auto x: xb)
				serial_port->write_bytes(x->buf,x->size);
		}
		void output_func(Bag<IO_Type>& xb)
		{
			QemuComputer<IO_Type>::output_func(xb);
			// Send data produced by the serial port
			int num_bytes = 0;
			while ((num_bytes = serial_port->num_bytes_to_read()) > 0)
			{
				char* buf = new char[num_bytes];
				serial_port->read_bytes(buf);
				xb.insert(new computer_io_type(buf,num_bytes));
			}
		}
		void gc_output(Bag<IO_Type>& gb)
		{
			for (auto x: gb)
				delete x;
		}
		~x86()
		{
			delete serial_port;
		}
	private:
		QemuSerialPort* serial_port;
};

int main()
{
	SerialEcho* echo = new SerialEcho();
	/**
	 * When the simulated computer boots, you can send and receive
	 * messages from the serial port using the script call_response.sh
	 *
	 * For example:
	 *
	 * sh call_response.sh Test
	 *
	 * will cause the message to be send and then echoed, character
	 * by character, over 5 simulated minutes (T e s t \n).
	 */
	x86* computer = new x86();
	SimpleDigraph<IO_Type>* model = new SimpleDigraph<IO_Type>();
	model->add(echo);
	model->add(computer);
	model->couple(echo,computer);
	model->couple(computer,echo);
	Simulator<IO_Type>* sim = new Simulator<IO_Type>(model);
	int count = 0;
	double treal = omp_get_wtime(), tsim = 0.0;
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		if ((++count) % 100 == 0)
		{
			treal = omp_get_wtime()-treal;
			tsim = sim->nextEventTime()-tsim;
			double speedup = tsim/treal;
			cout << "speedup=" << speedup << endl;
			cout << computer->get_mean_timing_error() << " " <<
				computer->get_max_timing_error() << endl;
			count = 0;
			tsim = sim->nextEventTime();
			treal = omp_get_wtime();
		} 
		sim->execNextEvent();
	}
	delete sim;
	delete model;
	return 0;
}
