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
	- [x] Lights
	- [ ] Global Illumination
		- [x] Diffuse Illumination with Raytraced Irradiance Fields
		- [ ] Specular Reflections
	- [ ] Post Processing
		- [x] Tone mapping
		- [x] Alpha correction
		- [ ] White balancing
		- [ ] Bloom
	- [x] Deferred renderer
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
- [GLFW](https://github.com/glfw/glfw): automatically fetched
- [GTest](https://github.com/google/googletest): automatically fetched
- [Imgui](https://github.com/ocornut/imgui): automatically fetched
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross): automatically fetched
- [volk](https://github.com/zeux/volk): automatically fetched
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): automatically fetched
- [EnTT](https://github.com/skypjack/entt.git): automatically fetched
- [tinyGLTF](https://github.com/syoyo/tinygltf): included in this repository