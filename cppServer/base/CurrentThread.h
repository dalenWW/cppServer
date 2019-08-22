#pragma once

namespace CurrentThread
{
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread const char* t_threadName;
    
    void cacheTid();
    
    inline int tid()
    {
        if(t_cachedTid == 0)
            cacheTid();
        return t_cachedTid;
    }

    inline const char* tidString() 
    {
        return t_tidString;
    }

    inline const char* threadName() 
    {
        return t_threadName;
    }
} 