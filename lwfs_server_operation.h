#ifndef _LWFS_SERVER_OPERATION_H_
#define _LWFS_SERVER_OPERATION_H_
#include "msg.h"
#include <iostream>

#define PATHADAPT(path, origin_path)                                                                  \
    do                                                                                                \
    {                                                                                                 \
        path = data_dir + (origin_path[0] == '/' ? "" : "/") + std::string(origin_path);              \
        printf("[%s: %s: %d]: %s ==> %s\n", __FILE__, __func__, __LINE__, origin_path, path.c_str()); \
    } while (0)

struct operation_func
{
    int (*func)(const operation &);
    std::string comments;
};

class lwfsServer
{
public:
    static std::string data_dir;
    static int port;
    static std::map<std::string, int> file_fd;
    static int ctrl_fd;
    static int data_fd;
    static operation_func op_table[OP_NUM];

public:
    lwfsServer(std::string &_data_dir, int _port)
    {
        data_dir = _data_dir;
        port = _port;
    }
    ~lwfsServer() {}
    int Run();
    int Init();

    static int Open(const operation &recv_msg);
    static int Close(const operation &recv_msg);
    static int Listdir(const operation &recv_msg);
    static int CreateDirector(const operation &recv_msg);
    static int Rename(const operation &recv_msg);
    static int Rmdir(const operation &recv_msg);
    static int Delete(const operation &recv_msg);
    static int Getattr(const operation &recv_msg);
    static int Access(const operation &recv_msg);
    static int Chmod(const operation &recv_msg);
    static int Mknod(const operation &recv_msg);
    static int Write(const operation &recv_msg);
    static int Read(const operation &recv_msg);
    static int Truncate(const operation &recv_msg);
    static int Symlink(const operation &recv_msg);
    static int Link(const operation &recv_msg);
    static int Unlink(const operation &recv_msg);
    static int Readlink(const operation &recv_msg);
    static int Setxattr(const operation &recv_msg);
    static int Getxattr(const operation &recv_msg);
    static int Listxattr(const operation &recv_msg);
    static int Removexattr(const operation &recv_msg);
    static void *handle_conn(void *args);
};

#endif