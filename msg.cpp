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
int lwfs_connection_client(std::string &ip, int port)
{
	char *servInetAddr = (char *)ip.c_str();
	int socketfd;
	struct sockaddr_in sockaddr;
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, servInetAddr, &sockaddr.sin_addr);
	if ((connect(socketfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr))) < 0)
	{
		printf("connect error %s errno: %d\n", strerror(errno), errno);
		exit(0);
	}
	return socketfd;
}


int send_operation_msg(operation &msg, int connfd)
{
	writen(connfd, &msg, sizeof(operation));
}
int recv_operation_msg(operation &msg, int connfd)
{
	readn(connfd, &msg, sizeof(operation));
}