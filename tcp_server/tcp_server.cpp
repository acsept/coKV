#include "tcp_server.h"
#include <string>
#include <fcntl.h>

namespace server{

TcpServer::TcpServer(IOManager* accept,IOManager* worker)
:mAcceptor(accept)
,mWorker(worker)
,mIsStop(true)
{
    mLisSock = Socket::createTcp();
}

bool server::TcpServer::bind(Address::ptr& addr){
    while(mLisSock->bind(addr) != 0){
        sleep(2);
    }
    if(mLisSock->listen() != 0){
        return false;
    }
    return true;
}

void server::TcpServer::start(){
    if(!mIsStop){
        return;
    }
    mIsStop = false;
    mAcceptor->addTask(std::bind(&TcpServer::handleAccept,shared_from_this()));
}

void server::TcpServer::stop(){
    mIsStop = true;
}

void server::TcpServer::handleAccept(){
    Socket::ptr newClient;
    while(!mIsStop){
        newClient.reset();
        newClient = mLisSock->accept();
        if(!newClient){
            continue;
        }
        mWorker->addTask(std::bind(&TcpServer::handleClient,shared_from_this(),newClient));
    }
    mLisSock->close();
}

void server::TcpServer::handleClient(Socket::ptr sock){
    while(1){
        char buf[1024];
        memset(buf,0,sizeof(buf));
        int len = sock->recv(buf,1024);
        if(len == 0){
            if(sock->close() == -1){

            }
            sock->close();
            return;
        }else if(len < 0){
            sock->close();
            return;
        }else{
            std::string sendbuf = "hello world!\r\n";
            ssize_t len = sock->send(sendbuf.c_str(),strlen(sendbuf.c_str()));
            if(len != (ssize_t)strlen(sendbuf.c_str())){
                return;
            }
        }
    }
}

}