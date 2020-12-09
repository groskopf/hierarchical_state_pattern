cmake_minimum_required(VERSION 3.16)
project(HierarchicalStatePattern)


# library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include)

# unit test 
if(UNIT_TESTS)
	add_compile_options(-ggdb)
	enable_testing()
	find_package(GTest REQUIRED)
	add_subdirectory(test)
endif()
