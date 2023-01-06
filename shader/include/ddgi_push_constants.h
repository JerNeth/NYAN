#ifndef DDGI_PUSH_CONSTANTS_GLSL
#define DDGI_PUSH_CONSTANTS_GLSL

struct DDGIPushConstants {
	uint accBinding;
	uint sceneBinding;
	uint meshBinding;
	uint ddgiBinding;
	uint ddgiCount;
	uint ddgiIndex;
	uint renderTarget;
	uint resampleTarget;
	vec4 randomRotation;
};
#ifndef __cplusplus
	layout(std430, push_constant) uniform PushConstants
	{
		DDGIPushConstants constants;
	} pushConstants;
#endif

#endif