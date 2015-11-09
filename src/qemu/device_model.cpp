#include "adevs_qemu.h"
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <arpa/inet.h>

static void* pthread_func(void* data)
{
	static_cast<adevs::QemuDeviceModel*>(data)->read_loop();
	return NULL;
}

/**
 * This function will exit when the file descriptor is closed by the
 * destructor.
 */
void adevs::QemuDeviceModel::read_loop()
{
	adevs::QemuDeviceModel::io_buffer* buf;
	while ((buf = read()) != NULL)
	{
		pthread_mutex_lock(&lock);
		q.push_back(buf);
		pthread_mutex_unlock(&lock);
	}
}

void adevs::QemuDeviceModel::start()
{
	pthread_create(&read_thread,NULL,pthread_func,(void*)this);
}

adevs::QemuDeviceModel::QemuDeviceModel()
{
	pthread_mutex_init(&lock,NULL);
}

int adevs::QemuDeviceModel::num_bytes_to_read()
{
	int size = 0;
	pthread_mutex_lock(&lock);
	if (!q.empty())
		size = q.front()->get_size();
	pthread_mutex_unlock(&lock);
	return size;
}

void adevs::QemuDeviceModel::read_bytes(void* data)
{
	adevs::QemuDeviceModel::io_buffer* buf;
	pthread_mutex_lock(&lock);
	buf = q.front();
	q.pop_front();
	pthread_mutex_unlock(&lock);
	memcpy(data,buf->get_data(),buf->get_size());
	delete buf;
}

adevs::QemuDeviceModel::~QemuDeviceModel()
{
	pthread_join(read_thread,NULL);
	pthread_mutex_destroy(&lock);
	while (!q.empty())
	{
		delete q.front();
		q.pop_front();
	}
}

