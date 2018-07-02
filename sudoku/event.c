#include <stdlib.h>
#include <errno.h>
#include "event.h"

#include "event_select.c"

eventLoop *createEventLoop(int size)
{
	int i = 0;
	struct eventLoop *el;
	
	el = (struct eventLoop *)malloc(sizeof(struct eventLoop));
	if (NULL == el)
		goto err;
	
	el->files = (struct fileEvent*)malloc(sizeof(struct fileEvent) * size);
	el->activeds = (struct fileActivedEvent*)malloc(sizeof(struct fileActivedEvent) * size);

	if (NULL == el->files ||
		NULL == el->activeds)
			goto err;

	el->setSize = size;
	el->maxFd = -1;
	apiCreate(el);

	for (i = 0;i < size; ++i)
	{
		el->files[i].mask = EVENT_NONE;
	}

	return el;

err:
	if (el)
	{
		free(el->activeds);
		free(el->files);
		free(el);
	}
}


void delEventLoop(eventLoop *el)
{
	apiDel(el);
	free(el->activeds);
	free(el->files);
	free(el);
}


int addOneEvent(eventLoop *el, int fd, int mask, fileProc proc, void *clientData)
{
	if (fd > el->setSize)
	{
		errno = ERANGE;
		return 1;
	}
	
	apiAddEvent(el, fd, mask);
	
	fileEvent *fe = &el->files[fd];
	fe->mask |= mask;
	fe->data = clientData;

	if (mask & EVENT_READ)
	{
		fe->rProc = proc;
	}

	if (mask & EVENT_WRITE)
	{
		fe->wProc = proc;
	}

	if (fd > el->maxFd)
	{
		el->maxFd = fd;
	}

	return 0;
}


int delOneEvent(eventLoop *el, int fd, int mask)
{
	if (fd > el->maxFd)
		return;

	fileEvent *fe = &el->files[fd];

	if (fe->mask == EVENT_NONE)
		return;

	apiDelEvent(el, fd, mask);

	fe->mask &= ~(mask);

	
	if (fd == el->maxFd &&
		fe->mask == EVENT_NONE)
	{
		int j = 0;

		for (j = el->maxFd - 1; j >= 0; --j)
		{
			if (el->files[j].mask != EVENT_NONE)
			{
				break;
			}
		}
	
		el->maxFd = j;
	}
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
	int num =0, i = 0, fd = 0, mask = 0;

	num = apiPoll(el);
	
	for (i = 0; i < num; ++i)
	{
		fd = el->activeds[i].fd;
		mask = el->activeds[i].mask;

		fileEvent *fe = &el->files[fd];

		if (mask & fe->mask & EVENT_READ)
		{
			fe->rProc(el, fd, fe->data);
		}

		if (mask & fe->mask & EVENT_WRITE)
		{
			fe->wProc(el, fd, fe->data);
		}
	}
}

char *getEventName()
{
	return apiName();
}
