#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "address.h"
#include "socket.h"
#include "iomanager.h"
#include <functional>
#include <memory>
#include <cstring>

namespace server{

class TcpServer : public std::enable_shared_from_this<TcpServer>{
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(IOManager* accept = IOManager::GetThis(),IOManager* worker = IOManager::GetThis());
    bool bind(Address::ptr& addr);
    void start();
    void stop();
    virtual ~TcpServer() = default;
protected:
    void handleAccept();
    virtual void handleClient(Socket::ptr sock);
protected:
    Socket::ptr mLisSock;
    IOManager* mAcceptor;
    IOManager* mWorker;
    bool mIsStop;
};


}

#endif