#ifndef __HOOK_H_
#define __HOOK_H_

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <timer.h>
#include <unistd.h>

namespace server{

    bool id_hook_enable();

    void set_hook_enable(bool flag);

}

extern "C"{
    typedef int (*socket_fun)(int fomain,int type,int protocol);
    extern socket_fun socket_f;

    // typedef int (*connect_fun)(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
    // extern connect_fun connect_f;

    typedef int (*accept_fun)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    extern accept_fun accept_f;

    typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
    extern read_fun read_f;

    typedef ssize_t (*recv_fun)(int sockfd, void *buf, size_t len, int flags);
    extern recv_fun recv_f;

    typedef ssize_t (*write_fun)(int fd,const void *buf, size_t count);
    extern write_fun write_f;

    typedef ssize_t (*send_fun)(int sockfd,const void *buf, size_t len, int flags);
    extern send_fun send_f;

    typedef int (*close_fun)(int fd);
    extern close_fun close_f;

}



#endif