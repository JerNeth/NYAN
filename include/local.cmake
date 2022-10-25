
# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

include_directories(
    ${CMAKE_SOURCE_DIR}/include/Math
    ${CMAKE_SOURCE_DIR}/include/Geometry
    ${CMAKE_SOURCE_DIR}/include/Core
    ${CMAKE_SOURCE_DIR}/include/GLFWWrapper
    ${CMAKE_SOURCE_DIR}/include/VulkanWrapper
    ${CMAKE_SOURCE_DIR}/include
)
include_directories(SYSTEM
	${CMAKE_SOURCE_DIR}/include/third-party
)



set(GLOB MATH_H_DIR ${CMAKE_CURRENT_LIST_DIR}/include/Math)
set(GLOB CORE_H_DIR ${CMAKE_CURRENT_LIST_DIR}/include/Core)
set(GLOB GLFWWRAPPER_H_DIR ${CMAKE_CURRENT_LIST_DIR}/include/GLFWWrapper)
set(GLOB FBXREADER_H_DIR ${CMAKE_CURRENT_LIST_DIR}/include/FBXReader)
set(GLOB VULKANWRAPPER_H_DIR ${CMAKE_CURRENT_LIST_DIR}/include/VulkanWrapper)
set(GLOB RENDERER_H_DIR ${CMAKE_CURRENT_LIST_DIR}/include/Renderer)
set(GLOB UTILITY_H_DIR ${CMAKE_CURRENT_LIST_DIR}/include/Utility)
set(GLOB GLTFREADER_H_DIR ${CMAKE_CURRENT_LIST_DIR}/include/GLTFReader)
set(GLOB INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR})

file(GLOB MATH_H CONFIGURE_DEPENDS
    ${MATH_H_DIR}/*.h
    ${MATH_H_DIR}/*.hpp
    )
file(GLOB CORE_H CONFIGURE_DEPENDS
    ${CORE_H_DIR}/*.h
    ${CORE_H_DIR}/*.hpp
    )
file(GLOB GLFWWRAPPER_H CONFIGURE_DEPENDS
    ${GLFWWRAPPER_H_DIR}/*.h
    ${GLFWWRAPPER_H_DIR}/*.hpp
    )
file(GLOB VULKANWRAPPER_H CONFIGURE_DEPENDS
    ${VULKANWRAPPER_H_DIR}/*.h
    ${VULKANWRAPPER_H_DIR}/*.hpp
    )
file(GLOB RENDERER_H CONFIGURE_DEPENDS
    ${RENDERER_H_DIR}/*.h
    ${RENDERER_H_DIR}/*.hpp
    )
file(GLOB UTILITY_H CONFIGURE_DEPENDS
    ${UTILITY_H_DIR}/*.h
    ${UTILITY_H_DIR}/*.hpp
    )
file(GLOB GLTFREADER_H CONFIGURE_DEPENDS
    ${GLTFREADER_H_DIR}/*.h
    ${GLTFREADER_H_DIR}/*.hpp
    )
file(GLOB FBXREADER_H CONFIGURE_DEPENDS 
	${FBXREADER_H_DIR}/*.h
	${FBXREADER_H_DIR}/*.hpp
)