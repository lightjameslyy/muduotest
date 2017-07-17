#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

void runInThread()
{
  printf("runInThread(): pid = %d, tid = %d\n",
         getpid(), CurrentThread::tid());
}

int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), CurrentThread::tid());

  EventLoopThread loopThread;  //io线程对象（不代表系统上面实际的线程）还没被启动
  EventLoop* loop = loopThread.startLoop();//（线程）创建一个io线程，指针指向io线程当中一个eventloop对象
  // 异步(跨线程)调用runInThread，即将runInThread添加到loop对象所在IO线程，让该IO线程执行
  loop->runInLoop(runInThread);//异步调用把回调函数放到队列当中，然后唤醒IO线程（loop所在的线程）执行poll
  sleep(1);
  // runAfter内部也调用了runInLoop，所以这里也是异步调用
  loop->runAfter(2, runInThread);//让loop所在的io线程调用
  sleep(3);
  loop->quit();

  printf("exit main().\n");
}