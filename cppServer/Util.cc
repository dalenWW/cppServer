#include"Util.h"
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<signal.h>
#include <fcntl.h>
#include<assert.h>
#include"base/Logging.h"

const int kBuffer = 65536;

ssize_t readn(int fd,void* buf,size_t n)      // read n until EAGAIN
{
    size_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char *ptr = (char*)buf;
    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)    //  from head to read
                nread = 0;
            else if (errno == EAGAIN)
                return readSum;
            else
                return -1;
        }
        else if (nread == 0)
            break;
        readSum += nread;
        nleft -= nread;
        ptr += nread;
    }
    return readSum;
}

ssize_t readn(int fd, std::string &inBuffer)
{
    // ssize_t nread = 0;
    // ssize_t readSum = 0;
    // char buff[kBuffer];
    // if ((nread = read(fd, buff, kBuffer)) < 0)
    // {          
    //     LOG << "read error";
    //     return -1;    
    // }
    // readSum += nread;
    // inBuffer += std::string(buff, buff + nread);
    // return readSum;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char buffer[kBuffer];
    while(1)
    {
        nread = read(fd,buffer,kBuffer);
        if(nread > 0)
        {
            inBuffer.append(buffer,nread);
            readSum += nread;
            if(nread < kBuffer)
                return readSum;
            else
                continue;
        }
        else if(nread < 0)
        {
            if(errno == EAGAIN)
            {
                return readSum;
            }
            else if (errno == EINTR)
			{
				LOG << "errno == EINTR";
				continue;
			}
			else
			{
				//可能是RST
				perror("recv error");
				return -1;
			}
        }
        else
        {
            return 0;
        }
    }
}

ssize_t writen(int fd, void *buff, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    char *ptr = (char*)buff;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)     // buffer already full,return 
                    return writeSum;
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}

ssize_t writen(int fd, std::string &outbuffer)
{
    ssize_t nwrite = 0;
    ssize_t writeSum = 0;
    size_t length = outbuffer.size();
    if(length >= kBuffer)
    {
        length = kBuffer;
    }
    while(1)
    {
        nwrite = write(fd,outbuffer.c_str(),length);
        if(nwrite > 0)
        {
            writeSum += nwrite;
            outbuffer.erase(0,nwrite);
            length = outbuffer.size();
            if(length >= kBuffer)
                length = kBuffer;
            if(length == 0)
                return writeSum;
        }
        else if(nwrite < 0)    // error
        {
            if (errno == EAGAIN)     //系统缓冲区满，非阻塞返回
			{
				LOG << "write errno = EAGAIN";
				return writeSum;
			}
			else if (errno == EINTR)
			{
				LOG << "write errno = EINTR";
				continue;
			}
			else if (errno == EPIPE)
			{
				//客户端已经close，并发了RST，继续write会报EPIPE，返回0，表示close
				perror("write error");
				LOG << "write errno == client send RST";
				return -1;
			}
			else
			{
				perror("write error");//Connection reset by peer
				LOG << "write error, unknown error";
				return -1;
			}
        }
        else
        {
            return 0;
        }
    }
}

int socket_bind_listen(int port,struct sockaddr_in server_addr)
{
    if (port < 0 || port > 65535)
        return -1;

    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        return -1;

    // if(listen_fd = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP) == -1)
    //     return -1;

    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET,  SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        return -1;

    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    
    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        return -1;

    if(listen(listen_fd, 2048) == -1)
        return -1;

    if(listen_fd == -1)
    {
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}

void shutDownWR(int fd)
{
    shutdown(fd, SHUT_WR);
}

void sigpipe_handler()
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    assert(sigaction(SIGPIPE, &sa, NULL) == 0);
}

int setSocketNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}

int setSocketNonBlockingAndCloseOnExec(int fd)
{
    // set nonBlocking
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    // close on exec
    flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;
    
    flag |= FD_CLOEXEC;
    if(fcntl(fd,F_SETFL,flag) == -1)
        return -1;
    return 0;
}

void setSocketNodelay(int fd)
{
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
}

int acceptfd(int sockfd,struct sockaddr_in* addr)
{
    socklen_t addrlen = sizeof(*addr);
    int connfd = accept4(sockfd, (struct sockaddr*)addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        int saved_errno = errno;
        LOG << "accept socket failed";
        switch (saved_errno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: 
            case EPERM:
            case EMFILE: 
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG << "unexpected error of accept " << saved_errno;
                break;
            default:
                LOG << "unknown error of accept " << saved_errno;
                break;
        }
    }
    return connfd;
}