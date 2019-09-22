.PHONY: clean

CFLAGS  := -Wall -Werror -g 
LD      := g++
LDLIBS  := ${LDLIBS} -libverbs -lpthread -lfuse

APPS    := lwfs-client lwfs-server

all: ${APPS}

lwfs-client: msg.o lwfs_client.o lwfs_client_operation.o
	${LD} -o $@ $^ ${LDLIBS}

lwfs-server: msg.o lwfs_server.o lwfs_server_operation.o
	${LD} -o $@ $^ ${LDLIBS}

clean:
	rm -f *.o ${APPS}
