#ifndef VKLOGICALDEVICE_H
#define VKLOGICALDEVICE_H
#pragma once
#include "VulkanIncludes.h"
#include "Framebuffer.h"
#include "Utility.h"
#include "LinAlg.h"
#include "Shader.h"
#include "Instance.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "Sampler.h"
#include "Swapchain.h"
#include "Allocator.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Manager.h"
#include "Buffer.h"
namespace vulkan {
	class LogicalDevice;
	class Instance;
	//Important to delete the device after everything else
	class DeviceWrapper {
	public:
		DeviceWrapper(VkDevice device, const VkAllocationCallbacks* allocator) :
			m_vkHandle(device), 
			m_allocator(allocator)
		{
		}
		~DeviceWrapper() {
			if (m_vkHandle != VK_NULL_HANDLE) {
				vkDestroyDevice(m_vkHandle, m_allocator);
				m_vkHandle = VK_NULL_HANDLE;
			}
		}
		DeviceWrapper(DeviceWrapper&) = delete;
		DeviceWrapper(DeviceWrapper&& other) noexcept:
			m_vkHandle(other.m_vkHandle),
			m_allocator(other.m_allocator)
		{
			other.m_vkHandle = VK_NULL_HANDLE;
		};
		DeviceWrapper& operator=(DeviceWrapper&) = delete;
		DeviceWrapper& operator=(DeviceWrapper&& other) noexcept {
			m_vkHandle = other.m_vkHandle;
			m_allocator= other.m_allocator;
			other.m_vkHandle = VK_NULL_HANDLE;
			return *this;
		};
		operator VkDevice() const { return m_vkHandle; }
	private:
		VkDevice m_vkHandle = VK_NULL_HANDLE;
		const VkAllocationCallbacks* m_allocator;
	};
	
	struct Vertex {
		std::array<float, 3> pos;
		std::array<float, 3> color;
		std::array<float, 2> texcoords;
	};
	
