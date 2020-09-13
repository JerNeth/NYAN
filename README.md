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
- [x] Window management
	- [x] Create a window
	- [x] Get Vulkan Context
- [ ] Vulkan
	- [ ] Rendergraph
	- [x] Pipeline abstraction
	- [x] Renderpass abstraction
	- [ ] Descriptor abstraction
	- [ ] Command Buffer abstraction
	- [ ] Buffer abstraction
	- [ ] Image abstraction
- [ ] Mesh Representations
- [ ] Scene Representation

## Requirements

This engine depends on the following libraries:

- [Vulkan SDK](https://vulkan.lunarg.com/): must be to be manually installed
- [GLFW](https://github.com/glfw/glfw): automatically fetched
- [GTest](https://github.com/google/googletest): automatically fetched
- [Imgui](https://github.com/ocornut/imgui): automatically fetched
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): included in this repository
- [stb](https://github.com/nothings/stb): included in this repository