
# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

file(GLOB_RECURSE VULKAN_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/VulkanWrapper/*.cpp
)

file(GLOB_RECURSE THIRD_PARTY_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/third-party/*.cpp
)

file(GLOB_RECURSE GLFW_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/GLFWWrapper/*.cpp
)
set(DEMO_SRC "${PROJECT_SOURCE_DIR}/src/Demo.cpp"
)
set(DEMO2_SRC "${PROJECT_SOURCE_DIR}/src/Demo_two.cpp"
)