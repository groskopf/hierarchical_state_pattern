
add_executable(hsm_test 
	hsm_choice_point_test.cpp
	hsm_external_transition_test.cpp
	hsm_hierarchy_test.cpp
	hsm_history_state_test.cpp
	hsm_simple_test.cpp
	hsm_transition_guard_test.cpp
)

set_property(TARGET hsm_test PROPERTY CXX_STANDARD 17)

target_link_libraries(hsm_test 
PRIVATE
	hsm
	gtest
	gmock
	gtest_main
	gmock_main
	pthread	
)

include(GoogleTest)
gtest_discover_tests(hsm_test hsm_test)

