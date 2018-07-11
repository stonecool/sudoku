#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "event.h"

#define HAVE_POLL

#ifdef HAVE_SELECT
	#include "event_select.c"
#else
	#ifdef HAVE_POLL
		#include "event_poll.c"
	#else
		#include "event_epoll.c"
	#endif
#endif

eventLoop *createEventLoop(int size)
{
	struct eventLoop *pel = NULL;
	struct fileEvent *pfe = NULL;
	struct fileActivedEvent *pae = NULL;
	
	pel = (struct eventLoop *)malloc(sizeof(struct eventLoop));
	pfe = (struct fileEvent*)malloc(sizeof(struct fileEvent) * size);
	pae = (struct fileActivedEvent*)malloc(sizeof(struct fileActivedEvent) * size);

	if (NULL == pel || NULL == pfe || NULL == pae)
		goto err;

	memset(pel, 0, sizeof(struct eventLoop));
	memset(pfe, 0, (sizeof(struct fileEvent) * size));
	memset(pae, 0, (sizeof(struct fileActivedEvent) * size));

	pel->setSize = size;
	pel->maxFd = -1;
	pel->files = pfe;
	pel->activeds = pae;
	apiCreate(pel);

	return pel;

err:
	free(pae);
	free(pfe);
	free(pel);

	return NULL;
}


void delEventLoop(eventLoop *el)
{
	apiDel(el);
	free(el->activeds);
	free(el->files);
	free(el);
}


int resizeEventLoop(eventLoop *pel)
{
	int newSize = pel->setSize * 2;

	pel->setSize = newSize;
	pel->files = (struct fileEvent *)realloc(pel->files, sizeof(struct fileEvent) * newSize);
	if (NULL == pel->files)
		return 1;
	pel->activeds = (struct fileActivedEvent *)realloc(pel->activeds, sizeof(struct fileActivedEvent) * newSize);
	if (NULL == pel->activeds)
	{
		return 1;
	}

	apiResize(pel);
	return 0;
}

int addOneEvent(eventLoop *el, int fd, int mask, fileProc proc, void *clientData)
{
	if (fd >= el->setSize)
	{
		resizeEventLoop(el);
	}
	apiAddEvent(el, fd, mask);
	
	fileEvent *fe = &el->files[fd];
	fe->mask |= mask;
	fe->data = clientData;

	if (mask & MASK_READ)
		fe->rProc = proc;

	if (mask & MASK_WRITE)
		fe->wProc = proc;

	if (fd > el->maxFd)
		el->maxFd = fd;

	return 0;
}


int delOneEvent(eventLoop *el, int fd, int mask)
{
	int j = 0;
	
	if (fd > el->maxFd)
	{
		perror("delOneEvent");
		return -1;
	}

	fileEvent *fe = &el->files[fd];

	if (fe->mask == MASK_NONE)
	{
		perror("fd already none");
		return -1;
	}
	
	apiDelEvent(el, fd, mask);
	fe->mask &= ~(mask);
	
	if (fd == el->maxFd && fe->mask == MASK_NONE)
	{
		for (j = el->maxFd - 1; j >= 0; --j)
		{
			if (el->files[j].mask != MASK_NONE)
				break;
		}
	
		el->maxFd = j;
	}

	return 0;
}


void eventMain(eventLoop *el)
{
	while (1)
	{
		runOneEventLoop(el);	
	}
}


int runOneEventLoop(eventLoop *	el)
{
	int num = 0, i = 0, fd = 0, mask = 0;

	num = apiPoll(el);
	
	for (i = 0; i < num; ++i)
	{
		fd = el->activeds[i].fd;
		mask = el->activeds[i].mask;

		fileEvent *fe = &el->files[fd];

		if (mask & fe->mask & MASK_READ)
			fe->rProc(el, fd, fe->data);
	
		// 此处可能重新realloc，所以地址可能发生变化，此处要重新取
		fe = &el->files[fd];
		if (mask & fe->mask & MASK_WRITE)
			fe->wProc(el, fd, fe->data);
	}

	return 0;
}

char *getEventName()
{
	return apiName();
}
