#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include "../peer.h"

#define HELP "Usage: \n\
	-c concurence \n\
	-n total request \n\
	-s ip \n\
	-p port \n\
	-m message\n"

struct foo {
	int					f_count;	// 当前已发请求数
	pthread_rwlock_t	f_lock;		// 读写锁
};

// 090005000607000010020400908000070800205090104008050000709003060050000703000100095

char *message = NULL;
char *ip = NULL;
int port = 0;
int request_num = 0;

// 本例中线程同步使用读写锁来实现
struct foo* lock_init()
{
	struct foo *fp = (struct foo *)malloc(sizeof(struct foo));
	if (NULL == fp)
	{
		perror("lock malloc");
		return NULL;
	}
	else
	{
		//TODO
		fp->f_count = 0;
		pthread_rwlock_init(&fp->f_lock, NULL);

		return fp;
	}
}


int lock_get(struct foo *fp)
{
	int count = 0;

	if (pthread_rwlock_rdlock(&fp->f_lock) != 0)
	{
		perror("rdlock");
		exit(1);
	}

	count = fp->f_count;
	pthread_rwlock_unlock(&fp->f_lock);

	return count;
}

int lock_add(struct foo * fp)
{
	int flag = 1;

	if (pthread_rwlock_wrlock(&fp->f_lock) != 0)
	{
		perror("wrlock");
		exit(1);
	}

	if (fp->f_count++ >= request_num)
		flag = 0;

	pthread_rwlock_unlock(&fp->f_lock);

	return flag;
}

void lock_relase(struct foo *fp)
{
	pthread_rwlock_destroy(&fp->f_lock);
	free(fp);
}


void* pthread_fun(void *ptr)
{
	peer *p = newPeer();

	int sock = 0;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	if (connect(sock, (const struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("connect");
		pthread_exit(NULL);
	}
	
	while(lock_add((struct foo *)ptr))
	{
		resetPeer(p);
		cpToPeer(p, message, strlen(message));

		while (1)
		{
			if (writeToPeer(sock, p) > 0)
				break;
		}

		resetPeer(p);

		while (1)
		{
			if (readFromPeer(sock, p) > 0)
				break;
		}

		printf("tid: %u, recv :%s\n", pthread_self(), p->buf);
	}
	

	releasePeer(p);

	close(sock);
	
	return ptr;
}

void help()
{
	fprintf(stderr, HELP);
}

int main(int argc, char** argv)
{
	char c = 0;
	int thread_num = 0, usec = 0;
	struct timeval begin, end;
	struct foo *fp;

	pthread_t *thread;
	int i = 0;

	while ((c = getopt(argc, argv, "c:n:s:p:m:")) != -1)
	{
		switch (c)
		{
			case 'c':
				if (NULL != optarg)
				{
					thread_num = strtol(optarg, NULL, 10);
					break;
				}
			case 'n':
				if (NULL != optarg)
				{
					request_num = strtol(optarg, NULL, 10);
					break;
				}
			case 's':
				if (NULL != optarg)
				{
					ip = optarg;
					break;
				}
			case 'p':
				if (NULL != optarg)
				{
					port = strtol(optarg, NULL, 10);
					break;
				}
			case 'm':
				if (NULL != optarg)
				{
					message = optarg;
					break;
				}
			default:
				help();
				exit(1);
		}
	}
	
	if (thread_num < 1||
		request_num < 1||
		port < 1||
		NULL == ip ||
		NULL == message)
	{
		help();
		exit(1);
	}
	else
	{
		printf("thread num: %d\n", thread_num);
		printf("total request: %d\n", request_num);
		printf("ip: %s\n", ip);
		printf("port: %d\n", port);
		printf("message: %s\n", message);
	}

	thread = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);
	if (NULL == thread)
	{
		perror("pthread_t error");
		exit(EXIT_FAILURE);
	}

	fp = lock_init();

	gettimeofday(&begin, NULL);
	
	for(i = 0;i < thread_num; ++i)
	{
		if (pthread_create(&thread[i], NULL, pthread_fun, (void*)fp) < 0)
		{
			fprintf(stderr, "create thread fail. i: %d\n", i);
			break;
		}
		else
		{
			// printf("create thread: %u sucess\n", thread[i]);
		}
	}

	for (i = 0;i < thread_num; ++i)
	{
		pthread_join(thread[i], NULL);
	}

	gettimeofday(&end, NULL);
	
	usec += (end.tv_sec - begin.tv_sec) * 1000000;
	usec += end.tv_usec - begin.tv_usec;

	printf("request: %d\nusec: %0.2f\ncon: %0.2f\n", request_num, usec * 1.0 / 1000000, 1.0 * request_num / usec * 1000000);

	lock_relase(fp);
	free(thread);
	
	return 0;
}
