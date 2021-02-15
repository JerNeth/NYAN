
# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

include_directories(
    ${CMAKE_SOURCE_DIR}/include/Math
    ${CMAKE_SOURCE_DIR}/include/Utility
    ${CMAKE_SOURCE_DIR}/include/Geometry
    ${CMAKE_SOURCE_DIR}/include/Core
    ${CMAKE_SOURCE_DIR}/include/GLFWWrapper
    ${CMAKE_SOURCE_DIR}/include/VulkanWrapper
    ${CMAKE_SOURCE_DIR}/include
)
include_directories(SYSTEM
	${CMAKE_SOURCE_DIR}/include/third-party
)