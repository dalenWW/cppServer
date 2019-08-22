#include"LogFile.h"
#include<time.h>
#include <unistd.h>

LogFile::LogFile(const std::string& basename,int flushEveryN)
    : basename_(basename),
      flushEveryN_(flushEveryN),
      count_(0),
      mutex_(new MutexLock())
{
    // file_.reset(new File(getLogFileName()));
    file_.reset(new File(basename_));
}

LogFile::~LogFile()
{
}

void LogFile::append(const char* logline,int len)
{
    MutexLockGuard lock(*mutex_);
    append_unlocked(logline,len);
}

void LogFile::flush()
{
    MutexLockGuard lock(*mutex_);
    file_->flush();
}

void LogFile::append_unlocked(const char* logline,int len)
{
    file_->append(logline,len);
    ++count_;
    if (count_ >= flushEveryN_)
    {
        count_ = 0;
        file_->flush();
    }
}

// std::string LogFile::getLogFileName()
// {
//     std::string filename;
//     filename.reserve(basename_.length()+64);
//     filename = basename_;
    
//     char timebuf[32];
//     char pidbuf[32];
//     struct tm now_time;
//     time_t now = time(NULL);
//     localtime_r(&now,&now_time);
//     strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &now_time);
//     filename += timebuf;
//     snprintf(pidbuf,sizeof pidbuf,".%d",getpid());
//     filename += pidbuf;
//     filename += ".log";
//     return filename;
// }

