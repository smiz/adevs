#include "qqq_rpc.h"
#include <unistd.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
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
#include <arpa/inet.h>
#include <stdint.h>

void QEMU_Machine::write_mem_value(unsigned val)
{
	uint32_t msg = htonl(val);
	if (write(fd[0],&msg,sizeof(uint32_t)) != sizeof(uint32_t))
		perror("Write error to qemu socket");
}

unsigned QEMU_Machine::read_mem_value()
{
	uint32_t msg;
	if (read(fd[0],&msg,sizeof(uint32_t)) != sizeof(uint32_t))
		return 0;
	return ntohl(msg);
}

QEMU_Machine::QEMU_Machine(const char* executable, const std::vector<std::string>& args):
	CompSysEmulator(),
	e(0)
{
	// Create sockets that we will used to exchange data 
	if (socketpair(AF_UNIX,SOCK_STREAM,0,fd) < 0)
		throw qemu_exception(strerror(errno));
	// Fork a process for qemu
	if ((pid = fork()) == 0)
	{
		// Fork qemu
		char** cargs = new char*[args.size()+4];
		cargs[0] = new char[strlen(executable)+1];
		strcpy(cargs[0],executable);
		cargs[1] = new char[strlen("-external_sim")+1];
		strcpy(cargs[1],"-external_sim");
		cargs[2] = new char[1000];
		sprintf(cargs[2],"sock=%d",fd[1]);
		for (unsigned i = 0; i < args.size(); i++)
		{
			cargs[i+3] = new char[args[i].length()+1];
			strcpy(cargs[i+3],args[i].c_str());
		}
		cargs[args.size()+3] = NULL;
		errno = 0;
		execvp(executable,cargs);
		throw qemu_exception(strerror(errno));
		// We should never get here
		assert(false);
	}
	close(fd[1]);
}

bool QEMU_Machine::is_alive()
{
	return (waitpid(pid,NULL,WNOHANG) >= 0);
}

void QEMU_Machine::run(unsigned usecs)
{
	// Write the time advance
	write_mem_value(usecs);
}

void QEMU_Machine::join()
{
	// Wait for qemu to reach that time
	e = read_mem_value();
}

QEMU_Machine::~QEMU_Machine()
{
	// Close our socket. This will force qemu to exit
	close(fd[0]);
	// Wait for the process 
	kill(pid,SIGTERM);
	waitpid(pid,NULL,0);
}
