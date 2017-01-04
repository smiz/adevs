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

const double uCsim_Machine::mega_hz = 11.096;
// Clock frequency in Mhz divided by clock oscillations per instruction
const double uCsim_Machine::instrs_per_usec = uCsim_Machine::mega_hz/12.0;

#define STOP_CHAR '#'
#define STOP_COUNT 5

uCsim_Machine::uCsim_Machine(
	const char* executable,
	const std::vector<std::string>& args):
		Basic_Machine(),
		adevs::ComputerMemoryAccess(),
		elapsed_secs(0.0)
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
		cargs[2] = new char[STOP_COUNT+2];
		for (int i = 0; i < STOP_COUNT; i++) cargs[2][i] = STOP_CHAR;
		cargs[2][STOP_COUNT] = '\n';
		cargs[2][STOP_COUNT+1] = 0x00;
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
	scan_to_prompt(run_buf);
	pthread_mutex_unlock(&mtx);
}

bool uCsim_Machine::is_alive()
{
	return (waitpid(pid,NULL,WNOHANG) >= 0);
}

int uCsim_Machine::run(unsigned usecs)
{
	static const char* state_inst = "state\n";
	static const int state_inst_len = 7;
	double t_start = elapsed_secs;
	pthread_mutex_lock(&mtx);
	while (usecs > 0)
	{
		int instrs = (int)(instrs_per_usec*usecs);
		if (instrs == 0) instrs = 1;
		sprintf(run_buf,"step %d\n",instrs);
		int buf_len = strlen(run_buf);
		if (write(write_pipe[1],run_buf,buf_len) <= 0)
			perror("ucSim_Machine::run"); 
		// Wait for the advance to finish
		scan_to_prompt(run_buf);
		// Get the elapsed time
		if (write(write_pipe[1],state_inst,state_inst_len) <= 0)
			perror("ucSim_Machine::run"); 
		scan_to_prompt(run_buf);
		int count = 0, pos = 0;
		while (count < 4)
		{
			if (run_buf[pos] == '=') count++;
			pos++;
		}
		sscanf(run_buf+pos,"%lf",&elapsed_secs);
		int reduce = ((elapsed_secs-t_start)*1E6);
		usecs -= (reduce > 0) ? reduce : 1;
	}
	pthread_mutex_unlock(&mtx);
	return (int)((elapsed_secs-t_start)*1E6);
}

void uCsim_Machine::write_mem(unsigned addr, unsigned data)
{
	sprintf(write_buf,"set memory sfr 0x%02x 0x%02x\n",(unsigned char)addr,(unsigned char)data);
	int buf_len = strlen(write_buf);
	pthread_mutex_lock(&mtx);
	if (write(write_pipe[1],write_buf,buf_len) <= 0)
		perror("ucSim_Machine::write_mem");
	// Read to the prompt
	scan_to_prompt(write_buf);
	pthread_mutex_unlock(&mtx);
}

unsigned uCsim_Machine::read_mem(unsigned addr)
{
	unsigned value;
	sprintf(read_buf,"get sfr 0x%02x\n",(unsigned char)addr);
	int buf_len = strlen(read_buf);
	pthread_mutex_lock(&mtx);
	if (write(write_pipe[1],read_buf,buf_len) <= 0)
		perror("ucSim_Machine::read_mem");
	// Read to the prompt
	scan_to_prompt(read_buf);
	pthread_mutex_unlock(&mtx);
	sscanf(read_buf,"\n0x%x %x",&addr,&value);
	return value;
}

void uCsim_Machine::scan_to_prompt(char* scan_buf)
{
	int stop_count = 0, pos = 0;
	while (stop_count < STOP_COUNT+1)
	{
		int got;
		if ((got = read(read_pipe[0],scan_buf+pos,100)) <= 0)
			perror("uCsim_Machine::scan_to_prompt");
		for (int i = pos; i < pos+got; i++)
		{
			if (stop_count < STOP_COUNT && scan_buf[i] == STOP_CHAR)
				stop_count++;
			else if (stop_count == STOP_COUNT && scan_buf[i] == '\n')
				stop_count++;
			else
				stop_count = 0;
		}
		pos += got;
	}
	scan_buf[pos] = '\0';
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
