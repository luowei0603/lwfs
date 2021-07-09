#ifndef _LWFS_CLIENT_OPERATION_H_
#define _LWFS_CLIENT_OPERATION_H_
#include "msg.h"
#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64
#include <fuse.h>

#include <iostream>

class lwfsClient {
 public:
  static struct fuse_operations fuse_oper;
  static int connfd_ctrl;
  static int connfd_data;

 private:
  std::string server_ip;
  int server_port;

 public:
  lwfsClient(std::string &ip, int port) : server_ip(ip), server_port(port) {}
  ~lwfsClient() {}
  int Run(int argc, char *argv[]);

 private:
  int build_connection_with_server();
  int Init();
  static int fuse_open(const char *path, struct fuse_file_info *fi);
  static int fuse_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi);
  static int fuse_write(const char *path, const char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi);
  static int fuse_mknod(const char *path, mode_t mode, dev_t rdev);
  static int fuse_access(const char *path, int mask);
  static int fuse_chmod(const char *path, mode_t mode);
  static int fuse_mkdir(const char *path, mode_t mode);
  static int fuse_rmdir(const char *path);
  static int fuse_rename(const char *from, const char *to);
  static int fuse_truncate(const char *path, off_t size);
  static int fuse_unlink(const char *path);
  static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi);
  static int fuse_getattr(const char *path, struct stat *st);
  static int fuse_symlink(const char *oldpath, const char *newpath);
  static int fuse_link(const char *oldpath, const char *newpath);
  static int fuse_readlink(const char *path, char *buf, size_t bufsize);
  static int fuse_setxattr(const char *path, const char *key, const char *value,
                           size_t size, int flag);
  static int fuse_getxattr(const char *path, const char *key, char *value,
                           size_t size);
  static int fuse_listxattr(const char *path, char *list, size_t size);
  static int fuse_removexattr(const char *path, const char *key);
  static int fuse_chown(const char *path, uid_t uid, gid_t gid);
  static int fuse_utime(const char *path, struct utimbuf *time_stamp);
  static int fuse_fsync(const char *path, int fd, struct fuse_file_info *fi);
  static int fuse_release(const char *path,
                          struct fuse_file_info *fi);  // 这个就是close函数
  static int fuse_flush(const char *path,
                        struct fuse_file_info *fi);  // 这个就是flush
};
#endif