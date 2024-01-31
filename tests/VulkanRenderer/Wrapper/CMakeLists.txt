add_executable(VulkanWrapperTests VulkanWrapperTests.cpp)

enable_testing()
target_link_libraries(VulkanWrapperTests 
	PRIVATE 
			NYAN::VulkanWrapper
			GTest::gtest_main)

			
include(GoogleTest)
gtest_discover_tests(VulkanWrapperTests)