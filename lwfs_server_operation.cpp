#include "lwfs_server_operation.h"

#include <sys/xattr.h>

#include <sstream>

int lwfsServer::port = 0;
std::string lwfsServer::data_dir = "";
operation_func lwfsServer::op_table[OP_NUM];

int lwfsServer::Open(int conn_fd, const operation &recv_msg) {
  std::string path;
  PATHADAPT(path, recv_msg.file_path);
  operation send_msg;
  send_msg.ret = open(path.c_str(), O_CREAT | O_RDWR, recv_msg.mode);
  send_operation_msg(send_msg, conn_fd);
  if (send_msg.ret > 0) {
    close(send_msg.ret);
  }
  return send_msg.ret;
}

int lwfsServer::Listdir(int conn_fd, const operation &recv_msg) {
  std::string path;
  PATHADAPT(path, recv_msg.file_path);
  operation send_msg;
  DIR *dp;
  struct dirent *dirp;
  const int malloc_num = 20;
  file_info *file_list = NULL;
  dp = opendir(path.c_str());
  if (dp == NULL) {
    printf("opendir error  %s \n", strerror(errno));
    return -1;
  }
  int num = 0;
  while ((dirp = readdir(dp)) != NULL) {
    if (num % malloc_num == 0) {
      file_list = (file_info *)realloc(file_list,
                                       (num + malloc_num) * sizeof(file_info));
    }
    lstat(dirp->d_name, &file_list[num].file_stat);
    strcpy(file_list[num].d_name, dirp->d_name);
    num++;
  }

  closedir(dp);
  send_msg.opcode = LISTDIR;
  send_msg.file_num = num;
  send_operation_msg(send_msg, conn_fd);
  writen(conn_fd, (char *)file_list, num * sizeof(file_info));
  free(file_list);
  return 0;
}

