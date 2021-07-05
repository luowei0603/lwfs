#include "lwfs_server_operation.h"
#include <sstream>

int lwfsServer::port = 0;
std::string lwfsServer::data_dir = "";
std::map<std::string, int> lwfsServer::file_fd;
operation_func lwfsServer::op_map[OP_NUM];
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
    return 0;
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
        stat(dirp->d_name, &file_list[num].file_stat);
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
    PATHADAPT(path, recv_msg.file_path);
    mkdir(path.c_str(), recv_msg.mode);
    return 0;
}

int lwfsServer::Rename(const operation &recv_msg)
{
    std::string oldpath;
    std::string newpath;
    PATHADAPT(oldpath, recv_msg.file_path);
    PATHADAPT(newpath, recv_msg.new_file_path);
    return rename(oldpath.c_str(), newpath.c_str());
}

int lwfsServer::Rmdir(const operation &recv_msg)
{
    std::string newpath;
    PATHADAPT(newpath, recv_msg.file_path);
    int ret;
    return rmdir(newpath.c_str());
}

int lwfsServer::Delete(const operation &recv_msg)
{
    std::string path;
    PATHADAPT(path, recv_msg.file_path);
    int ret;
    return remove(path.c_str());
}

int lwfsServer::Getattr(const operation &recv_msg)
{
    std::string path;
    PATHADAPT(path, recv_msg.file_path);
    operation send_msg;
    send_msg.ret = stat(path.c_str(), &send_msg.file_stat);
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
    return 0;
}

int lwfsServer::Close(const operation &recv_msg)
{
    close(recv_msg.fd);
    return 0;
}

int lwfsServer::Chmod(const operation &recv_msg)
{
    std::string path;
    PATHADAPT(path, recv_msg.file_path);
    chmod(path.c_str(), recv_msg.mode);
    return 0;
}

int lwfsServer::Mknod(const operation &recv_msg)
{
    std::string path;
    PATHADAPT(path, recv_msg.file_path);
    mknod(path.c_str(), recv_msg.mode, recv_msg.dev);
    return 0;
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
    readn(data_fd, buf, recv_msg.size);
    write(recv_msg.fd, buf, recv_msg.size);
    free(buf);
    return 0;
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
            printf("-------------\nnow exec: %s\n", op_map[recv_msg.opcode].comments.c_str());
            int ret = op_map[recv_msg.opcode].func(recv_msg);
            if (ret < 0)
            {
                printf("exec failed: %s\n", op_map[recv_msg.opcode].comments.c_str());
            }
        }
    }
}

int lwfsServer::Init()
{
    op_map[MKDIR] = {CreateDirector, "mkdir"};
    op_map[RENAME] = {Rename, "rename"};
    op_map[OPEN] = {Open, "open"};
    op_map[CLOSE] = {Close, "close"};
    op_map[READ] = {Read, "read"};
    op_map[DELETE] = {Delete, "delete"};
    op_map[WRITE] = {Write, "write"};
    op_map[LISTDIR] = {Listdir, "listdir"};
    op_map[GETATTR] = {Getattr, "getattr"};
    op_map[ACCESS] = {Access, "access"};
    op_map[RENAMEDIR] = {Rename, "renamedir"};
    op_map[RMDIR] = {Delete, "rmdir"};
    op_map[CHMOD] = {Chmod, "chmod"};
    op_map[MKNOD] = {Mknod, "mknode"};
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