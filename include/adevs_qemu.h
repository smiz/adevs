#ifndef _adevs_qemu_h_
#define _adevs_qemu_h_
#include "adevs.h"
#include <vector>
#include <cstring>
#include <pthread.h>
#include <string>
#include <list>
#include <sys/un.h>

namespace adevs
{

/**
 * This is the interface to be implemented by all device models that will
 * be encapsulated in a model of the computer. The interface is written
 * from the perspective of a person outside the computer.
 */
class QemuDeviceModel
{
	public:
		/**
		 * Returns the number of bytes available to be read from the device.
		 * For message oriented devices, this will be the size of the next
		 * message (e.g., an ethernet frame).
		 */
		int num_bytes_to_read();
		/**
		 * Copy the available data to the supplied buffer.
		 */
		void read_bytes(void* buf);
		/**
		 * Cause data to arrive at the external, physical port. This call
		 * will block if the corresponding write to the underlying file
		 * descriptor blocks.
		 */
		void write_bytes(void* data, int num_bytes);
		/**
		 * Arguments to be appended to the qemu argument vector when
		 * qemu is forked to simulate the computer.
		 */
		virtual void append_qemu_arguments(std::vector<std::string>& args) = 0;
		virtual ~QemuDeviceModel();
		QemuDeviceModel();
		// Called by the reading thread to execute the read loop
		void read_loop();
		// Called by the writing thread to execute the write loop
		void write_loop();
	protected:
		// Start the read and writing thread running on the file descriptor and return.
		void start();
		class io_buffer
		{
			public:
				io_buffer(int size):
					size(size),data(new char[size]){}
				void* get_data() { return (void*)data; }
				int get_size() { return size; }
				void reset_size(int size) { this->size = size; }
				~io_buffer() { delete [] data; }
			private:
				int size;
				char* data;
		};
		// Read a message, return NULL on error
		virtual io_buffer* read() = 0;
		virtual void write(void* data, int num_bytes) = 0;
	private:
		/**
		 * Reading is done continuously in its own thread to prevent
		 * qemu from blocking on a write to the underlying file
		 * descriptor.
		 */
		pthread_t read_thread;
		pthread_mutex_t read_lock;
		std::list<io_buffer*> read_q;
		/**
		 * Writing is done in seperate thread to prevent use from blocking
		 * if qemu is not draining the corresponding buffers while it is
		 * stalled waiting for the simulator to catch up to its timeslice.
		 */
		pthread_t write_thread;
		pthread_mutex_t write_lock;
		pthread_cond_t write_cond;
		std::list<io_buffer*> write_q;

		/**
		 * Scheduling policies for the threads
		 */
		pthread_attr_t io_sched_attr[2];
		struct sched_param fifo_param[2];

