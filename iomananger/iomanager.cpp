#include "iomanager.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <assert.h>

namespace server{

    //static thread_local IOManager* main_iom = nullptr;

IOManager::FdContext::EventContext& IOManager::FdContext::
getEventContext(Event event){
    if(event == READ){
        return read;
    }else{
        return write;
    }
}

void IOManager::FdContext::resetEventContext(EventContext& ctx){
    ctx.cb = nullptr;
    ctx.co.reset();
    ctx.scheduler = nullptr;
}

void IOManager::FdContext::trigerEvent(Event ev){
    events = (Event)(events & ~ev);

    EventContext& ctx = getEventContext(ev);
    if(ctx.cb){
        ctx.scheduler->addTask(&ctx.cb);
    }else{
        ctx.scheduler->addTask(&ctx.co);
    }
    ctx.scheduler = nullptr;
    return;
}


IOManager::IOManager(size_t threads,bool use_caller )
:Scheduler(threads,use_caller)
{
    m_epfd = epoll_create(1);
#if 1
    m_eventfd = eventfd(0,EFD_NONBLOCK);
#elif
    int ret = pipe(m_tickleFds);
#endif
    epoll_event ev;
    ev.data.fd = m_eventfd;
    ev.events = EPOLLIN | EPOLLET;

    epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_eventfd,&ev);

    contextResize(32);

    start();

}
IOManager::~IOManager(){
    stop();
    close(m_epfd);
    close(m_eventfd);
    //int i -> unsigned
    for(unsigned int i = 0;i<m_fdContexts.size();i++){
        if(m_fdContexts[i]){
            delete m_fdContexts[i];
        }
    }
}

void IOManager::contextResize(size_t size){
    m_fdContexts.resize(size);
    for(size_t i = 0;i<m_fdContexts.size();i++){
        if(!m_fdContexts[i]){
            m_fdContexts[i] = new FdContext;
        }
    }
}

