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
		delOneEvent(el, fd, MASK_WRITE);
		addOneEvent(el, fd, MASK_READ, sockReadProc, clientData);
	}
}

// 普通socket的 read 处理函数
void sockReadProc(eventLoop* el, int fd, void *clientData)
{
	resetPeer((peer *)clientData);	
	int ret = readFromPeer(fd, clientData);

	delOneEvent(el, fd, MASK_READ);
	if (ret > 0)
	{	
		sudoku(((peer *)clientData)->buf);

		((peer *)clientData)->offset = 0;
		addOneEvent(el, fd, MASK_WRITE, sockWriteProc, clientData);
	}

	//else if (ret == 0)
	//{
//		close(fd);
	//	releasePeer(clientData);
	//}

}

// 监听端口的read 处理函数
void listenSockProc(eventLoop *el, int fd, void* clientData)
{
	struct sockaddr_in clientname;
	size_t size = sizeof(struct sockaddr);
	
	int new = accept(fd, (struct sockaddr*)&clientname, (socklen_t*)&size);
	printf("accept: %d\n", new);
	if (new < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}
	
	fprintf(stderr, "Server: connect from host: %s, port: %hd\n", inet_ntoa(clientname.sin_addr), ntohs(clientname.sin_port));
	
	peer *p = newPeer();
	addOneEvent(el, new, MASK_READ, sockReadProc, p);
}


int main(int argc, char** argv)
{
	int sock = make_socket(PORT);
	
	if (listen(sock, 200) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	printf("listen: %d\n", sock);
	printf("%s, num: %d\n", getEventName(), FD_SETSIZE);
	eventLoop *el = createEventLoop(FD_SETSIZE);
	addOneEvent(el, sock, MASK_READ, listenSockProc, NULL);
	eventMain(el);

	return 0;
}
