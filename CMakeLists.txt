cmake_minimum_required(VERSION 2.6)
project(workloadmanager)

set (CMAKE_CXX_STANDARD 11)
set (BUILD_SHARED_LIBS TRUE)
enable_testing()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set (_link_LIBRARIES
  Threads::Threads
)

set (_wlm_sources
  Task.cxx
  WorkloadManager.cxx
)

set (_wlm_headers
  Task.hxx
  WorkloadManager.hxx
)

add_library(workloadmanager ${_wlm_sources})
target_link_libraries(workloadmanager ${_link_LIBRARIES})
#target_include_directories(workloadmanager 


install(TARGETS workloadmanager LIBRARY DESTINATION lib)
#install(TARGETS workloadmanager EXPORT FindWorkloadManager
#        LIBRARY DESTINATION lib)
install(FILES ${_wlm_headers} DESTINATION include/workloadmanager)
#install(EXPORT workloadmanager DESTINATION lib/cmake/workloadmanager)

add_subdirectory(Test)
