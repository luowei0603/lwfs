#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64
#include <fuse.h>

#include "lwfs_client_operation.h"

static struct fuse_operations fuse_oper;
int connfd_ctrl, connfd_data;
std::map<std::string,int>file_fd;
static int fuse_open(const char *path, struct fuse_file_info *fi)
{
    operation send_msg;
    send_msg.opcode=OPEN;
    strcpy(send_msg.file_path,path);
    send_operation_msg(send_msg,connfd_ctrl);
    operation recv_msg;
    recv_operation_msg(recv_msg,connfd_ctrl);
    int fd=recv_msg.fd;
    std::string filename=std::string(path);
    file_fd[filename]=fd;
    return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    operation send_msg;
    strcpy(send_msg.file_path,path);
    send_msg.offset=offset;
    send_msg.size=size;
    send_msg.opcode=READ;
    std::string filename=std::string(path);
    send_msg.fd=file_fd[filename];
    send_operation_msg(send_msg,connfd_data);
    readn(connfd_data, buf, size);
    return size;
}

static int fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{

    operation send_msg;
    strcpy(send_msg.file_path,path);
    send_msg.offset=offset;
    send_msg.size=size;
    send_msg.opcode=WRITE;
    std::string filename=std::string(path);
    send_msg.fd=file_fd[filename];
    send_operation_msg(send_msg,connfd_data);
    writen(connfd_data, buf, size);

    return size;
}

static int fuse_mknod(const char *path, mode_t mode, dev_t rdev)
{
    operation send_msg;
    send_msg.opcode=OPEN;
    strcpy(send_msg.file_path,path);
    send_operation_msg(send_msg,connfd_ctrl);
    operation recv_msg;
    recv_operation_msg(recv_msg,connfd_ctrl);
    int fd=recv_msg.fd;
    std::string filename=std::string(path);
    file_fd[filename]=fd;
    return 0;
}

static int fuse_access(const char *path, int mask)
{
    int res;
    res = lwfsAccess(path, 0,connfd_ctrl);
    if (res == 0)
        return 0;
    else
        return -2;
}

static int fuse_chmod(const char *path, mode_t mode)
{
    return 0;
}

static int fuse_mkdir(const char *path, mode_t mode)
{
    int ret = lwfsCreateDirectory(path,0,connfd_ctrl);
    return 0;
}

static int fuse_rmdir(const char *path)
{
    lwfsDelete(path, connfd_ctrl);
    return 0;
}
static int fuse_rename(const char *from, const char *to)
{
    lwfsRename(from, to, connfd_ctrl);
    return 0;
}

static int fuse_truncate(const char *path, off_t size)
{
   
    return 0;
}
static int fuse_statfs(const char *path, struct statvfs *stbuf)
{
    
    return 0;
}
static int fuse_unlink(const char *path)
{

    lwfsDelete(path, connfd_ctrl);

    return 0;
}
static int fuse_release(const char *path, struct fuse_file_info *fi)
{
    return 0;
}

static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi)
{
    struct stat st;
    operation send_msg;
	strcpy(send_msg.file_path, path);
	send_msg.opcode = LISTDIR;
	send_operation_msg(send_msg, connfd_ctrl);
	operation recv_msg;
	recv_operation_msg(recv_msg, connfd_ctrl);
	

    //except . & ..
    for (int i = 0; i < recv_msg.file_num; i++)
    {

        st.st_mode = recv_msg.st_mode[i];
        filler(buf, recv_msg.d_name[i], &st, 0);
    }

    return 0;
}

static int fuse_getattr(const char *path, struct stat *st)
{
   
    memset(st, 0, sizeof(struct stat));
    
    operation send_msg;
    strcpy(send_msg.file_path, path);
    send_msg.opcode = GETATTR;
    send_operation_msg(send_msg,connfd_ctrl);
    operation recv_msg;
    recv_operation_msg(recv_msg,connfd_ctrl);

   
    if (recv_msg.file_mode == 5)
    {
        printf("fuse_getattr()  no exist  over!!!!!!!\n");
        return -2;
    }
    if (recv_msg.file_mode == 1)
    {
        
        st->st_mode = 0755 | S_IFDIR;
        st->st_size = 4096;
       
    }
    else if (recv_msg.file_mode == 0)
    {
        st->st_mode = 0644 | S_IFREG;
        st->st_size = recv_msg.i_size;
    }
    st->st_nlink = 1;
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    st->st_atime = 0;
    st->st_mtime = 0;
    st->st_ctime = 0;

    printf("fuse_getattr() ## return 0 ## OK  over!!!!!!!\n");
    return 0;
}

int main(int argc, char *argv[])
{

    fuse_oper.open = fuse_open;
    fuse_oper.write = fuse_write;
    fuse_oper.mknod = fuse_mknod;
    fuse_oper.getattr = fuse_getattr;
    fuse_oper.access = fuse_access;
    fuse_oper.chmod = fuse_chmod;
    fuse_oper.readdir = fuse_readdir;

    fuse_oper.mkdir = fuse_mkdir;
    fuse_oper.unlink = fuse_unlink;
    fuse_oper.rmdir = fuse_rmdir;
    fuse_oper.rename = fuse_rename;
    fuse_oper.release = fuse_release;
    fuse_oper.read = fuse_read;
    fuse_oper.truncate = fuse_truncate;
    fuse_oper.statfs = fuse_statfs;
    std::string ip="127.0.0.1";
    connfd_ctrl = lwfs_connection_client(ip, 18001);
    connfd_data = lwfs_connection_client(ip, 18001);
    fuse_main(argc, argv, &fuse_oper,NULL);
}
