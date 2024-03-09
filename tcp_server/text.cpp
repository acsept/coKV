#include "tcp_server.h"


void run(){
    server::TcpServer::ptr server(new server::TcpServer);
    server::Address::ptr addr(new server::Address("0",8888));
    server->bind(addr);
    server->start();
}

static int thread_cnt = 2;

int main(){

    server::IOManager iom(thread_cnt);
    iom.addTask(run);

    return 0;
}