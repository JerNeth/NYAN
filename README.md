# Not Yet Another eNgine

## Description

This is supposed to be a simple lightweight engine based on vulkan

## Features

None, yet.

## Progress

- [x] Linear Algebra
	- [x] Vectors
	- [x] Matrizes
	- [x] Quaternions
- [ ] Utility
	- [x] Hash
	- [x] HashMap
	- [x] LinkedBucketList
	- [x] Pool
- [x] Window management
	- [x] Create a window
	- [x] Get Vulkan Context
- [ ] Renderer
	- [ ] Rendergraph
	- [ ] UI
	- [ ] Lights
	- [ ] Deferred renderer
	- [ ] Forward renderer
	- [ ] Mesh representations
		- [ ] Skeletal mesh
		- [ ] Static mesh
		- [ ] Heightmap
- [ ] Vulkan
	- [x] Pipeline abstraction
	- [x] Renderpass abstraction
	- [x] Descriptor abstraction
	- [x] Swapchain abstraction
	- [x] Framebuffer abstraction
	- [x] Sampler abstraction
	- [x] Descriptor set management 
	- [ ] Command Buffer abstraction
	- [ ] Buffer abstraction
	- [ ] Image abstraction
- [ ] Scene representation

## Requirements

The engine depends on the following libraries:

- [Vulkan SDK](https://vulkan.lunarg.com/): must be manually installed
- [GLFW](https://github.com/glfw/glfw): automatically fetched
- [GTest](https://github.com/google/googletest): automatically fetched
- [Imgui](https://github.com/ocornut/imgui): automatically fetched
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): included in this repository
- [stb](https://github.com/nothings/stb): included in this repository