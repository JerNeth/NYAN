
# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

file(GLOB VULKAN_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/VulkanWrapper/*.cpp
)
file(GLOB CORE_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/Core/*.cpp
)
file(GLOB UTIL_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/Util/*.cpp
)
file(GLOB MATH_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/Math/*.cpp
)

file(GLOB THIRD_PARTY_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/third-party/*.cpp
)

file(GLOB GLFW_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/GLFWWrapper/*.cpp
)
file(GLOB RENDERER_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/Renderer/*.cpp
)

file(GLOB FBXREADER_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/FBXReader/*.cpp
)

file(GLOB GLTFREADER_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/src/GLTFReader/*.cpp
)

set(DEMO_SRC "${PROJECT_SOURCE_DIR}/src/Demo.cpp"
)
set(SWAPTCHAIN_TEST_SRC "${PROJECT_SOURCE_DIR}/src/SwapchainTest.cpp"
)
set(HEADLESS_TEST_SRC "${PROJECT_SOURCE_DIR}/src/HeadlessTest.cpp"
)