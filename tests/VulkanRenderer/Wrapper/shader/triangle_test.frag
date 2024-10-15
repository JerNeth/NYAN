#version 450

layout(location = 0) out vec4 outColor;

void main() {
    //outColor = texture(texSampler, fragTexCoord);
    outColor = vec4(0.0, 0.0, 1.0, 1.0);
}