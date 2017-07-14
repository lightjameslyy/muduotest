// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <muduo/base/Timestamp.h>

namespace muduo
{
namespace net
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd（socketfd,pipefd)
class Channel : boost::noncopyable
{
 public:
  typedef boost::function<void()> EventCallback;
  typedef boost::function<void(Timestamp)> ReadEventCallback;//读事件的回调处理，需要多长的时间戳

  Channel(EventLoop* loop, int fd);//只能在一个eventlooop,由一个eventloop负责
  ~Channel();

  void handleEvent(Timestamp receiveTime);
  void setReadCallback(const ReadEventCallback& cb)
  { readCallback_ = cb; }
  void setWriteCallback(const EventCallback& cb)
  { writeCallback_ = cb; }
  void setCloseCallback(const EventCallback& cb)
  { closeCallback_ = cb; }
  void setErrorCallback(const EventCallback& cb)
  { errorCallback_ = cb; }

  //跟tcpconnection的生存期有关系
  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const boost::shared_ptr<void>&);

  int fd() const { return fd_; }//channel对应的文件描述符
  int events() const { return events_; }//channel注册的事件保存在events_
  void set_revents(int revt) { revents_ = revt; } // used by pollers
  // int revents() const { return revents_; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() { events_ |= kReadEvent; update(); }//调用channel的update，导致...(关注可读事件，把通道注册到eventloop所持有的poller对象当中）
  // void disableReading() { events_ &= ~kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; update(); }
  void disableWriting() { events_ &= ~kWriteEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }
  bool isWriting() const { return events_ & kWriteEvent; }

  // for Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  // for debug//把事件转换成字符串
  string reventsToString() const;

  void doNotLogHup() { logHup_ = false; }

  EventLoop* ownerLoop() { return loop_; }
  void remove();

 private:
  void update();
  void handleEventWithGuard(Timestamp receiveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;			// 所属EventLoop
  const int  fd_;			// 文件描述符，但不负责关闭该文件描述符
  int        events_;		// 关注的事件
  int        revents_;		// poll/epoll返回的事件
  int        index_;		// used by Poller.表示在poll的事件数组中的序号//poll(struct pollfd *fds.....),在数组fds中的序号
							//epoll表示通道的状态；
							//如果是小于0，说明是一个新增的事件，还没添加到数组当中，添加到fds的末尾;
 bool       logHup_;		// for POLLHUP

  boost::weak_ptr<void> tie_;//负责生存期控制
  bool tied_;//负责生存期控制
  bool eventHandling_;		// 是否处于处理事件中
  ReadEventCallback readCallback_;//读事件的回调，多了一个时间戳
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}
}
#endif  // MUDUO_NET_CHANNEL_H
