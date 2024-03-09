#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "selfco.h"
#include "thread.h"
#include "util.h"
#include "lock.h"
#include <vector>
#include <list>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>

namespace server{

 class Selfco;

class Scheduler: public std::enable_shared_from_this<Scheduler>
{
public:
    typedef Mutex MutexType;
    typedef std::shared_ptr<Scheduler> ptr;
    Scheduler(int threadcount = 2,bool use_caller = true);
    virtual ~Scheduler();

    void start();
    void stop();

    bool hasIdlethreads(){return m_idlethreadcount > 0;}

    static Scheduler* GetThis();
    static void SetThis(Scheduler* s);
    static Selfco* GetSchedulerSelfco();
public:
    template<typename T>
    void addTask(T fc,int thread_num = -1){
        bool need_tickle = false;
        {
            
            MutexType::Lock lock(m_mutex);
            
            need_tickle = addTaskNoLock(fc,thread_num);
        }

        if(need_tickle){
            tickle();
        }
    }

    template<typename T>
    void addTask(T begin,T end){
        bool need_tickle = false;
        {

            MutexType::Lock lock(m_mutex);
            while(begin != end){
                need_tickle = addTaskNoLock(&*begin) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle){
            tickle();
        }
    }
    struct Task{
        std::function<void()> cb;
        server::Selfco::ptr selfco;
        int thread_num = -1;
        Task(Selfco::ptr co,int thr)
        :selfco(co)
        ,thread_num(thr){

        }
        Task(Selfco::ptr* co,int thr)
        :thread_num(thr){
            selfco.swap(*co);
        }
        Task(std::function<void()>f,int thr)
        :cb(f)
        ,thread_num(thr){

        }
        Task(std::function<void()>* f,int thr)
        :thread_num(thr){
            cb.swap(*f);
        }
        Task(){
            thread_num = -1;
        }
        void reset(){
            cb = nullptr;
            selfco = nullptr;
            thread_num = -1;
        }

    };
private:
    template<typename T>
    bool addTaskNoLock(T t,int thread_num = -1){
        bool need_tickle = m_tasks.empty();
        Task task(t,thread_num);
        if(task.selfco || task.cb){
            m_tasks.push_back(task);
        }
        return need_tickle;
    }

protected:
    void run();
    virtual void idle();
    virtual bool stopping();
    virtual void tickle();

private:
    MutexType m_mutex;
    std::vector<server::Thread::ptr> m_threadpoll;
    std::list<Task> m_tasks;
    Selfco::ptr m_schedulerco;
protected:
    std::vector<int> m_threadIds;
    int m_rootthread = 0;
    int m_threadcount = 0;
    bool m_stopping = true;
    bool m_autostop = false;

    std::atomic<int> m_activethreadcount = {0};
    std::atomic<int> m_idlethreadcount = {0};

};





}




#endif