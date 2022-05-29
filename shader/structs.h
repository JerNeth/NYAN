struct Mesh {
	uint materialBinding;
	uint materialId;
	Indices indicesAddress;
	Positions positionsAddress;
	Uvs uvsAddress;
	Normals normalsAddress;
	Tangents tangentsAddress;
};

struct Material {
	uint albedoTexId;
	uint albedoSampler;
	uint normalTexId;
	uint normalSampler;
	uint pbrTexId;
	uint pbrSampler;
	float metalness;
	float roughness;
	float anisotropy;
	float IoR;
};

struct DirectionalLight {
	vec3 dir;
	float intensity;
	vec3 color;
	bool enabled;
};

struct PointLight {
	vec3 pos;
	float intensity;
	vec3 color;
	float attenuationDistance;
};
struct Scene {
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 invView;
	mat4 invProj;
	mat4 invViewProj;
	DirectionalLight dirLight1;
	DirectionalLight dirLight2;
	uint numLights;
	float viewerPosX;
	float viewerPosY;
	float viewerPosZ;
	PointLight pointLights[256];
};

struct Instance {
	vec4 modelRow1;
	vec4 modelRow2;
	vec4 modelRow3;
	uint meshId;
	uint pad1;
	uint pad2;
	uint pad3;
};
