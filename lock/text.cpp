
#include <iostream>
#include "lock.h"
#include <mutex>


int main(){
    //pthread_rwlock_t mtx;
    {
    server::RWMutex lock;
    
    server::RWMutex::ReadLock Rlock(lock);//初始化加锁 析构解锁

    //Rlock.lock();

    //Rlock.unlock();
    }
    return 0;
}