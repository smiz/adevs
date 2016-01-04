#include "adevs_qemu.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

const int adevs::uCsimSerialPort::buf_size = 256;

void adevs::uCsimSerialPort::initialize_io_structures()
{
	if ((write_fd = open(write_file,O_WRONLY)) < 0)
		perror("uCsimSerialPort::write");
	if ((read_fd = open(read_file,O_RDONLY)) < 0)
		perror("uCsimSerialPort::read");
}

void adevs::uCsimSerialPort::write(void* data, int num_bytes)
{
	if (::write(write_fd,(char*)data,num_bytes) < num_bytes)
		perror("uCsimSerialPort write write failed");
}

adevs::QemuDeviceModel::io_buffer* adevs::uCsimSerialPort::read()
{
	// The call to read will not return until ucsim closes its port,
	// so we are forced to poll I/O and check occasionally for the
	// exit condition
	fd_set read_fds, write_fds, except_fds;
	struct timeval timeout;
	// Wait for input to become ready or until the time out; the first parameter is
	// 1 more than the largest file descriptor in any of the sets
	while (true)
	{
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		FD_ZERO(&except_fds);
		FD_SET(read_fd,&read_fds);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		if (select(read_fd+1,&read_fds,&write_fds,&except_fds,&timeout) == 1)
		{
			int num_read = 0;
			adevs::QemuDeviceModel::io_buffer* buf =
				new adevs::QemuDeviceModel::io_buffer(buf_size);
			if ((num_read = ::read(read_fd,buf->get_data(),buf_size)) <= 0)
			{
				delete buf;
				return NULL;
			}
			buf->reset_size(num_read);
			return buf;
		}
		if (exit_read)
			return NULL;
	}
}

adevs::uCsimSerialPort::uCsimSerialPort():
	adevs::QemuDeviceModel(),
	read_fd(-1),
	write_fd(-1),
	exit_read(false)

{
	sprintf(write_file,"./write_fifo_%ld",(unsigned long)(this));
	sprintf(read_file,"./read_fifo_%ld",(unsigned long)(this));
	errno = 0;
	if (mkfifo(write_file,0660) < 0)
	{
		throw adevs::exception(strerror(errno));
	}
	errno = 0;
	if (mkfifo(read_file,0600) < 0)
	{
		throw adevs::exception(strerror(errno));
	}
	start();
}

void adevs::uCsimSerialPort::append_qemu_arguments(std::vector<std::string>& args)
{
	char serial_port_arg[200];
	sprintf(serial_port_arg,"-Sin=%s,out=%s",write_file,read_file);
	args.push_back(serial_port_arg);
}

adevs::uCsimSerialPort::~uCsimSerialPort()
{
	exit_read = true;
	close(write_fd);
	close(read_fd);
	unlink(write_file);
	unlink(read_file);
}

