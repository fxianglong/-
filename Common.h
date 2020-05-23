#pragma once
#include<iostream>
#include<assert.h>
#include<thread>
#include<mutex>
#include<map>
#ifdef _WIN32
#include<Windows.h>
#endif

//using namespace std;
using std::cout;
using std::endl;
const size_t MAX_SIZE = 64 * 1024;
const size_t NFREE_LIST = MAX_SIZE / 8;
const size_t MAX_PAGES = 129;
const size_t PAGE_SHIFT = 12;//4K为页移位

inline void*& NextObj(void* obj){
	return *((void**)obj);
}

class FreeList{
public:
	void Push(void *obj){
		//头插
		NextObj(obj) = _freelist;
		_freelist = obj;
		++_num;
	}
	void PushRange(void*& start, void*& tail, size_t num)

	{
		NextObj(tail) = _freelist;
		_freelist = start;
		_num += num;
	}
	void* Pop()
	{
		//头删
		void* obj = _freelist;
		_freelist = NextObj(obj);
		--_num;
		return obj;
	}
	size_t PopRange(void*& start, void*& end, size_t num)
	{
		size_t actualNum = 0;
		void* prev = nullptr;
		void* cur = _freelist;
		for (; actualNum < num && cur != nullptr; ++actualNum)
		{
			prev = cur;
			cur = NextObj(cur);
		}

		start = _freelist;
		end = prev;
		_freelist = cur;
		_num -= num;

		return actualNum;
	}
	size_t Size()
	{
		return _num;
	}
	bool Empty()
	{
		return _freelist == nullptr;
	}
	void Clear()
	{
		_freelist = nullptr;
		_num = 0;
	}
private:
	void* _freelist = nullptr;
	size_t _num = 0;
};


class SizeClass
{
public:

	//static size_t ListIndex(size_t size)
	//{
	//	if (size % 8 == 0)
	//	{
	//		return size / 8 - 1;
	//	}
	//	else
	//	{
	//		return size / 8;
	//	}
	//}

	//static size_t RoundUp(size_t size)//向上对齐 5byte申请8byte;8byte申请8byte
	//{
	//	if (size % 8 != 0)
	//	{
	//		return (size / 8 + 1) * 8;
	//	}
	//	else
	//	{
	//		return size;
	//	}
	//}
	//上边两个函数进行优化

	//[9-16]+7=[16,23]&(~7)=16;
	// 控制在12%左右的内碎片浪费
	// [1,128] 8byte对齐 freelist[0,16)
	// [129,1024] 16byte对齐 freelist[16,72)
	// [1025,8*1024] 128byte对齐 freelist[72,128)
	// [8*1024+1,64*1024] 1024byte对齐 freelist[128,184)
	static size_t _RoundUp(size_t size, int alignment)//向上对齐 5byte申请8byte;8byte申请8byte alignment--对齐位数
	{
		return (size + alignment - 1)&(~(alignment - 1));
	}
	static inline size_t RoundUp(size_t size)
	{
		assert(size <= MAX_SIZE);
		if (size <= 128){
			return _RoundUp(size, 8);
		}
		else if (size <= 1024){
			return _RoundUp(size, 16);
		}
		else if (size <= 8192){
			return _RoundUp(size, 128);
		}
		else if (size <= 65536){
			return _RoundUp(size, 1024);
		}
		return -1;
	}

	static size_t _ListIndex(size_t size, size_t align_shift)
	{
		return (size + (1 << align_shift) - 1) >> align_shift - 1;
	}
	static inline size_t ListIndex(size_t size)
	{
		assert(size <= MAX_SIZE);
		// 每个区间有多少个链
		static int group_array[4] = { 16, 56, 56, 56 };
		if (size <= 128){
			return _ListIndex(size, 3);
		}
		else if (size <= 1024){
			return _ListIndex(size - 128, 4) + group_array[0];
		}
		else if (size <= 8192){
			return _ListIndex(size - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (size <= 65536){
			return _ListIndex(size - 8192, 10) + group_array[2] + group_array[1] +
				group_array[0];
		}
		assert(false);
		return -1;
	}

	//[2,512]个之间   参数调优
	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		int num = (int)(MAX_SIZE / size);
		if (num < 2)
			num = 2;

		if (num > 512)
			num = 512;
		return num;
	}
	// 计算一次向系统获取几个页
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num*size;//总子节数

		npage >>= 12;//4K
		if (npage == 0)
			npage = 1;
		return npage;
	}
};



//////////////////////////////////////////
//Span   跨度--管理页为单位的内存对象，本质是为了方便合并，解决内存碎片
//针对windows
#ifdef _WIN32
typedef unsigned int PAGE_ID;
#else
typedef unsigned long long PANG_ID;
#endif

struct Span
{
	PAGE_ID _pageid = 0;//页号
	PAGE_ID _pagesize = 0;//页的数量

	FreeList _freeList;//对象自由链表
	size_t _objSize = 0;//自由链表对象的大小
	int _usecount = 0;//使用计数

	//size_t objsize;//对象大小

	//双向链表
	Span* _next = nullptr;//链表的下一个
	Span* _prev = nullptr;//链表的上一个


};

class SpanList{
public:
	SpanList(){
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	Span* Begin()
	{
		return _head->_next;
	}
	Span* End()
	{
		return _head;
	}

	void PushFront(Span* newspan)
	{
		Insert(_head->_next, newspan);
	}
	void PopFront()
	{
		Erase(_head->_next);
	}

	void PushBack(Span* newspan)
	{
		Insert(_head, newspan);
	}
	void PopBack()
	{
		Erase(_head->_prev);
	}

	void Insert(Span* pos, Span* newspan)
	{
		Span* prev = pos->_prev;

		prev->_next = newspan;
		newspan->_next = pos;
		pos->_prev = newspan;
		newspan->_prev = prev;
	}

	void Erase(Span* pos)
	{
		assert(pos != _head);

		Span* prev = pos->_prev;
		Span* next = pos->_next;

		prev->_next = next;
		next->_prev = prev;
	}
	bool Empty()
	{
		return Begin() == End();
	}
	void Lock()
	{
		_mtx.lock();
	}
	void Unlock()
	{
		_mtx.unlock();
	}
private:
	Span* _head;
	std::mutex _mtx;
};

inline static void* SystemAlloc(size_t num_page)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, num_page*(1 << PAGE_SHIFT),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// brk mmap等
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}

inline static void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	// sbrk unmmap等
#endif
}