cmake_minimum_required(VERSION 3.2)
project(easynet)

set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_FLAGS -DNDEBUG)

#include(GNUInstallDirs)

find_package(Threads REQUIRED)

list(APPEND SRCS
        ${PROJECT_SOURCE_DIR}/src/net/tcpclient.cpp
        ${PROJECT_SOURCE_DIR}/src/net/connector.cpp
        ${PROJECT_SOURCE_DIR}/src/net/buffer.cpp
        ${PROJECT_SOURCE_DIR}/src/net/socket.cpp
        ${PROJECT_SOURCE_DIR}/src/net/netutil.cpp
        ${PROJECT_SOURCE_DIR}/src/net/inetaddress.cpp
        ${PROJECT_SOURCE_DIR}/src/net/acceptor.cpp
        ${PROJECT_SOURCE_DIR}/src/net/tcpconnection.cpp
        ${PROJECT_SOURCE_DIR}/src/net/tcpserver.cpp
        ${PROJECT_SOURCE_DIR}/src/net/acceptor.cpp
        
        ${PROJECT_SOURCE_DIR}/src/eventloopthreadpool.cpp
        ${PROJECT_SOURCE_DIR}/src/eventloopthread.cpp
        ${PROJECT_SOURCE_DIR}/src/eventloop.cpp
        ${PROJECT_SOURCE_DIR}/src/timestamp.cpp
        ${PROJECT_SOURCE_DIR}/src/timerManager.cpp
        ${PROJECT_SOURCE_DIR}/src/timer.cpp
        ${PROJECT_SOURCE_DIR}/src/timerfd.cpp
        ${PROJECT_SOURCE_DIR}/src/channel.cpp
        ${PROJECT_SOURCE_DIR}/src/poller.cpp
        ${PROJECT_SOURCE_DIR}/src/util.cpp
        ${PROJECT_SOURCE_DIR}/src/logging.cpp
        )

if (CMAKE_HOST_APPLE)
    add_definitions(-DOS_MACOSX)
elseif (CMAKE_HOST_UNIX)
    add_definitions(-DOS_LINUX)
else (CMAKE_HOST_APPLE)
    message(FATAL_ERROR "Platform not supported")
endif (CMAKE_HOST_APPLE)

option(BUILD_SHARED_LIBRARY "Build Shared Library" OFF)
option(BUILD_STATIC_LIBRARY "Build Static Library" ON)
option(BUILD_EXAMPLES "Build Examples" OFF)

## Shared Library
if (BUILD_SHARED_LIBRARY)
    add_library(easynet SHARED ${SRCS})
    target_include_directories(easynet 
                                PUBLIC ${PROJECT_SOURCE_DIR}/src 
                                PUBLIC ${PROJECT_SOURCE_DIR}/src/net)
    target_link_libraries(easynet ${CMAKE_THREAD_LIBS_INIT})
    install(TARGETS easynet DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif (BUILD_SHARED_LIBRARY)

# Static library
if (BUILD_STATIC_LIBRARY)
    add_library(easynet_s STATIC ${SRCS})
    target_include_directories(easynet_s 
                                PUBLIC ${PROJECT_SOURCE_DIR}/src/ 
                                PUBLIC ${PROJECT_SOURCE_DIR}/src/net/)
    target_link_libraries(easynet_s ${CMAKE_THREAD_LIBS_INIT})
    install(TARGETS easynet_s DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif (BUILD_STATIC_LIBRARY)

if (BUILD_SHARED_LIBRARY OR BUILD_STATIC_LIBRARY)
    install(FILES
            ${PROJECT_SOURCE_DIR}/src/net/tcpclient.h
            ${PROJECT_SOURCE_DIR}/src/net/connector.h
            ${PROJECT_SOURCE_DIR}/src/net/buffer.h
            ${PROJECT_SOURCE_DIR}/src/net/socket.h
            ${PROJECT_SOURCE_DIR}/src/net/netutil.h
            ${PROJECT_SOURCE_DIR}/src/net/inetaddress.h
            ${PROJECT_SOURCE_DIR}/src/net/acceptor.h
            ${PROJECT_SOURCE_DIR}/src/net/tcpconnection.h
            ${PROJECT_SOURCE_DIR}/src/net/tcpserver.h
            ${PROJECT_SOURCE_DIR}/src/net/callback.h

            ${PROJECT_SOURCE_DIR}/src/eventloopthreadpool.h
            ${PROJECT_SOURCE_DIR}/src/eventloopthread.h
            ${PROJECT_SOURCE_DIR}/src/eventloop.h
            ${PROJECT_SOURCE_DIR}/src/timerManager.h
            ${PROJECT_SOURCE_DIR}/src/timestamp.h
            ${PROJECT_SOURCE_DIR}/src/timer.h
            ${PROJECT_SOURCE_DIR}/src/timerfd.h
            ${PROJECT_SOURCE_DIR}/src/channel.h
            ${PROJECT_SOURCE_DIR}/src/poller.h
            ${PROJECT_SOURCE_DIR}/src/logging.h
            ${PROJECT_SOURCE_DIR}/src/util.h
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif (BUILD_SHARED_LIBRARY OR BUILD_STATIC_LIBRARY)

function(add_my_executable EXECUTABLE_NAME EXECUTABLE_SOURCES)
    add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_SOURCES})
    target_link_libraries(${EXECUTABLE_NAME} easynet_s)
    target_include_directories(${EXECUTABLE_NAME} 
                                PUBLIC ${PROJECT_SOURCE_DIR}/src
                                PUBLIC ${PROJECT_SOURCE_DIR}/src/net)
    install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction(add_my_executable)

if (BUILD_EXAMPLES)
    add_my_executable(test_logger test/test_logger.cpp)
    add_my_executable(test_loop_timerfd test/test_loop_timerfd.cpp)
    add_my_executable(test_loop_timer test/test_loop_timer.cpp)
    add_my_executable(test_eventloopthread test/test_eventloopthread.cpp)
    add_my_executable(test_acceptor test/test_acceptor.cpp)
    add_my_executable(test_tcpserver_discard test/test_tcpserver_discard.cpp)
    add_my_executable(test_tcpserver_echo test/test_tcpserver_echo.cpp)
    add_my_executable(test_tcpserver_send test/test_tcpserver_send.cpp)
    add_my_executable(test_tcpserver_chargen test/test_tcpserver_chargen.cpp)
    add_my_executable(test_connector test/test_connector.cpp)
    add_my_executable(test_tcpclient test/test_tcpclient.cpp)
endif (BUILD_EXAMPLES)
