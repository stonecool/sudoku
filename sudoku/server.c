#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include "../peer.h"
#include "event.h"

#define PORT	6666

extern int make_socket(uint16_t port);
void sudoku(char *input);

void sockReadProc(eventLoop* el, int fd, void *clientData);
// 普通socket的write处理函数
void sockWriteProc(eventLoop* el, int fd, void *clientData)
{
	int ret = writeToPeer(fd, clientData);

	if (ret > 0)
	{
		delOneEvent(el, fd, EVENT_WRITE);
		if (ret == 15)
		{
			close(fd);
			releasePeer(clientData);
		}
		else
		{
			resetPeer((peer *)clientData);	
			addOneEvent(el, fd, EVENT_WRITE, sockReadProc, clientData);
		}
	}
}

// 普通socket的 read 处理函数
void sockReadProc(eventLoop* el, int fd, void *clientData)
{
	int ret = readFromPeer(fd, clientData);

	if (ret > 0)
	{	
		if (ret == 85)
			sudoku(((peer *)clientData)->buf);

		((peer *)clientData)->offset = 0;
		delOneEvent(el, fd, EVENT_READ);
		addOneEvent(el, fd, EVENT_WRITE, sockWriteProc, clientData);
	}
}

// 监听端口的read 处理函数
void listenSockProc(eventLoop *el, int fd, void* clientData)
{
	struct sockaddr_in clientname;
	size_t size = sizeof(struct sockaddr);
	
	int new = accept(fd, (struct sockaddr*)&clientname, (socklen_t*)&size);
	if (new < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}
	
	// fprintf(stderr, "Server: connect from host: %s, port: %hd\n", inet_ntoa(clientname.sin_addr), ntohs(clientname.sin_port));
	
	peer *p = newPeer();
	addOneEvent(el, new, EVENT_READ, sockReadProc, p);
}


int main(int argc, char** argv)
{
	eventLoop *el = createEventLoop(FD_SETSIZE);

	int sock = make_socket(PORT);
	
	if (listen(sock, 100) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	addOneEvent(el, sock, EVENT_READ, listenSockProc, NULL);
	eventMain(el);
}
