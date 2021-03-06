#pragma once

#include"Common.h"

class CentralCache{
public:

	// 从中心缓存获取一定数量的对象给thread cache
	size_t FetchRangeObj(void*& start, void*& end, size_t num, size_t size);
	// 将一定数量的对象释放到span跨度
	void ReleaseListToSpans(void* start, size_t size);
	// 从spanlist或者pagecache获取一个span
	Span* GetOneSpan(size_t size);
private:
	SpanList _spanLists[NFREE_LIST];
};

static CentralCache centralcacheInst;//单例模式