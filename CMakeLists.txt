cmake_minimum_required(VERSION 3.16)
project(new)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -pthread")


#add_compile_options(-Wall -Wextra -pedantic -Werror)
#find_package(Threads REQUIRED)

#set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories(
    fd_manager
    hook
    iomananger
    KVserver
    lock
    nocopyable
    scheduler
    selfco
    skiplist
    socket
    tcp_server
    thread
    time
    util
)

set(SRC fd_manager/fd_manager.cpp
        hook/hook.cpp
        iomananger/iomanager.cpp
        KVserver/kvserver.cpp
        lock/lock.cpp
        nocopyable/nocopyable.cpp
        scheduler/scheduler.cpp
        selfco/selfco.cpp
        socket/socket.cpp
        socket/address.cpp
        tcp_server/tcp_server.cpp
        thread/thread.cpp
        time/time.cpp
        util/util.cpp
)


add_library(server SHARED ${SRC})

#target_link_libraries(server PUBLIC pthread)
set(LIBS server
         dl
         pthread)


add_compile_options(-std=c++14 -pthread)

add_executable(main main.cpp)

target_link_libraries(main ${LIBS})
