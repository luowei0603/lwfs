
#include "lwfs_server_operation.h"

int lwfsServer::port = 0;
std::string lwfsServer::data_dir = "";
std::map<std::string, int> lwfsServer::file_fd;

int lwfsServer::Open(int connfd, const char *_path)
{
    std::string path;
    PATHADAPT(path, _path);
    int fd = open(path.c_str(), O_CREAT | O_RDWR, 0777);
    operation send_msg;
    send_msg.fd = fd;
    send_operation_msg(send_msg, connfd);
    file_fd[path] = fd;
    return fd;
}

int lwfsServer::Listdir(int connfd, const char *_path)
{
    std::string path;
    PATHADAPT(path, _path);
    operation send_msg;
    DIR *dp;
    struct dirent *dirp;
    dp = opendir(path.c_str());
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
    // for (i = 0; i < nums; i++)
    // {
    //     printf("name=%s\tmode=%d\tsize=%d\n", send_msg.d_name[i], send_msg.st_mode[i], send_msg.st_size[i]);
    // }
    closedir(dp);
    send_msg.opcode = LISTDIR;
    send_msg.file_num = nums;
    send_operation_msg(send_msg, connfd);
    return nums;
}

int lwfsServer::CreateDirector(int connfd, const char *_path, mode_t mode)
{
    std::string path;
    PATHADAPT(path, _path);
    printf("call mkdirs (path : %s , mode : %o)\n", path, mode);
    mkdir(path.c_str(), 0777);
    return 0;
}

int lwfsServer::Rename(int connfd, const char *from, const char *to)
{
    std::string oldpath;
    std::string newpath;
    PATHADAPT(oldpath, from);
    PATHADAPT(newpath, to);
    int rc;

    rc = rename(oldpath.c_str(), newpath.c_str());
    if (rc != 0)
    {
        printf("rename error\n");
        return 0;
    }
    return 1;
}

int lwfsServer::Rmdir(int connfd, const char *_path)
{
    std::string newpath;
    PATHADAPT(newpath, _path);
    int ret;
    ret = rmdir(newpath.c_str());
    if (ret < 0)
    {
        printf("rmdir error\n");
        return -1;
    }
    return 0;
}

int lwfsServer::Delete(int connfd, const char *_path)
{
    std::string path;
    PATHADAPT(path, _path);
    int ret;
    ret = remove(path.c_str());
    if (ret < 0)
    {
        printf("remove file error\n");
        return -1;
    }
    return 0;
}

int lwfsServer::Getattr(int connfd, const char *_path)
{
    std::string path;
    PATHADAPT(path, _path);
    struct stat buf;
    int mask;
    operation send_msg;
    if (stat(path.c_str(), &buf) == 0)
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

int lwfsServer::Access(int connfd, const char *_path)
{
    std::string path;
    PATHADAPT(path, _path);
    struct stat buf;
    int mask;
    operation send_msg;
    if (stat(path.c_str(), &buf) == 0)
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

void *lwfsServer::handle_conn_ctrl(void *args)
{

    int connfd = *(int *)args;
    while (1)
    {
        operation recv_msg;
        recv_operation_msg(recv_msg, connfd);
        switch (recv_msg.opcode)
        {
        case MKDIR:
            CreateDirector(connfd, recv_msg.file_path, 0);
            break;
        case RENAME:
            Rename(connfd, recv_msg.file_path, recv_msg.new_file_path);
            break;
        case LISTDIR:
            Listdir(connfd, recv_msg.file_path);
            break;
        case GETATTR:
            Getattr(connfd, recv_msg.file_path);
            break;
        case ACCESS:
            Access(connfd, recv_msg.file_path);
            break;
        case OPEN:
            Open(connfd, recv_msg.file_path);
            break;
        case DELETE:
            Delete(connfd, recv_msg.file_path);
        default:
            break;
        }
    }
}

void *lwfsServer::handle_conn_data(void *args)
{
    int connfd = *(int *)args;
    while (1)
    {
        operation recv_msg;
        recv_operation_msg(recv_msg, connfd);
        if (recv_msg.opcode == READ)
        {
            char *buf = (char *)malloc(recv_msg.size);
            read(recv_msg.fd, buf, recv_msg.size);
            writen(connfd, buf, recv_msg.size);
        }
        if (recv_msg.opcode == WRITE)
        {
            char *buf = (char *)malloc(recv_msg.size);
            readn(connfd, buf, recv_msg.size);
            std::cout << buf << std::endl;
            write(recv_msg.fd, buf, recv_msg.size);
        }
    }
}

int lwfsServer::Run()
{
    mkdir(data_dir.c_str(), 0777);

    int listenfd, connfd_ctrl, connfd_data;
    pid_t childpid;
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
        connfd_ctrl = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        pthread_t ctrl;
        pthread_create(&ctrl, NULL, handle_conn_ctrl, &connfd_ctrl);
        // connection for handle data msg
        connfd_data = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        pthread_t data;
        pthread_create(&data, NULL, handle_conn_data, &connfd_data);
    }
    close(listenfd);
    return 0;
}