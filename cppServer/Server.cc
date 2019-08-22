#include"Server.h"
#include"Util.h"
#include<arpa/inet.h>
#include<unistd.h>
#include"EventLoopThreadPool.h"
#include"Channel.h"
#include<string>
#include <netinet/in.h>
#include"base/Logging.h"

Server::Server(EventLoop* loop,int port,int numThread)
    : loop_(loop),
      numThread_(numThread),
      pool_(new EventLoopThreadPool(loop_,numThread)),
      port_(port),
      listenfd_(socket_bind_listen(port_,addr_)),
      acceptChannel_(new Channel(loop)),
      count_(0)
{
    acceptChannel_->setFd(listenfd_);
    sigpipe_handler();
    if (setSocketNonBlocking(listenfd_) < 0)
    {
        LOG << "set socket non block failed";
        exit(1);
    }
    //
    LOG << "ip address is " << inet_ntoa(addr_.sin_addr) << ":" << port_;
}

Server::~Server()
{
    close(listenfd_);
}

void Server::start()
{
    pool_->start();
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    // acceptChannel_->enableRead();
    acceptChannel_->setReadCallBack(std::bind(&Server::handleNewConn, this));
    loop_->addChannel(acceptChannel_.get());
}

void Server::handleNewConn()
{
    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(struct sockaddr_in));
    socklen_t peer_addr_len = sizeof(peer_addr);
    // int connfd = acceptfd(listenfd_,&peer_addr);
    // LOG << "create connfd = " << connfd;
    // EventLoop* loop = pool_->getNextLoop();
    // LOG << "new Connection from " << inet_ntoa(peer_addr.sin_addr) << ":" << ntohs(peer_addr.sin_port);
    // std::shared_ptr<Connection> conn = std::make_shared<Connection>(loop, connfd, addr_, peer_addr);

    // conn->setConnCb(connCallback_);
    // conn->setMessageCb(messageCallback_);
    // conn->setWriteFinishCb(writeFinishCallback_);
    // conn->setCloseCb(closeCallback_);
    // //  
    // fd2Connection_[connfd] = conn;
    // 在分配到的线程上注册事件
    int connfd = 0;
    while((connfd = accept(listenfd_, (struct sockaddr*)&peer_addr, &peer_addr_len)) > 0)
    {
        LOG << "connfd = " << connfd;
        EventLoop* loop = pool_->getNextLoop();
        LOG << "new Connection from " << inet_ntoa(peer_addr.sin_addr) << ":" << ntohs(peer_addr.sin_port);
        if(++count_ > kMAXCONNECTION)    
        {
            close(connfd);
            continue;
        }
        if (setSocketNonBlocking(connfd) < 0)
        {
            LOG << "Set non block failed!";
            //perror("Set non block failed!");
            return;
        }
        setSocketNodelay(connfd);
        std::shared_ptr<Connection> conn = std::make_shared<Connection>(loop, connfd, addr_, peer_addr);
        conn->setConnCb(connCallback_);
        conn->setMessageCb(messageCallback_);
        conn->setWriteFinishCb(writeFinishCallback_);
        conn->setCloseCb(closeCallback_);
        conn->setConnRemove(std::bind(&Server::removeConnection,this,std::placeholders::_1));
        {
            MutexLockGuard lock(mutex_);
            fd2Connection_[connfd] = conn;
        }
        if(connCallback_)
            connCallback_(conn);
        loop_->runInLoop(std::bind(&Connection::Register, conn));
    }
    // loop_->runInLoop(std::bind(&Connection::Register, conn));
}


// 这里应该由主loop来执行，多线程加锁删除
void Server::removeConnection(std::shared_ptr<Connection> conn)
{
    {
        MutexLockGuard lock(mutex_);
        --count_;
        fd2Connection_.erase(conn->getFd());
    }
}
