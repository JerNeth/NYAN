add_subdirectory(shader EXCLUDE_FROM_ALL)

add_executable(RendererUsage RendererUsage.cpp)

#set_target_properties(RendererUsage PROPERTIES 
#    LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
#target_link_options(RendererUsage PUBLIC "-mwindows")
#target_compile_definitions(RendererUsage PRIVATE EWINMAIN=1)
#target_link_options(RendererUsage PUBLIC "-Wl,--subsystem,windows")
#set_target_properties(RendererUsage PROPERTIES LINK_FLAGS "-mwindows")

target_link_libraries(RendererUsage 
	PRIVATE 
		NYAN::Assert
		NYAN::Vulkan
		NYAN::Renderer 
		NYAN::Log
		NYAN::Data
		NYAN::SDLWrapper
		sample_renderer_shaders
)