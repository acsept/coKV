#include "iomanager.h"
#include "../hook/hook.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>


void text(){
    sleep(1);
    std::cout<<"Task"<<"\n";
}

static server::Timer::ptr s_timer;

void test_timer()
{
    server::IOManager iom(1);
    //iom.schedule(&test_fiber1);
    
    s_timer = iom.addTimer(2000, [](){
        static int i = 0;

        //LOG_INFO(g_logger) << "timer test " << i;
        // if(++i == 3)
        // {
        //     //s_timer->refresh();
        //     //s_timer->reset(2000, true);
        //     s_timer->cancel();
        // }
        std::cout<<"定时器触发"<<"\n";
        
    }, true);
    getchar();

}


int main(){
    server::IOManager::ptr iom (new server::IOManager(1,false));
    iom->addTask(text);
    iom->addTimer(1000,[](){
        std::cout<<"定时器触发"<<"\n";
    },true);
    // test_timer();
    getchar();
    return 0;
}