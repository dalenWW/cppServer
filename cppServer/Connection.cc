#include"Connection.h"
#include"Channel.h"
#include"EventLoop.h"
#include"base/Logging.h"
#include <unistd.h>
#include <sys/socket.h>
#include"Util.h"

Connection::Connection(EventLoop* loop, int connfd, struct sockaddr_in local_addr,struct sockaddr_in peer_addr)
      : loop_(loop),
        connfd_(connfd),
        disconnected_(false),
        halfclose_(false),
        local_addr_(local_addr),
        peer_addr_(peer_addr),
        connChannel_(new Channel(loop,connfd))
{
      connChannel_->setReadCallBack(std::bind(&Connection::handleRead,this));
      connChannel_->setWriteCallBack(std::bind(&Connection::handleWrite,this));
      connChannel_->setCloseCallBack(std::bind(&Connection::handleClose,this));
      connChannel_->setEvents(EPOLLIN | EPOLLET);     // ET mode
}

Connection::~Connection()
{
      loop_->removeChannel(connChannel_.get());
      close(connfd_);
}

// 连接建立后在分配的线程上注册事件
void Connection::Register()
{
    loop_->addChannel(connChannel_.get());
}

// 往对端发送消息:
// void Connection::send(const void* data, size_t len)
// {
//       ssize_t nwritten = 0;
//       // 当output buffer为空时直接write而不经过缓冲区
//       if(outBuffer_.empty())
//       {
//             nwritten = write(connfd_,data,len);
//             if(nwritten > 0)
//             {
//                   // 数据已写完
//                   if((size_t)nwritten == len && writeFinishCallback_)
//                         writeFinishCallback_(shared_from_this());
//             }
//             else
//             {
//                   nwritten = 0;
//                   if(errno != EWOULDBLOCK)
//                         LOG << "write error";
//             }
//       }
//       // 数据未能一次性写完或者缓冲区不为空
//       if((size_t)nwritten < len)
//       {
//             outBuffer_.append(static_cast<const char*>(data) + nwritten,len - nwritten);
//             if(!connChannel_->IsWriting())
//             {
//                   connChannel_->enableWrite();
//                   loop_->modChannel(connChannel_.get());
//             }
//       }
// }

void Connection::send(const std::string& message)
{
    // send(message.data(), message.size());
      outBuffer_ += message;
      if(loop_->isInLoopThread())
            sendInLoop();
      else
            loop_->queueInLoop(std::bind(&Connection::sendInLoop,shared_from_this()));
}

void Connection::sendInLoop()
{
      if(disconnected_)
            return;
      ssize_t ret = writen(connfd_,outBuffer_);
      if(ret > 0) 
      {
            __uint32_t events = connChannel_->getEvents();
            if(outBuffer_.size() > 0)     //  kernal buffer is already full
            {
                  connChannel_->setEvents(events | EPOLLOUT);    ////缓冲区满了，数据没发完，就设置EPOLLOUT事件触发
                  loop_->modChannel(connChannel_.get());
            }
            else
            {
                  //数据已发完
                  connChannel_->setEvents(events & (~EPOLLOUT));
                  if(writeFinishCallback_)
                        writeFinishCallback_(shared_from_this());
                  // 
                  if(halfclose_)
                        handleClose();
            }         
      }
      else if(ret < 0)
      {
            LOG << "send error";
            handleError();
      }
      else
      {
            handleClose();
      }
}

void Connection::shutdown()
{
      if(loop_->isInLoopThread())
            shutdownInLoop();
      else
            loop_->queueInLoop(std::bind(&Connection::shutdownInLoop,shared_from_this()));
}

void Connection::shutdownInLoop()
{
      if(disconnected_)
            return;
      LOG << "shutdown";
      closeCallback_(shared_from_this());
      loop_->queueInLoop(std::bind(connRemove_,shared_from_this()));
      disconnected_ = true;
}

// 处理可读事件
void Connection::handleRead()
{
      ssize_t readNum = readn(connfd_,inBuffer_);
      if(readNum > 0)
      {
            if(messageCallback_)
                  messageCallback_(shared_from_this(),inBuffer_);
      }
      else if(readNum < 0)
      {
            LOG << "read error";
            handleError();
      }
      else
      {
            handleClose();
      }
}

void Connection::handleWrite()
{
      ssize_t n = writen(connfd_,outBuffer_);
      if(n > 0)
      {
            // outBuffer_ = outBuffer_.substr(n);
            // if(outBuffer_.empty())
            // {
            //       connChannel_->DisableWrite();
            //       loop_->modChannel(connChannel_.get());
            //       if(writeFinishCallback_)
            //             writeFinishCallback_(shared_from_this());
            // }
            __uint32_t events = connChannel_->getEvents();
            if(outBuffer_.size() > 0)     //  kernal buffer is already full
            {
                  connChannel_->setEvents(events | EPOLLOUT);    ////缓冲区满了，数据没发完，就设置EPOLLOUT事件触发
                  loop_->modChannel(connChannel_.get());
            }
            else
            {
                  //数据已发完
                  connChannel_->setEvents(events & (~EPOLLOUT));
                  if(writeFinishCallback_)
                        writeFinishCallback_(shared_from_this());
                  // 
                  if(halfclose_)
                        handleClose();
            }
      }
      else if(n < 0)
      {
            LOG << "handle write error";
            handleError();
      }     
      else
      {
            handleClose();
      }
}

void Connection::handleError()
{
      if(disconnected_)
            return;
      loop_->queueInLoop(std::bind(connRemove_,shared_from_this()));
      disconnected_ = true;
}

void Connection::handleClose()
{
      if(disconnected_)
            return;
      if(outBuffer_.size() > 0 || inBuffer_.size() > 0)
      {
            halfclose_ = true;
            if(inBuffer_.size() > 0)
            {
                  messageCallback_(shared_from_this(),inBuffer_);
            }
      }
      else
      {
            loop_->queueInLoop(std::bind(connRemove_,shared_from_this()));
            closeCallback_(shared_from_this());
            disconnected_ = true;
      }
}
