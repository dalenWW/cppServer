#pragma once
#include"noncopyable.h"
#include<string>

// not thread safe
class File : noncopyable
{
public:
    explicit File(const std::string& filename);
    ~File();

    void append(const char* logline,const size_t len);
    void flush();

private:
    size_t write(const char *logline, size_t len);
    FILE* fp_;
    char buffer_[64*1024];   // 64KB
};