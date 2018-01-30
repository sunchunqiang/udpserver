/*
 * lockfreequeue.h
 *
 *  Created on: Jan 25, 2018
 *      Author: csun
 */

#ifndef LOCKFREEQUEUE_H_
#define LOCKFREEQUEUE_H_
#include <atomic>
#include <memory>
#include <thread>
#include <iostream>
#include <chrono>
#include <string>

using namespace std;



#ifdef __DEBUG__
	extern atomic<int> deleted_nodes;
	extern atomic<int> pushed_nodes;
	extern atomic<int> poped_nodes;
	extern atomic<int> settail_nodes;
	extern atomic<int> released_nodes;
	extern atomic<int> k1;
	extern atomic<int> k2;
	extern atomic<int> k3;
	extern int maxnum  ;
#endif

template<class T>
class lockfreequeue {
public:

private:
	class node;
	struct nodeptr {
		int externcounter;
		node* ptr;
	};
	struct counter {
		int internalcounter :30  ;
		int remainedcounter :2  ;
	};
	struct node {
		atomic<T*> data;
		atomic<counter> cnt;
		atomic<nodeptr> next;

#ifdef __DEBUG__
		bool bdeleted;
		bool bfirstdeleted;
		bool bseconddeleted;
		bool binit ;
		~node() {
			if (bdeleted) {
				cout << "the node is already deleted" << endl;
			}
			bdeleted = true;
		}
#endif
		node() {
#ifdef __DEBUG__
			bdeleted = false;
			bfirstdeleted = false;
			bseconddeleted = false;
#endif
			data = nullptr;
			counter newcounter;
			newcounter.internalcounter = 0;
			newcounter.remainedcounter = 2;
			binit = true;
			cnt.store(newcounter);
			next = {0};		}
		static void release_ptr(node* thisnode, int  bpushed = 0) {
			counter oldcnt = thisnode->cnt.load();
			counter newcnt;
			do {
				newcnt = oldcnt;
				--newcnt.internalcounter;
			} while (!thisnode->cnt.compare_exchange_weak(oldcnt, newcnt));
			oldcnt.internalcounter = newcnt.internalcounter;

			if (!oldcnt.internalcounter && !oldcnt.remainedcounter) {

#ifdef __DEBUG__
				deleted_nodes++;
#endif
				delete thisnode;
			}
		}
	};
	atomic<nodeptr> head, tail;
	static void increase_ptr_counter(atomic<nodeptr>& ptr,nodeptr& old_ptr)
	{
		nodeptr new_ptr;
		counter cnt;
		do {

			new_ptr = old_ptr;
			++new_ptr.externcounter;
			cnt = old_ptr.ptr->cnt.load();
		} while (!ptr.compare_exchange_strong(old_ptr, new_ptr));
		old_ptr.externcounter = new_ptr.externcounter;
		maxnum = max(maxnum,old_ptr.externcounter );
		if(!cnt.remainedcounter)
		{
			cout <<"increase an deleted node " << endl;
		}


	}
	static void free_external_counter(nodeptr& old_node_ptr
			,bool bfirst = true

	) {
		node* ptr = old_node_ptr.ptr;
		counter old_cnt = ptr->cnt.load();
		int increasenum = old_node_ptr.externcounter - 2;
		counter new_cnt;

		do {
			new_cnt = old_cnt;
			--new_cnt.remainedcounter;
			new_cnt.internalcounter += increasenum;
		} while (!ptr->cnt.compare_exchange_weak(old_cnt, new_cnt));



		if (!new_cnt.internalcounter && !new_cnt.remainedcounter) {
#ifdef __DEBUG__
			deleted_nodes++;
#endif
			delete ptr;
		}
	}
	void set_tail(nodeptr& old_tail) {
		nodeptr new_tail;
		new_tail.externcounter = 1;
		new_tail.ptr = new node;
		new_tail.ptr->next = { 0 };
		nodeptr new_next = { 0 };
		node* const current_tail_ptr = old_tail.ptr;

		if (!old_tail.ptr->next.compare_exchange_strong(new_next, new_tail)) {
			delete new_tail.ptr;
			new_tail = new_next;
		}

#ifdef __DEBUG__
		else
		{
			settail_nodes++;
		}
#endif


		while (!tail.compare_exchange_weak(old_tail, new_tail)
				&& old_tail.ptr == current_tail_ptr)
			;

		if (old_tail.ptr == current_tail_ptr)
			free_external_counter(old_tail);
		else
			node::release_ptr(current_tail_ptr);

	}
public:


	lockfreequeue() {
		nodeptr newptr;
		newptr.externcounter = 1;
		newptr.ptr = new node;
		head = newptr;
		tail = newptr;
	}

	void push(T& data)
	{
		push(move(data));
	}

	void push(T&& data) {

		unique_ptr<T> newdata(new T(move(data)));
		nodeptr old_tail = tail.load();
#ifdef __DEBUG__
		pushed_nodes++;
#endif
		for (;;) {
			increase_ptr_counter(tail, old_tail);
			T* olddata = nullptr;
			if (old_tail.ptr->data.compare_exchange_weak(olddata,
					newdata.get())) {
				set_tail(old_tail);
				newdata.release();
				break;
			} else {
			 	set_tail(old_tail);
			}
		}
	}
	unique_ptr<T> pop() {
		nodeptr old_head = head.load();

		for (;;) {
			increase_ptr_counter(head, old_head);

			node* oldptr = old_head.ptr;

			if (oldptr == tail.load().ptr) {
				node::release_ptr(oldptr, 11111);
				unique_ptr<T> res = unique_ptr<T>(nullptr);
				return res;
			}
			nodeptr next=oldptr->next.load();

			if (head.compare_exchange_strong(old_head, next)) {
				T* const res = old_head.ptr->data.load();
				free_external_counter(old_head, false);
#ifdef __DEBUG__
				poped_nodes++;
#endif
				return std::unique_ptr<T>(res);
			} else {
				node::release_ptr(oldptr,2);
			}
		}
		return unique_ptr<T>(nullptr);
	}
};



#endif /* LOCKFREEQUEUE_H_ */
