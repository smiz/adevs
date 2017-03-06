#include "adevs_qemu.h"
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <arpa/inet.h>

static void* pthread_read_func(void* data)
{
	static_cast<adevs::QemuDeviceModel*>(data)->init_func();
	static_cast<adevs::QemuDeviceModel*>(data)->read_loop();
	return NULL;
}

void adevs::QemuDeviceModel::init_func()
{
	initialize_io_structures();
}

void adevs::QemuDeviceModel::read_loop()
{
	adevs::QemuDeviceModel::io_buffer* buf;
	// read should return NULL when the file descriptor is closed
	while ((buf = this->read()) != NULL)
	{
		pthread_mutex_lock(&read_lock);
		read_q.push_back(buf);
		pthread_mutex_unlock(&read_lock);
	}
}

void adevs::QemuDeviceModel::start()
{
	/**
	 * Read thread has high priority because getting data out of qemu
	 * should happen as soon as possible to avoid I/O spanning a time step.
	 */
	pthread_attr_init(&io_sched_attr);
	pthread_attr_setinheritsched(&io_sched_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&io_sched_attr,SCHED_OTHER);
	fifo_param.sched_priority = sched_get_priority_max(SCHED_OTHER);
	pthread_attr_setschedparam(&io_sched_attr,&fifo_param);
	pthread_create(&read_thread,&io_sched_attr,pthread_read_func,(void*)this);
}

adevs::QemuDeviceModel::QemuDeviceModel()
{
	pthread_mutex_init(&read_lock,NULL);
}

int adevs::QemuDeviceModel::num_bytes_to_read()
{
	int size = 0;
	pthread_mutex_lock(&read_lock);
	if (!read_q.empty())
		size = read_q.front()->get_size();
	pthread_mutex_unlock(&read_lock);
	return size;
}

void adevs::QemuDeviceModel::read_bytes(void* data)
{
	adevs::QemuDeviceModel::io_buffer* buf;
	pthread_mutex_lock(&read_lock);
	buf = read_q.front();
	read_q.pop_front();
	pthread_mutex_unlock(&read_lock);
	memcpy(data,buf->get_data(),buf->get_size());
	delete buf;
}

adevs::QemuDeviceModel::~QemuDeviceModel()
{
	// Wait read thread to terminate
	pthread_join(read_thread,NULL);
	// Clean up
	pthread_attr_destroy(&io_sched_attr);
	pthread_mutex_destroy(&read_lock);
	while (!read_q.empty())
	{
		delete read_q.front();
		read_q.pop_front();
	}
}
