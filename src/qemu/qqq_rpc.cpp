#include "qqq_rpc.h"
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <cerrno>
#include <string>
#include <cstring>
#include <exception>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <cerrno>
#include <cstdio>
#include <stdint.h>
#include <climits>
#include <sys/types.h>
#include <sys/wait.h>

QEMU_Machine::QEMU_Machine(const char* executable, const std::vector<std::string>& args):
	CompSysEmulator(),e(0),h(0)
{
	// Fork a process for qemu
	if ((pid = fork()) == 0)
	{
		// Fork qemu
		char** cargs = new char*[args.size()+3];
		cargs[0] = new char[strlen(executable)+1];
		strcpy(cargs[0],executable);
		cargs[1] = new char[strlen("-external-sim")+1];
		strcpy(cargs[1],"-external-sim");
		for (unsigned i = 0; i < args.size(); i++)
		{
			cargs[i+2] = new char[args[i].length()+1];
			strcpy(cargs[i+2],args[i].c_str());
		}
		cargs[args.size()+2] = NULL;
		errno = 0;
		execvp(executable,cargs);
		throw qemu_exception(strerror(errno));
		// We should never get here
		assert(false);
	}
	s.handshake();
}

void QEMU_Machine::run(emulator_time_t nsecs)
{
	// Write the time advance
	s.run(nsecs);
}

void QEMU_Machine::join()
{
	// Wait for qemu to reach that time
	s.sync(&e,&h);
}

QEMU_Machine::~QEMU_Machine()
{
	// Wait for the process 
	kill(pid,SIGTERM);
	waitpid(pid,NULL,0);
}
