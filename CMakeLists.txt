cmake_minimum_required(VERSION 2.6)
project(workloadmanager)

set (CMAKE_CXX_STANDARD 11)
set (BUILD_SHARED_LIBS TRUE)
enable_testing()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_library(CPPUNIT_LIBRARIES NAMES cppunit)
#find_package(cppunit REQUIRED)

add_subdirectory(src)
