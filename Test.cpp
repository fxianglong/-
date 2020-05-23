#include"ThreadCache.h"
#include"ConcurrentMalloc.h"
#include<vector>
//
//void UnitThreadTest()
//{
//	ThreadCache tc;
//	vector<void*> v;
//	for (size_t i = 0; i < 21; i++)
//	{
//		v.push_back(tc.Allocte(7));
//	}
//	for (size_t i = 0; i < v.size(); i++){
//		printf("%d->[%p]\n", i, v[i]);
//	}
//	for (auto ptr : v)
//	{
//		tc.Deallocte(ptr, 7);
//	}
//}

void UnitTestSizeClass()
{
	cout << SizeClass::RoundUp(1) << endl;
	cout << SizeClass::RoundUp(127) << endl;
	cout << SizeClass::RoundUp(129) << endl;
	cout << SizeClass::RoundUp(1023) << endl;
	cout << endl;
	cout << SizeClass::RoundUp(1025) << endl;
	cout << SizeClass::RoundUp(8 * 1024 - 1) << endl;
	cout << SizeClass::RoundUp(8 * 1024 + 1) << endl;
	cout << SizeClass::RoundUp(64 * 1024 - 1) << endl;
	cout << endl << endl;

	cout << SizeClass::ListIndex(1) << endl;
	cout << SizeClass::ListIndex(127) << endl;
	cout << SizeClass::ListIndex(129) << endl;
	cout << SizeClass::ListIndex(1023) << endl;
	cout << SizeClass::ListIndex(1025) << endl;
	cout << SizeClass::ListIndex(8 * 1024 - 1) << endl;
	cout << SizeClass::ListIndex(8 * 1024 + 1) << endl;
	cout << SizeClass::ListIndex(64 * 1024 - 1) << endl;
}

void UnitTestSystemAlloc()
{
	void * ptr = SystemAlloc(MAX_PAGES - 1);
	PAGE_ID id = (PAGE_ID)ptr >> PAGE_SHIFT;
	void* ptrshift = (void*)(id << PAGE_SHIFT);
	char* obj1 = (char*)ptr + 8;
	char* obj2 = (char*)ptr + 16;
	char* obj = (char*)ptr;
	char* obj4 = (char*)ptr + 4 * 1024;
	Sleep(1000);

}

void TestThreadCache()
{
	ThreadCache tc;
	void* ptr = tc.Allocte(6);
	void* ptr2 = tc.Allocte(8);
}


void fun(int n)
{
	for (size_t i = 0; i < n;)
	{
		cout << i << endl;
		i++;
	}
}
int main()
{
	//UnitThreadTest();
	//UnitTestSizeClass();
	//UnitTestSystemAlloc();
	//std::thread t1(fun,100);
	//std::thread t2(fun, 100);


	////std::cout << "主线程等待" << endl;
	//t1.join();
	//t2.join();
	//TestThreadCache();
	void* ptr1 = ConcurrentMalloc(1 << PAGE_SHIFT);
	void* ptr2 = ConcurrentMalloc(65 << PAGE_SHIFT);
	void* ptr3 = ConcurrentMalloc(129 << PAGE_SHIFT);

	ConcurrentFree(ptr1);
	ConcurrentFree(ptr2);
	ConcurrentFree(ptr3);

	system("pause");
	return 0;
}