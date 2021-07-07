#include "lwfs_client_operation.h"

struct fuse_operations lwfsClient::fuse_oper;
int lwfsClient::connfd_ctrl = 0;
int lwfsClient::connfd_data = 0;
std::map<std::string, int> lwfsClient::file_fd;

int lwfsClient::fuse_open(const char *path, struct fuse_file_info *fi)
{
	operation send_msg;
	send_msg.opcode = OPEN;
	strcpy(send_msg.file_path, path);
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	int fd = recv_msg.fd;
	std::string filename = std::string(path);
	file_fd[filename] = fd;
	return 0;
}

int lwfsClient::fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.offset = offset;
	send_msg.size = size;
	send_msg.opcode = READ;
	std::string filename = std::string(path);
	send_msg.fd = file_fd[filename];
	send_operation_msg(send_msg, connfd_data);
	readn(connfd_data, buf, size);
	return size;
}

int lwfsClient::fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.offset = offset;
	send_msg.size = size;
	send_msg.opcode = WRITE;
	std::string filename = std::string(path);
	send_msg.fd = file_fd[filename];
	send_operation_msg(send_msg, connfd_data);
	writen(connfd_data, buf, size);

	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	return recv_msg.size;
}

int lwfsClient::fuse_mknod(const char *path, mode_t mode, dev_t rdev)
{
	operation send_msg;
	send_msg.opcode = MKNOD;
	strcpy(send_msg.file_path, path);
	send_msg.mode = mode;
	send_msg.dev = rdev;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	return recv_msg.ret;
}

int lwfsClient::fuse_access(const char *path, int mask)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.mode = mask;
	send_msg.opcode = ACCESS;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	return recv_msg.ret;
}

int lwfsClient::fuse_chmod(const char *path, mode_t mode)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.mode = mode;
	send_msg.opcode = CHMOD;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	return recv_msg.ret;
}

int lwfsClient::fuse_mkdir(const char *path, mode_t mode)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = MKDIR;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	return recv_msg.ret;
}

int lwfsClient::fuse_rmdir(const char *path)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = DELETE;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	return recv_msg.ret;
}

int lwfsClient::fuse_rename(const char *from, const char *to)
{
	operation send_msg;
	strcpy(send_msg.file_path, from);
	strcpy(send_msg.new_file_path, to);
	send_msg.opcode = RENAME;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	return recv_msg.ret;
}

int lwfsClient::fuse_truncate(const char *path, off_t size)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = TRUNCATE;
	send_msg.size = size;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	return recv_msg.ret;
}

int lwfsClient::fuse_unlink(const char *path)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = UNLINK;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);

	return recv_msg.ret;
}

int lwfsClient::fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
							 off_t offset, struct fuse_file_info *fi)
{
	struct stat st;
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = LISTDIR;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);

	int malloc_size = recv_msg.file_num * sizeof(file_info);
	char *recv_buf = (char *)malloc(malloc_size);
	readn(connfd_ctrl, recv_buf, malloc_size);
	file_info *file_list = (file_info *)recv_buf;
	for (int i = 0; i < recv_msg.file_num; i++)
	{
		filler(buf, file_list[i].d_name, &file_list[i].file_stat, 0);
	}
	free(recv_buf);

	return 0;
}

int lwfsClient::fuse_getattr(const char *path, struct stat *st)
{
	memset(st, 0, sizeof(struct stat));

	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = GETATTR;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);

	memcpy(st, &recv_msg.file_stat, sizeof(struct stat));
	if (recv_msg.ret != 0)
	{
		return -2;
	}

	return 0;
}

int lwfsClient::fuse_symlink(const char * oldpath, const char * newpath)
{
	operation send_msg;
	strcpy(send_msg.file_path, oldpath);
	strcpy(send_msg.new_file_path, newpath);
	send_msg.opcode = SYMLINK;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);

	return 0;
}

int lwfsClient::fuse_readlink(const char * path, char * buf, size_t bufsize)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = READLINK;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	memcpy(buf, recv_msg.new_file_path, recv_msg.size);

	return recv_msg.size;
}

int lwfsClient::fuse_link(const char * oldpath, const char * newpath)
{
	operation send_msg;
	strcpy(send_msg.file_path, oldpath);
	strcpy(send_msg.new_file_path, newpath);
	send_msg.opcode = LINK;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);

	return recv_msg.ret;
}

int lwfsClient::build_connection_with_server()
{
	char *servInetAddr = (char *)server_ip.c_str();
	int socketfd;
	struct sockaddr_in sockaddr;
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(server_port);
	inet_pton(AF_INET, servInetAddr, &sockaddr.sin_addr);
	if ((connect(socketfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr))) < 0)
	{
		printf("connect error %s errno: %d\n", strerror(errno), errno);
		exit(0);
	}
	return socketfd;
}

int lwfsClient::Init()
{
	fuse_oper.open = fuse_open;
	fuse_oper.write = fuse_write;
	fuse_oper.mknod = fuse_mknod;
	fuse_oper.getattr = fuse_getattr;
	fuse_oper.access = fuse_access;
	fuse_oper.chmod = fuse_chmod;
	fuse_oper.readdir = fuse_readdir;
	fuse_oper.mkdir = fuse_mkdir;
	fuse_oper.unlink = fuse_unlink;
	fuse_oper.rmdir = fuse_rmdir;
	fuse_oper.rename = fuse_rename;
	fuse_oper.read = fuse_read;
	fuse_oper.truncate = fuse_truncate;
	fuse_oper.symlink = fuse_symlink;
	fuse_oper.link = fuse_link;
	fuse_oper.readlink = fuse_readlink;
}

int lwfsClient::Run(int argc, char *argv[])
{
	Init();
	connfd_ctrl = build_connection_with_server();
	connfd_data = build_connection_with_server();
	fuse_main(argc - 2, argv + 2, &fuse_oper, NULL);
	return 0;
}