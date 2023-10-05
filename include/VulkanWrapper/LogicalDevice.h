#ifndef VKLOGICALDEVICE_H
#define VKLOGICALDEVICE_H
#pragma once

#include <functional>
#include <span>
#include <expected>

#include "Util"

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"
#include "Manager.h"
#include "VulkanWrapper/DescriptorSet.h"
#include "VulkanWrapper/DescriptorPool.hpp"
#include "VulkanWrapper/Queue.hpp"
#include "VulkanWrapper/DeviceWrapper.hpp"
#include "VulkanWrapper/VulkanError.hpp"
#include "VulkanWrapper/Sampler.hpp"
#include "VulkanWrapper/DeletionQueue.hpp"
#include "VulkanWrapper/Allocator.hpp"

namespace vulkan {
	//Important to delete the device after everything else

	struct InputData {
		const void* ptr{ nullptr };
		size_t size{ 0 };
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
		struct WSIState {
			//VkSemaphore aquire = VK_NULL_HANDLE;
			//VkSemaphore present = VK_NULL_HANDLE;
			std::vector<VkSemaphore> aquireSemaphores;
			std::vector<VkSemaphore> presentSemaphores;
			std::vector<ImageHandle> swapchainImages;
			bool swapchain_touched = false;
			uint32_t index = 0;
		};
		struct ImageBuffer {
			BufferHandle buffer;
			std::vector<VkBufferImageCopy> blits;
		};
		class FrameResource {
		public:
			FrameResource(LogicalDevice& device);
			~FrameResource();
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
			bool has_graphics_cmd() const noexcept;
			bool has_transfer_cmd() const noexcept;
			bool has_compute_cmd() const noexcept;
		private:
			void read_queries();
			void reset_command_pools();

			LogicalDevice& r_device;
			uint32_t frameIndex = 0;


			std::vector<CommandPool> graphicsPool;
			std::vector<CommandPool> computePool;
			std::vector<CommandPool> transferPool;
			std::vector<CommandBufferHandle> submittedGraphicsCmds;
			std::vector<CommandBufferHandle> submittedComputeCmds;
			std::vector<CommandBufferHandle> submittedTransferCmds;
			std::vector<TimestampQueryPool> timestamps;
			
			std::vector<VkSemaphore> signalSemaphores;
			std::vector<FenceHandle> waitForFences;

		};
	public:
		struct Viewport {
			VkViewport viewport;
			VkRect2D scissor;
		};
		struct QueueInfos
		{
			uint32_t graphicsFamilyQueueIndex;
			uint32_t computeFamilyQueueIndex;
			uint32_t transferFamilyQueueIndex;
			std::vector<float> graphicsQueuePriorities;
			std::vector<float> computeQueuePriorities;
			std::vector<float> transferQueuePriorities;
		};
	public:

		LogicalDevice(const vulkan::Instance& parentInstance,
			const vulkan::PhysicalDevice& physicalDevice,
			VkDevice device, uint32_t graphicsFamilyQueueIndex,
			uint32_t computeFamilyQueueIndex, uint32_t transferFamilyQueueIndex);
		~LogicalDevice();
		LogicalDevice(LogicalDevice&) = delete;
		LogicalDevice& operator=(LogicalDevice&) = delete;

		LogicalDevice(LogicalDevice&&) noexcept;
		LogicalDevice& operator=(LogicalDevice&&) noexcept;

		void wait_idle();

		BufferHandle create_buffer(const BufferInfo& info, const std::vector<InputData>& initialData, bool flush = true);
		ImageViewHandle create_image_view(const ImageViewCreateInfo& info);
		ImageHandle create_image(const ImageInfo& info, InitialImageData* initialData = nullptr, vulkan::FenceHandle* fence = nullptr);
		[[deprecated]] ImageHandle create_sparse_image(const ImageInfo& info, InitialImageData* initialData = nullptr, vulkan::FenceHandle* fence = nullptr);
		[[deprecated]] void downsize_sparse_image(Image& handle, uint32_t targetMipLevel);
		[[deprecated]] bool upsize_sparse_image(Image& handle, InitialImageData* initialData, uint32_t targetMipLevel);

		[[deprecated]] FenceHandle request_empty_fence();
		[[deprecated]] void destroy_semaphore(VkSemaphore semaphore);
		[[deprecated]] VkSemaphore request_semaphore();
		[[deprecated]] CommandBufferHandle request_command_buffer(CommandBufferType type);
		[[deprecated]] Image* request_render_target(uint32_t width, uint32_t height, VkFormat format, uint32_t index = 0, VkImageUsageFlags usage = 0, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, uint32_t arrayLayers = 1);

		[[deprecated]] VkSemaphore get_present_semaphore();
		[[deprecated]] bool swapchain_touched() const noexcept;
		[[deprecated]] VkQueue get_graphics_queue()  const noexcept;
		[[deprecated]] void aquired_image(uint32_t index, VkSemaphore semaphore) noexcept;

