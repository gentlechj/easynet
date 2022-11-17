cmake_minimum_required(VERSION 3.2)
project(easynet)

set(CMAKE_CXX_STANDARD 11)

#include(GNUInstallDirs)

find_package(Threads REQUIRED)

list(APPEND SRCS
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
    add_library(server SHARED ${SRCS})
    target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})
    install(TARGETS server DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif (BUILD_SHARED_LIBRARY)

# Static library
if (BUILD_STATIC_LIBRARY)
    add_library(server_s STATIC ${SRCS})
    target_include_directories(server_s PUBLIC ${PROJECT_SOURCE_DIR}/src/)
    target_link_libraries(server_s ${CMAKE_THREAD_LIBS_INIT})
    install(TARGETS server_s DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif (BUILD_STATIC_LIBRARY)

if (BUILD_SHARED_LIBRARY OR BUILD_STATIC_LIBRARY)
    install(FILES
            ${PROJECT_SOURCE_DIR}/src/logging.h
            ${PROJECT_SOURCE_DIR}/src/util.h
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif (BUILD_SHARED_LIBRARY OR BUILD_STATIC_LIBRARY)

function(add_my_executable EXECUTABLE_NAME EXECUTABLE_SOURCES)
    add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_SOURCES})
    target_link_libraries(${EXECUTABLE_NAME} server_s)
    target_include_directories(${EXECUTABLE_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)
    install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction(add_my_executable)

if (BUILD_EXAMPLES)
    add_my_executable(logger test/logger.cpp)
endif (BUILD_EXAMPLES)
