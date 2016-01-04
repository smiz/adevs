#include "adevs_qemu.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

const int adevs::uCsimSerialPort::buf_size = 256;

void adevs::uCsimSerialPort::write(void* data, int num_bytes)
{
	if (::write(write_fd,(char*)data,num_bytes) < num_bytes)
		perror("Serial port write failed");
}

adevs::QemuDeviceModel::io_buffer* adevs::uCsimSerialPort::read()
{
	int num_read = 0;
	adevs::QemuDeviceModel::io_buffer* buf = new adevs::QemuDeviceModel::io_buffer(buf_size);
	if ((num_read = ::read(read_fd,buf->get_data(),buf_size)) <= 0)
	{
		delete buf;
		return NULL;
	}
	buf->reset_size(num_read);
	return buf;
}

adevs::uCsimSerialPort::uCsimSerialPort():
	adevs::QemuDeviceModel()
{
	sprintf(write_file,"./write_fifo_%ld",(unsigned long)(this));
	sprintf(read_file,"./read_fifo_%ld",(unsigned long)(this));
	write_fd = mkfifo(write_file,0x666);
	errno = 0;
	if (write_fd < 0)
	{
		throw adevs::exception(strerror(errno));
	}
	errno = 0;
	read_fd = mkfifo(read_file,0x666);
	if (read_fd < 0)
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
	close(write_fd);
	close(read_fd);
	unlink(write_file);
	unlink(read_file);
}

