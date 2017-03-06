#include "adevs_qemu.h"
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cerrno>
#include <linux/limits.h>
#include <fcntl.h>
#include <arpa/inet.h>

const int adevs::QemuSerialPort::buf_size = 256;

void adevs::QemuSerialPort::initialize_io_structures()
{
	while (connect(fd,(struct sockaddr*)&(address),sizeof(struct sockaddr_un)) != 0);
}

void adevs::QemuSerialPort::write_bytes(void* data, int num_bytes)
{
	if (::write(fd,(char*)data,num_bytes) < num_bytes)
		perror("Serial port write failed");
}

adevs::QemuDeviceModel::io_buffer* adevs::QemuSerialPort::read()
{
	int num_read = 0;
	adevs::QemuDeviceModel::io_buffer* buf =
		new adevs::QemuDeviceModel::io_buffer(buf_size);
	if ((num_read = recv(fd,buf->get_data(),buf_size,0)) <= 0)
	{
		delete buf;
		return NULL;
	}
	buf->reset_size(num_read);
	return buf;
}

adevs::QemuSerialPort::QemuSerialPort():
	adevs::QemuDeviceModel()
{
	sprintf(socket_file,"./pc_serial_%ld",(unsigned long)(this));
	errno = 0;
	fd = socket(PF_UNIX,SOCK_STREAM,0);
	if (fd < 0)
	{
		throw adevs::exception(strerror(errno));
	}
	memset(&(address),0,sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	sprintf((address.sun_path),"%s",socket_file);
	start();
}

void adevs::QemuSerialPort::append_qemu_arguments(std::vector<std::string>& args)
{
	char serial_port_arg[200];
	sprintf(serial_port_arg,"unix:%s,server",socket_file);
	args.push_back("-serial");
	args.push_back(serial_port_arg);
}

adevs::QemuSerialPort::~QemuSerialPort()
{
	shutdown(fd,0);
	close(fd);
	unlink(socket_file);
}

