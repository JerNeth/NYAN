 cmake_minimum_required(VERSION 3.14)

function(add_spirv_shader INPUT_FILE OUTPUT_FILE)
    add_custom_command(
            OUTPUT ${OUTPUT_FILE} 
            COMMAND "glslc" "${INPUT_FILE}" "-o${OUTPUT_FILE}" #glslc is on the system path on my computer, so I am not currently worried about `find_package`ing it.
            MAIN_DEPENDENCY ${INPUT_FILE}
    ) 
endfunction()


file(GLOB_RECURSE SHADER_SRC CONFIGURE_DEPENDS 
	${PROJECT_SOURCE_DIR}/shader/*.vert
    ${PROJECT_SOURCE_DIR}/shader/*.frag
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