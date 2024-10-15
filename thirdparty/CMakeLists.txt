cmake_minimum_required(VERSION 3.14)

add_subdirectory(EnTT)


add_subdirectory(SDL)

add_subdirectory(Imgui)

add_subdirectory(MagicEnum)

add_subdirectory(NYANUtil)

add_subdirectory(SPIRV-Cross)

add_subdirectory(STB)

add_subdirectory(Volk)

add_subdirectory(VulkanMemoryAllocator)


target_link_libraries(vma_implementation 
        PUBLIC volk::volk)