int lwfsServer::CreateDirector(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);
  send_msg.ret = mkdir(path.c_str(), recv_msg.mode);

  send_msg.opcode = MKDIR;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Rename(int conn_fd, const operation &recv_msg) {
  std::string oldpath;
  std::string newpath;
  operation send_msg;
  PATHADAPT(oldpath, recv_msg.file_path);
  PATHADAPT(newpath, recv_msg.new_file_path);
  send_msg.ret = rename(oldpath.c_str(), newpath.c_str());
  send_msg.opcode = RENAME;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Rmdir(int conn_fd, const operation &recv_msg) {
  std::string newpath;
  operation send_msg;
  PATHADAPT(newpath, recv_msg.file_path);
  send_msg.ret = rmdir(newpath.c_str());
  send_msg.opcode = RMDIR;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Delete(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);
  send_msg.ret = remove(path.c_str());
  send_msg.opcode = DELETE;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Getattr(int conn_fd, const operation &recv_msg) {
  std::string path;
  PATHADAPT(path, recv_msg.file_path);
  operation send_msg;
  // lstat可以获取软链接信息，而stat不行
  send_msg.ret = lstat(path.c_str(), &send_msg.file_stat);
  send_msg.opcode = GETATTR;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Access(int conn_fd, const operation &recv_msg) {
  std::string path;
  PATHADAPT(path, recv_msg.file_path);
  operation send_msg;
  send_msg.ret = access(path.c_str(), recv_msg.mode);
  send_msg.opcode = ACCESS;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Chmod(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);
  send_msg.ret = chmod(path.c_str(), recv_msg.mode);
  send_msg.opcode = CHMOD;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Mknod(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);
  send_msg.ret = mknod(path.c_str(), recv_msg.mode, recv_msg.dev);
  send_msg.opcode = MKNOD;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Truncate(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);
  send_msg.ret = truncate(path.c_str(), recv_msg.size);
  send_msg.opcode = TRUNCATE;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Read(int conn_fd, const operation &recv_msg) {
  std::string path;
  PATHADAPT(path, recv_msg.file_path);
  int fd = open(path.c_str(), O_RDONLY);
  lseek(fd, recv_msg.offset, 0);
  char *buf = (char *)malloc(recv_msg.size);
  read(fd, buf, recv_msg.size);
  close(fd);
  writen(conn_fd, buf, recv_msg.size);
  free(buf);

  return 0;
}

int lwfsServer::Write(int conn_fd, const operation &recv_msg) {
  std::string path;
  PATHADAPT(path, recv_msg.file_path);
  int fd = open(path.c_str(), O_WRONLY);
  lseek(fd, recv_msg.offset, 0);
  char *buf = (char *)malloc(recv_msg.size);
  operation send_msg;
  readn(conn_fd, buf, recv_msg.size);
  send_msg.size = write(fd, buf, recv_msg.size);
  close(fd);
  free(buf);
  send_msg.opcode = WRITE;
  send_operation_msg(send_msg, conn_fd);

  return 0;
}

int lwfsServer::Symlink(int conn_fd, const operation &recv_msg) {
  std::string oldpath;
  std::string newpath;
  operation send_msg;
  PATHADAPT(oldpath, recv_msg.file_path);
  PATHADAPT(newpath, recv_msg.new_file_path);
  send_msg.ret = symlink(oldpath.c_str(), newpath.c_str());
  send_msg.opcode = SYMLINK;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Link(int conn_fd, const operation &recv_msg) {
  std::string oldpath;
  std::string newpath;
  operation send_msg;
  PATHADAPT(oldpath, recv_msg.file_path);
  PATHADAPT(newpath, recv_msg.new_file_path);
  send_msg.ret = link(oldpath.c_str(), newpath.c_str());
  send_msg.opcode = LINK;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Unlink(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);
  send_msg.ret = unlink(path.c_str());
  send_msg.opcode = UNLINK;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Readlink(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);
  send_msg.size = readlink(path.c_str(), send_msg.new_file_path, recv_msg.size);
  send_msg.opcode = READLINK;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.size;
}

int lwfsServer::Setxattr(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);

  send_msg.ret = setxattr(path.c_str(), recv_msg.xattr_key,
                          recv_msg.xattr_value, recv_msg.size, recv_msg.mode);
  send_msg.opcode = SETXATTR;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Getxattr(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);

  printf("getattr path:%s key:%s size:%d\n", path.c_str(), recv_msg.xattr_key,
         recv_msg.size);

  send_msg.size = getxattr(path.c_str(), recv_msg.xattr_key,
                           send_msg.xattr_value, recv_msg.size);
  send_msg.opcode = GETXATTR;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.size;
}

int lwfsServer::Listxattr(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);

  char *list_buf = (char *)malloc(recv_msg.size);
  send_msg.size = listxattr(path.c_str(), list_buf, recv_msg.size);
  send_msg.opcode = LISTXATTR;
  send_operation_msg(send_msg, conn_fd);
  writen(conn_fd, list_buf, send_msg.size);

  return send_msg.size;
}

int lwfsServer::Removexattr(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);

  send_msg.ret = removexattr(path.c_str(), recv_msg.xattr_key);
  send_msg.opcode = REMOVEXATTR;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Chown(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);

  send_msg.ret = chown(path.c_str(), recv_msg.uid, recv_msg.gid);
  send_msg.opcode = CHOWN;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

int lwfsServer::Utime(int conn_fd, const operation &recv_msg) {
  std::string path;
  operation send_msg;
  PATHADAPT(path, recv_msg.file_path);

  send_msg.ret = utime(path.c_str(), &recv_msg.time_stamp);
  send_msg.opcode = UTIME;
  send_operation_msg(send_msg, conn_fd);

  return send_msg.ret;
}

void *lwfsServer::handle_conn(void *args) {
  int conn_fd = *(int *)args;
  printf("in handle_conn connfd=%d\n", conn_fd);
  while (1) {
    operation recv_msg;
    recv_operation_msg(recv_msg, conn_fd);
    if (recv_msg.magic != 25535) {
      printf("-----------\nerror magic and close connection\n");
      close(conn_fd);
      break;
    }
    if (recv_msg.opcode < 0 || recv_msg.opcode >= OP_NUM) {
      printf("-----------\nerror opcode and close connection\n");
      close(conn_fd);
      break;
    } else {
      printf("-------------\nnow exec: %s\n",
             op_table[recv_msg.opcode].comments.c_str());
      int ret = op_table[recv_msg.opcode].func(conn_fd, recv_msg);
      if (ret < 0) {
        printf("exec failed: %s reason:%s\n",
               op_table[recv_msg.opcode].comments.c_str(), strerror(errno));
      }
    }
  }
}

int lwfsServer::Init() {
  op_table[MKDIR] = {CreateDirector, "mkdir"};
  op_table[RENAME] = {Rename, "rename"};
  op_table[OPEN] = {Open, "open"};
  op_table[READ] = {Read, "read"};
  op_table[DELETE] = {Delete, "delete"};
  op_table[WRITE] = {Write, "write"};
  op_table[LISTDIR] = {Listdir, "listdir"};
  op_table[GETATTR] = {Getattr, "getattr"};
  op_table[ACCESS] = {Access, "access"};
  op_table[RENAMEDIR] = {Rename, "renamedir"};
  op_table[RMDIR] = {Delete, "rmdir"};
  op_table[CHMOD] = {Chmod, "chmod"};
  op_table[MKNOD] = {Mknod, "mknode"};
  op_table[TRUNCATE] = {Truncate, "truncate"};
  op_table[SYMLINK] = {Symlink, "symlink"};
  op_table[LINK] = {Link, "hardlink"};
  op_table[UNLINK] = {Unlink, "unlink"};
  op_table[READLINK] = {Readlink, "readlink"};
  op_table[UTIME] = {Readlink, "readlink"};
  op_table[SETXATTR] = {Setxattr, "setxattr"};
  op_table[GETXATTR] = {Getxattr, "getxattr"};
  op_table[LISTXATTR] = {Listxattr, "listxattr"};
  op_table[REMOVEXATTR] = {Removexattr, "removexattr"};
  op_table[CHOWN] = {Chown, "chown"};
  op_table[UTIME] = {Utime, "utime"};
}

int lwfsServer::Run() {
  int listenfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt,
                 sizeof(opt))) {
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
  const int max_conn_num = 10000;
  int conn_fds[max_conn_num];
  int i = 0;
  while (1) {
    if (i >= max_conn_num) {
      printf("conn_num overflow:%d\n", max_conn_num);
      continue;
    }
    conn_fds[i] = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    printf("in main connfd=%d\n", conn_fds[i]);
    pthread_t pid;
    pthread_create(&pid, NULL, handle_conn, &conn_fds[i]);
    i++;
  }
  close(listenfd);
  return 0;
}