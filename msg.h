#ifndef _MSG_H_
#define _MSG_H_
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <map>
enum type
{
    MKDIR,
    RENAME,
    OPEN,
    CLOSE,
    READ,
    WRITE,
    DELETE,
    LISTDIR,
    GETATTR,
    ACCESS,
    RENAMEDIR,
    RMDIR
};
struct operation
{
    type opcode;
    long offset;
    long size;
    char file_path[255];
    char new_file_path[255];
    int fd;
    int file_mode;
    long i_size;
    char d_name[100][255];
    int st_mode[100];
    long st_size[100];
    int file_num;
    int ret;
};
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
int lwfs_connection_client(std::string &ip, int port);

int send_operation_msg(operation &msg, int connfd);
int recv_operation_msg(operation &msg, int connfd);

#endif
