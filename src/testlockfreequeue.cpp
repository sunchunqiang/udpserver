#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include "lockfreequeue.h"
#include  <assert.h>
using namespace std;

const int allnodesnums = 1000000;
const int pushthreadnums = 1 ;
const int popthreadnums = 10;

 
atomic<int> num;    
atomic<bool> bstop;
void test_push(lockfreequeue<int>* q) {

	while (!bstop.load() && num.load() <= allnodesnums) {
		//   cout << "push " << num.load() << endl;
		int n = num.load();
		q->push(n);
		//	this_thread::sleep_for(100ms);
		num++;
	}
	bstop = true;
//	cout << "quit thread " << this_thread::get_id() << endl;
}
atomic<int> rec;
void test_pop(lockfreequeue<int>* q) {
	while (true) {
		unique_ptr<int> dd = q->pop();
		if (dd != nullptr) {
			//dd.release();
			//	   cout << "pop " << *dd << endl;
			rec.store(max(rec.load(), *dd));
		} else {
			if (bstop.load()) {
				break;
			}
		}
		//	this_thread::sleep_for(100ms);
	}
	//cout << "quit thread " << this_thread::get_id() << endl;

}

typedef lockfreequeue<int>  mylockfreequeue;


atomic<int> pushed_nodes   ;
atomic<int> poped_nodes   ;
atomic<int> deleted_nodes  ;
atomic<int> settail_nodes;
atomic<int> released_nodes;
atomic<int> k1  ;
atomic<int> k2  ;
atomic<int> k3  ;


int maxnum = 0 ;
void test_queue() {

	num.store(0);
	rec.store(0);
	released_nodes = 0 ;
	bstop.store(false);
	pushed_nodes = 0;
	poped_nodes = 0;
	deleted_nodes = 0;
	settail_nodes = 0 ; 
	 k1 = 0;
	  k2 = 0;
	 k3 = 0;

	mylockfreequeue q;
	vector<thread> pushthread;
	vector<thread> popthread;
	if(pushthreadnums ==0)
	{
		test_push(&q);

	}
	for(int i = 0 ; i < pushthreadnums; i++)
	{
		pushthread.push_back(thread(test_push, &q));
	}

	for(int i =0 ; i < popthreadnums; i++)
	{
		popthread.push_back(move(thread(test_pop, &q)));
	}

	for(auto& t:pushthread)
	{
		t.join();
	}
	for(auto& t:popthread)
	{
		t.join();
	}

	for (;;) {
		unique_ptr<int> dd = q.pop();
		if (dd != nullptr) {
		} else {
			break;
		}
		//	this_thread::sleep_for(100ms);
	}	 



	cout << "poped_nodes: " << poped_nodes.load() << endl;
	cout << "settail_nodes: " << settail_nodes.load() << endl;
	cout << "pushed_nodes: " << pushed_nodes.load() << endl;
	cout << "deleted_nodes: " << deleted_nodes.load() << endl;

	assert(poped_nodes.load() == pushed_nodes.load());
	assert(poped_nodes.load() == deleted_nodes.load() );
}

