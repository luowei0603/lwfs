#include "lwfs_server_operation.h"
void *handle_conn_ctrl(void *args)
{

	int connfd = (int)(*(int *)args);
	while (1)
	{
		operation recv_msg;
		recv_operation_msg(recv_msg, connfd);
		switch (recv_msg.opcode)
		{
		case MKDIR:
			lwfsCreateDirector(connfd, recv_msg.file_path, 0);
			break;
		case RENAME:
			lwfsRename(connfd, recv_msg.file_path, recv_msg.new_file_path);
			break;
		case LISTDIR:
			lwfsListdir(connfd, recv_msg.file_path);
			break;
		case GETATTR:
			lwfsGetattr(connfd, recv_msg.file_path);
			break;
		case ACCESS:
			lwfsAccess(connfd, recv_msg.file_path);
			break;
		case OPEN:
			lwfsOpen(connfd, recv_msg.file_path);
			break;
		case DELETE:
			lwfsDelete(connfd, recv_msg.file_path);
		default:
			break;
		}
	}
}
void *handle_conn_data(void *args)
{
	int connfd = (int)(*(int *)args);
	while (1)
	{
		operation recv_msg;
		recv_operation_msg(recv_msg, connfd);
		if (recv_msg.opcode == READ)
		{
			std::cout << "this is read" << std::endl;
			char *buf = (char *)malloc(recv_msg.size);
			read(recv_msg.fd, buf, recv_msg.size);
			writen(connfd, buf, recv_msg.size);
		}
		if (recv_msg.opcode == WRITE)
		{
			std::cout << "this is write" << std::endl;
			char *buf = (char *)malloc(recv_msg.size);
			readn(connfd, buf, recv_msg.size);
			std::cout << buf << std::endl;
			write(recv_msg.fd, buf, recv_msg.size);
		}
	}
}

int lwfs_connection_server(int port)
{
	int listenfd, connfd_ctrl, connfd_data;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt)))
	{
		perror("setsockopt");
		return -1;
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	clilen = sizeof(cliaddr);
	listen(listenfd, 5);
	while (1)
	{
		// connection for handle ctrl msg
		connfd_ctrl = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
		pthread_t ctrl;
		pthread_create(&ctrl, NULL, handle_conn_ctrl, &connfd_ctrl);
		// connection for handle data msg
		connfd_data = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
		pthread_t data;
		pthread_create(&data, NULL, handle_conn_data, &connfd_data);
	}
	close(listenfd);
	return 0;
}
int main()
{
	int port = 18001;
	mkdir(REMOTE_DIR, 0777);
	lwfs_connection_server(port);
}
