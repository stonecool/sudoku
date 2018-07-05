#ifndef PEER_H
#define PEER_H

#define DEFAULT_BUFSIZE		256			// 最大长度

typedef struct peer {
	char *buf;		// buf首地址
	int offset;		// 当前偏移量
	int size;		
	int length;		// 当前buf总长度
} peer;


// 申请
peer* newPeer();

// 释放
void releasePeer(peer *p);

// 重新分配buf大小，默认当前大小 * 2
void reallocPeer(peer *p, int length);

// 重置 offset
void resetPeer(peer *p);

// 从socket读
int readFromPeer(int sock, peer *p);

// 写入socket
int writeToPeer(int sock, peer *p);

//  讲str复制到peer
void cpToPeer(peer *p, char *str, int size);

#endif
