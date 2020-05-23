#pragma once
#include"Common.h"
#include"CentralCache.h"
class ThreadCache{
public:
	//申请内存和释放内存
	void *Allocte(size_t size);
	void Deallocte(void* ptr, size_t size);

	//从中心缓存获取对象
	void* FetchFromCentralCache(size_t index);
	//如果自由链表中对象过多，需要返回给centralcache
	void ListTooLong(FreeList& freeList, size_t num, size_t size);
private:
	FreeList _freeLists[NFREE_LIST];
	//ThreadCache* _next;
	//int threadid;
};

//ThreadCache* tc_list;


//线程TLS——线程局部存储
_declspec (thread) static ThreadCache* tlslist = nullptr;