add_executable(VulkanRendererTests VulkanRendererTests.cpp)

enable_testing()
target_link_libraries(VulkanRendererTests 
	PRIVATE 
			NYAN::VulkanRenderer
			GTest::gtest_main)

			
include(GoogleTest)
gtest_discover_tests(VulkanRendererTests)