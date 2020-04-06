set (_link_LIBRARIES
  Threads::Threads
)

set (_wlm_sources
  Task.cxx
  WorkloadManager.cxx
  DefaultAlgorithm.cxx
)

set (_wlm_headers
  Task.hxx
  WorkloadManager.hxx
  WorkloadAlgorithm.hxx
  DefaultAlgorithm.hxx
)

add_library(workloadmanager ${_wlm_sources})
target_link_libraries(workloadmanager ${_link_LIBRARIES})
target_include_directories(workloadmanager PUBLIC
                          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                          $<INSTALL_INTERFACE:include>)


install(TARGETS workloadmanager EXPORT WorkloadManagerCfg
        LIBRARY DESTINATION lib)
install(FILES ${_wlm_headers} DESTINATION include/workloadmanager)
install(EXPORT WorkloadManagerCfg DESTINATION lib/cmake/workloadmanager
        FILE WorkloadManagerConfig.cmake)

add_subdirectory(Test)
