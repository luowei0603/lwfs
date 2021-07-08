#include "lwfs_server_operation.h"
#include <sstream>
#include <sys/xattr.h>

int lwfsServer::port = 0;
std::string lwfsServer::data_dir = "";
std::map<std::string, int> lwfsServer::file_fd;
operation_func lwfsServer::op_table[OP_NUM];
int lwfsServer::ctrl_fd;
int lwfsServer::data_fd;

int lwfsServer::Open(const operation &recv_msg)
{
    std::string path;
    PATHADAPT(path, recv_msg.file_path);
    int fd = open(path.c_str(), O_CREAT | O_RDWR, recv_msg.mode);
    operation send_msg;
    send_msg.fd = fd;
    send_operation_msg(send_msg, ctrl_fd);
    file_fd[path] = fd;
    return fd;
}

int lwfsServer::Listdir(const operation &recv_msg)
{
    std::string path;
    PATHADAPT(path, recv_msg.file_path);
    operation send_msg;
    DIR *dp;
    struct dirent *dirp;
    const int malloc_num = 20;
    file_info *file_list = NULL;
    dp = opendir(path.c_str());
    if (dp == NULL)
    {
        printf("opendir error  %s \n", strerror(errno));
        return -1;
    }
    int num = 0;
    while ((dirp = readdir(dp)) != NULL)
    {
        if (num % malloc_num == 0) {
            file_list = (file_info *)realloc(file_list, (num + malloc_num) * sizeof(file_info));
        }
        lstat(dirp->d_name, &file_list[num].file_stat);
        strcpy(file_list[num].d_name, dirp->d_name);
        num++;
    }

    closedir(dp);
    send_msg.opcode = LISTDIR;
    send_msg.file_num = num;
    send_operation_msg(send_msg, ctrl_fd);
    writen(ctrl_fd, (char *)file_list, num * sizeof(file_info));
    free(file_list);
    return 0;
}

