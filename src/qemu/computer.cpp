#include "adevs_qemu.h"
#include "qqq_rpc.h"

struct adevs::qemu_thread_func_t
{
	int elapsed;
	Basic_Machine* machine;
};

int adevs::get_qemu_elapsed(adevs::qemu_thread_func_t* q)
{
	return q->elapsed;
}

void adevs::set_qemu_elapsed(adevs::qemu_thread_func_t* q, int elapsed)
{
	q->elapsed = elapsed;
}

adevs::qemu_thread_func_t* adevs::launch_qemu(const char* exec_file, std::vector<std::string>& args)
{
	adevs::qemu_thread_func_t* q = new adevs::qemu_thread_func_t();
	q->elapsed = 0;
	q->machine = new QEMU_Machine(exec_file,args);
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

adevs::qemu_thread_func_t* adevs::launch_ucsim(
	const char* exec_file, std::vector<std::string>& args, adevs::ComputerMemoryAccess** obj)
{
	uCsim_Machine* machine = new uCsim_Machine(exec_file,args);
	adevs::qemu_thread_func_t* q = new adevs::qemu_thread_func_t();
	q->elapsed = 0;
	q->machine = machine;
	if (obj != NULL)
		*obj = new uCsimMachineWrapper(machine);
	return q;
}

void adevs::shutdown_qemu(adevs::qemu_thread_func_t* q)
{
	delete q->machine;
	delete q;
}

bool adevs::qemu_is_alive(qemu_thread_func_t* q)
{
	return q->machine->is_alive();
}

void* adevs::qemu_thread_func(void* opaque)
{
	adevs::qemu_thread_func_t* data = static_cast<adevs::qemu_thread_func_t*>(opaque);
	// Run for as long as instructed
	data->elapsed = data->machine->run(data->elapsed);
	return NULL;
}
