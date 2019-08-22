#include"Logging.h"
#include"CurrentThread.h"
#include"Thread.h"
#include<time.h>
#include<sys/time.h>
#include"AsyncLogging.h"


static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging* AsyncLogger_;

std::string Logger::logFileName_ = "/WebServer.log";

void once_init()
{
    AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
    AsyncLogger_->start(); 
}

Logger::Impl::Impl(const char* fileName,int line)
    : basename_(fileName),
      line_(line),
      stream_()
{
    formatTime();
    CurrentThread::tid();
    stream_ << CurrentThread::t_tidString; 
}

void Logger::Impl::formatTime()
{
    struct timeval tv;
    time_t time;
    char str_time[26] = {0};
    gettimeofday(&tv,NULL);
    time = tv.tv_sec;     //seconds from 1970.01.01
    struct tm now_time;
    localtime_r(&time,&now_time);
    strftime(str_time, 26, "%Y-%m-%d %H:%M:%S\n", &now_time);
    stream_ << str_time;
}

void defaultOutput(const char* msg,int len)    // from buf to stdout
{
    size_t n = fwrite(msg,1,len,stdout);
}

void output(const char* msg,int len)
{
    pthread_once(&once_control_, once_init);
    AsyncLogger_->append(msg, len);
}

Logger::Logger(const char* fileName,int line)
    : impl_(fileName,line)
{
}

Logger::~Logger()
{
    impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
    const LogStream::Buffer& buf(stream().buffer());
    // defaultOutput(buf.data(),buf.length());
    output(buf.data(), buf.length());
}
