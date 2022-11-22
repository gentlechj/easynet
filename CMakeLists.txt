cmake_minimum_required(VERSION 3.2)
project(easynet)

set(CMAKE_CXX_STANDARD 11)

#include(GNUInstallDirs)

find_package(Threads REQUIRED)

list(APPEND SRCS
        ${PROJECT_SOURCE_DIR}/src/eventloop.cpp
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
    target_include_directories(easynet PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(easynet ${CMAKE_THREAD_LIBS_INIT})
    install(TARGETS easynet DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif (BUILD_SHARED_LIBRARY)

# Static library
if (BUILD_STATIC_LIBRARY)
    add_library(easynet_s STATIC ${SRCS})
    target_include_directories(easynet_s PUBLIC ${PROJECT_SOURCE_DIR}/src/)
    target_link_libraries(easynet_s ${CMAKE_THREAD_LIBS_INIT})
    install(TARGETS easynet_s DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif (BUILD_STATIC_LIBRARY)

if (BUILD_SHARED_LIBRARY OR BUILD_STATIC_LIBRARY)
    install(FILES
            ${PROJECT_SOURCE_DIR}/src/eventloop.h
            ${PROJECT_SOURCE_DIR}/src/timerManager.h
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
    target_include_directories(${EXECUTABLE_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)
    install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction(add_my_executable)

if (BUILD_EXAMPLES)
    add_my_executable(test_logger test/test_logger.cpp)
    add_my_executable(test_loop_timerfd test/test_loop_timerfd.cpp)
    add_my_executable(test_loop_timer test/test_loop_timer.cpp)
endif (BUILD_EXAMPLES)
