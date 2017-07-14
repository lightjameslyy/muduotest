// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/poller/PollPoller.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Types.h>
#include <muduo/net/Channel.h>

#include <assert.h>
#include <poll.h>

using namespace muduo;
using namespace muduo::net;

PollPoller::PollPoller(EventLoop* loop)
  : Poller(loop)
{
}

PollPoller::~PollPoller()
{
}

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
  // XXX pollfds_ shouldn't change
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);//pollfds_是输入输出参数
  Timestamp now(Timestamp::now());
  if (numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
  }
  else if (numEvents == 0)
  {
    LOG_TRACE << " nothing happended";
  }
  else
  {
    LOG_SYSERR << "PollPoller::poll()";
  }
  return now;
}

void PollPoller::fillActiveChannels(int numEvents,
                                    ChannelList* activeChannels) const
{
  for (PollFdList::const_iterator pfd = pollfds_.begin();
      pfd != pollfds_.end() && numEvents > 0; ++pfd)//pollfds_是输入输出参数
  {
    if (pfd->revents > 0)//事件的个数
    {
      --numEvents;//每处理一个--
      ChannelMap::const_iterator ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());//断言通道是注册过的
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);
      // pfd->revents = 0;
      activeChannels->push_back(channel);//以便在eventloop.cc的loop()里进行处理
    }
  }
}

void PollPoller::updateChannel(Channel* channel)
{
  Poller::assertInLoopThread();//io线程里调用才可以
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0)
  {
	// index < 0说明是一个新的通道
    // a new one, add to pollfds_
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    int idx = static_cast<int>(pollfds_.size())-1;
    channel->set_index(idx);//已经存在不等以-1了
    channels_[pfd.fd] = channel;
  }
  else
  {
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
	// 将一个通道暂时更改为不关注事件，但不从Poller中移除该通道
    if (channel->isNoneEvent())
    {
      // ignore this pollfd
	  // 暂时忽略该文件描述符的事件
	  // 这里pfd.fd 可以直接设置为-1(-1就是一个合法的文件描述符，为什么还要减1，因为有0的文件描述符的存在，不然的话-0还是0)
      pfd.fd = -channel->fd()-1;	// 这样子设置是为了removeChannel优化
    }
  }
}

void PollPoller::removeChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent());//如果还在关注事件将不能移除，所以首先要用updateChannel更新为noneevent
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
  const struct pollfd& pfd = pollfds_[idx]; (void)pfd;
  assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());//根据key移除
  assert(n == 1); (void)n;
  if (implicit_cast<size_t>(idx) == pollfds_.size()-1)//移除的是最后一个
  {
    pollfds_.pop_back();
  }
  else
  {
	// 这里移除的算法复杂度是O(1)，将待删除元素与最后一个元素交换再pop_back
    int channelAtEnd = pollfds_.back().fd;//最后一个文件描述符
    iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);
    if (channelAtEnd < 0)
    {
      channelAtEnd = -channelAtEnd-1;
    }
    channels_[channelAtEnd]->set_index(idx);
    pollfds_.pop_back();
  }
}

