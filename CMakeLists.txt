cmake_minimum_required (VERSION 2.8)

project (codeReason)
set(CMAKE_BUILD_TYPE Debug)

#lets use boost for smart pointers
set(BOOST_USE_MULTITHREADED ON)
if(WIN32)
    set(Boost_USE_STATIC_LIBS ON)
endif(WIN32)
find_package(Boost COMPONENTS thread system filesystem program_options date_time regex REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

#find the place where VEX lives ... 
find_path(VEX_INCLUDE_DIR libvex.h REQUIRED PATHS ./vexTRUNK/pub NO_DEFAULT_PATH)

include_directories(${VEX_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
#include_directories(${LLVM_INC_DIR})
include_directories(${PROTOBUF_INCLUDE_DIR})

IF(WIN32)
    #on win32, vex is pre-built
    find_library(VEX_LIB VEX.lib HINTS ./build_for_windows/lib32 NO_DEFAULT_PATH)
ELSE(WIN32)
    find_library(VEX_LIB libvex.a HINTS ./vexTRUNK NO_DEFAULT_PATH)
ENDIF(WIN32)

add_definitions(-D__i386__)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(gtest_force_shared_crt ON)

add_subdirectory(tools)
add_subdirectory(libs)
add_subdirectory(lua-5.1.4)
add_subdirectory(udis86)
add_subdirectory(gtest-1.6.0)
