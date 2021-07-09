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
#include <utime.h>

enum type
{
    MKDIR = 0,
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
    RMDIR,
    CHMOD,
    MKNOD,
    TRUNCATE,
    SYMLINK,
    LINK,
    UNLINK,
    READLINK,
    SETXATTR,
    GETXATTR,
    LISTXATTR,
    REMOVEXATTR,
    CHOWN,
    UTIME,
    OP_NUM,
};

struct operation
{
    type opcode;
    long offset;
    long size;
    char file_path[255];
    char new_file_path[255];
    struct stat file_stat;
    int fd;
    int mode;
    dev_t dev;
    int file_num;
    char xattr_key[255];
    char xattr_value[255];
    uid_t uid;
    gid_t gid;
    struct utimbuf time_stamp;
    int ret;
};

struct file_info
{
    char d_name[255];
    struct stat file_stat;
};

ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);

int send_operation_msg(operation &msg, int connfd);
int recv_operation_msg(operation &msg, int connfd);

#endif
