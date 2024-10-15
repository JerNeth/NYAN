#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = texture(texSampler, fragTexCoord);
    //debugPrintfEXT("F");
    outColor = vec4(0.0, 1.0, 1.0, 1.0);
}