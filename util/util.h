#ifndef __UTIL_H__
#define __UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <string>


namespace server{
    pid_t getThreadId();

    uint64_t GetCurrentMS();


}



#endif