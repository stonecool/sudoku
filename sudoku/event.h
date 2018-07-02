#ifndef EVENT_H_
#define EVENT_H_

#define EVENT_NONE	0
#define EVENT_READ	1
#define EVENT_WRITE	2

struct eventLoop;

typedef void fileProc(struct eventLoop *lp, int fd, void *clientData); 

// 常规fd
typedef struct fileEvent {
	int mask;
	fileProc *rProc;
	fileProc *wProc;
	void *data;
}fileEvent;


// 激活的fd
typedef struct fileActivedEvent {
	int fd;
	int mask;	
}fileActivedEvent;

// 事件驱动数据结构
typedef struct eventLoop {
	int setSize;
	int maxFd;
	fileEvent *files;
	fileActivedEvent *activeds;
	void *apiData;
} eventLoop;

// 初始化
eventLoop* createEventLoop(int size);

// 释放
void delEventLoop(eventLoop *el);

// 添加事件
int addOneEvent(eventLoop *el, int fd, int mask, fileProc proc, void *clientData);

// 移除事件
int delOneEvent(eventLoop *el, int fd, int mask);

void eventMain(eventLoop *el);

// 轮训
int runOneEventLoop(eventLoop *el);

// 返回当前api名字
char *getEventName();

#endif
