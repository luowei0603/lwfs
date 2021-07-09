#include "lwfs_client_operation.h"

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("./lwfs-clinet ip port /mnt/fuse -f\n");
    return 0;
  }
  std::string ip = std::string(argv[1]);
  int port = atol(argv[2]);
  lwfsClient *c = new lwfsClient(ip, port);
  c->Run(argc, argv);
  return 0;
}
