#include "qqq_rpc.h"
#include <sys/mman.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <cerrno>
#include <string>
#include <cstring>
#include <exception>
#include <cstdlib>
#include <cassert>
#include <iostream>

void QEMU_Machine::write_mem_value(int val)
{
	// I AM ASSUMING THAT THE MEMORY WRITE WILL BE ATOMIC!
	(*((volatile int*)shm)) = val;
}

int QEMU_Machine::read_mem_value()
{
	// I AM ASSUMING THAT THE MEMORY READ WILL BE ATOMIC!
	return (*((volatile int*)shm));
}

QEMU_Machine::QEMU_Machine(const char* executable, const std::vector<std::string>& args):
	shm(NULL)
{
	// Create a pipe that we will use to coordinate the creation
	// of a shared memory regions
	int pipefd[2];
	if (pipe(pipefd) < 0)
		throw qemu_exception(strerror(errno));
	// Fork a process for qemu
	if ((pid = fork()) == 0)
	{
		// Child waits for the parent to create the shared memory
		// region that qemu will attach to
		char c;
		if (read(pipefd[0],&c,1) != 1)
			throw qemu_exception(strerror(errno));
		// Done with the pipe!
		close(pipefd[0]);
		// Fork qemu
		char** cargs = new char*[args.size()+2];
		for (unsigned i = 0; i < args.size(); i++)
		{
			cargs[i+1] = new char[args[i].length()+1];
			strcpy(cargs[i+1],args[i].c_str());
		}
		cargs[args.size()+1] = NULL;
		cargs[0] = new char[strlen(executable)];
		strcpy(cargs[0],executable);
		errno = 0;
		if (execvp(executable,cargs) != 0)
			throw qemu_exception(strerror(errno));
		// We should never get here
		assert(false);
	}
	// Create the shared memory region using the child pid in the key
	sprintf(key,"/qemu_%d",pid);
	errno = 0;
	int fd = shm_open(key,O_RDWR|O_CREAT,S_IRWXU);
	if (fd == -1)
		throw qemu_exception(strerror(errno));
	if (ftruncate(fd,sizeof(int)) == -1)
		throw qemu_exception(strerror(errno));
	// Map the memory region
	shm = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if (shm == NULL)
		throw qemu_exception(strerror(errno));
	// Qemu will pause waiting for the first time advance
	write_mem_value(-1);
    // Tell the child to go ahead
	if (write(pipefd[1],(char*)&pid,1) != 1)
		throw qemu_exception(strerror(errno));
	close(pipefd[1]);
	// Done with the shm file descriptor
	close(fd);
}

bool QEMU_Machine::is_alive()
{
	return (waitpid(pid,NULL,WNOHANG) >= 0);
}

int QEMU_Machine::run(int usecs)
{
	int elapsed;
	// Write the time advance
	write_mem_value((usecs > 0) ? usecs : 1);
	// Wait for qemu to reach that time
	while ((elapsed = read_mem_value()) > 0 && is_alive());
	// Return the actual time that was advanced
	return -elapsed;
}

QEMU_Machine::~QEMU_Machine()
{
	// Instruct the child to exit
	write_mem_value(0);
	// Wait for it to exit then cleanup
	waitpid(pid,NULL,0);
	munmap(shm,sizeof(int));
	shm_unlink(key);
}
