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

#define STOP_CHAR '#'
#define STOP_COUNT 10

uCsim_Machine::uCsim_Machine(
	const char* executable,
	const std::vector<std::string>& args,
	double khz,
	int cycles_per_instr):
		Basic_Machine(),
		khz(khz),
		cycles_per_instr(cycles_per_instr)
{
	if (pipe(read_pipe) != 0)
		throw qemu_exception(strerror(errno));
	if (pipe(write_pipe) != 0)
		throw qemu_exception(strerror(errno));
	// Fork a process for ucsim
	if ((pid = fork()) == 0)
	{
		// Fork ucsim
		char** cargs = new char*[args.size()+5];
		for (unsigned i = 0; i < args.size(); i++)
		{
			cargs[i+5] = new char[args[i].length()+1];
			strcpy(cargs[i+5],args[i].c_str());
		}
		// First argument is the executable
		cargs[0] = new char[strlen(executable)+1];
		strcpy(cargs[0],executable);
		cargs[1] = new char[3];
		// Second and third arguments set the prompt
		strcpy(cargs[1],"-p");
		cargs[2] = new char[STOP_COUNT+1];
		for (int i = 0; i < STOP_COUNT; i++) cargs[2][i] = STOP_CHAR;
		cargs[2][STOP_COUNT] = 0x00;
		// Fourth and fifth are the frequency
		cargs[3] = new char[3];
		strcpy(cargs[3],"-X");
		cargs[4] = new char[1000];
		sprintf(cargs[4],"%fk",khz);
		// End the line of arguments
		cargs[args.size()+3] = NULL;
		// Redirect stdin and stdout
		dup2(write_pipe[0],STDIN_FILENO);
		dup2(read_pipe[1],STDOUT_FILENO);
		close(write_pipe[0]);
		close(write_pipe[1]);
		close(read_pipe[0]);
		close(read_pipe[1]);
		// Execute the emulator
		errno = 0;
		if (execvp(executable,cargs) != 0)
			throw qemu_exception(strerror(errno));
		// We should never get here
		assert(false);
	}
	close(write_pipe[0]);
	close(read_pipe[1]);
}

bool uCsim_Machine::is_alive()
{
	return (waitpid(pid,NULL,WNOHANG) >= 0);
}

int uCsim_Machine::run(int usecs)
{
	char command[100];
	int instrs_per_usec = int((khz*1E3/double(cycles_per_instr))*1E-6)+1; 
	char stop_code;
	int stop_count = 0;
	int bytes;
	// Tell the emulator to advance one step
	sprintf(command,"step %d\n",(instrs_per_usec*usecs));
	bytes = write(write_pipe[1],command,strlen(command));
	assert(bytes > 0);
	// Wait for the advance to finish
	while (stop_count < STOP_COUNT)
	{
		bytes = read(read_pipe[0],&stop_code,1);
		assert(bytes > 0);
		if (stop_code == STOP_CHAR)
			stop_count++;
		else stop_count = 0;
	}
	return usecs;
}

uCsim_Machine::~uCsim_Machine()
{
	int bytes = write(write_pipe[1],"quit\n",strlen("quit\n"));
	assert(bytes > 0);
	close(write_pipe[1]);
	close(read_pipe[0]);
	// Wait for it to exit then cleanup
	waitpid(pid,NULL,0);
}
