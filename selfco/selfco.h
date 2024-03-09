#ifndef __SELFCO_H_
#define __SELFCO_H_

#include <iostream>
#include <functional>
#include <ucontext.h>
#include <memory>

namespace server{
//class Scheduler;

class Selfco :public std::enable_shared_from_this<Selfco>
{
public:
    typedef std::shared_ptr<Selfco> ptr;
    enum State{
        INIT,
        EXEC,
        HOLD,
        TERM,
        READY
    };


private:
    Selfco();

public:
    Selfco(std::function<void()> cb,int stacksize = 0,bool use_caller = false);
    ~Selfco();
    static Selfco::ptr getThis();
    void SetThis(Selfco* co);
    void resuem();
    void yield();
    void reset(std::function<void()> cb);
    static void yieldtohold();
    static void MainFunc();
    void setState(State state){m_state = state;}
    State getState() const {return m_state;}
private:
    ucontext_t m_ctx;
    void* m_stack = nullptr;
    int m_stacksize = 0;

    std::function<void()> m_cb;
    State m_state = INIT;
    
};


}




#endif
