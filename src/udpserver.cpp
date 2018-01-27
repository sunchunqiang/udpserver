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
#include <string>
#include <atomic>
#include <thread>

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
class A
{
public:
	int i;
	string name;
};
int main() {
//	test_queue();
	A a={1,"A is my name"};
	cout << a.i <<"," << a.name << endl;
	A b =move(a);
	cout << a.i <<"," << a.name << endl;
	cout << b.i <<"," << b.name << endl;
	cout << std::thread::hardware_concurrency() << endl;
	return 0;
}
