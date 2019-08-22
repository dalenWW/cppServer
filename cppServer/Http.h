#pragma once
#include<string>
#include<unordered_map>
#include"base/noncopyable.h"

class HttpRequest
{
public:
    std::string method;
	std::string url;
	std::string version;
	std::unordered_map<std::string, std::string> header;
	std::string body;
};

class HttpResponse
{
public:
    std::string version;
    std::string statecode;
    std::string statemsg;
	std::unordered_map<std::string, std::string> header;
	std::string body;
};

class Http : noncopyable
{
public:
    Http();
    ~Http();
    //解析HTTP报文
    bool parseHttpRequest(std::string &s); 
    //处理报文
    void httpHandle(std::string &responsecontext); 
    //错误消息报文组装，404等
    void httpError(const int err_num,const std::string errmsg,std::string &responsecontext);
    //判断长连接
    bool isKeepAlive() { return keepAlive_; }

private:
    HttpRequest httpRequest_;
    bool parseState_;
    //Http响应报文相关成员   
    std::string responseBody_;    
    std::string errorMsg_;
    std::string path_;
    std::string queryString_;

//长连接标志
    bool keepAlive_;
    std::string bodyBuffer;
};