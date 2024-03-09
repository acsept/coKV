#include "address.h"

namespace server{

Address::Address(std::string ip,uint16_t port){
    bzero(&m_Addr,sizeof(m_Addr));
    m_Addr.sin_family = AF_INET;
    if(port >= 0){
        m_Addr.sin_port = htons(port);
    }
    if(ip == "0"){
        m_Addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }else{
        m_Addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
}

Address::Address(sockaddr_in addr){
    m_Addr = addr;
}

Address::~Address(){
    memset(&m_Addr,0,sizeof(m_Addr));
}

uint32_t Address::createSubNetMask(uint32_t prefix){
    return ~((1<<(32-prefix))-1);
}

Address::ptr Address::broadcastAddress(uint32_t prefix){
    if(prefix > 32)
        return nullptr;
    sockaddr_in addr = m_Addr;
    addr.sin_addr.s_addr |= htonl(~createSubNetMask(prefix));

    return Address::ptr(new Address(addr));
}

Address::ptr Address::networkAddress(uint32_t prefix){
    if(prefix > 32){
        return nullptr;
    }
    sockaddr_in addr = m_Addr;
    addr.sin_addr.s_addr = htonl(~createSubNetMask(prefix));
    return Address::ptr(new Address(addr));
}

Address::ptr Address::subnetAddress(uint32_t prefix) {
    if(prefix > 32)
        return nullptr;
    sockaddr_in addr = m_Addr;
    addr.sin_addr.s_addr = htonl(createSubNetMask(prefix));
    return Address::ptr(new Address(addr));
}

sockaddr* Address::getAddr() const{
    return (sockaddr*) &m_Addr;
}

std::string Address::getDottenDecimalIP() const{
    char buf[20];
    return inet_ntop(AF_INET,&m_Addr.sin_addr.s_addr,buf,sizeof(buf));
}

int Address::getFamily() const {
    return m_Addr.sin_family;
}

int Address::getAddrLen() const {
    return sizeof m_Addr;
}

int Address::getPort() const {
    return ntohs(m_Addr.sin_port);
}

void Address::setPort(uint16_t port) {
    m_Addr.sin_port = htons(port);
}








}