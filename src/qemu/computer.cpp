#include "adevs_qemu.h"
#include "qqq_rpc.h"

/**
 * Data that is shared between the run thread for the emulator
 * and the main thread of the simulator
 */
struct thread_data_t
{
	Basic_Machine* machine;
	int elapsed;
	int mode; // 0 idle, 1 run, 2 quit
	pthread_cond_t cond;
	pthread_mutex_t mtx;
};

static void* thread_func(void* opaque)
{
	thread_data_t* data = static_cast<thread_data_t*>(opaque);
	pthread_mutex_lock(&(data->mtx));
	while (true)
	{
		while (data->mode == 0)
			pthread_cond_wait(&(data->cond),&(data->mtx));
		// Run for as long as instructed
		if (data->mode == 1)
		{
			data->elapsed = data->machine->run(data->elapsed);
			data->mode = 0;
			pthread_cond_signal(&(data->cond));
		}
		else break;
	}
	pthread_cond_signal(&(data->cond));
	pthread_mutex_unlock(&(data->mtx));
	return NULL;
}

class GenericEmulator:
	public adevs::CompSysEmulator
{
	public:
		GenericEmulator():
			adevs::CompSysEmulator()
		{
			data.machine = NULL;
			data.elapsed = 0;
			data.mode = 0;
			pthread_mutex_init(&(data.mtx),NULL);
			pthread_cond_init(&(data.cond),NULL);
		}
		~GenericEmulator()
		{
			// Nothing to stop if we haven't launched the machine
			if (data.machine != NULL)
			{
				// Tell the thread to exit
				pthread_mutex_lock(&(data.mtx));
				data.mode = 2;
				pthread_cond_signal(&(data.cond));
				pthread_mutex_unlock(&(data.mtx));
				// Wait for it to exit
				pthread_join(thr,NULL);
				// Delete the actual emulator
				if (data.machine != NULL)
					delete data.machine;
			}
			// Clean up mutex and conditional
			pthread_mutex_destroy(&(data.mtx));
			pthread_cond_destroy(&(data.cond));
		}
		int elapsed() { return data.elapsed; }
		bool is_alive()
		{
			return data.machine->is_alive();
		}
		void run(int us)
		{
			pthread_mutex_lock(&(data.mtx));
			// Set the amount of time to execute
			data.elapsed = us;
			// Start the thread
			data.mode = 1;
			pthread_cond_signal(&(data.cond));
			pthread_mutex_unlock(&(data.mtx));
		}
		void join()
		{
			// Wait for the running time slice to complete
			pthread_mutex_lock(&(data.mtx));
			while (data.mode != 0)
				pthread_cond_wait(&(data.cond),&(data.mtx));
			pthread_mutex_unlock(&(data.mtx));
		}

	thread_data_t data;
	pthread_t thr;
};

adevs::CompSysEmulator* adevs::CompSysEmulator::launch_qemu(const char* exec_file, std::vector<std::string>& args)
{
	GenericEmulator* q = new GenericEmulator();
	q->data.machine = new QEMU_Machine(exec_file,args);
	pthread_create(&(q->thr),NULL,thread_func,(void*)(&(q->data)));
	return q;
}

class uCsimMachineWrapper:
	public adevs::ComputerMemoryAccess
{
	public:
		uCsimMachineWrapper(uCsim_Machine* machine):
			adevs::ComputerMemoryAccess(),
			machine(machine)
		{
		}
		unsigned read_mem(unsigned addr)
		{
			return machine->read_mem(addr);
		}
		void write_mem(unsigned addr, unsigned data)
		{
			machine->write_mem(addr,data);
		}
		~uCsimMachineWrapper(){}
	private:
		uCsim_Machine* machine;
};

adevs::CompSysEmulator* adevs::CompSysEmulator::launch_ucsim(
	const char* exec_file, std::vector<std::string>& args, adevs::ComputerMemoryAccess** obj)
{
	uCsim_Machine* machine = new uCsim_Machine(exec_file,args);
	GenericEmulator* q = new GenericEmulator();
	q->data.machine = machine;
	if (obj != NULL)
		*obj = new uCsimMachineWrapper(machine);
	pthread_create(&(q->thr),NULL,thread_func,(void*)(&(q->data)));
	return q;
}

