#include <poll.h>


typedef struct poll_state {
	int num;
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
	pps->num = 0;

	return 0;
}


static int apiResize(struct eventLoop* pel)
{
	struct pollfd *pp = ((struct poll_state *)pel->apiData)->ppf;

	pp = (struct pollfd *)realloc(pp, sizeof(struct pollfd) * pel->setSize);
	if (NULL == pp)
	{
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	((struct poll_state *)pel->apiData)->ppf = pp;

	return 0;
}

static int apiDel(struct eventLoop *el)
{
	poll_state *pps = el->apiData;

	free(pps->ppf);
	free(pps);

	return 0;
}


static int apiAddEvent(struct eventLoop *el, int fd, int mask)
{
	poll_state *pps = el->apiData;

	pps->ppf[pps->num].fd = fd;
	if (mask & MASK_READ)
		pps->ppf[pps->num].events |= POLLIN;
	
	if (mask & MASK_WRITE)
		pps->ppf[pps->num].events |= POLLOUT;

	++pps->num;
	return 0;
}



static int apiDelEvent(struct eventLoop *el, int fd, int mask)
{
	poll_state *pps = el->apiData;
	int i = 0;

	for (i = 0; i < pps->num; ++i)
	{
		if (fd == pps->ppf[i].fd)
		{
			pps->ppf[i].events &= ~(mask);
			break;
		}		
	}

//	if (pps->ppf[i].events == MASK_NONE)
//	{
		for (; i < pps->num; ++i)
		{
			pps->ppf[i].fd = pps->ppf[i + 1].fd;
			pps->ppf[i].events = pps->ppf[i + 1].events;
		}

		--pps->num;
//	}

	return 0;
}



static int apiPoll(struct eventLoop *el)
{
	int ret = 0, i = 0, num = 0, mask = 0;
	poll_state *pps = el->apiData;

	ret = poll(pps->ppf, pps->num, -1);
	if (-1 == ret)
	{
		perror("poll error");
		return -1;
	}

	for (i = 0;i < pps->num; ++i)
	{
		mask = 0;
		
		if (pps->ppf[i].revents & POLLIN)
			mask |= MASK_READ;
		if (pps->ppf[i].revents & POLLOUT)
			mask |= MASK_WRITE;

		if (mask & MASK_NONE)
			continue;

		el->activeds[num].fd = pps->ppf[i].fd;
		el->activeds[num++].mask = mask;
	}
	
	return num;
}


static char* apiName()
{
	return "poll";
}
