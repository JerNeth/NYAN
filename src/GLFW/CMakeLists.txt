add_library(GLFWWrapper)

target_sources(GLFWWrapper
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		Allocator.cppm
		Error.cppm
		ModuleSettings.cppm
		Library.cppm
		Monitor.cppm
		Window.cppm
		Wrapper.cppm
)
	
target_compile_features(GLFWWrapper PUBLIC cxx_std_23)

target_compile_definitions(GLFWWrapper PRIVATE NOMINMAX)

add_library(NYAN::GLFWWrapper ALIAS GLFWWrapper)


target_link_libraries(GLFWWrapper 
	PRIVATE 
		glfw
		NYAN::Log
		NYAN::Data
		NYAN::Assert
)