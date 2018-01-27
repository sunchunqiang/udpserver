//============================================================================
// Name        : udpserver.cpp
// Author      : csun
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iostream>
#include <chrono>
#include <mutex>
#include <atomic>

using namespace std;
using namespace std::chrono;

void test_queue();

mutex  m;
void testmutex()
{
	lock_guard<mutex> lk(m);
	int k = 0 ;

};
atomic<int> k;
void testatomic()
{
	int kk = 0 ;
	if(k.compare_exchange_strong(kk, kk+1))
	{

	}
};

int main() {
	test_queue();


	return 0;
}
