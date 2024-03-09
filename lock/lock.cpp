#include "lock.h"
#include "scheduler.h"
#include <stdexcept>

namespace server{
    SemaphoreLock::SemaphoreLock(uint32_t count){
        if(sem_init(&m_semaphore,0,count)){
            throw std::logic_error("sem_init error");
        }
    }
    SemaphoreLock::~SemaphoreLock(){
        sem_destroy(&m_semaphore);
    }
    void SemaphoreLock::wait(){
        sem_wait(&m_semaphore);
    }
    void SemaphoreLock::notify(){
        sem_post(&m_semaphore);
    }

}