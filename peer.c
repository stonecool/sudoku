#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "peer.h"

peer* newPeer()
{
	struct peer *p = (struct peer *)malloc(sizeof(struct peer));
	if (NULL == p)
	{
		perror("newPeer");
		exit(EXIT_FAILURE);
	}

	p->offset = 0;
	p->length = DEFAULT_BUFSIZE;

	// TODO
	// 为了可直接打印，最后一定留一个0字节
	p->buf = (char *)malloc(p->length + 1);
	if (NULL == p->buf)
	{
		perror("newPeer");
		exit(EXIT_FAILURE);
	}

	memset(p->buf, 0, (size_t)p->length + 1);

	return p;
}


void reallocPeer(peer *p, int length)
{
	if (0 == length)
	{
		p->length *= 2;
	}
	else
	{
		p->length = length;
	}

	p->buf = realloc(p->buf, p->length + 1);
	if (NULL == p->buf)
	{
		perror("reallocPeer");
		exit(EXIT_FAILURE);
	}

	memset(p->buf + p->offset, 0, p->offset + 1);
}


void resetPeer(peer *p)
{
	p->offset = 0;
	memset(p->buf, 0, (size_t)p->length + 1);
}


void releasePeer(peer *p)
{
	free(p->buf);
	free(p);
}


int readFromPeer(int sock, peer* p)
{
	int ret = 0, last = 0;

	while (1)
	{
		last = p->length - p->offset;

		ret = read(sock, p->buf + p->offset, last);
		if (ret < 0)
		{
			perror("read");
			exit(EXIT_FAILURE);
		}

		p->offset += ret;
		if (ret == last)
		{
			reallocPeer(p, 0);
		}
		else
		{
			break;
		}
	}

	return 0;
}


int writeToPeer(int sock, peer *p)
{
	int ret = 0;

	ret = write(sock, p->buf, p->offset);
	
	if (ret < 0)
	{
		perror("write");
		exit(EXIT_FAILURE);
	}

	p->offset -= ret;
	if (0 !=  p->offset)
	{
		strncpy(p->buf, p->buf + ret, p->offset);
		return 1;
	}
	else
	{
		return 0;
	}
}


void cpToPeer(peer *p, char *str)
{
	int size = strlen(str);
	
	if (size > p->length)
	{
		reallocPeer(p, size);
	}
	
	p->offset = size;
	memset(p->buf + size, 0, p->length - size);
	strncpy(p->buf, str, size);
}
