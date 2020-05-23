#pragma once
#include"ThreadCache.h"
#include"CentralCache.h"
void *ThreadCache::Allocte(size_t size){
	size_t index = SizeClass::ListIndex(size); // Index(size);
	FreeList& freeList = _freeLists[index];
	if (!freeList.Empty())
	{
		return freeList.Pop();
	}
	else{
		return FetchFromCentralCache(SizeClass::RoundUp(size));//size需要对齐
	}
}
void ThreadCache::Deallocte(void* ptr, size_t size){
	size_t index = SizeClass::ListIndex(size);//后边改
	FreeList& freeList = _freeLists[index];
	freeList.Push(ptr);

	//对象个数满足一定条件/内存大小
	size_t num = SizeClass::NumMoveSize(size);
	if (freeList.Size() >= SizeClass::NumMoveSize(size))
	{
		ListTooLong(freeList, num, size);
	}
}

void ThreadCache::ListTooLong(FreeList& freeList, size_t num, size_t size)
{
	void* start = nullptr;
	void* end = nullptr;
	freeList.PopRange(start, end, num);

	NextObj(end) = nullptr;
	centralcacheInst.ReleaseListToSpans(start, size);
}
//独立测试threadcache
//void* ThreadCache::FetchFromCentralCache(size_t index){
//	size_t num = 20;
//	//模拟取内存对象的代码，测试ThreadCache的逻辑
//	size_t size = (index + 1) * 8;
//	char* start = (char*)malloc(size*num);
//	char* cur = start;
//	for (int i = 0; i < num-1; i++)
//	{
//		char* next = cur + size;
//		NextObj(cur) = next;
//
//		cur = next;
//	}
//	NextObj(cur) = nullptr;
//	void* head = NextObj(start);
//	void* tail = cur;
//
//	_freeLists[index].PushRange(head,tail);
//	return start;
//}


void* ThreadCache::FetchFromCentralCache(size_t size)//size--单个对象的大小
{
	size_t num = SizeClass::NumMoveSize(size);
	//获取内存
	void* start = nullptr;//失败会抛异常，至少会申请一个
	void* end = nullptr;
	size_t ActualNum = centralcacheInst.FetchRangeObj(start, end, num, size);
	if (ActualNum == 1)
	{
		return start;
	}
	else//申请一个以上
	{
		size_t index = SizeClass::ListIndex(size);
		FreeList& list = _freeLists[index];
		list.PushRange(NextObj(start), end, ActualNum - 1);

		return start;
	}
}