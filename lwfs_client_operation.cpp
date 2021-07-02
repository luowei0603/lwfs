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

	return size;
}

int lwfsClient::fuse_mknod(const char *path, mode_t mode, dev_t rdev)
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

int lwfsClient::fuse_access(const char *path, int mask)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = ACCESS;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);

	if (recv_msg.file_mode == 5)
	{
		return -2;
	}

	return 0;
}

int lwfsClient::fuse_chmod(const char *path, mode_t mode)
{
	return 0;
}

int lwfsClient::fuse_mkdir(const char *path, mode_t mode)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = MKDIR;
	send_operation_msg(send_msg, connfd_ctrl);
	// operation recv_msg;
	// recv_operation_msg(recv_msg,connfd_ctrl);
	// if(recv_msg.ret==-1){
	// 	std::cout<<"createDirectory error"<<std::endl;
	// }
	return 0;
}

int lwfsClient::fuse_rmdir(const char *path)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = DELETE;
	send_operation_msg(send_msg, connfd_ctrl);
	// operation recv_msg;
	// recv_operation_msg(recv_msg,connfd_ctrl);
	// if(recv_msg.ret==-1){
	// 	std::cout<<"delete error"<<std::endl;
	// }
	return 0;
}

int lwfsClient::fuse_rename(const char *from, const char *to)
{
	int type;
	long size;
	operation send_msg;
	strcpy(send_msg.file_path, from);
	strcpy(send_msg.new_file_path, to);
	send_msg.opcode = RENAME;
	send_operation_msg(send_msg, connfd_ctrl);
	// operation recv_msg;
	// recv_operation_msg(recv_msg,connfd_ctrl);
	// if(recv_msg.ret==-1){
	// 	std::cout<<"rename error"<<std::endl;
	// }

	return 0;
}

int lwfsClient::fuse_truncate(const char *path, off_t size)
{

	return 0;
}
int lwfsClient::fuse_statfs(const char *path, struct statvfs *stbuf)
{

	return 0;
}
int lwfsClient::fuse_unlink(const char *path)
{

	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = DELETE;
	send_operation_msg(send_msg, connfd_ctrl);
	// operation recv_msg;
	// recv_operation_msg(recv_msg,connfd);
	// if(recv_msg.ret==-1){
	// 	std::cout<<"delete error"<<std::endl;
	// }

	return 0;
}

int lwfsClient::fuse_release(const char *path, struct fuse_file_info *fi)
{
	return 0;
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

	//except . & ..
	for (int i = 0; i < recv_msg.file_num; i++)
	{

		st.st_mode = recv_msg.st_mode[i];
		filler(buf, recv_msg.d_name[i], &st, 0);
	}

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

	if (recv_msg.file_mode == 5)
	{
		return -2;
	}
	if (recv_msg.file_mode == 1)
	{

		st->st_mode = 0755 | S_IFDIR;
		st->st_size = 4096;
	}
	else if (recv_msg.file_mode == 0)
	{
		st->st_mode = 0644 | S_IFREG;
		st->st_size = recv_msg.i_size;
	}
	st->st_nlink = 1;
	st->st_uid = 0;
	st->st_gid = 0;
	st->st_rdev = 0;
	st->st_atime = 0;
	st->st_mtime = 0;
	st->st_ctime = 0;

	return 0;
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
	fuse_oper.release = fuse_release;
	fuse_oper.read = fuse_read;
	fuse_oper.truncate = fuse_truncate;
	fuse_oper.statfs = fuse_statfs;
}

int lwfsClient::Run(int argc, char *argv[])
{
	Init();
	connfd_ctrl = build_connection_with_server();
	connfd_data = build_connection_with_server();
	fuse_main(argc - 2, argv + 2, &fuse_oper, NULL);
	return 0;
}