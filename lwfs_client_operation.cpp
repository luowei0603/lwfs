#include "lwfs_client_operation.h"

int lwfsCreateDirectory(const char *_path, mode_t mode, int connfd)
{

	printf("lwfsCreateDirectory path=%s\n", _path);
	operation send_msg;
	strcpy(send_msg.file_path, _path);
	send_msg.opcode = MKDIR;
	send_operation_msg(send_msg, connfd);
	// operation recv_msg;
	// recv_operation_msg(recv_msg,connfd);
	// if(recv_msg.ret==-1){
	// 	std::cout<<"createDirectory error"<<std::endl;
	// }
	return 0;
}

int lwfsRename(const char *from, const char *to, int connfd)
{
	printf("RENAME  old path : %s , new path : %s \n", from, to);

	int type;
	long size;
	operation send_msg;
	strcpy(send_msg.file_path, from);
	strcpy(send_msg.new_file_path, to);
	send_msg.opcode = RENAME;
	send_operation_msg(send_msg, connfd);
	// operation recv_msg;
	// recv_operation_msg(recv_msg,connfd);
	// if(recv_msg.ret==-1){
	// 	std::cout<<"rename error"<<std::endl;
	// }

	return 0;
}

int lwfsRenameDir(const char *from, const char *to, int connfd)
{

	operation send_msg;
	strcpy(send_msg.file_path, from);
	strcpy(send_msg.new_file_path, to);
	send_msg.opcode = RENAMEDIR;
	send_operation_msg(send_msg, connfd);
	return 0;
}

int lwfsRmdir(const char *path, int connfd)
{

	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = RMDIR;
	send_operation_msg(send_msg, connfd);
	return 0;
}

int lwfsDelete(const char *_path, int connfd)
{

	operation send_msg;
	strcpy(send_msg.file_path, _path);
	send_msg.opcode = DELETE;
	send_operation_msg(send_msg, connfd);
	// operation recv_msg;
	// recv_operation_msg(recv_msg,connfd);
	// if(recv_msg.ret==-1){
	// 	std::cout<<"delete error"<<std::endl;
	// }

	return 0;
}

int lwfsAccess(const char *path, int mask, int connfd)
{
	operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = ACCESS;
	send_operation_msg(send_msg, connfd);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd);

	if (recv_msg.file_mode == 5)
	{
		return -1;
	}

	return 0;
}