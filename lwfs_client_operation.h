#ifndef _LWFS_CLIENT_OPERATION_H_
#define _LWFS_CLIENT_OPERATION_H_
#include "msg.h"

int lwfsCreateDirectory(const char *_path, mode_t mode, int connfd);
int lwfsRename(const char *from, const char *to, int connfd);
int lwfsRenameDir(const char *from, const char *to, int connfd);
int lwfsRmdir(const char *path, int connfd);
int lwfsDelete(const char *_path, int connfd);
int lwfsAccess(const char *path, int mask,int connfd);
#endif