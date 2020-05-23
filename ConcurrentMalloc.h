#pragma once

#include"ThreadCache.h"
#include"PageCache.h"
#include"Common.h"

void* ConcurrentMalloc(size_t size)
{


	if (size <= MAX_SIZE)//[1bute,64Kb]
	{
		if (tlslist == nullptr)
			tlslist = new ThreadCache;

		return tlslist->Allocte(size);
	}
	else if (size <= ((MAX_PAGES - 1) << PAGE_SHIFT))//[64byte,128*4k]
	{
		size_t align_size = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);
		size_t pagenum = (align_size >> PAGE_SHIFT);
		Span* span = pageCacheInst.NewSpan(pagenum);
		span->_objSize = align_size;
		void* ptr = (void*)(span->_pageid >> PAGE_SHIFT);
		return ptr;
	}
	else//[128*4kb,~]
	{
		size_t aligh_size = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);
		size_t pagenum = (aligh_size >> PAGE_SHIFT);
		return SystemAlloc(pagenum);
	}
	return tlslist->Allocte(size);
}
void ConcurrentFree(void* ptr)
{
	size_t pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
	Span* span = pageCacheInst.GetIdToSpan(pageid);
	if (span == nullptr)
	{
		SystemFree(ptr);
		return;
	}
	size_t size = span->_objSize;
	if (size <= MAX_SIZE)//[1bute,64Kb]
	{
		tlslist->Deallocte(ptr, size);
	}
	else if (size <= ((MAX_PAGES - 1) << PAGE_SHIFT))//[64byte,128*4k]
	{
		pageCacheInst.ReleaseSpanToPageCache(span);
	}
	return tlslist->Deallocte(ptr, 8);
}