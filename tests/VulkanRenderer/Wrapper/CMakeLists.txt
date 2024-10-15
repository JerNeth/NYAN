add_subdirectory(shader EXCLUDE_FROM_ALL)

add_executable(VulkanWrapperTests VulkanWrapperTests.cpp)


target_link_libraries(VulkanWrapperTests 
	PRIVATE 
			NYAN::Vulkan
			GTest::gtest_main
			test_shaders)

		
include(GoogleTest)
gtest_discover_tests(VulkanWrapperTests)