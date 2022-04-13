#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (location = 0) in vec2 outUV;

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = subpassLoad(inputColor);
    outColor = vec4(0.2, 0.2, 0.8, 1.0);
}