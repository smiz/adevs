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
		void init_func();
		volatile bool _is_done_with_init;
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
		// Write data to the device
		virtual void write(void* data, int num_bytes) = 0;
		// Perform any global initializations
		virtual void initialize_io_structures() = 0;
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
		void initialize_io_structures(){}
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
		void initialize_io_structures();
	private:
		char socket_file[100];
		struct sockaddr_un address;
		int fd;
		static const int buf_size;
};

/**
 * A model of a serial port. This will connect to a
 * ucsim serial port emulator that sends and receives data
 * via a pair of pipes. 
 */
class uCsimSerialPort:
	public QemuDeviceModel
{
	public:
		uCsimSerialPort();
		void append_qemu_arguments(std::vector<std::string>& args);
		~uCsimSerialPort();
	protected:
		void write(void* data, int num_bytes);
		io_buffer* read();
		void initialize_io_structures();
	private:
		char read_file[100];
		char write_file[100];
		int read_fd, write_fd;
		volatile bool exit_read;
		static const int buf_size;
};

/**
 * For emulators that support access to memory locations (right now,
 * this is only for the ucsim based emulator), this is the interface
 * used to read and write memory locations. Its primary purpose is to
 * enable access to the i/o registers in small microprocessors.
 */
class ComputerMemoryAccess
{
	public:
		virtual ~ComputerMemoryAccess(){}
		virtual unsigned read_mem(unsigned addr) = 0;
		virtual void write_mem(unsigned addr, unsigned dat) = 0;
};

/**
 * Class for managing the execution of an emulator via a thread that monitors
 * the emulators progress.
 */
class CompSysEmulator
{
	public:
		CompSysEmulator(){}
		// Shutdown the emulator
		virtual ~CompSysEmulator(){}
		// Returns microseconds actual elapsed in the last call to run
		virtual int elapsed() = 0;
		// Launch a thread that will fork the emulator and regulate its progress
		static CompSysEmulator* launch_qemu(const char* exec_file, std::vector<std::string>& args);
		// Launches the thread and returns a pointer to a ComputerMemoryAccess object, which should be freed by the
		// caller when done. Presently only supports access to special function registers.
		static CompSysEmulator* launch_ucsim(const char* exec_file, std::vector<std::string>& args, ComputerMemoryAccess** obj);
		// Returns true if the emulator is still running, false if it has exitted
		virtual bool is_alive() = 0;
		// Run the emulator through the set elapsed time
		virtual void run(int us) = 0;
		// Join on the last run call
		virtual void join() = 0;
};

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
		double get_qemu_time() const { return qemu_time; }
		virtual ~QemuComputer();
	protected:
		void create_x86(
				std::vector<std::string>& qemu_args,
				std::string disk_img,
				int mb_ram = 2048);
		void create_8052(
				std::vector<std::string>& ucsim_args,
				std::string flash_img,
				ComputerMemoryAccess** obj = NULL);
	private:
		const double quantum;
		CompSysEmulator* emulator; 
		double ttg, qemu_time, sim_time;
		void inject_input(void* buf, unsigned size);
		enum { CATCHUP, THREAD_RUNNING, IDLE } mode;

		void internal_and_confluent();
};

template <typename X>
QemuComputer<X>::QemuComputer(double quantum_seconds):
	Atomic<X>(),
	quantum(quantum_seconds),
	emulator(NULL),
	ttg(0.0),
	qemu_time(0.0),
	sim_time(0.0),
	mode(IDLE)
{
}

template <typename X>
QemuComputer<X>::~QemuComputer()
{
	if (emulator != NULL)
	{
		if (mode == THREAD_RUNNING)
			emulator->join();
		delete emulator;
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
		emulator->join();
		// Get the time elapsed in the qemu thread
		qemu_time += emulator->elapsed()/1E6;
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
	else if (emulator->is_alive())
	{
		assert(mode != THREAD_RUNNING);
		emulator->run(quantum*1E6);
		mode = THREAD_RUNNING;
		ttg = quantum;
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
	if (emulator->is_alive())
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
	emulator = CompSysEmulator::launch_qemu("qemu-system-i386",args);
	assert(emulator->is_alive());
}

template <typename X>
void QemuComputer<X>::create_8052(
	std::vector<std::string>& args,
	std::string flash_image,
	ComputerMemoryAccess** obj)
{
	args.push_back(flash_image);
	emulator = CompSysEmulator::launch_ucsim("s51",args,obj);
	assert(emulator->is_alive());
}

}

#endif
