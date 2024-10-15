# Not Yet Another eNgine

🚜👷🚧🏗️

Massive refactor underway.

Refactor Goal:
- No Exceptions (via std::expected)
- Async + coroutines
- C++20 module based

## Description

TODO

## Getting started

The engine is primarly intended to be included via cmake fetch content.
However, manual inclusion via ExternalProject_Add should also be possible.

```
FetchContent_Declare(
	nyan
	GIT_REPOSITORY https://github.com/JerNeth/NYAN
	GIT_TAG        master
	GIT_SHALLOW    true
)
FetchContent_MakeAvailable(nyan)

target_link_libraries(<your executable here> PRIVATE nyan)
```

To use nyan a simple import is enough

```
import NYAN;
```

For more detailed instructions and how to run the samples, etc. head over to [getting started](./doc/getting_started.md).

## Dependencies

The following third party dependencies are necessary:

### Manual

- [Vulkan SDK](https://vulkan.lunarg.com/): must be manually installed (1.3 required)

### Managed
The following dependencies are managed and either included or automatically fetched via [CPM](https://github.com/cpm-cmake/CPM.cmake).

- [SDL](https://github.com/libsdl-org/SDL): automatically fetched
- [GTest](https://github.com/google/googletest): automatically fetched (Testing only)
- [Imgui](https://github.com/ocornut/imgui): automatically fetched
- [MagicEnum](https://github.com/Neargye/magic_enum): automatically fetched
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross): automatically fetched
- [volk](https://github.com/zeux/volk): automatically fetched
- [STB](https://github.com/nothings/stb): automatically fetched
- [EnTT](https://github.com/skypjack/entt.git): automatically fetched
- [tinyGLTF](https://github.com/syoyo/tinygltf): included in this repository
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): automatically fetched
- [RTM](https://github.com/nfrechette/rtm/tree/develop): TODO
- [ACL](https://github.com/nfrechette/acl): TODO
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo): TODO
- [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog): TODO