#ifndef _QQQ_RPC_H_
#define _QQQ_RPC_H_
#include "adevs_qemu.h"
#include <unistd.h>
#include <exception>
#include <string>
#include <vector>

/**
 * This class encapsulates a QEMU Machine.
 */
class QEMU_Machine:
	public adevs::CompSysEmulator
{
	public:
		/**
		 * Instantiate a machine using the supplied command line
		 * arguments to fork its process. 
		 */
		QEMU_Machine(const char* executable, const std::vector<std::string>& arguments);
		/**
		 * These methods are from the CompSysEmulator interface
		 */
		int elapsed() { return e; }
		bool is_alive();
		void run(unsigned usecs);
		void join();
		/**
		 * Shut the machine down now.
		 */
		virtual ~QEMU_Machine();

	private:
		unsigned pid;
		// Socket for talking to qemu
		int fd[2];
		int e;

		void write_mem_value(unsigned val);
		unsigned read_mem_value();
};

/**
 * This class encapsulates a uCsim Machine.
 */
class uCsim_Machine:
	public adevs::CompSysEmulator,
	public adevs::ComputerMemoryAccess
{
	public:
		/**
		 * Instantiate a machine using the supplied command line
		 * arguments to fork its process. 
		 */
		uCsim_Machine(
				const char* executable,
				const std::vector<std::string>& arguments);
		/**
		 * These methods are from CompSysEmulator
		 */
		int elapsed() { return e; }
		bool is_alive();
		void run(unsigned usecs);
		void join(){}
		/**
		 * Shut the machine down now.
		 */
		virtual ~uCsim_Machine();

		unsigned read_mem(unsigned addr);
		void write_mem(unsigned addr, unsigned data);

	private:
		double elapsed_secs;
		int e;
		unsigned pid;
		int read_pipe[2];
		int write_pipe[2];
		static const double mega_hz;
		static const double instrs_per_usec;
		pthread_mutex_t mtx;
		char run_buf[1000];
		char write_buf[1000];
		char read_buf[1000];

		void scan_to_prompt(char* scan_buf);
};

/**
 * Exceptions thrown when there are problems with the emulator.
 */
class qemu_exception:
	public std::exception
{
	public:
		qemu_exception(const char* err_msg) throw():
			std::exception(),
			err_msg(err_msg)
		{
		}
		qemu_exception(const std::exception& other) throw():
			std::exception(),
			err_msg(other.what())
		{
		}
		exception& operator=(const exception& other) throw()
		{
			err_msg = other.what();
			return *this;
		}
		const char* what() const throw()
		{
			return err_msg.c_str();
		}
		~qemu_exception() throw(){}
	private:
		std::string err_msg;
};

#endif
