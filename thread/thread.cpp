#include "thread.h"


namespace server{
    static thread_local Thread* t_thread = nullptr;//当前运行的线程


Thread::Thread(std::function<void()> cb)
:m_cb(cb){
    //std::cout<<"构造"<<std::endl;
    int ret = pthread_create(&m_thread,nullptr,&Thread::run,this);

    m_semaphore.wait();
}

Thread::~Thread(){
    //std::cout<<"析构"<<std::endl;
    if(m_thread){
        pthread_detach(m_thread);
    }
}
void Thread::join(){
    if(m_thread){
        pthread_join(m_thread,nullptr);
    }
}
void* Thread::run(void* arg){
    Thread* thread = (Thread*) arg;
    t_thread = thread;
    thread->m_id = server::getThreadId();
    //设置名字取消
    std::function<void()> cb;
    cb.swap(thread->m_cb);
    thread->m_semaphore.notify();
    cb();
    return 0;
}


}