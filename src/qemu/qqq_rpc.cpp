#include "qqq_rpc.h"
#include <unistd.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <sys/types.h>
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

void QEMU_Machine::write_mem_value(int val)
{
	errno = 0;
	if (write(write_fd,&val,sizeof(int)) != sizeof(int))
		perror("Write error to qemu pipe");
}

int QEMU_Machine::read_mem_value()
{
	int val;
	if (read(read_fd,&val,sizeof(int)) != sizeof(int))
		return 0;
	return val;
}

QEMU_Machine::QEMU_Machine(const char* executable, const std::vector<std::string>& args):
	Basic_Machine()
{
	// Ignore sigpipe which will kill us when qemu exits
	signal(SIGPIPE,SIG_IGN);
	// Create pipes that we will used to exchange data 
	int pipefd[2][2];
	if (pipe(pipefd[0]) < 0)
		throw qemu_exception(strerror(errno));
	if (pipe(pipefd[1]) < 0)
		throw qemu_exception(strerror(errno));
	// Get the pipes
	read_fd = pipefd[1][0];
	write_fd = pipefd[0][1];
	// Fork a process for qemu
	if ((pid = fork()) == 0)
	{
		close(read_fd);
		close(write_fd);
		// Get read and write pipe for qemu process
		read_fd = pipefd[0][0];
		write_fd = pipefd[1][1];
		// Fork qemu
		char** cargs = new char*[args.size()+4];
		cargs[0] = new char[strlen(executable)+1];
		strcpy(cargs[0],executable);
		cargs[1] = new char[5];
		strcpy(cargs[1],"-qqq");
		cargs[2] = new char[1000];
		sprintf(cargs[2],"write=%d,read=%d",write_fd,read_fd);
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
	close(pipefd[0][0]);
	close(pipefd[1][1]);
}

bool QEMU_Machine::is_alive()
{
	return (waitpid(pid,NULL,WNOHANG) >= 0);
}

int QEMU_Machine::run(int usecs)
{
	int elapsed;
	// Write the time advance
	write_mem_value(usecs);
	// Wait for qemu to reach that time
	elapsed = read_mem_value();
	// Return the actual time that was advanced
	return elapsed;
}

QEMU_Machine::~QEMU_Machine()
{
	// Close our pipes. This will force qemu to exit
	close(read_fd);
	close(write_fd);
	// Wait for the process 
	kill(pid,SIGTERM);
	waitpid(pid,NULL,0);
}
