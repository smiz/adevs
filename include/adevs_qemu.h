#ifndef _adevs_qemu_h_
#define _adevs_qemu_h_
#include "adevs.h"
#include <vector>
#include <cstring>
#include <cstdio>
#include <cassert>
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
		 * @return Number of bytes available to read
		 */
		int num_bytes_to_read();
		/**
		 * Copy the available data to the supplied buffer.
		 * @param buf Buffer with space sufficient to hold num_bytes_to_read()
		 */
		void read_bytes(void* buf);
		/**
		 * Cause data to arrive at the external, physical port. This call
		 * will block if the corresponding write to the underlying file
		 * descriptor blocks.
		 * @param data Data buffer to copy to device
		 * @param num_bytes Number of bytes to copy
		 */
		virtual void write_bytes(void* data, int num_bytes) = 0;
		/**
		 * The device may append arguments to the qemu command line
		 * by appending them to this vector. Distinct elements in
		 * the vector will be separated by a space on the command line.
		 * This method should called by the simulator prior to forking
		 * qemu.
		 */
		virtual void append_qemu_arguments(std::vector<std::string>& args) = 0;
		/// Destructor
		virtual ~QemuDeviceModel();
		/// Constructor
		QemuDeviceModel();
		/// Called by the reading thread to execute the read loop
		void read_loop();
		void init_func();
	protected:
		// Start the read thread and return.
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
		 * Scheduling policies for read thread
		 */
		pthread_attr_t io_sched_attr;
		struct sched_param fifo_param;
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
		/**
		 * Create a network card. If you need a specific MAC
		 * address that can be supplied as an argument.
		 * @param mac_addr Optional MAC address for card
		 */
		QemuNic(std::string mac_addr = "");
		/**
		 * Appends arguments that qemu needs to setup the network device.
		 */
		void append_qemu_arguments(std::vector<std::string>& args);
		/// Write bytes to the network card
		void write_bytes(void* data, int num_bytes);
		/// Destructor
		~QemuNic();
	protected:
		io_buffer* read();
		void initialize_io_structures(){}
	private:
		int fd[2];
		std::string mac_addr;
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
		void write_bytes(void* data, int num_bytes);
		~QemuSerialPort();
	protected:
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
		void write_bytes(void* data, int num_bytes);
		~uCsimSerialPort();
	protected:
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
		// Launches the thread and returns a pointer to a ComputerMemoryAccess object,
		// which should be freed by the caller when done. Presently only supports
		// access to special function registers.
		static CompSysEmulator* launch_ucsim(
				const char* exec_file, std::vector<std::string>& args, ComputerMemoryAccess** obj);
		// Returns true if the emulator is still running, false if it has exitted
		virtual bool is_alive() = 0;
		// Run the emulator through the set elapsed time
		virtual void run(unsigned us) = 0;
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
		double get_quantum_seconds() const { return quantum; }
		double get_mean_timing_error() const { return acc_error/(double)(error_samples+1); }
		double get_max_timing_error() const { return max_error; }
		virtual ~QemuComputer();

		enum EmulatorMode
		{
			PRECISE, // Use icount
			FAST, // kvm
		};

	protected:
		void create_x86(
				std::vector<std::string>& qemu_args,
				std::string disk_img,
				int mb_ram = 2048,
				EmulatorMode emulator_mode = PRECISE,
				double delayStart = 0.0);
		void create_x86(
				std::vector<std::string>& qemu_args,
				std::vector<std::string>& disks,
				std::vector<std::string>& disk_formats,
				std::string& cdrom,
				bool boot_cdrom,
				int mb_ram,
				EmulatorMode emulator_mode,
				double delayStart = 0.0);
		void create_8052(
				std::vector<std::string>& ucsim_args,
				std::string flash_img,
				ComputerMemoryAccess** obj = NULL,
				double delayStart = 0.0);

	private:
		ComputerMemoryAccess** ucsim_mem_obj;
		std::vector<std::string> emulator_arguments;
		std::string emulator_exec;
		const double quantum;
		CompSysEmulator* emulator; 
		double ttg, qemu_time, sim_time, acc_error, max_error;
		unsigned error_samples;
		void inject_input(void* buf, unsigned size);
		enum { CATCHUP, THREAD_RUNNING, IDLE , DELAY_START } mode;

		void internal_and_confluent();
};

