#ifndef _LWFS_SERVER_OPERATION_H_
#define _LWFS_SERVER_OPERATION_H_
#include "msg.h"
#include <iostream>

#define PATHADAPT(path, origin_path)                                                                           \
    do                                                                                                         \
    {                                                                                                          \
        path = data_dir + std::string(origin_path);                                                            \
        printf("[%s: %s: %d]:adapt path %s to %s\n", __FILE__, __func__, __LINE__, origin_path, path.c_str()); \
    } while (0)

class lwfsServer
{
public:
    static std::string data_dir;
    static int port;
    static std::map<std::string, int> file_fd;

public:
    lwfsServer(std::string &_data_dir, int _port)
    {
        data_dir = _data_dir;
        port = _port;
    }
    ~lwfsServer() {}
    int Run();

    static int Open(int connfd, const char *_path);
    static int Listdir(int connfd, const char *_path);
    static int CreateDirector(int connfd, const char *_path, mode_t mode);
    static int Rename(int connfd, const char *from, const char *to);
    static int Rmdir(int connfd, const char *path);
    static int Delete(int connfd, const char *_path);
    static int Getattr(int connfd, const char *_path);
    static int Access(int connfd, const char *_path);
    static int Delete(const char *_path, int connfd);
    static void *handle_conn_ctrl(void *args);
    static void *handle_conn_data(void *args);
};

#endif