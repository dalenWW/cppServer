#pragma once
#include"EventLoop.h"
#include<memory>
#include"base/noncopyable.h"
#include"Connection.h"
#include"base/MutexLock.h"

class Channel;
class EventLoopThreadPool;
#define kMAXCONNECTION 30000

class Server : noncopyable
{
public:
    Server(EventLoop* loop,int port,int numThread);
    ~Server();
    void start();
    
    void setConnCb(Connection::Callback cb) { connCallback_ = std::move(cb); }
    void setMessageCb(Connection::MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setWriteFinishCb(Connection::Callback cb) { writeFinishCallback_ = std::move(cb); }
    void setCloseCb(Connection::Callback cb) { closeCallback_ = std::move(cb); }
    EventLoop* getLoop() const { return loop_; }

private:
    EventLoop* loop_;      // the acceptor loop
    int numThread_;
    struct sockaddr_in addr_;
    std::unique_ptr<EventLoopThreadPool> pool_;
    int port_;
    int listenfd_;
    std::unique_ptr<Channel> acceptChannel_;   // listenfd
    // 描述符到连接的映射，用来保存所有连接
    int count_;

    std::map<int, std::shared_ptr<Connection> > fd2Connection_;
    //  保护tcp连接的互斥量
    MutexLock mutex_;
    // 连接建立后的回调函数
    Connection::Callback connCallback_;
    // 新消息到来时
    Connection::MessageCallback messageCallback_;
    // 答复消息完成时
    Connection::Callback writeFinishCallback_;
    Connection::Callback closeCallback_;

    void handleNewConn();

    // 移除连接
    void removeConnection(std::shared_ptr<Connection> conn);
};
