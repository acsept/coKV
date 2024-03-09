#include "util.h"
#include <sys/time.h>

namespace server{

pid_t getThreadId(){
    return syscall(SYS_gettid);
}

uint64_t GetCurrentMS(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

}