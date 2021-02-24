#version 450
layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D texSamplerNormal;
//layout(set = 1, binding = 0) uniform samplerCube cubeSampler;
layout(std430, push_constant) uniform PushConstants
{
    mat4 model;
} constants;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;


layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

void main() {
    vec3 lightDir = vec3(1, 1, 1);
    vec3 normal = texture(texSamplerNormal, fragTexCoord).rgb * 2.0 - 1.0;
    vec3 diffuseColor = texture(texSampler, fragTexCoord).rgb;
    //diffuseColor = vec3(1,1,1);
    float diffuse = dot(normal, lightDir);
    outColor = vec4(diffuse * diffuseColor, 1.0);
    //outNormal = vec4((fragWorld*normal)*0.5 +0.5, 1);
    outNormal = vec4(mat3(constants.model)*normal, 1);
    //outColor = vec4(fragTexCoord.xxx, 1);
    //outColor = vec4(fragColor.xyz*fragTexCoord.xxx, 1);
    outColor = vec4(diffuseColor, 1);
    //outColor = fragColor;
}