		[[deprecated]] void init_swapchain(const std::vector<VkImage>& swapchainImages, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
		[[deprecated]] const ImageView* get_swapchain_image_view() const noexcept;
		[[deprecated]] ImageView* get_swapchain_image_view() noexcept;
		[[deprecated]] ImageView* get_swapchain_image_view(size_t idx) noexcept;
		[[deprecated]] const Image* get_swapchain_image() const noexcept;
		[[deprecated]] Image* get_swapchain_image() noexcept;
		[[deprecated]] Image* get_swapchain_image(size_t idx) noexcept;

		[[deprecated]] void next_frame();
		[[deprecated]] void end_frame();
		[[deprecated]] void submit_queue(CommandBufferType type, FenceHandle* fence, uint32_t semaphoreCount = 0, VkSemaphore* semaphores = nullptr, uint64_t* semaphoreValues = nullptr);
		[[deprecated]] void add_wait_semaphore(CommandBufferType type, VkSemaphore semaphore, VkPipelineStageFlags2 stages, uint64_t value = 0, bool flush = false);
		[[deprecated]] void add_wait_semaphores(CommandBufferType type, const std::vector<VkSemaphoreSubmitInfo >& submitInfos, bool flush = false);
		[[deprecated]] void submit_empty(CommandBufferType type, FenceHandle* fence, uint32_t semaphoreCount, VkSemaphore* semaphore, uint64_t* semaphoreValues = nullptr);
		[[deprecated]] void submit_staging(CommandBufferHandle cmd, VkBufferUsageFlags usage, bool flush);
		[[deprecated]] void submit(CommandBufferHandle cmd, uint32_t semaphoreCount = 0, VkSemaphore* semaphores = nullptr, vulkan::FenceHandle* fence = nullptr, uint64_t* semaphoreValues = nullptr);
		[[deprecated]] void submit_flush(CommandBufferHandle cmd, uint32_t semaphoreCount = 0, VkSemaphore* semaphores = nullptr, vulkan::FenceHandle* fence = nullptr, uint64_t* semaphoreValues = nullptr);
		[[deprecated]] void wait_no_lock() noexcept;
		[[deprecated]] void clear_semaphores() noexcept;
		[[deprecated]] void add_fence_callback(VkFence fence, std::function<void(void)> callback);

		void create_pipeline_cache(const std::string& path);
		ShaderStorage& get_shader_storage();
		PipelineStorage2& get_pipeline_storage();

		FrameResource& frame();
		FrameResource& previous_frame();

		[[deprecated]] uint32_t get_thread_index() const noexcept;
		[[deprecated]] uint32_t get_thread_count() const noexcept;

		[[nodiscard]] const PhysicalDevice& get_physical_device() const noexcept
		{
			return r_physicalDevice;
		}
		[[nodiscard]] const LogicalDeviceWrapper& get_device() const noexcept
		{
			return m_device;
		}
		[[nodiscard]] DeletionQueue& get_deletion_queue() noexcept
		{
			return m_deletionQueue;
		}
		[[nodiscard]] VkDevice get_device_handle() const noexcept
		{
			return m_device.get_handle();
		}
		[[nodiscard]] VkAllocationCallbacks* get_allocator() const noexcept
		{
			return m_allocator;
		}
		[[nodiscard]] const Allocator& get_vma_allocator() const noexcept;

		[[nodiscard]] const VkPhysicalDeviceProperties& get_physical_device_properties() const noexcept;

		[[deprecated]] DescriptorSet& get_bindless_set() noexcept;
		[[deprecated]] DescriptorPool& get_bindless_pool() noexcept;
		[[deprecated]] PipelineLayout2& get_bindless_pipeline_layout() noexcept;


		[[deprecated]] Viewport get_swapchain_viewport_and_scissor() const noexcept;
		[[deprecated]] uint32_t get_swapchain_image_index() const noexcept;
		[[deprecated]] uint32_t get_swapchain_image_count() const noexcept;
		[[deprecated]] uint32_t get_swapchain_width() const noexcept;
		[[deprecated]] uint32_t get_swapchain_height() const noexcept;
		[[deprecated]] VkBool32 supports_sparse_textures() const noexcept;
		[[deprecated]] VkSparseImageMemoryRequirements get_sparse_memory_requirements(VkImage image, VkImageAspectFlags aspect);
		[[deprecated]] uint32_t get_compute_family() const noexcept;
		[[deprecated]] uint32_t get_graphics_family() const noexcept;
		VkPipelineCache get_pipeline_cache() const noexcept;
		Sampler* get_default_sampler(DefaultSampler samplerType) const noexcept;

		[[deprecated]] Queue& get_queue(CommandBufferType type) noexcept {
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

		[[nodiscard]] std::span<vulkan::Queue const> get_queues(vulkan::Queue::Type type) const noexcept
		{
			return m_queues[static_cast<size_t>(type)];
		}
		[[nodiscard]] std::span<vulkan::Queue> get_queues(vulkan::Queue::Type type) noexcept
		{
			return m_queues[static_cast<size_t>(type)];
		}

		static std::expected<LogicalDevice, vulkan::Error> create_device(const vulkan::Instance& parentInstance,
		                                                            const vulkan::PhysicalDevice& physicalDevice,
		                                                            VkDevice device, const QueueInfos& queueInfos) noexcept;
		LogicalDevice(const vulkan::Instance& parentInstance,
			const vulkan::PhysicalDevice& physicalDevice,
			VkDevice device,
			const QueueInfos& queueInfos);

	private:
		LogicalDevice(const vulkan::Instance& parentInstance,
			const vulkan::PhysicalDevice& physicalDevice,
			VkDevice device,
			const QueueInfos& queueInfos, int a);

		void create_queues(const QueueInfos& queueInfos) noexcept;
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
		//void create_vma_allocator();
		std::expected<void, vulkan::Error> create_default_sampler() noexcept;

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		//Last to destroy
		const Instance& r_instance;
		const PhysicalDevice& r_physicalDevice;
		LogicalDeviceWrapper m_device;

		VkAllocationCallbacks* m_allocator{ nullptr };
		Allocator m_vmaAllocator;
		DeletionQueue m_deletionQueue;

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
		LogicalDevice::Queue m_graphics;
		LogicalDevice::Queue m_compute;
		LogicalDevice::Queue m_transfer;

		std::array<std::vector<vulkan::Queue>, static_cast<size_t>(vulkan::Queue::Type::Size)> m_queues;

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