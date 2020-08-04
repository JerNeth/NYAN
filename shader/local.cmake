 cmake_minimum_required(VERSION 3.14)

function(add_spirv_shader SHADER_STAGE INPUT_FILE OUTPUT_FILE)
    add_custom_command(
            OUTPUT ${OUTPUT_FILE} 
            COMMAND "glslc" "${INPUT_FILE}" "-o${OUTPUT_FILE}" #glslc is on the system path on my computer, so I am not currently worried about `find_package`ing it.
            MAIN_DEPENDENCY ${INPUT_FILE}
    ) 
endfunction()

add_spirv_shader(vert ${CMAKE_SOURCE_DIR}/shader/basic.vert "basic_vert.spv")
add_spirv_shader(frag ${CMAKE_SOURCE_DIR}/shader/basic.frag "basic_frag.spv")


set(
    SHADERS
    basic_vert.spv
    basic_frag.spv
)

add_custom_target(shaders DEPENDS basic_vert.spv basic_frag.spv)