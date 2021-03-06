﻿#include <muduo/net/EventLoop.h>
//#include <muduo/net/EventLoopThread.h>
//#include <muduo/base/Thread.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;
int g_flag = 0;

void run4()
{
  printf("run4(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->quit();
} 

void run3()
{
  printf("run3(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->runAfter(3, run4);
  g_flag = 3;
}

void run2()
{
  printf("run2(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->queueInLoop(run3);//放到队列等待执行，run1()执行完毕(currentActiveChannel_->handleEvent(pollReturnTime_);)，
							//eventloop::loop()中执行到doPendingFunctors();会执行run3()
}

void run1()
{
  g_flag = 1;
  printf("run1(): pid = %d, flag = %d\n", getpid(), g_flag);
  g_loop->runInLoop(run2);//在io线程中，直接执行，没有放到队列
  g_flag = 2;
}

int main()
{
  printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);

  EventLoop loop;
  g_loop = &loop;

  loop.runAfter(2, run1);
  loop.loop();
  printf("main(): pid = %d, flag = %d\n", getpid(), g_flag);
}
