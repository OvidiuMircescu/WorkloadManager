set (_link_LIBRARIES
)

set (_test_wlm_sources
  test_workloadmanager.cxx
)

set (_test_wlm_headers
)

add_executable(test_workloadmanager ${_test_wlm_sources})
target_link_libraries(test_workloadmanager workloadmanager)
install(TARGETS test_workloadmanager RUNTIME DESTINATION bin)
