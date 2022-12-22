#ifndef VKLOGICALDEVICE_H
#define VKLOGICALDEVICE_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"

#include <functional>

#include "Util"
#include "Manager.h"
#include "DescriptorSet.h"
namespace vulkan {
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
		operator VkDevice() const noexcept { return m_vkHandle; }
		const VkDevice& get_handle() const noexcept { return m_vkHandle; };
		VkDevice& get_handle() noexcept { return m_vkHandle; };
	private:
		VkDevice m_vkHandle = VK_NULL_HANDLE;
		const VkAllocationCallbacks* m_allocator;
	};

	struct InputData {
		const void* ptr;
		size_t size;
		size_t stride {0};
	};

	static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

	struct InitialImageData {
		const void* data = nullptr;
		std::array<uint32_t, 16> mipOffsets{};
		uint32_t mipCounts = 0;
		//uint32_t rowLength = 0;
		//uint32_t height = 0;
	};
	class LogicalDevice {
	private:

		struct WSIState {
			//VkSemaphore aquire = VK_NULL_HANDLE;
			//VkSemaphore present = VK_NULL_HANDLE;
			std::vector<VkSemaphore> aquireSemaphores;
			std::vector<VkSemaphore> presentSemaphores;
			std::vector<ImageHandle> swapchainImages;
			bool swapchain_touched = false;
			uint32_t index = 0;
		};
		struct Queue {
			Queue() = default;
			Queue(uint32_t family) : familyIndex(family) {}
			operator VkQueue() const noexcept { return queue; }
			//std::vector<VkSemaphore> waitSemaphores;
			//std::vector<VkPipelineStageFlags> waitStages;
			std::vector<VkSemaphoreSubmitInfo> waitInfos;
			bool needsFence = false;
			bool supportsSparse = false;

			const uint32_t familyIndex = 0;
			VkQueue queue = VK_NULL_HANDLE;
		};
		struct ImageBuffer {
			BufferHandle buffer;
			std::vector<VkBufferImageCopy> blits;
		};
		class FrameResource {
		public:
			FrameResource(LogicalDevice& device);
			~FrameResource();
			void recycle_semaphore(VkSemaphore sempahore);
			std::vector<CommandBufferHandle>& get_submissions(CommandBufferType type) noexcept;
			vulkan::CommandPool& get_pool(CommandBufferType type) noexcept;
			vulkan::TimestampQueryPool& get_timestamps() noexcept;
			std::vector<VkSemaphore>& get_signal_semaphores() noexcept;
			void signal_semaphore(VkSemaphore semaphore) noexcept;
			void wait_for_fence(FenceHandle&& fence) noexcept;
			//Queue signal sempahores for deletion
			//Can't reuse them for some reason
			void delete_signal_semaphores() noexcept;
			void begin();
			void end();
			void clear_fences();
			void queue_framebuffer_deletion(VkFramebuffer framebuffer) noexcept;
			void queue_image_deletion(VkImage image) noexcept;
			void queue_image_view_deletion(VkImageView imageView) noexcept;
			void queue_buffer_view_deletion(VkBufferView bufferView) noexcept;
			void queue_image_sampler_deletion(VkSampler sampler) noexcept;
			void queue_acceleration_structure_deletion(VkAccelerationStructureKHR accelerationStructure) noexcept;
			void queue_descriptor_pool_deletion(VkDescriptorPool descriptorPool) noexcept;
			void queue_buffer_deletion(VkBuffer buffer) noexcept;
			void queue_allocation_deletion(VmaAllocation allocation) noexcept;
			bool has_graphics_cmd() const noexcept;
			bool has_transfer_cmd() const noexcept;
			bool has_compute_cmd() const noexcept;
		private:
			void read_queries();
			void reset_command_pools();
			void delete_resources();

			LogicalDevice& r_device;
			uint32_t frameIndex = 0;


			std::vector<CommandPool> graphicsPool;
			std::vector<CommandPool> computePool;
			std::vector<CommandPool> transferPool;
			std::vector<CommandBufferHandle> submittedGraphicsCmds;
			std::vector<CommandBufferHandle> submittedComputeCmds;
			std::vector<CommandBufferHandle> submittedTransferCmds;
			std::vector<TimestampQueryPool> timestamps;


			std::vector<VkBufferView> deletedBufferViews;
			std::vector<VkImageView> deletedImageViews;
			std::vector<VkSampler> deletedSampler;
			std::vector<VkImage> deletedImages;
			std::vector<VkBuffer> deletedBuffer;
			std::vector<VkAccelerationStructureKHR> deletedAccelerationStructures;
			std::vector<VmaAllocation> deletedAllocations;
			std::vector<VkFramebuffer> deletedFramebuffer;
			std::vector<VkSemaphore> deletedSemaphores;
			std::vector<VkSemaphore> recycledSemaphores;

			std::vector<VkSemaphore> signalSemaphores;
			std::vector<FenceHandle> waitForFences;

		};
	public:
		struct Viewport {
			VkViewport viewport;
			VkRect2D scissor;
		};
	public:

		LogicalDevice(const vulkan::Instance& parentInstance,
			const vulkan::PhysicalDevice& physicalDevice,
			VkDevice device, uint32_t graphicsFamilyQueueIndex,
			uint32_t computeFamilyQueueIndex, uint32_t transferFamilyQueueIndex);
		~LogicalDevice();
		LogicalDevice(LogicalDevice&) = delete;
		LogicalDevice& operator=(LogicalDevice&) = delete;
		LogicalDevice(LogicalDevice&&) = delete;
		LogicalDevice& operator=(LogicalDevice&&) = delete;
		void wait_idle();


		BufferHandle create_buffer(const BufferInfo& info, const std::vector<InputData>& initialData, bool flush = true);
		ImageViewHandle create_image_view(const ImageViewCreateInfo& info);
		ImageHandle create_image(const ImageInfo& info, InitialImageData* initialData = nullptr, vulkan::FenceHandle* fence = nullptr);
		ImageHandle create_sparse_image(const ImageInfo& info, InitialImageData* initialData = nullptr, vulkan::FenceHandle* fence = nullptr);
		void downsize_sparse_image(Image& handle, uint32_t targetMipLevel);
		bool upsize_sparse_image(Image& handle, InitialImageData* initialData, uint32_t targetMipLevel);

		FenceHandle request_empty_fence();
		void destroy_semaphore(VkSemaphore semaphore);
		VkSemaphore request_semaphore();
		CommandBufferHandle request_command_buffer(CommandBufferType type);
		Image* request_render_target(uint32_t width, uint32_t height, VkFormat format, uint32_t index = 0, VkImageUsageFlags usage = 0, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, uint32_t arrayLayers = 1);
		void resize_buffer(Buffer& buffer, VkDeviceSize newSize, bool copyData = false);

		VkSemaphore get_present_semaphore();
		bool swapchain_touched() const noexcept;
		VkQueue get_graphics_queue()  const noexcept;
		void aquired_image(uint32_t index, VkSemaphore semaphore) noexcept;

		void init_swapchain(const std::vector<VkImage>& swapchainImages, uint32_t width, uint32_t height, VkFormat format);
		const ImageView* get_swapchain_image_view() const noexcept;
		ImageView* get_swapchain_image_view() noexcept;
		ImageView* get_swapchain_image_view(size_t idx) noexcept;
		const Image* get_swapchain_image() const noexcept;
		Image* get_swapchain_image() noexcept;
		Image* get_swapchain_image(size_t idx) noexcept;

		void next_frame();
		void end_frame();
		void submit_queue(CommandBufferType type, FenceHandle* fence, uint32_t semaphoreCount = 0, VkSemaphore* semaphores = nullptr, uint64_t* semaphoreValues = nullptr);
		void queue_framebuffer_deletion(VkFramebuffer framebuffer) noexcept;
		void queue_image_deletion(VkImage image) noexcept;
		void queue_image_view_deletion(VkImageView imageView) noexcept;
		void queue_buffer_view_deletion(VkBufferView bufferView) noexcept;
		void queue_image_sampler_deletion(VkSampler sampler) noexcept;
		void queue_acceleration_structure_deletion(VkAccelerationStructureKHR accelerationStructure) noexcept;
		void queue_descriptor_pool_deletion(VkDescriptorPool descriptorPool) noexcept;
		void queue_buffer_deletion(VkBuffer buffer) noexcept;
		void queue_allocation_deletion(VmaAllocation allocation) noexcept;
		//void add_wait_semaphore(CommandBufferType type, VkSemaphore semaphore, VkPipelineStageFlags stages, bool flush = false);
		//void add_wait_semaphores(CommandBufferType type, const std::vector<VkSemaphore>& semaphores, const std::vector<VkPipelineStageFlags>& stages, bool flush = false);
		void add_wait_semaphore(CommandBufferType type, VkSemaphore semaphore, VkPipelineStageFlags2 stages, uint64_t value = 0, bool flush = false);
		void add_wait_semaphores(CommandBufferType type, const std::vector<VkSemaphoreSubmitInfo >& submitInfos, bool flush = false);
		void submit_empty(CommandBufferType type, FenceHandle* fence, uint32_t semaphoreCount, VkSemaphore* semaphore, uint64_t* semaphoreValues = nullptr);
		void submit_staging(CommandBufferHandle cmd, VkBufferUsageFlags usage, bool flush);
		void submit(CommandBufferHandle cmd, uint32_t semaphoreCount = 0, VkSemaphore* semaphores = nullptr, vulkan::FenceHandle* fence = nullptr, uint64_t* semaphoreValues = nullptr);
		void submit_flush(CommandBufferHandle cmd, uint32_t semaphoreCount = 0, VkSemaphore* semaphores = nullptr, vulkan::FenceHandle* fence = nullptr, uint64_t* semaphoreValues = nullptr);
		void wait_no_lock() noexcept;
		void clear_semaphores() noexcept;
		void add_fence_callback(VkFence fence, std::function<void(void)> callback);

		void create_pipeline_cache(const std::string& path);
		ShaderStorage& get_shader_storage();
		PipelineStorage2& get_pipeline_storage();

		FrameResource& frame();
		FrameResource& previous_frame();

		const Extensions& get_supported_extensions() const noexcept;
		uint32_t get_thread_index() const noexcept;
		uint32_t get_thread_count() const noexcept;
		const PhysicalDevice& get_physical_device() const noexcept;
		VkDevice get_device() const noexcept;
		operator VkDevice() const noexcept;
		VkAllocationCallbacks* get_allocator() const noexcept;
		Allocator* get_vma_allocator() const noexcept;
		const VkPhysicalDeviceProperties& get_physical_device_properties() const noexcept;

		DescriptorSet& get_bindless_set() noexcept;
		DescriptorPool& get_bindless_pool() noexcept;
		PipelineLayout2& get_bindless_pipeline_layout() noexcept;


		Viewport get_swapchain_viewport_and_scissor() const noexcept;
		uint32_t get_swapchain_image_index() const noexcept;
		uint32_t get_swapchain_image_count() const noexcept;
		uint32_t get_swapchain_width() const noexcept;
		uint32_t get_swapchain_height() const noexcept;
		VkBool32 supports_sparse_textures() const noexcept;
		VkSparseImageMemoryRequirements get_sparse_memory_requirements(VkImage image, VkImageAspectFlags aspect);
		uint32_t get_compute_family() const noexcept;
		uint32_t get_graphics_family() const noexcept;
		VkPipelineCache get_pipeline_cache() const noexcept;
		Sampler* get_default_sampler(DefaultSampler samplerType) const noexcept;

		void wait_on_idle_queue(CommandBufferType type);

	private:
		Queue& get_queue(CommandBufferType type) noexcept {
			switch (type) {
			case CommandBufferType::Generic:
				return m_graphics;
			case CommandBufferType::Compute:
				return m_compute;
			case CommandBufferType::Transfer:
				return m_transfer;
			default:
				assert(false);
				return m_graphics;
			}
		}
		ImageBuffer create_staging_buffer(const ImageInfo& info, InitialImageData* initialData, uint32_t baseMipLevel = 0);
		ImageHandle create_image(const ImageInfo& info, VkImageUsageFlags usage);
		ImageHandle create_sparse_image(const ImageInfo& info, VkImageUsageFlags usage);
		void transition_image(ImageHandle& handle, VkImageLayout oldLayout, VkImageLayout newLayout);
		void update_image_with_buffer(const ImageInfo& info, Image& image, const ImageBuffer& buffer, vulkan::FenceHandle* fence = nullptr);
		void update_sparse_image_with_buffer(const ImageInfo& info, Image& image, const ImageBuffer& buffer, vulkan::FenceHandle* fence = nullptr, uint32_t mipLevel = 0);
		bool resize_sparse_image_up(Image& handle, uint32_t baseMipLevel = 0);
		void resize_sparse_image_down(Image& handle, uint32_t baseMipLevel);

		std::vector<CommandBufferHandle>& get_current_submissions(CommandBufferType type);
		CommandPool& get_pool(CommandBufferType type);
		void create_vma_allocator();
		void create_default_sampler();

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		//Last to destroy
		const Instance& r_instance;
		const PhysicalDevice& r_physicalDevice;
		DeviceWrapper m_device;

		VkAllocationCallbacks* m_allocator = NULL;
		std::unique_ptr<Allocator> m_vmaAllocator;


		FenceManager m_fenceManager;
		SemaphoreManager m_semaphoreManager;

		std::vector<std::unique_ptr<FrameResource>> m_frameResources;

		Utility::Pool<CommandBuffer> m_commandBufferPool;
		std::unique_ptr<Utility::Pool<Allocation>> m_allocationPool;
		std::unique_ptr<Utility::LinkedBucketList<Buffer>> m_bufferPool;
		std::unique_ptr<Utility::LinkedBucketList<ImageView>> m_imageViewPool;
		std::unique_ptr<Utility::LinkedBucketList<Image>> m_imagePool;
		WSIState m_wsiState;

		std::unique_ptr<AttachmentAllocator> m_attachmentAllocator;
		Queue m_graphics;
		Queue m_compute;
		Queue m_transfer;

		size_t m_currentFrame = 0;
		
		std::unordered_multimap<VkFence, std::function<void(void)>> m_fenceCallbacks;


		std::unique_ptr<ShaderStorage> m_shaderStorage;
		std::unique_ptr<PipelineStorage2> m_pipelineStorage2;


		std::array<std::unique_ptr<Sampler>, static_cast<size_t>(vulkan::DefaultSampler::Size)> m_defaultSampler;

		std::unique_ptr<PipelineCache> m_pipelineCache;

		DescriptorPool m_bindlessPool;
		DescriptorSet m_bindlessSet;
		std::unique_ptr < PipelineLayout2> m_bindlessPipelineLayout;
	};
}
#endif // VKLOGICALDEVICE_H