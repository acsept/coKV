#ifndef __KVSERVER_H__
#define __KVSERVER_H__

#include "tcp_server.h"
#include "skiplist.h"
#define CMDCOUNT 5
enum{
        KVS_CMD_SATRT = 0,
        KVS_CMD_SKSET = KVS_CMD_SATRT,
        KVS_CMD_SKGET,
        KVS_CMD_SKDEL,
        KVS_CMD_SKMOD,
        KVS_CMD_SKCOUNT,
        KVS_CMD_SIZE
    };

namespace server{
    static const char*  commands[CMDCOUNT] = {"SKSET","SKGET","SKDEL","SKMOD","SKCOUNT"};
//  #define CMDCOUNT 5
class KVS: public TcpServer{
public:
    friend TcpServer;
    typedef std::shared_ptr<KVS> ptr;

    KVS(/* int thread_count */);//创建iom 
    ~KVS();

    void handleClient(Socket::ptr sock);//1.实现跳表的创建
                                        //(要改文件创建的位置，不然每个链接的数据存在一起)
                                        //同时每一个链接要创建用户缓冲区 临时存数据
                                        //2.recv 数据分析
                                        //3.send 数据处理回发 
private:
    static int kvstore_parser_protocol(Socket::ptr sock,char** kvlist,char* buf,int count);//数据处理 回发
    static int kvstore_skiplist_token(char* buf,char** kvlist);//数据分析
private:
    //IOManager::ptr m_iom;
};




}






#endif