int IOManager::addEvent(int fd,Event ev,std::function<void()> cb,int flag){
    //获取
    FdContext* ctx = nullptr;
    MutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() > fd){
        ctx = m_fdContexts[fd];
        lock.unlock();
    }else{
        lock.unlock();
        MutexType::WriteLock lock2(m_mutex);
        contextResize(fd*1.5);
        ctx = m_fdContexts[fd];
    }
    //修改
    FdContext::MutexType::Lock lock3(ctx->mutex);
    ctx->fd = fd;
    if(ctx->events & ev){
        assert(!(ctx->events & ev));
    }
    Event old_event = ctx->events;
    ctx->events = (Event)(ctx->events | ev);
    FdContext::EventContext& event_ctx = ctx->getEventContext(ev);
    assert(!event_ctx.scheduler && !event_ctx.cb && !event_ctx.co);
    event_ctx.scheduler = Scheduler::GetThis();
    if(cb){
        event_ctx.cb.swap(cb);
    }else{
        event_ctx.co = Selfco::getThis();
    }
    //添加到epoll
    int op = old_event ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epev;
    epev.data.ptr = ctx;
    epev.events = EPOLLET | ctx->events;

    int ret = epoll_ctl(m_epfd,op,fd,&epev);
    if(ret){
        return -1;
    }
    ++m_pendingEventCount;
    return 0;
}
bool IOManager::delEvent(int fd,Event ev){
    //取
    FdContext* ctx = nullptr;
    MutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() > fd){
        ctx = m_fdContexts[fd];
        lock.unlock();
    }else{
        lock.unlock();
        MutexType::WriteLock lock2(m_mutex);
        contextResize(1.5 * fd);
        ctx = m_fdContexts[fd];
    }
    //改
    FdContext::MutexType::Lock lock3(ctx->mutex);
    if(!(ctx->events & ev)){
        return false;
    }
    ctx->events = (Event)(ctx->events & ~ev);
    FdContext::EventContext& event_ctx = ctx->getEventContext(ev);
    ctx->resetEventContext(event_ctx);
    //epoll中删除
    int op = ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epev;
    epev.data.ptr = ctx;
    epev.events = ctx->events;

    epoll_ctl(m_epfd,op,fd,&epev);
    --m_pendingEventCount;
    return true;
}
bool IOManager::cancelEvent(int fd,Event ev){
    FdContext* ctx = nullptr;
    MutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() > fd){
        ctx = m_fdContexts[fd];
        lock.unlock();
    }else{
        lock.unlock();
        MutexType::WriteLock lock2(m_mutex);
        contextResize(1.5 * fd);
        ctx = m_fdContexts[fd];
    }

    FdContext::MutexType::Lock lock3(ctx->mutex);
    if(!ctx->events & ev){
        return false;
    }
    //ctx->events & ~ev
    Event new_events = (Event)((ctx->events) & (~ev));
    int op = new_events? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epev;
    epev.data.ptr = ctx;
    epev.events = ctx->events;

    epoll_ctl(m_epfd,op,fd,&epev);

    ctx->trigerEvent(ev);
    --m_pendingEventCount;
    return true;
}
bool IOManager::cancelAll(int fd){
    FdContext* ctx = nullptr;
    MutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() > fd){
        ctx = m_fdContexts[fd];
        lock.unlock();
    }else{
        lock.unlock();
        MutexType::WriteLock lock2(m_mutex);
        contextResize(1.5 * fd);
        ctx = m_fdContexts[fd];
    }
    FdContext::MutexType::Lock lock3(ctx->mutex);
    if(!ctx->events){
        return false;
    }
    int op = EPOLL_CTL_DEL;
    epoll_event epev;
    epev.data.ptr = ctx;
    epev.events = 0;
    epoll_ctl(m_epfd,op,fd,&epev);

    if(ctx->events & READ){
        ctx->trigerEvent(READ);
        --m_pendingEventCount;
    }
    if(ctx->events & WRITE){
        ctx->trigerEvent(WRITE);
        --m_pendingEventCount;
    }
    return true;
}
IOManager* IOManager::GetThis(){
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle(){
    if(!hasIdlethreads()){
        return;
    }
    uint64_t flag = 1;
    int ret = write(m_eventfd,&flag,sizeof(flag));
}

bool IOManager::stopping(){
    return Scheduler::stopping() && m_pendingEventCount == 0;
}

void IOManager::idle(){
    const uint64_t MAX_EVENTS = 256;
    epoll_event* events = new epoll_event[MAX_EVENTS]();

    std::shared_ptr<epoll_event> shared_evnets(events,[](epoll_event*ptr){
        delete[] ptr;
    });

    while(true){
        uint64_t next_timeout = 0;

        if(stopping()){
            next_timeout = getNextTimer();
            if(next_timeout == ~0ull){
                break;
            }
        }

        int ret = 0;

        do{
            static const int MAX_TIMEOUT = 5000;

            if(next_timeout != ~0ull){
                next_timeout = (int) next_timeout > MAX_TIMEOUT?
                        MAX_TIMEOUT : next_timeout;
            }
            else{
                next_timeout = MAX_TIMEOUT;
            }

            ret = epoll_wait(m_epfd,events,64,(int)next_timeout);

            if(ret < 0 ){/* && errno = EINTR){ */

            }else{
                break;
            }         
        }while(true);

        std::vector<std::function<void()>> cbs;
        //将定时任务转移
        listExpiredCb(cbs);
        if(!cbs.empty()){
            addTask(cbs.begin(),cbs.end());
            cbs.clear();
        }

        for(int i = 0;i<ret;i++){
            epoll_event& event = events[i];
            if(event.data.fd == m_eventfd){
                uint64_t x;
                read(m_eventfd,&x,sizeof(x));
                continue;
            }

            FdContext* ctx = (FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(ctx->mutex);

            if(event.events & (EPOLLERR | EPOLLHUP)){
                event.events |= (EPOLLIN | EPOLLHUP) & ctx->events;
            }

            int real_events = NONE;
            if(event.events & EPOLLIN){
                real_events |= EPOLLIN;
            }
            if(event.events & EPOLLOUT){
                real_events |= EPOLLOUT;
            }

            if((ctx->events & real_events)==NONE){
                continue;
            }

            int left_events = (ctx->events & ~real_events);
            int op = left_events? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            epoll_ctl(m_epfd,op,ctx->fd,&event);

            if(real_events & READ){
                ctx->trigerEvent(READ);
                --m_pendingEventCount;
            }
            if(real_events & WRITE){
                ctx->trigerEvent(WRITE);
                --m_pendingEventCount;
            }
        }
        server::Selfco::yieldtohold();
    }
}

void IOManager::onTimerInsertedAtFront(){
    tickle();
}





}