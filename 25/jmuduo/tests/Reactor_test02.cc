#include <muduo/net/EventLoop.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;//全局变量所有线程都可以访问到

void threadFunc()
{
	g_loop->loop();//执行的线程和初始化的线程不一样
}

int main(void)
{
	EventLoop loop;
	g_loop = &loop;
	Thread t(threadFunc);
	t.start();
	//loop.loop();
	t.join();
	return 0;
}

