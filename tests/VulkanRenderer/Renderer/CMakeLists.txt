add_executable(RendererTests RendererTests.cpp)

target_link_libraries(RendererTests 
	PRIVATE 
			NYAN::Renderer
			GTest::gtest_main)

			
include(GoogleTest)
gtest_discover_tests(RendererTests)