
#include "lwfs_server_operation.h"
std::map<std::string, int> file_fd;
int lwfsOpen(int connfd, char *_path)
{
    char *path = pathadapt(_path);
    int fd = open(path, O_CREAT | O_RDWR, 0777);
    operation send_msg;
    send_msg.fd = fd;
    send_operation_msg(send_msg, connfd);
    std::string filename = std::string(path);
    file_fd[filename] = fd;
    return fd;
}

int lwfsListdir(int connfd, char *_path)
{

    operation send_msg;
    char *path = pathadapt(_path);
    DIR *dp;
    struct dirent *dirp;
    dp = opendir(path);
    if (dp == NULL)
    {
        printf("opendir error  %s \n", strerror(errno));
        return -1;
    }
    int nums = 0;
    while ((dirp = readdir(dp)) != NULL)
    {
        struct stat tmp;
        stat(dirp->d_name, &tmp);
        if (dirp->d_name[0] == '.')
            continue;
        strcpy(send_msg.d_name[nums], dirp->d_name);
        send_msg.st_mode[nums] = tmp.st_mode;
        send_msg.st_size[nums] = tmp.st_size;
        nums += 1;
    }
    int i;
    for (i = 0; i < nums; i++)
    {
        printf("\nname=%s\tmode=%d\tsize=%d\n", send_msg.d_name[i], send_msg.st_mode[i], send_msg.st_size[i]);
    }
    closedir(dp);
    send_msg.opcode = LISTDIR;
    send_msg.file_num = nums;
    send_operation_msg(send_msg, connfd);
    printf("send list back \n");
    return nums;
}

char *pathadapt(char *oldpath)
{

    char *addpath = REMOTE_DIR;

    char *newpath = (char *)malloc(strlen(oldpath) + strlen(addpath) + 1);

    strcpy(newpath, addpath);
    strcat(newpath, oldpath);

    printf("path new :%s\n", newpath);

    return newpath;
}

int lwfsCreateDirector(int connfd, const char *_path, mode_t mode)
{

    char *path = pathadapt((char *)_path);
    printf("call mkdirs (path : %s , mode : %o)\n", path, mode);
    mkdir(path, 0777);
    return 0;
}

int lwfsRename(int connfd, const char *from, const char *to)
{

    int rc;
    char *oldpath = NULL;
    oldpath = pathadapt((char *)from);
    char *newpath = pathadapt((char *)to);
    printf("old path is %s\n", oldpath);
    printf("new path is %s\n", newpath);

    rc = rename(oldpath, newpath);
    if (rc != 0)
    {
        printf("rename error\n");
        return 0;
    }
    return 1;
}
/*
lwfsRmdir used to relisize rmdir
*/
int lwfsRmdir(int connfd, const char *path)
{

    char *newpath = pathadapt((char *)path);
    int ret;
    ret = rmdir(newpath);
    if (ret < 0)
    {
        printf("rmdir error\n");
        return -1;
    }
    return 0;
}

int lwfsDelete(int connfd, const char *_path)
{

    char *newpath = pathadapt((char *)_path);
    int ret;
    ret = remove(newpath);
    if (ret < 0)
    {
        printf("remove file error\n");
        return -1;
    }
    return 0;
}

int lwfsGetattr(int connfd, const char *_path)
{

    struct stat buf;
    char *newpath = pathadapt((char *)_path);
    int mask;
    printf("get attr function .new path is %s\n", newpath);
    operation send_msg;
    if (stat(newpath, &buf) == 0)
    {
        mask = buf.st_mode & S_IFMT;
        if (mask == S_IFDIR)
            send_msg.file_mode = 1;
        else if (mask == S_IFREG)
            send_msg.file_mode = 0;
        send_msg.i_size = buf.st_size;
    }
    else
    {
        send_msg.file_mode = 5;
    }
    send_msg.opcode = GETATTR;
    send_operation_msg(send_msg, connfd);
    return 0;
}

int lwfsAccess(int connfd, const char *_path)
{

    struct stat buf;
    char *newpath = pathadapt((char *)_path);
    int mask;
    operation send_msg;
    if (stat(newpath, &buf) == 0)
    {

        mask = buf.st_mode & S_IFMT;
        if (mask == S_IFDIR)
            send_msg.file_mode = 1;
        else if (mask == S_IFREG)
            send_msg.file_mode = 0;
        send_msg.i_size = buf.st_size;
    }
    else
    {
        send_msg.file_mode = 5;
    }
    send_msg.opcode = ACCESS;
    send_operation_msg(send_msg, connfd);
    return 0;
}