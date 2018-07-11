#include <sys/select.h>
#include <string.h>

typedef struct select_state {
	fd_set read_fd_set, read_fd_set_bak;
	fd_set write_fd_set, write_fd_set_bak;
} select_state;


static int apiCreate(struct eventLoop *el)
{
	select_state *ss = (select_state *)malloc(sizeof(select_state));
	if (NULL == ss)
	{
		return 1;
	}

	FD_ZERO(&ss->read_fd_set_bak);
	FD_ZERO(&ss->write_fd_set_bak);
	el->apiData = ss;

	return 0;
}


static void apiDel(struct eventLoop *el)
{
	free(el->apiData);
}



static int apiAddEvent(struct eventLoop *el, int fd, int mask)
{
	select_state* ss = el->apiData;
	
	if (mask & MASK_READ)
		FD_SET(fd, &ss->read_fd_set_bak);
	
	if (mask & MASK_WRITE)
		FD_SET(fd, &ss->write_fd_set_bak);

	return 0;
}


static int apiDelEvent(struct eventLoop *el, int fd, int mask)
{
	select_state *ss = el->apiData;

	if (mask & MASK_READ)
		FD_CLR(fd, &ss->read_fd_set_bak);		

	if (mask & MASK_WRITE)
		FD_CLR(fd, &ss->write_fd_set_bak);

	return 0;
}

static int apiPoll(struct eventLoop *el)
{
	int ret = 0, i = 0, num = 0, mask = 0;
	select_state *ss = el->apiData;

	memcpy(&ss->read_fd_set, &ss->read_fd_set_bak, sizeof(fd_set));
	memcpy(&ss->write_fd_set, &ss->write_fd_set_bak, sizeof(fd_set));

	ret = select(el->maxFd + 1, &ss->read_fd_set, &ss->write_fd_set, NULL, NULL);
	if (ret < 0)
	{
		perror("select");
		return 1;
	}

	for (i = 0; i <= el->maxFd; ++i)
	{
		mask = 0;
		fileEvent *fe = &el->files[i];

		if (fe->mask == MASK_NONE)
			continue;

		if (fe->mask & MASK_READ && 
			FD_ISSET(i, &ss->read_fd_set))
			mask |= MASK_READ;

		if (fe->mask & MASK_WRITE &&
			FD_ISSET(i, &ss->write_fd_set))
			mask |= MASK_WRITE;

		el->activeds[num].fd = i;
		el->activeds[num++].mask = mask;
	}
	
	return num;
}

static char* apiName()
{
	return "select";
}
