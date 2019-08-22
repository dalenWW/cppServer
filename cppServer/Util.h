#pragma once
#include<cstdlib>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

ssize_t readn(int fd,void* buf,size_t size);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, void* buff, size_t n);
ssize_t writen(int fd, std::string &outbuffer);

void shutDownWR(int fd);
int socket_bind_listen(int port,struct sockaddr_in server_addr);
void sigpipe_handler();
int setSocketNonBlocking(int fd);
int setSocketNonBlockingAndCloseOnExec(int fd);
void setSocketNodelay(int fd);
int acceptfd(int sockfd,struct sockaddr_in* addr);