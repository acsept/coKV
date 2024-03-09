#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <functional>
#include <memory>
#include "nocopyable.h"
#include "lock.h"
#include "util.h"

namespace server{

class Thread : Nocopyable{
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb);
    ~Thread();
    pid_t getId() const { return m_id; }
    void join();
    //获取当前线程
    static Thread* GetThis();

    static void* run(void* arg);
private:
    pid_t m_id = -1;
    
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    SemaphoreLock m_semaphore;
};



}





#endif