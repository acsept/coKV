#include <iostream>
#include "scheduler.h"


void tt(){
    std::cout<<"addtask"<<std::endl;
}

void text1(){
    static int count = 5;
    sleep(1);
    while(count-- >= 0){
        server::Scheduler::GetThis()->addTask(tt);
    }
}

// void tt(){
//     std::cout<<"addtask"<<std::endl;
// }

int main(){
    server::Selfco::ptr co1(new server::Selfco(text1));

    server::Scheduler sc(1,false);

    sc.start();
    sc.addTask(co1);
    sc.stop();

    return 0;
}
