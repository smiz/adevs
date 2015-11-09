#ifndef _QQQ_RPC_H_
#define _QQQ_RPC_H_
#include <unistd.h>
#include <exception>
#include <string>
#include <vector>

/**
 * This class encapsulates a QEMU Machine.
 */
class QEMU_Machine
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
