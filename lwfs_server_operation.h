#ifndef _LWFS_SERVER_OPERATION_H_
#define _LWFS_SERVER_OPERATION_H_
#include <iostream>

#include "msg.h"

#define PATHADAPT(path, origin_path)                                  \
  do {                                                                \
    path = data_dir + (origin_path[0] == '/' ? "" : "/") +            \
           std::string(origin_path);                                  \
    printf("[%s: %s: %d]: %s ==> %s\n", __FILE__, __func__, __LINE__, \
           origin_path, path.c_str());                                \
  } while (0)

struct operation_func {
  int (*func)(int, const operation &);
  std::string comments;
};

class lwfsServer {
 public:
  static std::string data_dir;
  static int port;
  static operation_func op_table[OP_NUM];

 public:
  lwfsServer(std::string &_data_dir, int _port) {
    data_dir = _data_dir;
    port = _port;
  }
  ~lwfsServer() {}
  int Run();
  int Init();

  static int Open(int conn_fd, const operation &recv_msg);
  static int Listdir(int conn_fd, const operation &recv_msg);
  static int CreateDirector(int conn_fd, const operation &recv_msg);
  static int Rename(int conn_fd, const operation &recv_msg);
  static int Rmdir(int conn_fd, const operation &recv_msg);
  static int Delete(int conn_fd, const operation &recv_msg);
  static int Getattr(int conn_fd, const operation &recv_msg);
  static int Access(int conn_fd, const operation &recv_msg);
  static int Chmod(int conn_fd, const operation &recv_msg);
  static int Mknod(int conn_fd, const operation &recv_msg);
  static int Write(int conn_fd, const operation &recv_msg);
  static int Read(int conn_fd, const operation &recv_msg);
  static int Truncate(int conn_fd, const operation &recv_msg);
  static int Symlink(int conn_fd, const operation &recv_msg);
  static int Link(int conn_fd, const operation &recv_msg);
  static int Unlink(int conn_fd, const operation &recv_msg);
  static int Readlink(int conn_fd, const operation &recv_msg);
  static int Setxattr(int conn_fd, const operation &recv_msg);
  static int Getxattr(int conn_fd, const operation &recv_msg);
  static int Listxattr(int conn_fd, const operation &recv_msg);
  static int Removexattr(int conn_fd, const operation &recv_msg);
  static int Chown(int conn_fd, const operation &recv_msg);
  static int Utime(int conn_fd, const operation &recv_msg);

  static void *handle_conn(void *args);
};

#endif