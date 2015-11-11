/**
 * This example demonstrates two Linux computers pinging each other through a
 * simulated communication channel.
 */
#include "adevs_qemu.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <list>
using namespace std;
using namespace adevs;

/**
 * Data exchanged between the models will be the packet expelled by
 * qemu and stuffed into a byte buffer.
 */
struct computer_io_type
{
	char* buf;
	int size;
	computer_io_type(int size):
		buf(new char[size]),
		size(size)
	{
	}
	computer_io_type(char* buf, int size):
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
 * A simplex communication channel modeled as a server with a finite
 * queue. Two of these will be used to create a duplex channel
 * between the simulated machines.
 */
class CommChannel:
	public Atomic<IO_Type>
{
	public:
		CommChannel(double delay):
			Atomic<IO_Type>(),
			delay(delay),
			ttg(delay),
			t(0.0)
		{
		}
		void delta_int()
		{
			t += ta();
			ttg = delay;
			q.pop_front();
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			t += e;
			if (!q.empty())
				ttg -= e;
			for (auto x: xb)
			{
				printf("@ t = %f xmit packet contents:\n",t);
				for (int i = 0; i < x->size; i++)
					printf("%x ",x->buf[i]);
				printf("\n");
				q.push_back(new computer_io_type(*x));
			}
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(Bag<IO_Type>& yb)
		{
			yb.insert(q.front());
		}
		void gc_output(Bag<IO_Type>& yb)
		{
			for (auto x: yb)
				delete x;
		}
		~CommChannel()
		{
			for (auto msg: q)
			{
				delete msg;
			}
			q.clear();
		}
		double ta()
		{
			if (q.empty()) return adevs_inf<double>();
			return ttg;
		}

	private:
		const double delay;
		double ttg, t;
		std::list<computer_io_type*> q;
};

/**
 * We extend the QemuComputer class to add a network interface card
 * and to extend the state transition and output functions to 
 * send and receive packets produced by the card.
 */
class x86:
	public QemuComputer<IO_Type>
{
	public:
		x86(std::string disk_img):
			QemuComputer<IO_Type>(1E-4),
			sent(0),
			recvd(0),
			t(0.0),
			disk_name(disk_img)
		{
			// Add a nic to the computer
			vector<string> qemu_args;
			nic = new QemuNic();
			nic->append_qemu_arguments(qemu_args);
			create_x86(qemu_args,disk_img.c_str());
		}
		void delta_int()
		{
			t += ta();
			QemuComputer<IO_Type>::delta_int();
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			// Inject packets received from the network
			t += e;
			recvd += xb.size();
			QemuComputer<IO_Type>::delta_ext(e,xb);
			for (auto x: xb)
				nic->write_bytes(x->buf,x->size);
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			// Inject packets received from the network
			t += ta();
			recvd += xb.size();
			QemuComputer<IO_Type>::delta_conf(xb);
			for (auto x: xb)
				nic->write_bytes(x->buf,x->size);
		}
		void output_func(Bag<IO_Type>& xb)
		{
			QemuComputer<IO_Type>::output_func(xb);
			// Send packets that have been produced by the card
			int num_bytes = 0;
			while ((num_bytes = nic->num_bytes_to_read()) > 0)
			{
				char* buf = new char[num_bytes];
				nic->read_bytes(buf);
				// Long packets are DHCP requests and we just throw those out
				if (num_bytes < 300)
				{
					xb.insert(new computer_io_type(buf,num_bytes));
					sent++;
				}
				else delete [] buf;
			}
		}
		void gc_output(Bag<IO_Type>& gb)
		{
			for (auto x: gb)
				delete x;
		}
		~x86()
		{
			delete nic;
		}
	private:
		QemuNic* nic;
		int sent, recvd;
		double t;
		std::string disk_name;
};

int main()
{
	// These disk images are available at
	//
	// http://www.ornl.gov/~1qn/qemu-images/jill.img
	// http://www.ornl.gov/~1qn/qemu-images/jack.img
	//
	// When the computers boot, you will need to use ifconfig
	// to assign network addresses to the nic card with the commands
	//
	// jill: sudo ifconfig eth0 10.1.1.10
	// jack: sudo ifconfig eth1 10.1.1.11
	//
	// Then you can (from jack) ping 10.1.1.10. The reported round
	// trip times should be slightly longer than 2 x the channel
	// service time (i.e., about 200 milliseconds). 
	//
	// To shutdown cleanly, you must shutdown the machines as follows:
	//
	// sudo poweroff
	//
	x86* B = new x86("/home/nutarojj/Code/qemu/images/jill.img");
	x86* A = new x86("/home/nutarojj/Code/qemu/images/jack.img");
	SimpleDigraph<IO_Type>* model = new SimpleDigraph<IO_Type>();
	model->add(A);
	model->add(B);
	// Couple nics through a wire
	CommChannel* A_to_B = new CommChannel(0.1);
	CommChannel* B_to_A = new CommChannel(0.1);
	model->add(A_to_B);
	model->add(B_to_A);
	model->couple(A,A_to_B);
	model->couple(A_to_B,B);
	model->couple(B,B_to_A);
	model->couple(B_to_A,A);
	// Run the simulation
	Simulator<IO_Type>* sim = new Simulator<IO_Type>(model);
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		sim->execNextEvent();
	}
	delete sim;
	delete model;
	return 0;
}
