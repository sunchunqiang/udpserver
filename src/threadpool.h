/*
 * threadpool.h
 *
 *  Created on: Jan 27, 2018
 *      Author: csun
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_
#include <future>
#include <atomic>
#include <memory>
#include <vector>
#include "lockfreequeue.h"

using namespace std;

class taskwrapper {
	struct impl_base {
		virtual void call()=0;
		virtual ~impl_base() {
		}
	};
	std::unique_ptr<impl_base> impl;
	template<typename Task>
	struct impl_type: impl_base {
		Task task;
		impl_type(Task&& task_) :
				task(std::move(task_)) {
		}
		void call() {
			task();
		}
	};
public:
	template<typename F>
	taskwrapper(F&& f) :
			impl(new impl_type<F>(std::move(f))) {
	}
	void call() {
		impl->call();
	}
	void operator()() {
		call();
	}
	taskwrapper() = default;
	taskwrapper(taskwrapper&& other) :
			impl(std::move(other.impl)) {

	}
	taskwrapper& operator=(taskwrapper&& other) {
		impl = std::move(other.impl);
		return *this;
	}
	taskwrapper(const taskwrapper&) = delete;
	taskwrapper(taskwrapper&) = delete;
	taskwrapper& operator=(const taskwrapper&) = delete;
};

class thread_pool {
	lockfreequeue<taskwrapper> work_queue;
	atomic<bool> done;
	std::vector<std::thread> threads;

	void worker_thread()
	{
		while (!done)
		{
			unique_ptr<taskwrapper> task = work_queue.pop();
			if (task != nullptr)
			{
				(*task)();
			}
			else {
				std::this_thread::yield();
			}
		}
	}
public:

	thread_pool() :done(false)
	{

	}
	~thread_pool() {

	}
	void stop()
	{
		done = true;
		for(auto& th:threads)
		{
			th.join();
		}
	}
	void start(unsigned int  thread_count)
	{
		done = false;
		try {

			for (unsigned i = 0; i < thread_count; ++i) {
				threads.push_back(
						std::thread(&thread_pool::worker_thread, this));
			}
		}
		catch (...) {
			done = true;
			throw;
		}
	}

	template<typename Task>
	std::future<typename std::result_of<Task()>::type> submit(Task& f)
	{
		typedef typename std::result_of<Task()>::type result_type;
		std::packaged_task<result_type()> task(std::move(f));
		std::future<result_type> res(task.get_future());
		work_queue.push(std::move(task));
		return res;
	}
};

#endif /* THREADPOOL_H_ */
