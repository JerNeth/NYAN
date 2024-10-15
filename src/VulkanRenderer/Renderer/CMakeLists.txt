add_library(Renderer)

add_subdirectory(shaders)

target_sources(Renderer
	PRIVATE
		Graph.cpp
		PipelineBuilder.cpp
		RenderView.cpp
		Resources.cpp
		WSI.cpp
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		Error.cppm
		Graph.cppm
		ModuleSettings.cppm
		PipelineBuilder.cppm
		Renderer.cppm
		RenderView.cppm
		Resources.cppm
		Settings.cppm
		WSI.cppm
)
	
target_compile_features(Renderer PUBLIC cxx_std_23)

add_library(NYAN::Renderer ALIAS Renderer)

target_link_libraries(Renderer 
	PRIVATE 
		NYAN::Assert
		NYAN::GLFWWrapper
		NYAN::Vulkan
		NYAN::Log
		NYAN::Data
)

target_link_libraries(Renderer 
	PUBLIC 
		magic_enum::magic_enum
)

#TODO: They currently land in this mirrored output dir (i.e. not relative to the linking executable, which is not intended) => link as interface
target_link_libraries(Renderer 
	INTERFACE 
		renderer_shaders
)