#define __USE_GNU

#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>

#include "iomanager.h"
#include "fd_manager.h"
#include "hook.h"
#include "selfco.h"


#include "hook.h"


namespace server{

#define RECV 0
#define SEND 1

static thread_local bool t_hook_enable = false;
static uint64_t s_tcp_connect_timeout = -1;

#define HOOK_FUN(XX) \
    XX(socket) \
    XX(accept) \
    XX(read) \
    XX(recv) \
    XX(write) \
    XX(send) \
    XX(close)

void hook_init(){
    static bool is_inited = false;
    if(is_inited){
        return;
    }
    //std::cout<<"hook"<<"\n";
    #define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT,#name);
    HOOK_FUN(XX);
    #undef XX
}

struct HookIniter{
    HookIniter(){
        hook_init();
        s_tcp_connect_timeout = 5000;
    }
};

static HookIniter _hook_init;


bool is_hook_enable(){
    return t_hook_enable;
}

void set_hook_enable(bool flag){
    t_hook_enable = flag;
}

}

template <typename OriginFun,typename... Args>
static ssize_t io_func(int fd,uint32_t event,OriginFun fun,int type,Args&&... args){
    if(!server::t_hook_enable){
        return fun(fd,std::forward<Args>(args)...);
    }
    server::FdCtx::ptr ctx = server::FdManager::getInstance()->get(fd);
    if(!ctx){
        return fun(fd,std::forward<Args>(args)...);
    }

    if(ctx->isClose()){
        errno = EBADF;
        return -1;
    }
    if(!ctx->isSocket()){
        return fun(fd,std::forward<Args>(args)...);
    }
    int to = ctx->getTimeout(type);
Retry:
    ssize_t n = fun(fd,std::forward<Args>(args)...);
    while(n == -1 && errno == EINTR){
        n = fun(fd,std::forward<Args>(args)...);
    }
    if(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
        int cancel = 0;
        server::IOManager* iom = server::IOManager::GetThis();
        server::Timer::ptr timer;
        if(to != -1){
            timer = iom->addTimer(to*1000,[&cancel,iom,fd,event](){
                cancel = 1;
                iom->cancelEvent(fd,(server::IOManager::Event)event);
            });
        }
        int ret = iom->addEvent(fd,(server::IOManager::Event)event);
        //server::Selfco::yieldtohold();
        if(ret){
            if(timer){
                timer->cancel();
            }
            return -1;
        }else{

            server::Selfco::yieldtohold();
            if(timer){
                timer->cancel();
            }

            if(cancel){
                errno = ETIMEDOUT;
                return -1;
            }
            goto Retry;
        }

    }
    return n;
}


extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

// int socket(int domain,int type,int protocol){
//     int fd = socket_f(domain,type,protocol);
//     if(fd == -1){
//         return fd;
//     }
//     //server::FdManager::getInstance()->get(fd,true);
//     server::FdManager::getInstance()->get(fd,true);
//     return fd;
// }

int accept(int s,struct sockaddr* addr,socklen_t* addrlen){
    std::cout<<"hook accept"<<"\n";
    int fd = io_func(s,EPOLLIN,accept_f,RECV,addr,addrlen);
    if(fd >= 0){
        server::FdManager::getInstance()->get(fd,true);
    }
    return fd;
}

int socket(int domain,int type,int protocol){
    if(!server::t_hook_enable){
        return socket_f(domain,type,protocol);
    }
    std::cout<<"hook socket"<<"\n";
    int fd = socket_f(domain,type,protocol);
    if(fd == -1){
        return fd;
    }
    server::FdManager::getInstance()->get(fd,true);
    return fd;
}

ssize_t read(int fd,void* buf,size_t count){
    std::cout<<"hook read"<<"\n";
    return io_func(fd,EPOLLIN,read_f,RECV,buf,count);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    std::cout<<"hook recv"<<"\n";
    return io_func(sockfd, EPOLLIN, recv_f, RECV, buf, len, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    std::cout<<"hook write"<<"\n";
    return io_func(fd, EPOLLOUT, write_f, SEND, buf, count);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    std::cout<<"hook send"<<"\n";
    return io_func(s, EPOLLOUT, send_f, SEND, msg, len, flags);
}

int close(int fd) {
    server::FdCtx::ptr ctx = server::FdManager::getInstance()->get(fd);
    if(ctx) {
        server::IOManager::GetThis()->cancelAll(fd);
        server::FdManager::getInstance()->del(fd);
    }
    return close_f(fd);
}


}


