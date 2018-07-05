#include <sys/epoll.h>

typedef struct epoll_state {
	int efd;
	struct epoll_event *ee;
} epoll_state;


static int apiCreate(struct eventLoop *el)
{
	epoll_state *es = (epoll_state *)malloc(sizeof(struct epoll_state));
	struct epoll_event *ee = (struct epoll_event *)malloc(sizeof(struct epoll_event) * 100);

	if (NULL == es || NULL == ee)
	{
		perror("malloc");
		return 1;
	}
	
	es->efd = epoll_create(10);
	if (-1 == es->efd)
	{
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	es->ee = ee;
	el->apiData = es;

	return 0;
}


static void apiDel(struct eventLoop *el)
{
	epoll_state *es = el->apiData;
	free(es->ee);
	free(es);
}


static int apiAddEvent(struct eventLoop *el, int fd, int mask)
{
	epoll_state *es = el->apiData;
	struct epoll_event ev;

	// 不用边沿触发 EPOLLET TODO
	if (mask & EVENT_READ)
		ev.events |= EPOLLIN;

	if (mask & EVENT_WRITE)
		ev.events |= EPOLLOUT;

	ev.data.fd = fd;
	if (epoll_ctl(es->efd, EPOLL_CTL_ADD, fd, &ev) < 0)
	{
		perror("epoll_ctl");
		return 1;
	}

	return 0;
}


static int apiDelEvent(struct eventLoop *el, int fd, int mask)
{
	epoll_state *es = el->apiData;

	if (epoll_ctl(es->efd, EPOLL_CTL_DEL, fd, NULL) < 0)
	{
		perror("epoll_ctl");
		return 1;
	}

	return 0;
}


static int apiPoll(struct eventLoop *el)
{
	int i = 0, ret = 0, num = 0, mask = 0;
	epoll_state *es = el->apiData;
	
	ret = epoll_wait(es->efd, es->ee, 100, -1);
	if (-1 == ret)
	{
		perror("epoll_wait");
		return -1;
	}

	for (i = 0; i < ret; ++i)
	{
		mask = 0;

		if (es->ee[i].events & EPOLLIN)
			mask |= EVENT_READ;
		if (es->ee[i].events & EPOLLOUT)
			mask |= EVENT_WRITE;

		el->activeds[num].fd = es->ee[i].data.fd;
		el->activeds[num++].mask = mask;
	}
		
	return num;
}


static char* apiName()
{
	return "epoll";
}
