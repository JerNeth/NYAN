# Not Yet Another eNgine

## Description

This is supposed to be a simple render-engine based on vulkan.

![work-in-progress-screenshot](doc/sample.png)

## Features

Currently, a basic fbx-model-viewer with simple Blin-Phong shading has been implemented in the fbx demo. Furthermore, the foundations for complex render tasks have been sett with the abstractions
and rendergraph.

## Progress

- [ ] Renderer
	- [x] Rendergraph (partial)
	- [ ] UI
	- [ ] Lights
	- [ ] Global Illumination
		- [ ] Diffuse Illumination with Raytraced Irradiance Fields
		- [ ] Specular Reflections
	- [ ] Image Based Lighting
	- [ ] Screen Space Effects
		- [ ] Ambient Occlusion
		- [ ] SSR
		- [ ] SSSSS
	- [ ] Post Processing
		- [ ] Tone mapping
		- [ ] Alpha correction
		- [ ] White balancing
		- [ ] Bloom
	- [ ] Deferred renderer
	- [ ] Forward renderer
	- [ ] Mesh representations
		- [ ] Skeletal mesh
		- [ ] Morph targets/Blend shapes
		- [x] Static mesh
		- [ ] Heightmap
- [x] Vulkan 1.3
	- [x] Dynamic Rendering
	- [x] Bindless
	- [x] Sparse Images
	- [x] Basic Graphics Pipelines
	- [x] Compute Pipelines
	- [x] Raytrace Pipelines
	- [ ] Task/Mesh Shaders
- [ ] Scene representation
- [ ] I/O

## Dependencies

The engine depends on the following libraries:

- [Vulkan SDK](https://vulkan.lunarg.com/): must be manually installed (1.3 required)
- [FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2): must be manually installed (for now, in future optional) (Expects FBXSDK_DIR to be set in UserSettings.txt)
- [GLFW](https://github.com/glfw/glfw): automatically fetched
- [GTest](https://github.com/google/googletest): automatically fetched
- [Imgui](https://github.com/ocornut/imgui): automatically fetched
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross): automatically fetched
- [volk](https://github.com/zeux/volk): automatically fetched
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): automatically fetched
- [EnTT](https://github.com/skypjack/entt.git): automatically fetched
- [Optick](https://github.com/bombomby/optick): included in this repository
- [tinyGLTF](https://github.com/syoyo/tinygltf): included in this repository