#include "scheduler.h"
#include "hook.h"
#include <assert.h>

namespace server{
static thread_local Scheduler* sched = nullptr;
static thread_local Selfco* sched_co = nullptr;
//use_call使用主线程处理任务
Scheduler::Scheduler(int threadcount,bool use_caller)
{
    if(use_caller){
        Selfco::getThis();
        --threadcount;
        SetThis(this);

        m_schedulerco.reset(new server::Selfco(std::bind(&Scheduler::run,this),0,true));
        
        sched_co = m_schedulerco.get();
        m_rootthread = server::getThreadId();
        m_threadIds.push_back(m_rootthread);
    }else{
        m_rootthread = -1;
    }
    m_threadcount = threadcount;
}


Scheduler::~Scheduler(){

    assert(m_stopping);
    if(GetThis() == this){
        sched = nullptr;
    }
}

void Scheduler::start(){
    MutexType::Lock lock(m_mutex);
    if(!m_stopping){
        return;
    }
    m_stopping = false;
    assert(m_threadpoll.empty());

    m_threadpoll.resize(m_threadcount);

    for(int i = 0;i<m_threadcount;i++){
        m_threadpoll[i].reset(new Thread(std::bind(&Scheduler::run,this)));//ptr.reset()
        m_threadIds.emplace_back(m_threadpoll[i]->getId());
    }
}

void Scheduler::stop(){
    m_autostop = true;

    if(m_schedulerco
    && (m_schedulerco->getState() == Selfco::TERM  || m_schedulerco->getState() == Selfco::INIT)
    && m_threadcount == 0){
        if(stopping()){
            return;
        }
    }

    if(m_rootthread != -1){
        assert(GetThis() == this);
    }else{
        assert(GetThis() != this);
    }

    m_stopping = true;

    for(size_t i = 0;i<m_threadcount;i++){
        tickle();
    }

    if(m_schedulerco){
        tickle();
    }

    if(m_schedulerco){
        m_schedulerco->resuem();
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threadpoll);
    }

    for(auto & i : thrs){
        i->join();
    }

}

Scheduler* Scheduler::GetThis(){
    return sched;
}

void Scheduler::SetThis(Scheduler* s){
    sched = s;
}

Selfco* Scheduler::GetSchedulerSelfco(){
    return sched_co;
}

void Scheduler::run(){
    server::set_hook_enable(true);
    SetThis(this);
    if(server::getThreadId() != m_rootthread){
        sched_co = Selfco::getThis().get();
    }

    Selfco::ptr idle_co(new Selfco(std::bind(&Scheduler::idle,this)));
    Selfco::ptr cb_co;

    Task task;

    while(true){
        task.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            //m_mtx.lock();
            auto it = m_tasks.begin();
            while(it != m_tasks.end()){
                //该线程不是执行该任务的线程
                if(it->thread_num != -1 && it->thread_num != server::getThreadId()){
                    it++;
                    tickle_me = true;
                    continue;
                }
                assert(it->selfco || it->cb);
                if(it->selfco && it->selfco->getState() == Selfco::EXEC){
                    it++;
                    continue;
                }
                task = *it;
                m_tasks.erase(it++);
                ++m_activethreadcount;
                is_active = true;
                //找到执行任务退出
                break;
            }
            tickle_me |= (it != m_tasks.end());
        }
        //无任务
        if(tickle_me){
            tickle();
        }

        if(task.selfco && (task.selfco->getState() != Selfco::TERM)){
            task.selfco->resuem();
            --m_activethreadcount;
            if(task.selfco->getState() == Selfco::READY){
                addTask(task.selfco);
            }
            else if(task.selfco->getState() != Selfco::TERM){
                task.selfco->setState(Selfco::HOLD);
            }
            task.reset();
        }else if(task.cb){
            if(cb_co){
                cb_co->reset(task.cb);
            }else{
                cb_co.reset(new Selfco(task.cb));
            }
            task.reset();
            cb_co->resuem();
            --m_activethreadcount;
            if(cb_co->getState() == Selfco::READY){
                addTask(cb_co);
                cb_co.reset();
            }else if(cb_co->getState() == Selfco::TERM){
                cb_co->reset(nullptr);
            }else{
                cb_co->setState(Selfco::HOLD);
                cb_co.reset();
            }

        }else{
            if(is_active){
                --m_activethreadcount;
                continue;
            }
            if(idle_co->getState() == Selfco::TERM){
                break;
            }

            ++m_idlethreadcount;
            idle_co->resuem();
            --m_idlethreadcount;

            if(idle_co->getState() != Selfco::TERM){
                idle_co->setState(Selfco::HOLD);
            }
        }
    }
}
void Scheduler::idle(){
    while (!stopping())
    {   
        Selfco::yieldtohold();
    }
}

bool Scheduler::stopping(){
    MutexType::Lock lock(m_mutex);
    bool ret = m_autostop && m_stopping && m_tasks.empty() && m_activethreadcount == 0;
    return ret;
}

void Scheduler::tickle(){
    std::cout<<"tickle"<<std::endl;
}

}