int lwfsServer::CreateDirector(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);
    send_msg.ret = mkdir(path.c_str(), recv_msg.mode);

    send_msg.opcode = MKDIR;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Rename(const operation &recv_msg)
{
    std::string oldpath;
    std::string newpath;
    operation send_msg;
    PATHADAPT(oldpath, recv_msg.file_path);
    PATHADAPT(newpath, recv_msg.new_file_path);
    send_msg.ret = rename(oldpath.c_str(), newpath.c_str());
    send_msg.opcode = RENAME;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Rmdir(const operation &recv_msg)
{
    std::string newpath;
    operation send_msg;
    PATHADAPT(newpath, recv_msg.file_path);
    send_msg.ret = rmdir(newpath.c_str());
    send_msg.opcode = RMDIR;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Delete(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);
    send_msg.ret = remove(path.c_str());
    send_msg.opcode = DELETE;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Getattr(const operation &recv_msg)
{
    std::string path;
    PATHADAPT(path, recv_msg.file_path);
    operation send_msg;
    // lstat可以获取软链接信息，而stat不行
    send_msg.ret = lstat(path.c_str(), &send_msg.file_stat);
    send_msg.opcode = GETATTR;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Access(const operation &recv_msg)
{
    std::string path;
    PATHADAPT(path, recv_msg.file_path);
    operation send_msg;
    send_msg.ret = access(path.c_str(), recv_msg.mode);
    send_msg.opcode = ACCESS;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Close(const operation &recv_msg)
{
    close(recv_msg.fd);
    return 0;
}

int lwfsServer::Chmod(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);
    send_msg.ret = chmod(path.c_str(), recv_msg.mode);
    send_msg.opcode = CHMOD;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Mknod(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);
    send_msg.ret = mknod(path.c_str(), recv_msg.mode, recv_msg.dev);
    send_msg.opcode = MKNOD;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Truncate(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);
    send_msg.ret = truncate(path.c_str(), recv_msg.size);
    send_msg.opcode = TRUNCATE;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Read(const operation &recv_msg)
{
    char *buf = (char *)malloc(recv_msg.size);
    read(recv_msg.fd, buf, recv_msg.size);
    writen(data_fd, buf, recv_msg.size);
    free(buf);
    return 0;
}

int lwfsServer::Write(const operation &recv_msg)
{
    char *buf = (char *)malloc(recv_msg.size);
    operation send_msg;
    readn(data_fd, buf, recv_msg.size);
    send_msg.size = write(recv_msg.fd, buf, recv_msg.size);
    free(buf);
    send_msg.opcode = WRITE;
    send_operation_msg(send_msg, ctrl_fd);

    return 0;
}

int lwfsServer::Symlink(const operation &recv_msg)
{
    std::string oldpath;
    std::string newpath;
    operation send_msg;
    PATHADAPT(oldpath, recv_msg.file_path);
    PATHADAPT(newpath, recv_msg.new_file_path);
    send_msg.ret = symlink(oldpath.c_str(), newpath.c_str());
    send_msg.opcode = SYMLINK;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Link(const operation &recv_msg)
{
    std::string oldpath;
    std::string newpath;
    operation send_msg;
    PATHADAPT(oldpath, recv_msg.file_path);
    PATHADAPT(newpath, recv_msg.new_file_path);
    send_msg.ret = link(oldpath.c_str(), newpath.c_str());
    send_msg.opcode = LINK;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Unlink(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);
    send_msg.ret = unlink(path.c_str());
    send_msg.opcode = UNLINK;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Readlink(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);
    send_msg.size = readlink(path.c_str(), send_msg.new_file_path, recv_msg.size);
    send_msg.opcode = READLINK;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.size;
}

int lwfsServer::Setxattr(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);

    send_msg.ret = setxattr(path.c_str(), recv_msg.xattr_key, recv_msg.xattr_value, recv_msg.size, recv_msg.mode);
    send_msg.opcode = SETXATTR;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

int lwfsServer::Getxattr(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);

    printf("getattr path:%s key:%s size:%d\n", path.c_str(), recv_msg.xattr_key, recv_msg.size);

    send_msg.size = getxattr(path.c_str(), recv_msg.xattr_key, send_msg.xattr_value, recv_msg.size);
    send_msg.opcode = GETXATTR;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.size;
}

int lwfsServer::Listxattr(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);

    char *list_buf = (char *)malloc(recv_msg.size);
    send_msg.size = listxattr(path.c_str(), list_buf, recv_msg.size);
    send_msg.opcode = LISTXATTR;
    send_operation_msg(send_msg, ctrl_fd);
    writen(ctrl_fd, list_buf, send_msg.size);

    return send_msg.size;
}

int lwfsServer::Removexattr(const operation &recv_msg)
{
    std::string path;
    operation send_msg;
    PATHADAPT(path, recv_msg.file_path);

    send_msg.ret = removexattr(path.c_str(), recv_msg.xattr_key);
    send_msg.opcode = REMOVEXATTR;
    send_operation_msg(send_msg, ctrl_fd);

    return send_msg.ret;
}

void *lwfsServer::handle_conn(void *args)
{
    int connfd = *(int *)args;
    while (1)
    {
        operation recv_msg;
        recv_operation_msg(recv_msg, connfd);

        if (recv_msg.opcode < 0 || recv_msg.opcode >= OP_NUM)
        {
            printf("-----------\nerror opcode\n");
        }
        else
        {
            printf("-------------\nnow exec: %s\n", op_table[recv_msg.opcode].comments.c_str());
            int ret = op_table[recv_msg.opcode].func(recv_msg);
            if (ret < 0)
            {
                printf("exec failed: %s reason:%s\n", op_table[recv_msg.opcode].comments.c_str(), strerror(errno));
            }
        }
    }
}

int lwfsServer::Init()
{
    op_table[MKDIR] = {CreateDirector, "mkdir"};
    op_table[RENAME] = {Rename, "rename"};
    op_table[OPEN] = {Open, "open"};
    op_table[CLOSE] = {Close, "close"};
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
    
    // Getxattr时要去获取文件的security.selinux属性，我的ubuntu系统默认没有开启selinux，会报错，所以暂时注释
    // op_table[SETXATTR] = {Setxattr, "setxattr"};
    // op_table[GETXATTR] = {Getxattr, "getxattr"};
    // op_table[LISTXATTR] = {Listxattr, "listxattr"};
    // op_table[REMOVEXATTR] = {Removexattr, "removexattr"};
}

int lwfsServer::Run()
{
    int listenfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt)))
    {
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
    while (1)
    {
        // connection for handle ctrl msg
        ctrl_fd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        pthread_t ctrl;
        pthread_create(&ctrl, NULL, handle_conn, &ctrl_fd);
        // connection for handle data msg
        data_fd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        pthread_t data;
        pthread_create(&data, NULL, handle_conn, &data_fd);
    }
    close(listenfd);
    return 0;
}