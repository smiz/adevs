#ifndef _QQQ_RPC_H_
#define _QQQ_RPC_H_
#include <unistd.h>
#include <exception>
#include <string>
#include <vector>

/**
  * Basic interface to any emulated computer system.
  */
class Basic_Machine
{
	public:
		virtual int run(int usecs) = 0;
		virtual bool is_alive() = 0;
		virtual ~Basic_Machine(){};
};

/**
 * This class encapsulates a QEMU Machine.
 */
class QEMU_Machine:
	public Basic_Machine
{
	public:
		/**
		 * Instantiate a machine using the supplied command line
		 * arguments to fork its process. 
		 */
		QEMU_Machine(const char* executable, const std::vector<std::string>& arguments);
		/**
		 * Instruct the machine to execute for at most usec
		 * microseconds of simulated time and then return.
		 * The return value is the number of microseconds
		 * that actually advanced. 
		 */
		int run(int usecs);
		/**
		 * Returns true if qemu is still executing and false if it
		 * has terminated.
		 */
		bool is_alive();
		/**
		 * Shut the machine down now.
		 */
		virtual ~QEMU_Machine();

	private:
		unsigned pid;
		// Key to the shared memory region.
		char key[100]; 
		// Beginning of memory mapped region
		void* shm;

		void write_mem_value(int val);
		int read_mem_value();	
};

/**
 * This class encapsulates a uCsim Machine.
 */
class uCsim_Machine:
	public Basic_Machine
{
	public:
		/**
		 * Instantiate a machine using the supplied command line
		 * arguments to fork its process. 
		 */
		uCsim_Machine(
				const char* executable,
				const std::vector<std::string>& arguments,
				double khz = 11059.2E3,
				int cycles_per_instr = 12);
		/**
		 * Instruct the machine to execute for at most usec
		 * microseconds of simulated time and then return.
		 * The return value is the number of microseconds
		 * that actually advanced. 
		 */
		int run(int usecs);
		/**
		 * Returns true if qemu is still executing and false if it
		 * has terminated.
		 */
		bool is_alive();
		/**
		 * Shut the machine down now.
		 */
		virtual ~uCsim_Machine();

	private:
		unsigned pid;
		int read_pipe[2];
		int write_pipe[2];
		const double khz;
		const int cycles_per_instr;
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
