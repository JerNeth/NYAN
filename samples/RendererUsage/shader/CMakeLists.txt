 cmake_minimum_required(VERSION 3.20)
 
file(GLOB_RECURSE SHADER_H CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_LIST_DIR}/include/*.h
)
file(GLOB_RECURSE SHADER_GLSL CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_LIST_DIR}/*.glsl
)

list(FILTER SHADER_H EXCLUDE REGEX ".*\\tmp\\.*")
list(FILTER SHADER_GLSL EXCLUDE REGEX ".*\\tmp\\.*")

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

function(add_spirv_shader INPUT_FILE OUTPUT_FILE)

    #recursivly parse .h and .glsl dependencies found in shader, fairly broad check, 
    #i.e. changed, commented out files would trigger recompile, but since that only costs a bit of recompile it is acceptable
    set(DEPENDENCIES "")
    set(DEPENDENCIESREC "")
    list(APPEND DEPENDENCIESREC "${INPUT_FILE}")
    while(DEPENDENCIESREC)
        list(POP_FRONT DEPENDENCIESREC HEAD)
        file(READ ${HEAD} TMPTXT)
        foreach (header ${SHADER_H})
            get_filename_component(header_name ${header} NAME_WE)
            string(FIND "${TMPTXT}" "${header_name}" matchres)
            if((NOT ${matchres} EQUAL -1) AND (NOT "${header}" IN_LIST DEPENDENCIES))
                list(APPEND DEPENDENCIES "${header}")
                list(APPEND DEPENDENCIESREC "${header}")
            endif()
        endforeach()
        foreach (header ${SHADER_GLSL})
            get_filename_component(header_name ${header} NAME_WE)
            string(FIND "${TMPTXT}" "${header_name}" matchres)
            if((NOT ${matchres} EQUAL -1) AND (NOT "${header}" IN_LIST DEPENDENCIES))
                list(APPEND DEPENDENCIES "${header}")
                list(APPEND DEPENDENCIESREC "${header}")
            endif()
        endforeach()
    endwhile()

    add_custom_command(
            OUTPUT ${OUTPUT_FILE} 
            COMMAND ${glslc_executable} "--target-env=vulkan1.3" "${INPUT_FILE}" "-o${OUTPUT_FILE}" "-I${CMAKE_CURRENT_LIST_DIR}/include" "-I${CMAKE_CURRENT_LIST_DIR}/common"
            MAIN_DEPENDENCY ${INPUT_FILE}
            DEPENDS ${DEPENDENCIES}
    ) 
endfunction()


file(GLOB_RECURSE SHADER_SRC CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_LIST_DIR}/*.vert
    ${CMAKE_CURRENT_LIST_DIR}/*.frag
    ${CMAKE_CURRENT_LIST_DIR}/*.comp
    ${CMAKE_CURRENT_LIST_DIR}/*.rchit
    ${CMAKE_CURRENT_LIST_DIR}/*.rgen
    ${CMAKE_CURRENT_LIST_DIR}/*.rmiss
    ${CMAKE_CURRENT_LIST_DIR}/*.rahit
    ${CMAKE_CURRENT_LIST_DIR}/*.rint
    ${CMAKE_CURRENT_LIST_DIR}/*.rcall
    ${CMAKE_CURRENT_LIST_DIR}/*.mesh
    ${CMAKE_CURRENT_LIST_DIR}/*.task
    ${CMAKE_CURRENT_LIST_DIR}/*.geom
    ${CMAKE_CURRENT_LIST_DIR}/*.tese
    ${CMAKE_CURRENT_LIST_DIR}/*.tesc
)
list(FILTER SHADER_SRC EXCLUDE REGEX ".*\\tmp\\.*")

set(SHADERS "")
foreach(shader ${SHADER_SRC})
    get_filename_component(shader_name ${shader} NAME_WE)
    get_filename_component(shader_type ${shader} EXT)
    string(REPLACE "." "" shader_type ${shader_type})
    set(shaderoutput "${CMAKE_CURRENT_BINARY_DIR}/${shader_name}_${shader_type}.spv")
    add_spirv_shader(${shader} "${shaderoutput}")
    list(APPEND SHADERS "${shaderoutput}")
endforeach()


add_custom_target(sample_local_renderer_shaders DEPENDS ${SHADERS})

add_library(sample_renderer_shaders INTERFACE)
target_include_directories(sample_renderer_shaders INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
target_sources(sample_renderer_shaders INTERFACE ${SHADER_H})

add_dependencies(sample_renderer_shaders sample_local_renderer_shaders)