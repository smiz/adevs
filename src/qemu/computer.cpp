#include "adevs_qemu.h"
#include "qqq_rpc.h"

adevs::CompSysEmulator* adevs::CompSysEmulator::launch_qemu(const char* exec_file,
	std::vector<std::string>& args)
{
	return new QEMU_Machine(exec_file,args);
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
	if (obj != NULL)
		*obj = new uCsimMachineWrapper(machine);
	return machine;
}

