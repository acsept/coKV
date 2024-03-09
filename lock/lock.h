#ifndef __LOCK_H__
#define __LOCK_H__

#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <atomic>
#include <iostream>
#include "nocopyable.h"


namespace server{
//信号量
class SemaphoreLock : Nocopyable
{
public:
    SemaphoreLock(uint32_t count = 0);
    ~SemaphoreLock();
    void wait();
    void notify();
private:
    sem_t m_semaphore;
};
//局部锁
//初始化传一个锁 例如：mutex进去
template<class T>
class ScopedLockImpl{
public:
    ScopedLockImpl(T& mutex):m_mutex(mutex){
        lock();
    }
    ~ScopedLockImpl(){
        unlock();
    }

    void lock(){
        if(!m_locked){
            m_mutex.lock();
            m_locked = true;
        }
    }
    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

//互斥量
class Mutex : Nocopyable{
public:
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex(){
        pthread_mutex_init(&m_mutex,nullptr);
    }
    ~Mutex(){
        pthread_mutex_destroy(&m_mutex);
    }
    void lock(){
        pthread_mutex_lock(&m_mutex);
    }
    void unlock(){
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

class NullMutex : Nocopyable{
public:
    //局部锁
    typedef ScopedLockImpl<NullMutex> Lock;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};


//局部读锁模板实现
template<class T>
class ReadScopedLockImpl{
public:
    ReadScopedLockImpl(T& mutex) : m_mutex(mutex) {
        //std::cout<<"RLock init"<<std::endl;
        lock();
    }
    ~ReadScopedLockImpl(){
        //std::cout<<"RLock destroy"<<std::endl;
        unlock();
    }
    void lock(){
        if (!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }
    void unlock(){
        if(m_locked){
            m_locked = false;
            m_mutex.unlock();
        }
    }

private:
    T& m_mutex;
    bool m_locked = false;
};


//局部写锁模板实现
template<class T>
class WriteScopedLockImpl 
{
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) 
    {
        lock();
    }
    ~WriteScopedLockImpl() 
    {
        unlock();
    }
    void lock() 
    {
        if (!m_locked) 
        {
            m_mutex.wrlock();
            m_locked = true;
        }
    }
    void unlock() 
    {
        if (m_locked) 
        {
            m_locked = false;
            m_mutex.unlock();    
        }
    }
private:
    T& m_mutex;
    bool m_locked = false;
};
//空读写锁 用于调试
class NullRWMutex : Nocopyable{
public:
    typedef ReadScopedLockImpl<NullRWMutex> ReadLock;
    typedef WriteScopedLockImpl<NullRWMutex> WriteLock;
public:
    NullRWMutex() {}
    ~NullRWMutex() {}
    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};

//读写锁
class RWMutex : Nocopyable{
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

public:
    RWMutex(){
        //std::cout<<"RMWutex init"<<std::endl;
        pthread_rwlock_init(&m_lock,nullptr);
    }
    ~RWMutex(){
        //std::cout<<"RMWutex destroy"<<std::endl;
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock(){
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock(){
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock(){
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};

//自旋锁
class Spinlock : Nocopyable{
public:
    typedef ScopedLockImpl<Spinlock> Lock;
    Spinlock(){
        pthread_spin_init(&m_mutex,0);
    }
    ~Spinlock(){
        pthread_spin_destroy(&m_mutex);
    }
    void lock(){
        pthread_spin_lock(&m_mutex);
    }
    void unlock(){
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

//原子锁
class CASLock : Nocopyable
{
public:
    typedef ScopedLockImpl<CASLock> Lock;

    CASLock(){
        m_mutex.clear();
    }
    ~CASLock(){}
    void lock(){
        while(std::atomic_flag_test_and_set_explicit(&m_mutex,std::memory_order_acquire));
    }
    void unlock(){
        std::atomic_flag_clear_explicit(&m_mutex,std::memory_order_release);
    }
private:
    volatile std::atomic_flag m_mutex;
};


}




#endif