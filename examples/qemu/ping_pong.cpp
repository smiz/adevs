/**
 * This example demonstrates two Linux computers pinging each other through a
 * simulated communication channel.
 */
#include "adevs_qemu.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <list>
#include <unistd.h>
#include <fstream>
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
 * A simplex communication channel with unlimited throughput and a 
 * fixed transmission delay.
 */
class CommChannel:
	public Atomic<IO_Type>
{
	public:
		CommChannel(double delay):
			Atomic<IO_Type>(),
			delay(delay)
		{
		}
		void delta_int()
		{
			double dt = ta();
			q.pop_front();
			for (auto& msg: q)
				msg.ttg -= dt;
		}
		void delta_ext(double e, const Bag<IO_Type>& xb)
		{
			for (auto& msg: q)
				msg.ttg -= e;
			for (auto& x: xb)
			{
				msg_t msg;
				msg.ttg = delay;
			  	msg.msg = new computer_io_type(*x);
				q.push_back(msg);
			}
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(Bag<IO_Type>& yb)
		{
			yb.insert(q.front().msg);
		}
		void gc_output(Bag<IO_Type>& yb)
		{
			for (auto& x: yb)
				delete x;
		}
		~CommChannel()
		{
			for (auto msg: q)
			{
				delete msg.msg;
			}
			q.clear();
		}
		double ta()
		{
			if (q.empty()) return adevs_inf<double>();
			else return q.front().ttg;
		}

	private:
		const double delay;
		struct msg_t
		{
			computer_io_type *msg;
			double ttg;
		};
		std::list<msg_t> q;
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
		x86(std::string disk_img, bool is_cd = false, std::string mac_addr = "",
				EmulatorMode mode = PRECISE):
			QemuComputer<IO_Type>(0.001),
			sent(0),
			recvd(0),
			t(0.0),
			disk_name(disk_img)
		{
			// Add a nic to the computer
			vector<string> qemu_args;
			nic = new QemuNic(mac_addr);
			nic->append_qemu_arguments(qemu_args);
			vector<string> disk, disk_format;
			if (!is_cd)
			{
				std::string cd = "";
				disk.push_back(disk_img);
				disk_format.push_back("raw");
				create_x86(qemu_args,disk,disk_format,cd,false,2048,mode);
			}
			else
			{
				create_x86(qemu_args,disk,disk_format,disk_img,true,2048,mode);
			}
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
			for (auto& x: xb)
				nic->write_bytes(x->buf,x->size);
		}
		void delta_conf(const Bag<IO_Type>& xb)
		{
			// Inject packets received from the network
			t += ta();
			recvd += xb.size();
			QemuComputer<IO_Type>::delta_conf(xb);
			for (auto& x: xb)
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
				xb.insert(new computer_io_type(buf,num_bytes));
				sent++;
			}
		}
		void gc_output(Bag<IO_Type>& gb)
		{
			for (auto& x: gb)
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
	double min_time_err[2] = {DBL_MAX,DBL_MAX}, max_time_err[2] = {-DBL_MAX,-DBL_MAX}, avg_time_err[2] = {0.0,0.0};
	int time_err_count[2] = {0,0};
	double tstart = omp_get_wtime();
	double tnow = 0.0;
	x86* B = new x86("jill.img",false,"00:00:00:11:11:11",
			QemuComputer<IO_Type>::FAST);
	x86* A = new x86("jack.img",false,"00:00:00:11:11:12",
			QemuComputer<IO_Type>::FAST);
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
	// Run the simulation and collect speedup and timing error statistics
	Simulator<IO_Type>* sim = new Simulator<IO_Type>(model);
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		tnow = sim->nextEventTime();
		sim->execNextEvent();
		double Aerr = A->get_qemu_time()-tnow;
		double Berr = B->get_qemu_time()-tnow;
		if (A->ta() < adevs_inf<double>())
		{
			time_err_count[0]++;
			avg_time_err[0] += Aerr;
			min_time_err[0] = ::min(min_time_err[0],Aerr);
			max_time_err[0] = ::max(max_time_err[0],Aerr);
		}
		if (B->ta() < adevs_inf<double>())
		{
			time_err_count[1]++;
			avg_time_err[1] += Berr;
			min_time_err[1] = ::min(min_time_err[1],Berr);
			max_time_err[1] = ::max(max_time_err[1],Berr);
		}
	}
	delete sim;
	delete model;
	double tend = omp_get_wtime();
	cout << "sim: " << tnow << " , real: " << (tend-tstart) << endl;
	cout << "A: " << min_time_err[0] << " " << (avg_time_err[0]/(double)(time_err_count[0])) << " " << max_time_err[0] << endl;
	cout << "B: " << min_time_err[1] << " " << (avg_time_err[1]/(double)(time_err_count[1])) << " " << max_time_err[1] << endl;
	return 0;
}
