#include "lwfs_server_operation.h"

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("./lwfs-server port data_dir\n");
		exit(0);
	}
	int port = atol(argv[1]);
	std::string data_dir = std::string(argv[2]);
	lwfsServer *s = new lwfsServer(data_dir, port);
	s->Init();
	s->Run();
	return 0;
}