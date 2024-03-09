#include "kvserver.h"

void run(){
    std::cout<<"run\n";
    server::KVS::ptr server(new server::KVS);
    server::Address::ptr addr(new server::Address("0",8888));
    server->bind(addr);
    server->start();
}

int main(){
    
    server::IOManager iom(2,false);
    iom.addTask(run);

    return 0;
}