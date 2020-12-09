
add_executable(hsm_example 
	pump_control_hsm.cpp
	pump_control_hsm_states.cpp
	pump_control_hsm_test.cpp
)

set_property(TARGET hsm_example PROPERTY CXX_STANDARD 17)

target_link_libraries(hsm_example 
PRIVATE
	hsm
	gtest
	gmock
	gtest_main
	gmock_main
	pthread	
)

include(GoogleTest)
gtest_discover_tests(hsm_example hsm_example)