	constexpr std::array<Vertex, 8> vertices{
		Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		Vertex{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		/*
		Vertex{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		Vertex{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		Vertex{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
		*/
	};
	constexpr std::array<uint16_t, 6> indices = {
		0, 1, 2, 2, 3, 0,
		//4, 5, 6, 6, 7, 4
	};
	struct alignas(256) Ubo   {
		Math::mat44 model;
		Math::mat44 view;
		Math::mat44 proj;
	};
	constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
	enum class SwapchainRenderpassType {
		Color,
		Depth
	};
	struct InitialImageData {
		const void* data = nullptr;
		std::array<uint32_t, 16> mipOffsets;
		uint32_t mipCounts;
		uint32_t rowLength = 0;
		uint32_t height = 0;
	};
	class LogicalDevice {
		
		struct Extensions {
			unsigned descriptor_update_template : 1;
			unsigned swapchain : 1;
			unsigned timeline_semaphore : 1;
			unsigned fullscreen_exclusive : 1;
			unsigned extended_dynamic_state : 1;
			unsigned debug_utils : 1;
			unsigned debug_marker : 1;
		};
		struct WSIState {
			VkSemaphore aquire = VK_NULL_HANDLE;
			VkSemaphore present = VK_NULL_HANDLE;
			std::vector<ImageHandle> swapchainImages;
			bool swapchain_touched = false;
			uint32_t index = 0;
		};
		struct Queue {
			Queue() = default;
			Queue(uint32_t family) : familyIndex(family) {}
			std::vector<VkSemaphore> waitSemaphores;
			std::vector<VkPipelineStageFlags> waitStages;
			bool needsFence = false;
			bool supportsSparse = false;

			const uint32_t familyIndex = 0;
			VkQueue queue = VK_NULL_HANDLE;
		};
		struct ImageBuffer {
			BufferHandle buffer;
			std::vector<VkBufferImageCopy> blits;
		};
		struct FrameResource {
			FrameResource(LogicalDevice& device) : r_device(device){
				/*commandPool.reserve(device.get_thread_count());
				for (uint32_t i = 0; i < device.get_thread_count(); i++) {
					commandPool.emplace_back(device, device.m_graphicsFamilyQueueIndex);
				}*/
				graphicsPool.reserve(device.get_thread_count());
				for (uint32_t i = 0; i < device.get_thread_count(); i++) {
					graphicsPool.emplace_back(device, device.m_graphics.familyIndex);
				}
				computePool.reserve(device.get_thread_count());
				for (uint32_t i = 0; i < device.get_thread_count(); i++) {
					computePool.emplace_back(device, device.m_compute.familyIndex);
				}
				transferPool.reserve(device.get_thread_count());
				for (uint32_t i = 0; i < device.get_thread_count(); i++) {
					transferPool.emplace_back(device, device.m_transfer.familyIndex);
				}
			}
			LogicalDevice& r_device;
			uint32_t frameIndex = 0;

			~FrameResource();

			std::vector<CommandPool> graphicsPool;
			std::vector<CommandPool> computePool;
			std::vector<CommandPool> transferPool;
			std::vector<CommandBufferHandle> submittedGraphicsCmds;
			std::vector<CommandBufferHandle> submittedComputeCmds;
			std::vector<CommandBufferHandle> submittedTransferCmds;


			std::vector<VkBufferView> deletedBufferViews;
			std::vector<VkImageView> deletedImageViews;
			std::vector<VkSampler> deletedSampler;
			std::vector<VkImage> deletedImages;
			std::vector<VkBuffer> deletedBuffer;
			std::vector<VmaAllocation> deletedAllocations;
			std::vector<VkFramebuffer> deletedFramebuffer;
			std::vector<VkSemaphore> deletedSemaphores;
			std::vector<VkSemaphore> recycledSemaphores;

			std::vector<FenceHandle> waitForFences;

			//Sparse
			std::vector<VkSparseImageMemoryBind> sparseMemoryBinds;
			std::vector<VkSparseImageMemoryBindInfo> submittedSparseBinds;
			std::unordered_multimap< uint32_t, std::function<void(void)>> sparseFenceCallbacks;
			std::vector<VkSemaphore> sparseSemaphores;
			std::vector<FenceHandle> sparseFences;
			void begin();
		};
		friend class Swapchain;
		friend class DeviceWrapper;
		friend class Sampler;
		friend class Renderpass;
		friend class PipelineLayout;
		friend class Shader;
		friend class Pipeline;
		friend class Renderpass;
		friend class DescriptorSetAllocator;
		
	public:

		LogicalDevice(const Instance& parentInstance, VkDevice device, uint32_t graphicsQueueFamilyIndex , uint32_t computeFamilyQueueIndex, uint32_t transferFamilyQueueIndex, VkPhysicalDevice physicalDevice);
		~LogicalDevice();
		LogicalDevice(LogicalDevice&) = delete;
		LogicalDevice& operator=(LogicalDevice&) = delete;
		LogicalDevice(LogicalDevice&&) = delete;
		LogicalDevice& operator=(LogicalDevice&&) = delete;
		void wait_idle();


		BufferHandle create_buffer(const BufferInfo& info, const void * initialData = nullptr);
		ImageViewHandle create_image_view(const ImageViewCreateInfo& info);
		ImageHandle create_image(const ImageInfo& info, InitialImageData* initialData = nullptr);
		ImageHandle create_sparse_image(const ImageInfo& info, InitialImageData* initialData = nullptr);
		void downsize_sparse_image(Image& handle, uint32_t targetMipLevel);
		bool upsize_sparse_image(Image& handle, InitialImageData* initialData, uint32_t targetMipLevel);

		DescriptorSetAllocator* request_descriptor_set_allocator(const DescriptorSetLayout& layout);
		size_t register_shader(const std::vector<uint32_t>& shaderCode);
		//Shader* request_shader(const std::string& shaderName, const std::vector<uint32_t>& shaderCode);
		//Shader* request_shader(const std::string& shaderName) noexcept;
		Shader* request_shader(size_t id);
		Program* request_program(const std::vector<Shader*>& shaders);
		PipelineLayout* request_pipeline_layout(const ShaderLayout& layout);
		Renderpass* request_render_pass(const RenderpassCreateInfo& info);
		Renderpass* request_compatible_render_pass(const RenderpassCreateInfo& info);
		VkPipeline request_pipeline(const PipelineCompile& compile) noexcept;
		RenderpassCreateInfo request_swapchain_render_pass(SwapchainRenderpassType type) noexcept;
		Framebuffer* request_framebuffer(const RenderpassCreateInfo& info);
		VkSemaphore request_semaphore();
		CommandBufferHandle request_command_buffer(CommandBuffer::Type type);
		ImageView* request_render_target(uint32_t width, uint32_t height, VkFormat format, uint32_t index = 0, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
		void resize_buffer(Buffer& buffer, VkDeviceSize newSize, bool copyData = false);
		
		VkSemaphore get_present_semaphore();
		bool swapchain_touched() const noexcept;
		VkQueue get_graphics_queue()  const noexcept;
		void set_acquire_semaphore(uint32_t index, VkSemaphore semaphore) noexcept;

		void init_swapchain(const std::vector<VkImage>& swapchainImages, uint32_t width, uint32_t height, VkFormat format);
		const ImageView* get_swapchain_image_view() const noexcept;
		ImageView* get_swapchain_image_view() noexcept;

		void next_frame();
		void end_frame();
		void submit_queue(CommandBuffer::Type type, FenceHandle* fence, uint32_t semaphoreCount = 0, VkSemaphore* semaphores = nullptr);
		void queue_framebuffer_deletion(VkFramebuffer framebuffer) noexcept;
		void queue_image_deletion(VkImage image) noexcept;
		void queue_image_view_deletion(VkImageView imageView) noexcept;
		void queue_buffer_view_deletion(VkBufferView bufferView) noexcept;
		void queue_image_sampler_deletion(VkSampler sampler) noexcept;
		void queue_descriptor_pool_deletion(VkDescriptorPool descriptorPool) noexcept;
		void queue_buffer_deletion(VkBuffer buffer) noexcept;
		void queue_allocation_deletion(VmaAllocation allocation) noexcept;
		void add_wait_semaphore(CommandBuffer::Type type, VkSemaphore semaphore, VkPipelineStageFlags stages, bool flush = false);
		void submit_staging(CommandBufferHandle cmd, VkBufferUsageFlags usage, bool flush);
		void submit(CommandBufferHandle cmd, uint32_t semaphoreCount = 0, VkSemaphore *semaphores = nullptr, vulkan::FenceHandle* fence = nullptr);
		void submit(const VkSparseImageMemoryBindInfo& sparse, std::function<void(void)> fenceCallback);
		void wait_no_lock() noexcept;
		void clear_semaphores() noexcept;

		FrameResource& frame();

		const Extensions& get_supported_extensions() const noexcept {
			return m_supportedExtensions;
		}
		uint32_t get_thread_index() const noexcept {
			return 0;
		}
		uint32_t get_thread_count() const noexcept {
			return 1;
		}
		VkDevice get_device() const noexcept {
			return m_device;
		}
		VkAllocationCallbacks* get_allocator() const noexcept {
			return m_allocator;
		}
		Allocator* get_vma_allocator() const noexcept {
			return m_vmaAllocator.get();
		}
		

		uint32_t get_swapchain_image_index() const noexcept {
			return m_wsiState.index;
		}
		uint32_t get_swapchain_image_count() const noexcept {
			return static_cast<uint32_t>(m_wsiState.swapchainImages.size());
		}
		uint32_t get_swapchain_width() const noexcept {
			return get_swapchain_image_view()->get_image()->get_width();
		}
		uint32_t get_swapchain_height() const noexcept {
			return get_swapchain_image_view()->get_image()->get_height();
		}
		VkBool32 supports_sparse_textures() const noexcept {
			VkBool32 ret = true;
			ret &= m_physicalProperties.sparseProperties.residencyStandard2DBlockShape; //Desired
			m_physicalProperties.sparseProperties.residencyNonResidentStrict; //Optional
			ret &= m_physicalFeatures.features.sparseBinding; //Required
			ret &= m_physicalFeatures.features.sparseResidencyImage2D; //Required
			m_physicalFeatures.features.sparseResidencyAliased; //Ignore for now
			return ret;
		}
		VkSparseImageMemoryRequirements get_sparse_memory_requirements(VkImage image, VkImageAspectFlags aspect) {
			uint32_t sparseMemoryRequirementsCount;
			vkGetImageSparseMemoryRequirements(m_device, image, &sparseMemoryRequirementsCount, NULL);
			std::vector<VkSparseImageMemoryRequirements> sparseMemoryRequirements(sparseMemoryRequirementsCount);
			vkGetImageSparseMemoryRequirements(m_device, image, &sparseMemoryRequirementsCount, sparseMemoryRequirements.data());
			VkSparseImageMemoryRequirements sparseMemoryRequirement = sparseMemoryRequirements[0];
			assert(sparseMemoryRequirementsCount);
			for (uint32_t i = 0; i < sparseMemoryRequirementsCount; i++) {
				if (sparseMemoryRequirements[i].formatProperties.aspectMask & aspect) {
					return sparseMemoryRequirements[i];
				}
			}
			return sparseMemoryRequirement;
		}
		Sampler* get_default_sampler(DefaultSampler samplerType) const noexcept;

	private:
		Queue& get_queue(CommandBuffer::Type type) noexcept {
			switch (type) {
			case CommandBuffer::Type::Generic:
				return m_graphics;
			case CommandBuffer::Type::Compute:
				return m_compute;
			case CommandBuffer::Type::Transfer:
				return m_transfer;
			}
		}
		ImageBuffer create_staging_buffer(const ImageInfo& info, InitialImageData* initialData, uint32_t baseMipLevel = 0);
		ImageHandle create_image(const ImageInfo& info, VkImageUsageFlags usage);
		ImageHandle create_sparse_image(const ImageInfo& info, VkImageUsageFlags usage);
		void update_image_with_buffer(const ImageInfo& info, Image& image, const ImageBuffer& buffer, vulkan::FenceHandle* fence = nullptr);
		void update_sparse_image_with_buffer(const ImageInfo& info, Image& image, const ImageBuffer& buffer, vulkan::FenceHandle* fence = nullptr, uint32_t mipLevel = 0);
		bool resize_sparse_image_up(Image& handle, uint32_t baseMipLevel = 0);
		void resize_sparse_image_down(Image& handle, uint32_t baseMipLevel, VkFence fence);

		std::vector<CommandBufferHandle>& get_current_submissions(CommandBuffer::Type type);
		CommandPool& get_pool(uint32_t threadId, CommandBuffer::Type type);
		void create_vma_allocator();
		void create_default_sampler();

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		//Last to destroy
		const Instance& r_instance;
		DeviceWrapper m_device;
		VkPhysicalDevice m_physicalDevice;
		Extensions m_supportedExtensions{};
		VkAllocationCallbacks* m_allocator = NULL;
		std::unique_ptr<Allocator> m_vmaAllocator;

		
		FenceManager m_fenceManager;
		SemaphoreManager m_semaphoreManager;

		std::vector<std::unique_ptr<FrameResource>> m_frameResources;

		Utility::Pool<CommandBuffer> m_commandBufferPool;
		Utility::Pool<Buffer> m_bufferPool;
		Utility::Pool<Allocation> m_allocationPool;
		Utility::LinkedBucketList<ImageView> m_imageViewPool;
		Utility::LinkedBucketList<Image> m_imagePool;
		WSIState m_wsiState;

		AttachmentAllocator m_attachmentAllocator;
		FramebufferAllocator m_framebufferAllocator;
		Queue m_graphics;
		Queue m_compute;
		Queue m_transfer;
		VkPhysicalDeviceProperties m_physicalProperties;
		VkPhysicalDeviceFeatures2 m_physicalFeatures;

		size_t m_currentFrame = 0;
		

		std::unordered_map< DescriptorSetLayout, size_t, Utility::Hash<DescriptorSetLayout>> m_descriptorAllocatorIds;
		Utility::LinkedBucketList<DescriptorSetAllocator> m_descriptorAllocatorsStorage;

		//TODO use other data structures
		//std::unordered_map< std::string, size_t> m_shaderIds;
		Utility::LinkedBucketList<Shader> m_shaderStorage;

		std::unordered_map< std::vector<Shader*>, size_t, Utility::VectorHash<Shader*>> m_programIds;
		Utility::LinkedBucketList<Program> m_programStorage;

		std::unordered_map< ShaderLayout, size_t, Utility::Hash<ShaderLayout>> m_pipelineLayoutIds;
		Utility::LinkedBucketList<PipelineLayout> m_pipelineLayoutStorage;

		std::array<std::unique_ptr<Sampler>, static_cast<size_t>(vulkan::DefaultSampler::Size)> m_defaultSampler;

		std::unordered_map<Utility::HashValue, size_t> m_renderpassIds;
		std::unordered_map<Utility::HashValue, size_t> m_compatibleRenderpassIds;
		Utility::LinkedBucketList<Renderpass> m_renderpassStorage;

		PipelineStorage m_pipelineStorage;
	};
}
#endif // VKLOGICALDEVICE_H