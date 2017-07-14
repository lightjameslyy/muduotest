#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <sys/timerfd.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;//全局的指针指向对象
int timerfd;

void timeout(Timestamp receiveTime)
{
	printf("Timeout!\n");
	uint64_t howmany;
	::read(timerfd, &howmany, sizeof howmany);//把数据读走，不然会一直触发（poll这里电平触发）
	g_loop->quit();
}

int main(void)
{
	EventLoop loop;
	g_loop = &loop;

	timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	Channel channel(&loop, timerfd);
	channel.setReadCallback(boost::bind(timeout, _1));//通道的可读事件到来时会回调boost::bind这个函数，传递一个参数近来
													  //_1pollreturn回来的时间，传递到timeout函数当中
	channel.enableReading();//关注它的可读事件，这个时候poll或者epoll关注这个定时器的可读事件

	struct itimerspec howlong;//设定定时器的超时时间
	bzero(&howlong, sizeof howlong);//一次性定时器
	howlong.it_value.tv_sec = 1;
	::timerfd_settime(timerfd, 0, &howlong, NULL);

	loop.loop();//处于事件循环状态，一旦产生可读事件（定时器超时了），调用timeout

	::close(timerfd);
}