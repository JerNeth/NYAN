 cmake_minimum_required(VERSION 3.14)

function(add_spirv_shader INPUT_FILE OUTPUT_FILE)
    add_custom_command(
            OUTPUT ${OUTPUT_FILE} 
            COMMAND "glslc" "--target-env=vulkan1.3" "${INPUT_FILE}" "-o${OUTPUT_FILE}" #glslc is on the system path on my computer, so I am not currently worried about `find_package`ing it.
            MAIN_DEPENDENCY ${INPUT_FILE}
    ) 
endfunction()


file(GLOB_RECURSE SHADER_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/shader/*.vert
    ${PROJECT_SOURCE_DIR}/shader/*.frag
    ${PROJECT_SOURCE_DIR}/shader/*.comp
    ${PROJECT_SOURCE_DIR}/shader/*.rchit
    ${PROJECT_SOURCE_DIR}/shader/*.rgen
    ${PROJECT_SOURCE_DIR}/shader/*.rmiss
    ${PROJECT_SOURCE_DIR}/shader/*.rahit
    ${PROJECT_SOURCE_DIR}/shader/*.rint
    ${PROJECT_SOURCE_DIR}/shader/*.rcall
    ${PROJECT_SOURCE_DIR}/shader/*.mesh
    ${PROJECT_SOURCE_DIR}/shader/*.task
    ${PROJECT_SOURCE_DIR}/shader/*.geom
    ${PROJECT_SOURCE_DIR}/shader/*.tese
    ${PROJECT_SOURCE_DIR}/shader/*.tesc
)
set(SHADERS "")
foreach(shader ${SHADER_SRC})
    get_filename_component(shader_name ${shader} NAME_WE)
    get_filename_component(shader_type ${shader} EXT)
    string(REPLACE "." "" shader_type ${shader_type})
    set(shaderoutput "${PROJECT_BINARY_DIR}/${shader_name}_${shader_type}.spv")
    add_spirv_shader(${shader} "${shaderoutput}")
    list(APPEND SHADERS "${shaderoutput}")
endforeach()


add_custom_target(shaders DEPENDS ${SHADERS})