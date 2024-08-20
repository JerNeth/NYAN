add_library(VulkanRenderer)

add_subdirectory(shaders)

target_sources(VulkanRenderer
	PRIVATE
		"Renderer.cpp"
		"Settings.cpp"
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		"Error.ixx"
		"Renderer.ixx" 
		"Settings.ixx"
		"VulkanRenderer.ixx"
)
	
target_compile_features(VulkanRenderer PUBLIC cxx_std_23)

add_library(NYAN::VulkanRenderer ALIAS VulkanRenderer)

target_link_libraries(VulkanRenderer 
	PRIVATE 
		NYAN::VulkanWrapper
)

target_link_libraries(VulkanRenderer 
	INTERFACE 
		renderer_shaders
)
#TODO: They currently land in this mirrored output dir (i.e. not relative to the linking executable, which is not intended)

target_link_libraries(VulkanRenderer 
	PRIVATE 
		NYAN::Log
)