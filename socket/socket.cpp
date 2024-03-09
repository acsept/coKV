#include"socket.h"
#include<cstring>

namespace server{

Socket::Socket(Family ipverson,Type type){
    m_sockfd = ::socket(ipverson,type,0);
}

Socket::Socket(int fd){
    m_sockfd = fd;
    isconnected = true;
}

Socket::~Socket(){
   /*  if(wbuf){
    free(wbuf);
    }
    if(rbuf){
    free(rbuf);
    }
    if(kvlist){
    free(kvlist);
    } */
    m_sockfd = -1;
    isconnected = false;
}

Socket::ptr Socket::accept(Address::ptr addr){
    int sock = ::accept(m_sockfd,nullptr,nullptr);
    if(sock == -1){
        if(errno != EAGAIN && errno != EWOULDBLOCK){

        }
        return nullptr;
    }
    return Socket::ptr(new Socket(sock));
}

int Socket::connect(Address::ptr& addr){
    isconnected = true;
    return ::connect(m_sockfd,addr->getAddr(),addr->getAddrLen());
}

int Socket::bind(Address::ptr& addr){
    m_Addr = addr;
    return ::bind(m_sockfd,addr->getAddr(),addr->getAddrLen());
}

int Socket::close(){
    isconnected = false;
    return ::close(m_sockfd);
}

ssize_t Socket::send(const char* buf,int size,int flag){
    return ::send(m_sockfd,buf,size,flag);
}

ssize_t Socket::recv(char* buf,int size,int flag){
    return ::recv(m_sockfd,buf,size,flag);
}

Socket::ptr Socket::createTcp(){
    return Socket::ptr(new Socket(IPv4,TCP));
}

Socket::ptr Socket::createUdp(){
    return Socket::ptr(new Socket(IPv4,UDP));
}

}