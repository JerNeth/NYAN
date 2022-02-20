#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputDepth;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput inputNormal;
//layout(set = 0,binding = 3) uniform sampler2D texSampler;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = subpassLoad(inputColor);
    outColor = subpassLoad(inputNormal);

    //float depth = pow(subpassLoad(inputDepth).x, 100);
    //outColor = vec4(depth.xxx , 1.0);
    //outColor = vec4(texture(texSampler, inTexCoord).xxx, 1.0);
    //outColor = vec4(0.5, 0.2, 0.8, 1.0);
}