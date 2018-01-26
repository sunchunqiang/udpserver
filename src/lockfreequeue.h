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

using namespace std;

#define __DEBUG__

#ifdef __DEBUG__
	extern atomic<int> deleted_nodes;
	extern atomic<int> pushed_nodes;
	extern atomic<int> poped_nodes;
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
		int internalcounter :30;
		int remainedcounter :2;
	};
	struct node {
		atomic<T*> data;
		atomic<counter> cnt;
		atomic<nodeptr> next;
#ifdef __DEBUG__
		bool bdeleted;
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
#endif
			data = nullptr;
			counter newcounter;
			newcounter.internalcounter = 0;
			newcounter.remainedcounter = 2;
			cnt = newcounter;
			nodeptr newnext;
			newnext.externcounter = 0;
			newnext.ptr = nullptr;
			next = newnext;
		}
		static void release_ptr(node* thisnode) {
			counter oldcnt = thisnode->cnt.load();
			counter newcnt;
			do {
				newcnt = oldcnt;
				--newcnt.internalcounter;
			} while (thisnode->cnt.compare_exchange_strong(oldcnt, newcnt));
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
		old_ptr = ptr.load();
		nodeptr new_ptr;
		do {
			new_ptr = old_ptr;
			++new_ptr.externcounter;
		} while (!ptr.compare_exchange_strong(old_ptr, new_ptr));
		old_ptr.externcounter = new_ptr.externcounter;
	}
	static void free_external_counter(nodeptr& old_node_ptr) {
		node* ptr = old_node_ptr.ptr;
		counter old_cnt = ptr->cnt.load();
		int increasenum = old_node_ptr.externcounter - 2;
		counter new_cnt;
		do {
			new_cnt = old_cnt;
			--new_cnt.remainedcounter;
			new_cnt.internalcounter += increasenum;
		} while (ptr->cnt.compare_exchange_strong(old_cnt, new_cnt));
		if (!new_cnt.internalcounter && !new_cnt.remainedcounter) {
#ifdef __DEBUG__
			deleted_nodes++;
#endif
			delete old_node_ptr.ptr;
		}
	}
	void set_tail(nodeptr& old_tail) {
		nodeptr new_tail;
		new_tail.externcounter = 1;
		new_tail.ptr = new node;
		nodeptr new_next = { 0 };
		if (!old_tail.ptr->next.compare_exchange_strong(new_next, new_tail)) {
			delete new_tail.ptr;
			new_tail = new_next;
		}

		node* const current_tail_ptr = old_tail.ptr;

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

	void push(T& data) {
		unique_ptr<T> newdata(new T(data));
		nodeptr old_tail = tail.load();
#ifdef __DEBUG__
		pushed_nodes++;
#endif
		for (;;) {
			increase_ptr_counter(tail, old_tail);
			T* olddata = nullptr;
			if (old_tail.ptr->data.compare_exchange_strong(olddata,
					newdata.release())) {
				set_tail(old_tail);
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

			if (old_head.ptr == tail.load().ptr) {
				node::release_ptr(old_head.ptr);
				unique_ptr<T> res = unique_ptr<T>(nullptr);
				return res;
			}
			if (head.compare_exchange_strong(old_head, old_head.ptr->next)) {
				T* const res = old_head.ptr->data.exchange(nullptr);
				free_external_counter(old_head);
#ifdef __DEBUG__
				poped_nodes++;
#endif
				return std::unique_ptr<T>(res);
			} else {
				node::release_ptr(old_head.ptr);
			}
		}
		return unique_ptr<T>(nullptr);
	}
};



#endif /* LOCKFREEQUEUE_H_ */