template <typename X>
QemuComputer<X>::QemuComputer(double quantum_seconds):
	Atomic<X>(),
	ucsim_mem_obj(NULL),
	quantum(quantum_seconds),
	emulator(NULL),
	ttg(0.0),
	qemu_time(0.0),
	sim_time(0.0),
	acc_error(0.0),
	max_error(0.0),
	error_samples(0),
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
		// Get the scaled time elapsed in the qemu thread
		qemu_time += (emulator->elapsed()/1E6);
	}
}

template <typename X>
void QemuComputer<X>::internal_and_confluent()
{
	// Start of the emulator was delayed
	if (mode == DELAY_START)
	{
		mode = IDLE;
		ttg = 0.0;
		assert(sim_time == 0.0);
		assert(qemu_time == 0.0);
		assert(emulator == NULL);
		if (emulator_exec == "qemu-system-x86_64")
			emulator = CompSysEmulator::launch_qemu(emulator_exec.c_str(),emulator_arguments);
		else if (emulator_exec == "s51")
			emulator = CompSysEmulator::launch_ucsim(emulator_exec.c_str(),
				emulator_arguments,ucsim_mem_obj);
		assert(emulator->is_alive());
	}
	// Run the emulator
	sim_time += ttg;
	// If qemu is ahead of us then advance
	// the clock without running qemu. A 
	// simple test of qemu_time > sim_time
	// can become stuck if a floating point error
	// and small ttg combine such that
	// sim_time+ttg = sim_time.
	if (mode != CATCHUP && qemu_time > sim_time)
	{
		mode = CATCHUP;
		error_samples++;
		acc_error += get_timing_error();
		max_error = (max_error > get_timing_error()) ? max_error : get_timing_error();
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
	if (mode != DELAY_START)
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
	if (mode == DELAY_START || emulator->is_alive())
		return ttg;
	return adevs_inf<double>();
}

template <typename X>
void QemuComputer<X>::create_x86(
	std::vector<std::string>& args,
	std::vector<std::string>& disks,
	std::vector<std::string>& disk_formats,
	std::string& cdrom,
	bool boot_cdrom,
	int mb_ram,
	EmulatorMode emulator_mode,
	double delayStart)
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
	if (emulator_mode == PRECISE)
	{
		sprintf(arg_buf,"1,sleep=off");
		args.push_back("-icount");
		args.push_back(arg_buf);
	}
	else if (emulator_mode == FAST)
	{
		args.push_back("-cpu");
		args.push_back("host,kvm=off,-kvmclock");
		args.push_back("-enable-kvm"); 
	}
	// Attach our disk images 
	for (unsigned idx = 0; idx < disks.size(); idx++)
	{
		sprintf(arg_buf,"file=%s,index=%u,media=disk,format=%s",
			disks[idx].c_str(),idx,disk_formats[idx].c_str());
		args.push_back("-drive");
		args.push_back(arg_buf);
	}
	if (cdrom != "")
	{
		args.push_back("-cdrom");
		args.push_back(cdrom);
		if (boot_cdrom)
		{
			args.push_back("-boot");
			args.push_back("d");
		}
	}
	// Start the machine
	if (delayStart > 0.0)
	{
		mode = DELAY_START;
		ttg = delayStart;
		emulator_arguments = args;
		emulator_exec = "qemu-system-x86_64";
	}
	else
	{
		emulator = CompSysEmulator::launch_qemu("qemu-system-x86_64",args);
		assert(emulator->is_alive());
	}
}

template <typename X>
void QemuComputer<X>::create_x86(
	std::vector<std::string>& args,
	std::string disk_image,
	int mb_ram,
	QemuComputer<X>::EmulatorMode emulator_mode,
	double delayStart)
{
	std::string cdrom = "";
	std::vector<std::string> disks, disk_formats;
	disks.push_back(disk_image);
	disk_formats.push_back("raw");
	create_x86(args,disks,disk_formats,cdrom,false,mb_ram,emulator_mode,delayStart);
}

template <typename X>
void QemuComputer<X>::create_8052(
	std::vector<std::string>& args,
	std::string flash_image,
	ComputerMemoryAccess** obj,
	double delayStart)
{
	args.push_back("-t");
	args.push_back("8052");
	args.push_back(flash_image);
	if (delayStart > 0.0)
	{
		mode = DELAY_START;
		ttg = delayStart;
		emulator_arguments = args;
		emulator_exec = "s51";
		ucsim_mem_obj = obj;
	}
	else
	{
		emulator = CompSysEmulator::launch_ucsim("s51",args,obj);
		assert(emulator->is_alive());
	}
}

}

#endif
