#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "peer.h"

peer* newPeer()
{
	struct peer *p = (struct peer *)malloc(sizeof(struct peer));
	char *c = (char *)malloc(DEFAULT_BUFSIZE);
	if (NULL == p || NULL == c)
	{
		perror("malloc");
		goto err;
	}

	memset(c, 0, DEFAULT_BUFSIZE);
	
	p->buf = c;
	p->offset = 0;
	p->size = 0;
	p->length = DEFAULT_BUFSIZE;

	// TODO
	// 为了可直接打印，最后一定留一个0字节?
	// 暂时去掉，需要吗，这么处理需要吗？

	return p;

err:
	if (p)
	{
		free(p->buf);
		free(p);
	}
	
	return NULL;
}


void reallocPeer(peer *p, int length)
{
	if (0 == length)
		p->length *= 2;
	else
		p->length = length;

	p->buf = realloc(p->buf, p->length);
	if (NULL == p->buf)
	{
		perror("reallocPeer");
		exit(EXIT_FAILURE);
	}

	memset(p->buf + p->offset - sizeof(int), 0, p->offset);
}


void resetPeer(peer *p)
{
	p->offset = 0;
	p->size = 0;
	memset(p->buf, 0, p->length);
}


void releasePeer(peer *p)
{
	free(p->buf);
	free(p);
}


int readFromPeer(int sock, peer* p)
{
	int ret = 0;

	if (p->offset < sizeof(int))
	{
		ret = read(sock, (char*)&p->size + p->offset, sizeof(int) - p->offset);
		if (ret < 0)
		{
			// printf("read: %u\n", pthread_self());
			exit(EXIT_FAILURE);
		}
		else if (ret == 0)
		{
			return 0;
		}
		
		p->offset += ret;
		if (p->offset < sizeof(int))
			return -1;
	}

	if (p->size > p->length)
		reallocPeer(p, p->size);

	ret = read(sock, p->buf + p->offset - sizeof(int), p->size - p->offset);
	if (-1 == ret)
	{
		// printf("read: %u\n", pthread_self());
		exit(EXIT_FAILURE);
	}
	else if (ret == 0)
	{
		return 0;
	}
	
	p->offset += ret;

	if (p->offset == p->size)
		return p->offset;
	else
		return -1;
}


int writeToPeer(int sock, peer *p)
{
	int ret = 0;

	if (p->offset < sizeof(int))
	{
		ret = write(sock, (char *)&p->size + p->offset, sizeof(int) - p->offset);
		if (ret < 0)
		{
			perror("write");
			exit(EXIT_FAILURE);
		}

		p->offset += ret;
		if (p->offset < sizeof(int))
			return -1;
	}

	ret = write(sock, p->buf + p->offset - sizeof(int), p->size - p->offset);
	if (-1 == ret)
	{
		perror("write");
		exit(EXIT_FAILURE);
	}

	p->offset += ret;

	if (p->offset == p->size)
		return p->offset;
	else
		return -1;
}


void cpToPeer(peer *p, char *str, int size)
{
	if (NULL == p || NULL == str || size <= 0)
		return;

	if (sizeof(int) + size > p->length)
		reallocPeer(p, sizeof(int) + size);

	memset(p->buf, 0, p->length);
	memcpy(p->buf, str, size);

	p->offset = 0;
	p->size = size + sizeof(size);
}
