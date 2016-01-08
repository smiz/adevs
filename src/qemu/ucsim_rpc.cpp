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
#define STOP_COUNT 20

uCsim_Machine::uCsim_Machine(
	const char* executable,
	const std::vector<std::string>& args,
	double mega_hz,
	int cycles_per_instr):
		Basic_Machine(),
		adevs::ComputerMemoryAccess(),
		mega_hz(mega_hz),
		cycles_per_instr(cycles_per_instr)
{
	pthread_mutex_init(&mtx,NULL);
	pthread_mutex_lock(&mtx);
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
		sprintf(cargs[4],"%fM",mega_hz);
		// End the line of arguments
		cargs[args.size()+5] = NULL;
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
	scan_to_prompt();
	pthread_mutex_unlock(&mtx);
}

bool uCsim_Machine::is_alive()
{
	return (waitpid(pid,NULL,WNOHANG) >= 0);
}

int uCsim_Machine::run(int usecs)
{
	int instrs_per_usec = int(mega_hz/double(cycles_per_instr))+1; 
	sprintf(run_buf,"step %d\n",instrs_per_usec*usecs);
	int buf_len = strlen(run_buf);
	pthread_mutex_lock(&mtx);
	if (write(write_pipe[1],run_buf,buf_len) <= 0)
		perror("ucSim_Machine::run"); 
	// Wait for the advance to finish
	scan_to_prompt();
	pthread_mutex_unlock(&mtx);
	return usecs;
}

void uCsim_Machine::write_mem(unsigned addr, unsigned data)
{
	sprintf(write_buf,"set memory sfr 0x%02x 0x%02x\n",(unsigned char)addr,(unsigned char)data);
	int buf_len = strlen(write_buf);
	pthread_mutex_lock(&mtx);
	if (write(write_pipe[1],write_buf,buf_len) <= 0)
		perror("ucSim_Machine::write_mem");
	// Read to the prompt
	scan_to_prompt();
	pthread_mutex_unlock(&mtx);
}

unsigned uCsim_Machine::read_mem(unsigned addr)
{
	int pos = 0;
	unsigned value;
	sprintf(read_buf,"get sfr 0x%02x\n",(unsigned char)addr);
	int buf_len = strlen(read_buf);
	pthread_mutex_lock(&mtx);
	if (write(write_pipe[1],read_buf,buf_len) <= 0)
		perror("ucSim_Machine::read_mem");
	// Read "0x?? "
	while (1)
	{
		if (read(read_pipe[0],&(read_buf[pos]),1) != 1)
			perror("ucSim_Machine::read_mem");
		if (read_buf[pos] == '\n')
			break;
		pos++;
	}
	// Read to the prompt
	scan_to_prompt();
	pthread_mutex_unlock(&mtx);
	read_buf[pos] = 0x00; 
	sscanf(read_buf,"0x%x %x",&pos,&value);
	return value;
}

void uCsim_Machine::scan_to_prompt()
{
	int stop_count = 0;
	while (stop_count < STOP_COUNT)
	{
		int got;
		if ((got = read(read_pipe[0],scan_buf,100)) <= 0)
			perror("uCsim_Machine::scan_to_prompt");
		for (int i = 0; i < got; i++)
		{
			if (scan_buf[i] == STOP_CHAR)
				stop_count++;
			else stop_count = 0;
		}
	}
}

uCsim_Machine::~uCsim_Machine()
{
	if (write(write_pipe[1],"quit\n",strlen("quit\n")) <= 0)
		perror("uCsim_Machine::~uCsim_Machine");
	close(write_pipe[1]);
	close(read_pipe[0]);
	// Wait for it to exit then cleanup
	waitpid(pid,NULL,0);
	pthread_mutex_destroy(&mtx);
}
