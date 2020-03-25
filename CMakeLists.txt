cmake_minimum_required(VERSION 2.6)
project(workloadmanager)

set (CMAKE_CXX_STANDARD 11)
set (BUILD_SHARED_LIBS TRUE)
enable_testing()

set (_link_LIBRARIES
)

set (_wlm_sources
  Container.cxx
  ContainerType.cxx
  Resource.cxx
  Task.cxx
  WorkloadManager.cxx
)

set (_wlm_headers
  Container.hxx
  ContainerType.hxx
  Resource.hxx
  Task.hxx
  WorkloadManager.hxx
)

add_library(workloadmanager ${_wlm_sources})
target_link_libraries(workloadmanager ${_link_LIBRARIES})
#target_include_directories(workloadmanager 


install(TARGETS workloadmanager EXPORT FindWorkloadManager
        LIBRARY DESTINATION lib)
install(FILES ${_wlm_headers} DESTINATION include/workloadmanager)
install(EXPORT workloadmanager DESTINATION lib/cmake/workloadmanager)

add_executable(test_workloadmanager main.cpp)
install(TARGETS test_workloadmanager RUNTIME DESTINATION bin)
