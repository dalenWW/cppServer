#include"FileUtil.h"

File::File(const std::string& filename)
    : fp_(::fopen(filename.c_str(),"ae"))   //e for O_CLOEXEC
{
    setbuffer(fp_,buffer_,sizeof buffer_);
}

File::~File()
{
    fclose(fp_);
}

void File::append(const char* logline,const size_t len)
{
    size_t n = write(logline,len);
    size_t remain = len - n;
    while(remain > 0)
    {
        size_t x = write(logline+n,remain);
        if(x == 0)
        {
            int err = ferror(fp_);
            if(err)
            {
                fprintf(stderr, "AppendFile::append() failed !\n");
            }
            break;
        }
        n += x;
        remain = len -n;
    }
}

void File::flush()
{
    fflush(fp_);
}

size_t File::write(const char *logline, size_t len)
{
    return fwrite_unlocked(logline, 1, len, fp_);
}

