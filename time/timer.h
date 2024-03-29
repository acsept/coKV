 #ifndef __TIME_H__
 #define __TIME_H__

#include <memory>
#include <set>
#include <vector>
#include <functional>
#include "thread.h"

namespace server{
class TimerManager;

class Timer :public std::enable_shared_from_this<Timer>
{
    friend TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;
    bool cancel();
    bool refresh();
    bool reset(uint64_t ms,bool from_now);

private:
    Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager* manager);
    Timer(uint64_t next);
private:
    struct Comperator{
        bool operator()(const Timer::ptr& lhs,const Timer::ptr& rhs)const;
    };
private:
    bool m_recurring = false;
    uint64_t m_ms = 0;
    uint64_t m_next = 0;

    std::function<void()> m_cb;
    TimerManager* m_manager = nullptr;
};

class TimerManager{
    friend Timer;
public:
    typedef RWMutex RWMutexType;
    TimerManager();
    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms,std::function<void()> cb,bool recurring = false);
    Timer::ptr addConditionTimer(uint64_t ms,std::function<void()> cb,std::weak_ptr<void> weak_cond,bool recurring = false);

    uint64_t getNextTimer();
    void listExpiredCb(std::vector<std::function<void()>>& cbs);

protected:
    virtual void onTimerInsertedAtFront() = 0;

    void addTimer(Timer::ptr val,RWMutexType::WriteLock& lock);
private:
    bool detectClockRollover(uint64_t now_ms);
private:
    RWMutexType m_mutex;
    std::set<Timer::ptr,Timer::Comperator> m_timers;
    bool m_tickled = false;
    uint64_t m_previouseTime = 0;
};


}


#endif