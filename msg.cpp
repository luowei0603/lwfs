#include "msg.h"

ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char *ptr;
	ptr = (char *)vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nread = read(fd, ptr, nleft)) < 0)
		{
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if (nread == 0)
			break;
		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft);
}
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwriten;
	const char *ptr;

	ptr = (char *)vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nwriten = write(fd, ptr, nleft)) <= 0)
		{
			if (nwriten < 0 && errno == EINTR)
				nwriten = 0;
			else
				return -1;
		}
		nleft -= nwriten;
		ptr += nwriten;
	}
	return n;
}

int send_operation_msg(operation &msg, int connfd)
{
	msg.magic = MAGIC;
	writen(connfd, &msg, sizeof(operation));
}
int recv_operation_msg(operation &msg, int connfd)
{
	readn(connfd, &msg, sizeof(operation));
}