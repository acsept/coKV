#include "selfco.h"
#include "scheduler.h"
#include <atomic>
namespace server
{
    static thread_local Selfco* co = nullptr;
    static thread_local Selfco::ptr main_co = nullptr;
    static std::atomic<int> s_co{0};


    Selfco::Selfco(){
        m_state = EXEC;
        SetThis(this);
        getcontext(&m_ctx);
        s_co++;
    }

    Selfco::Selfco(std::function<void()> cb,int stacksize,bool use_caller)
    :m_cb(cb)
    {
        s_co++;
        m_stacksize = stacksize ? stacksize : 1024 * 1024;
        m_stack = malloc(m_stacksize);

        getcontext(&m_ctx);

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_size = m_stacksize;
        m_ctx.uc_stack.ss_sp = m_stack;

        if(use_caller){
            makecontext(&m_ctx,&server::Selfco::MainFunc,0);
        }else{
            makecontext(&m_ctx,&server::Selfco::MainFunc,0);
        }
    }

    Selfco::~Selfco(){
        --s_co;
        if(m_stack){
            free(m_stack);
        }else{
            Selfco* cur = co;
            if(cur = this){
                SetThis(nullptr);
            }
        }
        
    }

    void Selfco::SetThis(Selfco* cur){
        co = cur;
    }

    Selfco::ptr Selfco::getThis(){
        if(co){
            return co->shared_from_this();
        }else{
            Selfco::ptr cur(new Selfco);
            main_co = cur;
            return cur->shared_from_this();
        }
    }

    void Selfco::resuem(){
        SetThis(this);
        m_state = EXEC;

        swapcontext(&server::Scheduler::GetSchedulerSelfco()->m_ctx,&m_ctx);
    }

    void Selfco::yield(){
        SetThis(main_co.get());
        m_state = HOLD;

        swapcontext(&m_ctx,&server::Scheduler::GetSchedulerSelfco()->m_ctx);
    }

    void Selfco::yieldtohold(){
        Selfco* cur = Selfco::getThis().get();
        cur->m_state = Selfco::HOLD;
        cur->yield();
    }

    void Selfco::reset(std::function<void()> cb){
        m_cb = cb;

        getcontext(&m_ctx);
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx,&Selfco::MainFunc,0);
        m_state = INIT;
    }
    void Selfco::MainFunc(){
        Selfco::ptr cur = Selfco::getThis();
        if(cur->m_cb){
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        }
        auto p = cur.get();
        cur.reset();
        p->yield();

    }

} // namespace server


