add_library(SDLWrapper)

target_sources(SDLWrapper
	PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES FILES
		Library.cppm
		ModuleSettings.cppm
		Wrapper.cppm
)
	
target_compile_features(SDLWrapper PUBLIC cxx_std_23)

target_compile_definitions(SDLWrapper PRIVATE NOMINMAX)

add_library(NYAN::SDLWrapper ALIAS SDLWrapper)


target_link_libraries(SDLWrapper 
	PRIVATE 
		SDL3::SDL3-static
		NYAN::Log
		NYAN::Data
		NYAN::Assert
)