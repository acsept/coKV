#ifndef __SOCKET_H_
#define __SOCKET_H_
#include"address.h"
#include "skiplist.h"
#include<fcntl.h>
#include<memory>
#include<unistd.h>

namespace server{

class Socket{
public:
    typedef std::shared_ptr<Socket> ptr;
    enum Type{
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM,
    };
    enum Family{
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
    };
public:
    Socket(Family ipverson,Type type);
    Socket(int fd);
    Socket() = default;
    ~Socket();

    Address::ptr getLocalAddr() const { return m_Addr; }
    bool isConnect() const {return isconnected;}
    int getSockfd() const {return m_sockfd;}
    int listen() { return ::listen(m_sockfd,128);}
    ssize_t sendTo() {return 0;}
    ssize_t recvFrom() {return 0;}
    ssize_t send(const char* buf,int size,int flag = 0);
    ssize_t recv(char* buf,int size,int flag = 0);

    Socket::ptr accept(Address::ptr addr = nullptr);
    int connect(Address::ptr& addr);
    int bind(Address::ptr& addr);
    int close();
public:
    static Socket::ptr createTcp();
    static Socket::ptr createUdp();
    char* rbuf = nullptr;
    char* wbuf = nullptr;
    char** kvlist = nullptr;
    SkipList<std::string,std::string>* skipptr = nullptr;
private:
    int m_sockfd = -1;
    bool isconnected = false;
    Address::ptr m_Addr;
};


}




#endif