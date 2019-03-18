
#ifndef _EVENTLOOPTHREAD_
#define _EVENTLOOPTHREAD_

#include <thread>
#include <mutex>
#include <atomic>
#include <list>
#include <condition_variable>

#include "noncopyable.h"

class EventLoop; 
class TaskQueue;

class EventLoopThread:public noncopyable
{
private:
	EventLoop* loop_;   //线程安全
	std::thread thread_; //one loop per thread
	
	std::mutex mutex_;
	std::condition_variable cond_;
	void threadFunc();  //for call loop.loop()
	bool existing_;
public:
	EventLoopThread();//指向任务队列的指针
	EventLoop* getLoopOfCurrentThread();
	~EventLoopThread();
};


#endif