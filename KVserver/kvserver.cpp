#include "kvserver.h"
#include "skiplist.h"
#include <memory>
#include<stdlib.h>


namespace server{

KVS::KVS(/* int thread_count */)
{

}

KVS::~KVS(){

}

void KVS::handleClient(Socket::ptr sock){
    sock->skipptr = new SkipList<std::string,std::string>(6);//一个链接一个跳表
    //SkipList<int,std::string> skipList(6);
    std::shared_ptr<SkipList<std::string,std::string>> sptr(sock->skipptr,[](SkipList<std::string,std::string>* ptr){
        delete ptr;
    });

    sock->rbuf = (char*) new char[1024];/* malloc(1024); */
    std::shared_ptr<char> rptr(sock->rbuf,[](char* ptr){
        delete[] ptr;
    });
    sock->wbuf = (char*)new char[1024];
    std::shared_ptr<char> wptr(sock->wbuf,[](char* ptr){
        delete[] ptr;
    });

    sock->kvlist = (char**)new char[1024];
    std::shared_ptr<char*> listptr(sock->kvlist,[](char** ptr){
        delete[] ptr;
    });

    while(1){

        memset(sock->rbuf,0,sizeof(*(sock->rbuf)));
        int len = sock->recv(sock->rbuf,1024);
        if(len == 0){
            //free(rbuf);
            if(sock->close() == -1){

            }
            sock->close();
            return;
        }else if(len < 0){
            sock->close();
            return;
        }else{            
            int count = kvstore_skiplist_token(sock->rbuf,sock->kvlist);            
            kvstore_parser_protocol(sock,sock->kvlist,sock->wbuf,count);
            sock->send(sock->wbuf,1024);
        }
    }
}

int KVS::kvstore_skiplist_token(char* buf,char** kvlist){
    if(buf == NULL || kvlist == NULL) return -1;
    int idx = 0;
    char* token = strtok(buf," ");
    while(token != NULL){
        kvlist[idx++] = token;
        token = strtok(NULL," ");
    }
    return idx;
}

int KVS::kvstore_parser_protocol(Socket::ptr sock,char** kvlist,char* buf,int count){
    if(kvlist == NULL || buf == NULL || count == 0 || count == -1) return -1;

    int cmd = KVS_CMD_SATRT;

    for(cmd = KVS_CMD_SATRT;cmd<KVS_CMD_SIZE;cmd++){
        if(strcmp(commands[cmd],kvlist[0])==0)
            break;
    }
    std::cout<< cmd <<"\n";
    char* wbuf = buf;
    char* key = kvlist[1];
    char* value = kvlist[2];
    memset(wbuf,0,1024);

    switch(cmd){
        case KVS_CMD_SKSET:{
            int ret;
            bool val = sock->skipptr->search_element(key);
            if(val){
                // sock->skipptr->delete_element(key);
                // ret = sock->skipptr->insert_element(key,value);
                snprintf(wbuf,1024,"EXIST");
                break;
            }else{
            ret = sock->skipptr->insert_element(key,value);
            }
            if(!ret){
                snprintf(wbuf,1024,"SUCCESS");
            }else{
                snprintf(wbuf,1024,"FAILED");
            }
            break;
        }
        case KVS_CMD_SKGET:{
            bool val = sock->skipptr->search_element(key);
            if(val){
                std::string str = sock->skipptr->get_element(key);
                snprintf(wbuf,1024,"%s",str.c_str());
            }else{
                snprintf(wbuf,1024,"Not Found");
            }
            break;
        }
        case KVS_CMD_SKDEL:{
            bool val = sock->skipptr->search_element(key);
            if(val){
                sock->skipptr->delete_element(key);
            }else{
                snprintf(wbuf,1024,"Not Found");
            }
            break;
        }
        case KVS_CMD_SKMOD:{
            bool val = sock->skipptr->search_element(key);
            int ret;
            if(val){
                sock->skipptr->delete_element(key);
                ret = sock->skipptr->insert_element(key,value);
            }else{
                snprintf(wbuf,1024,"Not Found");
            }
            break;
        }
        case KVS_CMD_SKCOUNT:{
            int count = sock->skipptr->size();
            std::string str = std::to_string(count);
            snprintf(wbuf,1024,"%s",str.c_str());
            break;
        }

        default:{
            snprintf(wbuf,1024,"Wrong Operation");
        }
    }
    return 0;
}


}