		bool running;
};

/**
 * A model of a network interface card. This will connect to a
 * QEMU network card emulator via the file descriptor provided
 * by get_qemu_fd().
 */
class QemuNic:
	public QemuDeviceModel
{
	public:
		QemuNic();
		void append_qemu_arguments(std::vector<std::string>& args);
		~QemuNic();
	protected:
		void write(void* data, int num_bytes);
		io_buffer* read();
	private:
		int fd[2];
};

/**
 * A model of a serial port. This will connect to a
 * QEMU serial port emulator that sends and receives data
 * via a unix domain socket.
 */
class QemuSerialPort:
	public QemuDeviceModel
{
	public:
		QemuSerialPort();
		void append_qemu_arguments(std::vector<std::string>& args);
		~QemuSerialPort();
	protected:
		void write(void* data, int num_bytes);
		io_buffer* read();
	private:
		char socket_file[100];
		struct sockaddr_un address;
		int fd;
		bool connected;
		static const int buf_size;
};

struct qemu_thread_func_t;
// Returns microseconds
int get_qemu_elapsed(qemu_thread_func_t*);
// Supplied time should be microseconds
void set_qemu_elapsed(qemu_thread_func_t*,int);
qemu_thread_func_t* launch_qemu(const char* exec_file, std::vector<std::string>& args);
void shutdown_qemu(qemu_thread_func_t*);
bool qemu_is_alive(qemu_thread_func_t*);
void* qemu_thread_func(void*);

template <typename X>
class QemuComputer:
	public Atomic<X>
{
	public:
		QemuComputer(double quantum_seconds);
		void delta_int();
		void delta_ext(double e, const Bag<X>& xb);
		void delta_conf(const Bag<X>& xb);
		double ta();
		void output_func(Bag<X>& yb);
		// Get the seconds that qemu was ahead (> 0) or behind (< 0) the simulation
		// at the most recent synchronization point
		double get_timing_error() const { return qemu_time-sim_time; }
		double get_current_virtual_time() const { return qemu_time; }
		virtual ~QemuComputer();
	protected:
		void create_x86(
				std::vector<std::string>& qemu_args,
				std::string disk_img,
				int mb_ram = 2048);
	private:
		const double quantum;
		qemu_thread_func_t* thread_data;
		double ttg, qemu_time, sim_time;
		void inject_input(void* buf, unsigned size);
		pthread_t qemu_thread;
		enum { CATCHUP, THREAD_RUNNING, IDLE } mode;

		void internal_and_confluent();
};

template <typename X>
QemuComputer<X>::QemuComputer(double quantum_seconds):
	Atomic<X>(),
	quantum(quantum_seconds),
	thread_data(NULL),
	ttg(0.0),
	qemu_time(0.0),
	sim_time(0.0),
	mode(IDLE)
{
}

template <typename X>
QemuComputer<X>::~QemuComputer()
{
	if (thread_data != NULL)
	{
		if (mode == THREAD_RUNNING)
			pthread_join(qemu_thread,NULL);
		shutdown_qemu(thread_data);
	}
}

template <typename X>
void QemuComputer<X>::output_func(Bag<X>& yb)
{
	// Wait for the quantum to complete before looking
	// for output
	if (mode == THREAD_RUNNING)
	{
		mode = IDLE;
		pthread_join(qemu_thread,NULL);
		// Get the time elapsed in the qemu thread
		qemu_time += get_qemu_elapsed(thread_data)/1E6;
	}
}

template <typename X>
void QemuComputer<X>::internal_and_confluent()
{
	sim_time += ttg;
	// If qemu is ahead of us then advance
	// the clock without running qemu. A 
	// simple test of qemu_time > sim_time
	// can become stuck floating point error
	// and a small ttg combine such that
	// sim_time+ttg = sim_time.
	if (mode != CATCHUP && qemu_time > sim_time)
	{
		mode = CATCHUP;
		ttg = qemu_time - sim_time;
	}
	// Run the computer for another quantum if it is still alive
	else if (qemu_is_alive(thread_data))
	{
		assert(mode != THREAD_RUNNING);
		mode = THREAD_RUNNING;
		ttg = quantum;
		set_qemu_elapsed(thread_data,ttg*1E6);
		pthread_create(&qemu_thread,NULL,qemu_thread_func,(void*)thread_data);
	}
	else mode = IDLE;
}

template <typename X>
void QemuComputer<X>::delta_int()
{
	internal_and_confluent();
}

template <typename X>
void QemuComputer<X>::delta_ext(double e, const Bag<X>& xb)
{
	sim_time += e;
	ttg -= e;
}

template <typename X>
void QemuComputer<X>::delta_conf(const Bag<X>& xb)
{
	internal_and_confluent();
}

template <typename X>
double QemuComputer<X>::ta()
{
	if (qemu_is_alive(thread_data))
		return ttg;
	return adevs_inf<double>();
}

template <typename X>
void QemuComputer<X>::create_x86(
	std::vector<std::string>& args,
	std::string disk_image,
	int mb_ram)
{
	char arg_buf[1000];
	args.push_back("-vga");
	args.push_back("std");
	args.push_back("-m");
	sprintf(arg_buf,"%d",mb_ram);
	args.push_back(arg_buf);
	// No monitor
	args.push_back("-monitor");
	args.push_back("none");
	// Simulated computer will report virtual time and not attempt to track
	// the real system clock
	args.push_back("-rtc");
	args.push_back("clock=vm");
	// Time will track the instruction count
	sprintf(arg_buf,"1,sleep=off");
	args.push_back("-icount");
	args.push_back(arg_buf);
	// Attach our disk image (assume a raw formatted image)
	sprintf(arg_buf,"file=%s,index=0,media=disk,format=raw",disk_image.c_str());
	args.push_back("-drive");
	args.push_back(arg_buf);
	// Start the machine
	thread_data = launch_qemu("qemu-system-i386",args);
	assert(qemu_is_alive(thread_data));
}

}

#endif
