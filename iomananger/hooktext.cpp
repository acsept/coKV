#include "hook.h"

#include "iomanager.h"

#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>

void test_hook()
{
    //std::cout << "1111" << std::endl;
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    // fcntl(cfd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t addrlen = sizeof(addr);
    bind(cfd,(sockaddr*)&addr,sizeof(addr));
    //inet_pton(AF_INET, "112.80.248.75", &addr.sin_addr.s_addr);

    //LOG_INFO(g_logger) << "begin connect";
    // int rt = connect(cfd, (const sockaddr*)&addr, sizeof(addr));
    //LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;
    listen(cfd,10);

    int sockfd = accept(cfd,(sockaddr *)&addr,&addrlen);

    // if(rt)
    // {
    //     return;
    // }

    // const char data[] = "GET / HTTP/1.0\r\n\r\n";
    // rt = send(sockfd, data, sizeof(data), 0);
    //LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    // if(rt <= 0)
    // {
    //     return;
    // }

    std::string buff;
    buff.resize(4096);
    int rt = recv(sockfd, &buff[0], buff.size(), 0);
    //LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0)
    {
        return;
    }
    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sockfd, data, sizeof(data), 0);

    buff.resize(rt);
    //LOG_INFO(g_logger) << buff;

}



int main(){

    server::IOManager iom(1,false);
    iom.addTask(test_hook);

    getchar();

    return 0;
}