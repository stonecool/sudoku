#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "peer.h"


extern int make_socket(uint16_t port);

int main(int argc, char** argv)
{
	/*
	peer *peerList[10];
	int i = 0;

	for(i = 0; i < 10; ++i)
	{
		newPeer(&peerList[i]);
		printf("%s\n", peerList[i]->buf);
	}

	for (i = 0; i < 10; ++i)
	{
		releasePeer(&peerList[i]);
	}
	*/

	struct peer p;
	
	newPeer(&p);
	
	int sock =  make_socket(6666);
	size_t size;
	struct sockaddr_in clientname;

	if (listen(sock, 1) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	int new = accept(sock, (struct sockaddr *)&clientname, (socklen_t *)&size);
	if (new < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}

	int ret = 0;
	while (1)
	{
		ret = readFromPeer(new, &p);
		if (1 == ret)
		{
			break;
		}

		ret = writeToPeer(new, &p);
		if (ret)
		{
			perror("write error");	
		}
	}

	close(new);
}
