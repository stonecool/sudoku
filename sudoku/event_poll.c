#include <poll.h>


typedef struct poll_state {
	struct pollfd *ppf;
} poll_state;


static int apiCreate(struct eventLoop *el)
{
	poll_state *pps = (poll_state *)malloc(sizeof(poll_state));
	if (NULL == pps)
	{
		perror("malloc");
		return 1;
	}

	struct pollfd *ppf = (struct pollfd *)malloc(sizeof(struct pollfd) * el->setSize);
	if (NULL == ppf)
	{
		perror("malloc");
		free(pps);
	}

	el->apiData = pps;
	pps->ppf = ppf;

	return 0;
}


static int apiDel(struct eventLoop *el)
{
	poll_state *pps = el->apiData;

	free(pps->ppf);
	free(pps);
}


static int apiAddEvent(struct eventLoop *el, int fd, int mask)
{
	poll_state *pps = el->apiData;

	++el->maxFd;
	pps->ppf[el->maxFd].fd = fd;
	if (mask & EVENT_READ)
		pps->ppf[el->maxFd].events |= POLLIN;
	
	if (mask & EVENT_WRITE)
		pps->ppf[el->maxFd].events |= POLLOUT;

	return 0;
}



static int apiDelEvent(struct eventLoop *el, int fd, int mask)
{
	poll_state *pps = el->apiData;
	int i = 0;

	for (i = 0; i < el->maxFd; ++i)
	{
		if (fd == pps->ppf[i].fd)
		{
			pps->ppf[i].events &= ~(mask);
			break;
		}		
	}

	if (pps->ppf[i].events == 0)
	{
		for (; i < el->maxFd; ++i)
		{
			pps->ppf[i].fd = pps->ppf[i + 1].fd;
			pps->ppf[i].events = pps->ppf[i + 1].events;
		}

		el->maxFd -= 1;
	}

	return 0;
}



static int apiPoll(struct eventLoop *el)
{
	int ret = 0, i = 0, num = 0, mask = 0;
	poll_state *pps = el->apiData;

	ret = poll(pps->ppf, el->maxFd, -1);
	if (-1 == ret)
	{
		perror("poll error");
		return -1;
	}

	for (i = 0;i < ret; ++i)
	{
		mask = 0;

		if (pps->ppf[i].revents & POLLIN)
			mask |= EVENT_READ;
		if (pps->ppf[i].revents & POLLOUT)
			mask |= EVENT_WRITE;

		el->activeds[num].fd = pps->ppf[i].fd;
		el->activeds[num++].mask = mask;
	}
	
	return num;
}


static char* apiName()
{
	return "poll";
}
