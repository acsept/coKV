#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__

//#include "../nocopyable/nocopyable.h"
#include "timer.h"
#include "scheduler.h"
#include "selfco.h"
//#include "time.h"
#include <vector>
#include <sys/epoll.h>
#include <sys/types.h>
#include <functional>
#include <list>



namespace server{

class IOManager: public Scheduler , public TimerManager{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex MutexType;
    enum Event{
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x4
    };
public:
    IOManager(size_t threads = 1,bool use_caller = true);
    ~IOManager();
    int addEvent(int fd,Event ev,std::function<void()> cb = nullptr,int flag = EPOLLET);
    bool delEvent(int fd,Event ev);
    bool cancelEvent(int fd,Event ev);
    bool cancelAll(int fd);
    static IOManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;

    void onTimerInsertedAtFront() override;
    void contextResize(size_t size);

private:
    struct FdContext{
        typedef Mutex MutexType;
        struct EventContext{
            Scheduler* scheduler = nullptr;
            Selfco::ptr co;
            std::function<void()> cb;
        };
        EventContext& getEventContext(Event event);
        void resetEventContext(EventContext& ctx);
        void trigerEvent(Event ev);

        EventContext read;
        EventContext write;

        int fd;
        Event events = NONE;
        MutexType mutex;
    };
private:
    int m_epfd;
    int m_tickleFds[2];
    int m_eventfd;
    std::atomic<size_t> m_pendingEventCount = {0};

    std::vector<FdContext*> m_fdContexts;
    MutexType m_mutex;

};

}





#endif