#include "qemu_sync.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

// Used to ensure exclusive access the names of shared objects
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
#define NAME_SIZE 100
static char name_str[NAME_SIZE];
static const size_t memlen = 2*sizeof(long);
static const char* semname[3] =
{
	"/qemu_sem_a",
	"/qemu_sem_b",
	"/qemu_sem_c"
};
static const char* memname = "/qemu_mem";

qemu_sync::qemu_sync()
{
	// Shared segments are unique to each sim<->QEMU pair
	pthread_mutex_lock(&mtx);
	// Cleanup any left over garbage in the name space
	snprintf(name_str,NAME_SIZE,"%s_%d",memname,getpid());
	shm_unlink(name_str);
	// Create a new shared memory segment
  	if ((mem_fd =
			shm_open(name_str,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR)) == -1)
	{
		perror("shm_open failed");
		exit(0);
	}
	for (int idx = 0; idx < 3; idx++)
	{
		// Clean up garbage
		snprintf(name_str,NAME_SIZE,"%s_%d",semname[idx],getpid());
		sem_unlink(name_str);
		// Create a new semaphore
		if ((sem[idx] =
				sem_open(name_str,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR,0))
					== SEM_FAILED)
		{
			perror("sem_open failed");
			exit(0);
		}
	}
	if (ftruncate(mem_fd,memlen) == -1)
	{
		perror("ftruncate failed");
		exit(0);
	}
	void* addr = mmap(
			NULL,
			memlen,
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			mem_fd,
			0);
	if (addr == (void*)-1) {
		perror("mmap failed:");
		exit(0);
	}
	buf = (long*)addr;
}

void qemu_sync::handshake()
{
	sem_wait(sem[0]);
	pthread_mutex_unlock(&mtx);
}

void qemu_sync::run(long h)
{
	*buf = h;
	sem_post(sem[1]);
}

void qemu_sync::sync(long* e, long* h)
{
	sem_wait(sem[2]);
	*h = *buf;
	*e = *(buf+1);
}

qemu_sync::~qemu_sync()
{
	for (int idx = 0; idx < 3; idx++)
		sem_close(sem[idx]);
	munmap((void*)(buf),memlen);
	close(mem_fd);
}

