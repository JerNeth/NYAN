
const int SSBO_COUNT = 8 * 1024;
const int SAMPLER_COUNT = 256;
const int TEXTURE_COUNT = 512 * 1024;
const int IMAGE_COUNT = 8 * 1024;
const int ACC_COUNT = 256;
layout(set = 0, binding = 0, std430) buffer MeshData {
	Mesh meshes[];
} meshData[SSBO_COUNT];

layout(set = 0, binding = 0, std430) buffer Transforms {
	Transform transforms[];
} transforms[SSBO_COUNT];

layout(set = 0, binding = 0, std430) buffer Scenes {
	Scene scene;
} scenes[SSBO_COUNT];

layout(set = 0, binding = 0, std430) buffer Materials  {
	Material materials[];
} materials [SSBO_COUNT];

layout(set = 0, binding = 2) uniform sampler samplers[SAMPLER_COUNT];
layout(set = 0, binding = 3) uniform texture2D textures2D[TEXTURE_COUNT];
layout(set = 0, binding = 4, rgb10_a2) uniform image2D imagesRGB10_A2[IMAGE_COUNT];

//layout(set = 0, binding = 0) buffer SSBO {} ssbos [];
//layout(set = 0, binding = 1) uniform ubos[];
//layout(set = 0, binding = 2) uniform sampler samplers[256];
//layout(set = 0, binding = 3) uniform texture2D textures[512 * 1024];
//layout(set = 0, binding = 4) uniform image2D images[];
//layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[];
