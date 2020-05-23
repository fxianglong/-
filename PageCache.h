#pragma once
#include"Common.h"
class PageCache{
public:
	Span* NewSpan(size_t numpage);
	void ReleaseSpanToPageCache(Span* span);

	Span* GetIdToSpan(PAGE_ID id);
	// 向系统申请k页内存挂到自由链表
	void SystemAllocPage(size_t numpage);

private:
	SpanList _spanLists[MAX_PAGES];
	std::map<PAGE_ID, Span*> _idSpanMap;
};


static PageCache pageCacheInst;