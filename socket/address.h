#ifndef __ADDRESS_H_
#define __ADDRESS_H_

#include<memory>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<vector>
#include<map>


namespace server{
// class IPAddress;

// class Address{
// public:
//     typedef std::shared_ptr<Address> ptr;

//     virtual ~Address() {}
//     static Address::ptr Create(const sockaddr* addr,socklen_t addrlen);
//     static bool Lookup(std::vector<Address::ptr>& result,const std::string& host
//         ,int family = AF_INET,int type = 0,int protocol = 0);
//     static Address::ptr LookupAny(const std::string& host,
//         int family = AF_INET,int type = SOCK_STREAM,int protocol = 0);
//     static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
//         int family = AF_INET,int type = SOCK_STREAM,int protocol = 0);
//     static bool GetInterfaceAddresses(std::multimap<std::string,
//         std::pair<Address::ptr,uint32_t> >&result,int family = AF_INET);
//     static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr,uint32_t>>& result,
//         const std::string& iface,int family = AF_INET);
//     virtual const sockaddr* getAddr() const = 0;
//     virtual sockaddr* getAddr() = 0;
//     virtual socklen_t getAddrLen() const = 0;
//     virtual std::ostream& insert(std::ostream& os) const = 0;
//     int getFamily() const;
//     std::string toString() const;
//     bool operator<(const Address& rhs) const;
//     bool operator==(const Address& rhs) const;
//     bool operator!=(const Address& rhs) const;

// };
class Address{
public:
typedef std::shared_ptr<Address> ptr;
Address(std::string ip,uint16_t port = -1);
Address(sockaddr_in addr);
Address() = default;
~Address();

Address::ptr broadcastAddress(uint32_t prefix);
Address::ptr networkAddress(uint32_t prefix);
Address::ptr subnetAddress(uint32_t prefix);

std::string getDottenDecimalIP()const;
sockaddr* getAddr() const;
void setPort(uint16_t port);
int getPort() const;
int getFamily() const;
int getAddrLen() const;
private:

uint32_t createSubNetMask(uint32_t prefix);
sockaddr_in m_Addr;
};

}





#endif