#pragma once
#include"MutexLock.h"
#include"noncopyable.h"
#include<memory>
#include<string>
#include"FileUtil.h"

class LogFile : noncopyable
{
public:   
    LogFile(const std::string& basename,int flushEveryN=1024);
    ~LogFile();

    void append(const char* logline,int len);
    void flush();

private:
    void append_unlocked(const char* logline,int len);
    // std::string getLogFileName();

    const std::string basename_;
    const int flushEveryN_;    // times of append
    int count_;
    std::unique_ptr<MutexLock> mutex_;
    std::unique_ptr<File> file_;
};