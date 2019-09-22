#ifndef _LWFS_SERVER_OPERATION_H_
#define _LWFS_SERVER_OPERATION_H_
#define REMOTE_DIR "/home/lw/data"
#include "msg.h"
extern std::map<std::string,int>file_fd;
char *pathadapt(char *oldpath);
int lwfsListdir(int connfd, char *_path);
int lwfsCreateDirector(int connfd,const char* _path, mode_t mode);
int lwfsRename(int connfd,const char* from, const char* to);
int lwfsRmdir(int connfd,const char *path);
int lwfsDelete(int connfd,const char *_path);
int lwfsGetattr(int connfd,const char *_path);
int lwfsAccess(int connfd,const char *_path);
int lwfsDelete(const char *_path, int connfd);
int lwfsOpen(int connfd,char *_path);

#endif