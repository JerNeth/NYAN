#pragma once

namespace Utility {
	template<typename T, typename Container>
	class ObjectHandle;
	template<typename T, size_t bucketSize = 16>
	class LinkedBucketList;
	template<typename T>
	class Pool;
}
namespace vulkan {
	class AccelerationStructure;
	class AccelerationStructureBuilder;
	class LogicalDevice;
	class Allocator;
	class Allocation;
	template<typename T>
	class MappedMemoryHandle;
	class AttachmentAllocator;
	struct BufferInfo;
	class Buffer;
	class CommandBuffer;
	class CommandPool;

	enum class CommandBufferType {
		Generic,
		Compute,
		Transfer
	};
	struct DescriptorCreateInfo;
	class DescriptorPool;
	class DescriptorSet;
	class Image;
	class ImageView;
	struct ImageInfo;
	struct ImageViewCreateInfo;
	struct MipInfo;
	class Instance;
	class PhysicalDevice;
	class LogicalDevice;
	class Surface;
	struct Extensions;
	class DeviceWrapper;
	struct InputData;
	enum class SwapchainRenderpassType {
		Color,
		Depth
	};
	struct InitialImageData;
	class FenceHandle;
	class FenceManager;
	class SemaphoreManager;
	struct BlendAttachment;
	struct DynamicGraphicsPipelineState;
	struct GraphicsPipelineState;
	struct RenderingCreateInfo;
	struct GraphicsPipelineConfig;
	struct ComputePipelineConfig;
	struct Group;
	struct RaytracingPipelineConfig;

	class PipelineStorage2;
	class PipelineLayout2;
	class PipelineCache;
	class Pipeline2;
	class PipelineBind;
	class GraphicsPipelineBind;
	class ComputePipelineBind;
	class RaytracingPipelineBind;
	struct ResourceBinding;

	class Shader;
	class ShaderInstance;
	class ShaderStorage;
	class Sampler;

	using ShaderId = uint32_t;
	constexpr ShaderId invalidShaderId = ~ShaderId{ 0 };
	class WindowSystemInterface;

	enum class DefaultSampler : uint32_t {
		NearestClamp,
		LinearClamp,
		TrilinearClamp,
		NearestWrap,
		LinearWrap,
		TrilinearWrap,
		NearestShadow,
		LinearShadow,
		Size
	};

	using AccelerationStructureHandle = Utility::ObjectHandle<AccelerationStructure, Utility::LinkedBucketList<AccelerationStructure>>;
	using AllocationHandle = Utility::ObjectHandle<Allocation, Utility::Pool<Allocation>>;
	using BufferHandle = Utility::ObjectHandle<Buffer, Utility::LinkedBucketList<Buffer>>;
	using CommandBufferHandle = Utility::ObjectHandle<CommandBuffer, Utility::Pool<CommandBuffer>>;
	using ImageHandle = Utility::ObjectHandle<Image, Utility::LinkedBucketList<Image>>;
	using ImageViewHandle = Utility::ObjectHandle<ImageView, Utility::LinkedBucketList<ImageView>>;
	using PipelineId = uint32_t;
}