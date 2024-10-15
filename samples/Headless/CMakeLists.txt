add_subdirectory(shader EXCLUDE_FROM_ALL)

add_executable(Headless Headless.cpp)


target_link_libraries(Headless NYAN::Vulkan sample_shaders)