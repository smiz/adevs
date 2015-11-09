#include "adevs_qemu.h"
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <linux/limits.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>

adevs::QemuNic::QemuNic():
	adevs::QemuDeviceModel()
{
	errno = 0;
	if (socketpair(AF_UNIX,SOCK_STREAM,0,fd) < 0)
		throw exception(strerror(errno));
	start();
}

void adevs::QemuNic::write_data(void* data, int size)
{
	int msg_size = htonl(size);
	if (write(fd[0],(char*)&msg_size,sizeof(int)) != sizeof(int))
		throw exception(strerror(errno));
	if (write(fd[0],(char*)data,size) != size)
		throw exception(strerror(errno));
}

adevs::QemuDeviceModel::io_buffer* adevs::QemuNic::read()
{
	int msg_size = 0;
	if (::read(fd[0],(char*)&msg_size,sizeof(int)) != sizeof(int))
	{
		return NULL;
	}
	adevs::QemuDeviceModel::io_buffer* buf = new adevs::QemuDeviceModel::io_buffer(ntohl(msg_size));
	if (::read(fd[0],buf->get_data(),buf->get_size()) != buf->get_size())
	{
		delete buf;
		return NULL;
	}
	return buf;
}

void adevs::QemuNic::append_qemu_arguments(std::vector<std::string>& args)
{
	char nic_arg[100];
	sprintf(nic_arg,"socket,fd=%d",fd[1]);
	args.push_back("-net");
	args.push_back("nic");
	args.push_back("-net");
	args.push_back(nic_arg);
}

adevs::QemuNic::~QemuNic()
{
	shutdown(fd[0],0);
	close(fd[0]);
}
