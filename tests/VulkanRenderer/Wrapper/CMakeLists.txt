add_subdirectory(shader EXCLUDE_FROM_ALL)

add_executable(VulkanWrapperTests VulkanWrapperTests.cpp)


enable_testing()
target_link_libraries(VulkanWrapperTests 
	PRIVATE 
			NYAN::VulkanWrapper
			GTest::gtest_main
			test_shaders)

		
include(GoogleTest)
gtest_discover_tests(VulkanWrapperTests)