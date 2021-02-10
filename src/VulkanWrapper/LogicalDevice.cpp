#include "LogicalDevice.h"


static std::vector<uint32_t> read_binary_file(const std::string& filename) {

	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!(file.is_open())) {
		throw std::runtime_error("Could not open file: \"" + filename + "\"");
	}

	auto fileSize = file.tellg();
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();
	return buffer;
}
Vulkan::LogicalDevice::LogicalDevice(const Vulkan::Instance& parentInstance, VkDevice device, uint32_t graphicsFamilyQueueIndex,
					uint32_t computeFamilyQueueIndex, uint32_t transferFamilyQueueIndex, VkPhysicalDeviceProperties& properties) :
	r_instance(parentInstance),
	m_device(device, nullptr),
	m_graphics(graphicsFamilyQueueIndex),
	m_compute(computeFamilyQueueIndex == ~0 ? graphicsFamilyQueueIndex : computeFamilyQueueIndex),
	m_transfer(transferFamilyQueueIndex == ~0 ? graphicsFamilyQueueIndex : transferFamilyQueueIndex),
	m_physicalProperties(properties),
	m_framebufferAllocator(*this),
	m_fenceManager(*this),
	m_semaphoreManager(*this),
	m_pipelineStorage(*this),
	m_attachmentAllocator(*this)
{
	vkGetDeviceQueue(m_device, m_graphics.familyIndex, 0, &m_graphics.queue);
	vkGetDeviceQueue(m_device, m_compute.familyIndex, 0, &m_compute.queue);
	vkGetDeviceQueue(m_device, m_transfer.familyIndex, 0, &m_transfer.queue);
	create_vma_allocator();
	create_default_sampler();	
	m_frameResources.reserve(MAX_FRAMES_IN_FLIGHT);
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_frameResources.emplace_back(new FrameResource{*this});
	}
}
Vulkan::LogicalDevice::~LogicalDevice()
{
	if (m_wsiState.aquire != VK_NULL_HANDLE) {
		vkDestroySemaphore(get_device(), m_wsiState.aquire, get_allocator());
	}
	if (m_wsiState.present != VK_NULL_HANDLE) {
		vkDestroySemaphore(get_device(), m_wsiState.present, get_allocator());
	}
}

void Vulkan::LogicalDevice::set_acquire_semaphore(uint32_t index, VkSemaphore semaphore) noexcept
{
	m_wsiState.aquire = semaphore;
	m_wsiState.swapchain_touched = false;
	m_wsiState.index = index;
}

void Vulkan::LogicalDevice::init_swapchain(const std::vector<VkImage>& swapchainImages, uint32_t width, uint32_t height, VkFormat format)
{
	m_wsiState.swapchainImages.clear();
	wait_idle();
	auto info = ImageInfo::render_target(width, height, format);
	info.isSwapchainImage = true;
	assert(m_wsiState.aquire == VK_NULL_HANDLE);
	assert(m_wsiState.present == VK_NULL_HANDLE);

	m_wsiState.swapchain_touched = false;
	m_wsiState.index = 0u;

	for (const auto image : swapchainImages) {
		auto handle = m_imagePool.emplace(*this, image, info);
		handle->disown();
		handle->set_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		m_wsiState.swapchainImages.emplace_back(handle);
	}

}

const Vulkan::ImageView* Vulkan::LogicalDevice::get_swapchain_image_view() const noexcept
{
	return m_wsiState.swapchainImages[m_wsiState.index]->get_view();
}

Vulkan::ImageView* Vulkan::LogicalDevice::get_swapchain_image_view() noexcept
{
	return m_wsiState.swapchainImages[m_wsiState.index]->get_view();
}

void Vulkan::LogicalDevice::next_frame()
{
	end_frame();

	m_currentFrame++;
	if (m_currentFrame >= m_frameResources.size())
		m_currentFrame = 0;
	frame().begin();
	m_wsiState.swapchain_touched = false;
	if (m_wsiState.aquire != VK_NULL_HANDLE)
		frame().recycledSemaphores.push_back(m_wsiState.aquire);
	//m_wsiState.aquire = m_semaphoreManager.request_semaphore();
	//m_demoImage = m_swapchains[0]->aquire_next_image(m_wsiState.aquire);
}

void Vulkan::LogicalDevice::end_frame()
{
	if (!frame().submittedTransferCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		submit_queue(CommandBuffer::Type::Transfer, &fence);
		frame().waitForFences.emplace_back(std::move(fence));
		m_compute.needsFence = false;
	}
	if (!frame().submittedGraphicsCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		submit_queue(CommandBuffer::Type::Generic, &fence);
		frame().waitForFences.emplace_back(std::move(fence));
		m_compute.needsFence = false;
	}
	if (!frame().submittedComputeCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		submit_queue(CommandBuffer::Type::Compute,&fence);
		frame().waitForFences.emplace_back(std::move(fence));
		m_compute.needsFence = false;
	}
	//m_swapchains[0]->present_queue();
}

void Vulkan::LogicalDevice::submit_queue(CommandBuffer::Type type, FenceHandle* fence, uint32_t semaphoreCount, VkSemaphore* semaphores)
{
	auto& queue = get_queue(type);
	auto& submissions = get_current_submissions(type);
	assert(!submissions.empty());
	//Split commands into pre and post swapchain commands
	std::array<VkSubmitInfo, 2> submitInfos;
	uint32_t submitCounts = 0;
	std::vector<VkCommandBuffer> commands;
	std::array<std::vector<VkSemaphore>, 2> waitSemaphores;
	std::array<std::vector<VkFlags>, 2> waitStages;
	std::array<std::vector<VkSemaphore>, 2> signalSemaphores;
	for (uint32_t i = 0; i < queue.waitSemaphores.size(); i++) {
		waitStages[0].push_back(queue.waitStages[i]);
		waitSemaphores[0].push_back(queue.waitSemaphores[i]);
		frame().recycledSemaphores.push_back(queue.waitSemaphores[i]);
	}
	queue.waitStages.clear();
	queue.waitSemaphores.clear();
	commands.reserve(submissions.size());
	uint32_t firstSubmissionCount = 0;
	bool touched = false;
	for (auto& submission : submissions) {
		if (submission->swapchain_touched() && (firstSubmissionCount == 0) && !m_wsiState.swapchain_touched) {
			if (!commands.empty()) {
				VkSubmitInfo submitInfo{
					.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					.commandBufferCount = static_cast<uint32_t>(commands.size()),
					.pCommandBuffers = commands.data(),
				};
				submitInfos[submitCounts++] = submitInfo;
			}
			firstSubmissionCount = static_cast<uint32_t>(commands.size());
			touched = true;
		}
		commands.push_back(submission->get_handle());
	}
	if (!commands.empty()) {
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = static_cast<uint32_t>(commands.size()) - firstSubmissionCount,
			.pCommandBuffers = &commands[firstSubmissionCount],
		};
		submitInfos[submitCounts++] = submitInfo;
		if (touched && !m_wsiState.swapchain_touched) {
			if (m_wsiState.aquire != VK_NULL_HANDLE) {
				waitSemaphores[submitCounts - 1].push_back(m_wsiState.aquire);
				waitStages[submitCounts - 1].push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
				frame().recycledSemaphores.push_back(m_wsiState.aquire);
				m_wsiState.aquire = VK_NULL_HANDLE;
			}
			if (m_wsiState.present != VK_NULL_HANDLE)
				frame().recycledSemaphores.push_back(m_wsiState.present);
			m_wsiState.present = request_semaphore();
			signalSemaphores[submitCounts - 1].push_back(m_wsiState.present);
			m_wsiState.swapchain_touched = true;
		}
	}
	if (fence)
		*fence = m_fenceManager.request_fence();
	for (uint32_t i = 0; i < semaphoreCount; i++) {
		VkSemaphore sem = request_semaphore();
		signalSemaphores[submitCounts - 1].push_back(sem);
		assert(semaphores[i] == VK_NULL_HANDLE);
		semaphores[i] = sem;
	}
	for (uint32_t i = 0; i < submitCounts; i++) {
		submitInfos[i].waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores[i].size());
		submitInfos[i].pWaitSemaphores = waitSemaphores[i].data();
		submitInfos[i].pWaitDstStageMask = waitStages[i].data(); 

		submitInfos[i].signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores[i].size());
		submitInfos[i].pSignalSemaphores = signalSemaphores[i].data();

	}
#if 0
	for (auto i = 0; i < submitCounts; i++) {
		for (auto cmd = 0; cmd < submitInfos[i].commandBufferCount; cmd++) {
			std::cout << "Submitted :" << submitInfos[i].pCommandBuffers[cmd] << "\n";
		}
	}
#endif //  0
	VkFence localFence = fence ? *fence : VK_NULL_HANDLE;
	if (auto result = vkQueueSubmit(queue.queue, submitCounts, submitInfos.data(), localFence); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not submit to Queue, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not submit to Queue, out of device memory");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not submit to Queue, device lost");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	submissions.clear();
}

void Vulkan::LogicalDevice::queue_framebuffer_deletion(VkFramebuffer framebuffer) noexcept
{
	auto& curFrame = frame();
	if (auto res = std::find(curFrame.deletedFramebuffer.cbegin(), curFrame.deletedFramebuffer.cend(), framebuffer); res == curFrame.deletedFramebuffer.cend())
		curFrame.deletedFramebuffer.push_back(framebuffer);
}

void Vulkan::LogicalDevice::queue_image_deletion(VkImage image) noexcept
{
	frame().deletedImages.push_back(image);
}

void Vulkan::LogicalDevice::queue_image_deletion(VkImage image, VmaAllocation allocation) noexcept
{
	frame().deletedImageAllocations.push_back({ image, allocation });
}

void Vulkan::LogicalDevice::queue_image_view_deletion(VkImageView imageView) noexcept
{
	frame().deletedImageViews.push_back(imageView);
}

void Vulkan::LogicalDevice::queue_buffer_view_deletion(VkBufferView bufferView) noexcept
{
	frame().deletedBufferViews.push_back(bufferView);
}

void Vulkan::LogicalDevice::queue_image_sampler_deletion(VkSampler sampler) noexcept
{
	frame().deletedSampler.push_back(sampler);
}

void Vulkan::LogicalDevice::queue_buffer_deletion(VkBuffer buffer, VmaAllocation allocation) noexcept
{
	frame().deletedBufferAllocations.push_back({ buffer, allocation });
}

void Vulkan::LogicalDevice::add_wait_semaphore(CommandBuffer::Type type, VkSemaphore semaphore, VkPipelineStageFlags stages, bool flush)
{
	assert(stages != 0);
	auto& queue = get_queue(type);
	queue.waitSemaphores.push_back(semaphore);
	queue.waitStages.push_back(stages);
	queue.needsFence = true;
}

void Vulkan::LogicalDevice::submit_staging(CommandBufferHandle cmd, VkBufferUsageFlags usage, bool flush)
{
	auto access = BufferInfo::buffer_usage_to_possible_access(usage);
	auto stages = BufferInfo::buffer_usage_to_possible_stages(usage);
	VkQueue srcQueue = get_queue(cmd->get_type()).queue;

	if (srcQueue == m_graphics.queue && m_graphics.queue == m_compute.queue) {
		cmd->barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, stages, access);
		submit(cmd);
	}
	else {
		auto computeStages = stages & (VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | 
			VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
		auto computeAccess = access & (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
			VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
			VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
		auto graphicStages = stages;
		if (srcQueue == m_graphics.queue) {
			cmd->barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, graphicStages, access);
			if (computeStages != 0) {
				VkSemaphore sem = VK_NULL_HANDLE;;
				submit(cmd, 1, &sem);
				add_wait_semaphore(CommandBuffer::Type::Compute, sem, computeStages, flush);
			}
			else {
				submit(cmd);
			}
		}
		else if (srcQueue == m_compute.queue) {
			assert(false);
		}
		else {
			if (graphicStages != 0 && computeStages) {
				std::array<VkSemaphore, 2> semaphores{};
				submit(cmd, 2, semaphores.data());
				add_wait_semaphore(CommandBuffer::Type::Generic, semaphores[0], graphicStages, flush);
				add_wait_semaphore(CommandBuffer::Type::Compute, semaphores[1], computeStages, flush);
			}
			else if (graphicStages != 0) {
				VkSemaphore semaphore = VK_NULL_HANDLE;
				submit(cmd, 1, &semaphore);
				add_wait_semaphore(CommandBuffer::Type::Generic, semaphore, graphicStages, flush);
			}
			else if (computeStages != 0) {
				VkSemaphore semaphore = VK_NULL_HANDLE;
				submit(cmd, 1, &semaphore);
				add_wait_semaphore(CommandBuffer::Type::Compute, semaphore, computeStages, flush);
			}
			else {
				submit(cmd);
			}
		}
	}
}

void Vulkan::LogicalDevice::submit(CommandBufferHandle cmd, uint32_t semaphoreCount, VkSemaphore* semaphores)
{
	auto type = cmd->get_type();
	auto& submissions = get_current_submissions(type);
	cmd->end();
	submissions.push_back(cmd);
	if (semaphoreCount) {
		submit_queue(type, nullptr, semaphoreCount, semaphores);
	}
}

void Vulkan::LogicalDevice::wait_no_lock() noexcept
{
	if (!m_frameResources.empty())
		end_frame();

	vkDeviceWaitIdle(m_device);

	clear_semaphores();
	m_framebufferAllocator.clear();
	m_attachmentAllocator.clear();
	
	for (auto& frame : m_frameResources) {
		frame->waitForFences.clear();
		frame->begin();
	}
}

void Vulkan::LogicalDevice::clear_semaphores() noexcept
{
	for (auto& sem : m_graphics.waitSemaphores)
		vkDestroySemaphore(m_device, sem, m_allocator);
	for (auto& sem : m_compute.waitSemaphores)
		vkDestroySemaphore(m_device, sem, m_allocator);
	for (auto& sem : m_transfer.waitSemaphores)
		vkDestroySemaphore(m_device, sem, m_allocator);
	m_graphics.waitSemaphores.clear();
	m_compute.waitSemaphores.clear();
	m_transfer.waitSemaphores.clear();
	m_graphics.waitStages.clear();
	m_compute.waitStages.clear();
	m_transfer.waitStages.clear();

}

Vulkan::LogicalDevice::FrameResource& Vulkan::LogicalDevice::frame()
{
	return *m_frameResources[m_currentFrame];
}
Vulkan::LogicalDevice::ImageBuffer Vulkan::LogicalDevice::create_staging_buffer(const ImageInfo& info, const void* data, uint32_t rowLength, uint32_t height)
{
	uint32_t copyLevels;
	if (info.generate_mips())
		copyLevels = 1;
	else if (info.mipLevels == 0)
		copyLevels = calculate_mip_levels(info.width, info.height, info.depth);
	else
		copyLevels = info.mipLevels;

	MipInfo mipInfo(info.format, info.width, info.height, info.arrayLayers, copyLevels, info.depth);
	BufferInfo bufferInfo {
		.size = mipInfo.size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.offset = 0,
		.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU
	};
	auto buffer = create_buffer(bufferInfo);
	auto map = buffer->map_data();
	std::vector<VkBufferImageCopy> blits;
	blits.reserve(copyLevels);
	for (uint32_t level = 0; level < copyLevels; level++) {
		const auto& mip = mipInfo.mipLevels[level];
		blits.push_back(VkBufferImageCopy{
			.bufferOffset = mip.offset,
			.bufferRowLength = mip.blockWidth,
			.bufferImageHeight = mip.blockHeight,
			.imageSubresource {
				.aspectMask = ImageInfo::format_to_aspect_mask(info.format),
				.mipLevel = level,
				.baseArrayLayer = 0,
				.layerCount = info.arrayLayers
			},
			.imageExtent {
				.width = mip.width,
				.height = mip.height,
				.depth = mip.depth
			}
			});
		uint32_t blockStride = format_block_size(info.format);
		auto [blockSizeX, blockSizeY] = format_to_block_size(info.format);
		size_t rowSize = mip.blockCountX * blockStride;
		//size_t dstLayerSize = rowSize * mip.blockCountY;
		//Ignore layers for now
		//assume one level for now
		assert(level == 0);

		for (uint32_t y = 0; y < mip.blockCountY; y++)
			std::memcpy(map.get()+y *rowSize, reinterpret_cast<const uint8_t*>(data) + y * rowSize, rowSize);
		
	}
	return { buffer, blits };
}
Vulkan::ImageHandle Vulkan::LogicalDevice::create_image_with_staging_buffer(const ImageInfo& info, const ImageBuffer* initialData)
{

	std::array<uint32_t, 3> queueFamilyIndices;
	VkImageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = info.type,
		.format = info.format,
		.extent {
			.width = info.width,
			.height = info.height,
			.depth = info.depth
		},
		.mipLevels = info.generate_mips()? calculate_mip_levels(info.width, info.height, info.depth) : info.mipLevels,
		.arrayLayers = info.arrayLayers,
		.samples = info.samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = info.usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = queueFamilyIndices.data(),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	if (initialData)
		createInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocInfo{
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
	};
	if (info.concurrent_queue()) {
		createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		if (info.createFlags.any_of(ImageInfo::Flags::ConcurrentGraphics, ImageInfo::Flags::ConcurrentAsyncGraphics))
			queueFamilyIndices[createInfo.queueFamilyIndexCount++] = m_graphics.familyIndex;
		if (info.createFlags.any_of(ImageInfo::Flags::ConcurrentAsyncCompute) &&
			(!(createInfo.queueFamilyIndexCount != 0) || (m_graphics.familyIndex != m_compute.familyIndex)))
			queueFamilyIndices[createInfo.queueFamilyIndexCount++] = m_compute.familyIndex;
		if (info.createFlags.any_of(ImageInfo::Flags::ConcurrentAsyncTransfer) &&
			(!(createInfo.queueFamilyIndexCount != 0) || (m_graphics.familyIndex != m_transfer.familyIndex)))
			queueFamilyIndices[createInfo.queueFamilyIndexCount++] = m_transfer.familyIndex;
		if(createInfo.queueFamilyIndexCount == 1)
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	VmaAllocation allocation;
	VkImage image;
	if (auto result = vmaCreateImage(get_vma_allocator()->get_handle(), &createInfo, &allocInfo, &image, &allocation, nullptr); result != VK_SUCCESS) {
		throw std::runtime_error("Vk: error creating image");
	}
	bool needsView = (createInfo.usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
									VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT));
	auto tmp = info;
	tmp.mipLevels = createInfo.mipLevels;
	auto handle(m_imagePool.emplace(*this, image, tmp, allocation));
	handle->set_stage_flags(Image::possible_stages_from_image_usage(createInfo.usage));
	handle->set_access_flags(Image::possible_access_from_image_usage(createInfo.usage));
	if (initialData) {

		VkAccessFlags finalTransitionSrcAccess = info.generate_mips() ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_TRANSFER_WRITE_BIT;
		VkAccessFlags prepareSrcAccess = m_graphics.queue == m_transfer.queue ? VK_ACCESS_TRANSFER_WRITE_BIT : 0;
		
		bool needMipBarrier = true;
		bool needInitialBarrier = true;

		auto graphicsCmd = request_command_buffer(CommandBuffer::Type::Generic);
		auto transferCmd = graphicsCmd;
		if (m_transfer.queue != m_graphics.queue)
			transferCmd = request_command_buffer(CommandBuffer::Type::Transfer);

		transferCmd->image_barrier(*handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
									VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
		transferCmd->copy_buffer_to_image(*handle, *initialData->buffer, initialData->blits.size(), initialData->blits.data());

		if (m_transfer.queue != m_graphics.queue) {
			VkPipelineStageFlags dstStages = info.generate_mips() ? VK_PIPELINE_STAGE_TRANSFER_BIT : handle->get_stage_flags();
			if (!info.concurrent_queue() && m_transfer.familyIndex != m_graphics.familyIndex) {
				needMipBarrier = false;

				VkImageMemoryBarrier release{
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.dstAccessMask = 0,
					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.newLayout = info.generate_mips() ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : info.layout,
					.srcQueueFamilyIndex = m_transfer.familyIndex,
					.dstQueueFamilyIndex = m_graphics.familyIndex,
					.image = handle->get_handle(),
					.subresourceRange {
						.aspectMask = ImageInfo::format_to_aspect_mask(createInfo.format),
						.levelCount = info.generate_mips() ? 1: createInfo.mipLevels,
						.layerCount = createInfo.arrayLayers,
					}
				};
				needInitialBarrier = info.generate_mips();

				auto acquire = release;
				acquire.srcAccessMask = 0;
				acquire.dstAccessMask = info.generate_mips() ? VK_ACCESS_TRANSFER_READ_BIT :
					handle->get_access_flags() & Image::possible_access_from_image_layout(info.layout);
				transferCmd->barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
										0, nullptr, 0, nullptr, 1, &release);
				graphicsCmd->barrier(dstStages, dstStages,
										0, nullptr, 0, nullptr, 1, &acquire);
			}
			VkSemaphore sem = VK_NULL_HANDLE;
			submit(transferCmd, 1, &sem);
			add_wait_semaphore(graphicsCmd->get_type(), sem, dstStages);
		}
		if (info.generate_mips()) {
			graphicsCmd->mip_barrier(*handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
									VK_PIPELINE_STAGE_TRANSFER_BIT, needMipBarrier);
			graphicsCmd->generate_mips(*handle);
		}
		if (needInitialBarrier) {
			graphicsCmd->image_barrier(*handle, info.generate_mips() ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				info.layout, VK_PIPELINE_STAGE_TRANSFER_BIT, finalTransitionSrcAccess, handle->get_stage_flags(),
				handle->get_stage_flags() & Image::possible_access_from_image_layout(info.layout));
		}
		//Todo
		submit(graphicsCmd);
	}
	else if (info.layout != VK_IMAGE_LAYOUT_UNDEFINED) {
		//assert(false);
	}
	return handle;
}
std::vector<Vulkan::CommandBufferHandle>& Vulkan::LogicalDevice::get_current_submissions(CommandBuffer::Type type)
{
	switch (type) {
	case CommandBuffer::Type::Generic:
		return frame().submittedGraphicsCmds;
	case CommandBuffer::Type::Transfer:
		return frame().submittedTransferCmds;
	case CommandBuffer::Type::Compute:
		return frame().submittedComputeCmds;
	}
}

Vulkan::CommandPool& Vulkan::LogicalDevice::get_pool(uint32_t threadId, CommandBuffer::Type type)
{
	switch (type) {
	case CommandBuffer::Type::Compute:
		return frame().computePool[threadId];
	case CommandBuffer::Type::Generic:
		return frame().graphicsPool[threadId];
	case CommandBuffer::Type::Transfer:
		return frame().transferPool[threadId];
	}
}


void Vulkan::LogicalDevice::create_vma_allocator()
{
	#ifdef VMA_RECORDING_ENABLED
	VmaRecordSettings vmaRecordSettings{
		.flags = VMA_RECORD_FLUSH_AFTER_CALL_BIT,
		.pFilePath = "vma_replay.csv",
	};
	VmaAllocatorCreateInfo allocatorInfo{
		.physicalDevice = r_instance.m_physicalDevice,
		.device = m_device,
		.pRecordSettings = &vmaRecordSettings,
		.instance = r_instance.m_instance,
	};
	#else
	VmaAllocatorCreateInfo allocatorInfo{
		.physicalDevice = r_instance.m_physicalDevice,
		.device = m_device,
		.pRecordSettings = nullptr,
		.instance = r_instance.m_instance,
	};
	#endif
	VmaAllocator allocator;
	if (auto result = vmaCreateAllocator(&allocatorInfo, &allocator); result != VK_SUCCESS) {
		throw std::runtime_error("VMA: could not create allocator");
	}
	m_vmaAllocator = std::make_unique<Allocator>(allocator);
}

Vulkan::BufferHandle Vulkan::LogicalDevice::create_buffer(const BufferInfo& info,const void* initialData)
{
	assert(info.memoryUsage != VMA_MEMORY_USAGE_UNKNOWN);
	VkBuffer buffer;
	VmaAllocation allocation;

	auto usage = info.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	std::array<uint32_t, 3> sharing;
	VkBufferCreateInfo bufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = info.size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = sharing.data()
	};
	if (m_graphics.familyIndex != m_compute.familyIndex ||
		m_graphics.familyIndex != m_transfer.familyIndex) {
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		sharing[bufferCreateInfo.queueFamilyIndexCount++] = m_graphics.familyIndex;
		if (m_graphics.familyIndex != m_compute.familyIndex)
			sharing[bufferCreateInfo.queueFamilyIndexCount++] = m_compute.familyIndex;
		if (m_graphics.familyIndex != m_transfer.familyIndex)
			sharing[bufferCreateInfo.queueFamilyIndexCount++] = m_transfer.familyIndex;
	}
	VmaAllocationCreateInfo allocInfo{
		.usage = info.memoryUsage
	};
	vmaCreateBuffer(m_vmaAllocator->get_handle(), &bufferCreateInfo, &allocInfo, &buffer, &allocation, nullptr);
	BufferInfo tmp = info;
	tmp.usage = usage;
	auto handle(m_bufferPool.emplace(*this, buffer, allocation, tmp));
	if (initialData != nullptr && (info.memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY)) {
		//Need to stage
		tmp.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		auto stagingBuffer = create_buffer(tmp,initialData);
		auto cmd = request_command_buffer(CommandBuffer::Type::Transfer);
		cmd->copy_buffer(*handle, *stagingBuffer);
		submit_staging(cmd, info.usage, true);
	}
	else if (initialData != nullptr) {
		//No staging needed
		auto map = handle->map_data();
		std::memcpy(map.get(), initialData, info.size);
	}
	return handle;
}


Vulkan::ImageViewHandle Vulkan::LogicalDevice::create_image_view(const ImageViewCreateInfo& info)
{
	return m_imageViewPool.emplace(*this,info);
}

Vulkan::ImageHandle Vulkan::LogicalDevice::create_image(const ImageInfo& info, const void* data, uint32_t rowLength, uint32_t height)
{
	if (data) {
		auto buf = create_staging_buffer(info, data, rowLength, height);
		return create_image_with_staging_buffer(info, &buf);
	}
	else {
		return create_image_with_staging_buffer(info, nullptr);
	}
}



Vulkan::DescriptorSetAllocator* Vulkan::LogicalDevice::request_descriptor_set_allocator(const DescriptorSetLayout& layout)
{

	//std::hash <std::pair < DescriptorSetLayout, std::array<uint32_t, MAX_BINDINGS>>> h{};
	
	
	if (!m_descriptorAllocatorIds.contains(layout)) {
		m_descriptorAllocatorIds.emplace(layout, m_descriptorAllocatorsStorage.emplace_intrusive(*this, layout));
	}
	return m_descriptorAllocatorsStorage.get_ptr(m_descriptorAllocatorIds.at(layout));
}

size_t Vulkan::LogicalDevice::register_shader(const std::vector<uint32_t>& shaderCode)
{
	return m_shaderStorage.emplace_intrusive(*this, shaderCode);
}


Vulkan::Shader* Vulkan::LogicalDevice::request_shader(size_t id)
{
	return m_shaderStorage.get_ptr(id);
}

Vulkan::Program* Vulkan::LogicalDevice::request_program(const std::vector<Shader*>& shaders)
{
	
	if (!m_programIds.contains(shaders)) {
		m_programIds.emplace(shaders, m_programStorage.emplace_intrusive(shaders));
	}
	return m_programStorage.get_ptr(m_programIds.at(shaders));
}

Vulkan::PipelineLayout* Vulkan::LogicalDevice::request_pipeline_layout(const ShaderLayout& layout)
{
	
	if (!m_pipelineLayoutIds.contains(layout)) {
		m_pipelineLayoutIds.emplace(layout, m_pipelineLayoutStorage.emplace_intrusive(*this, layout));
	}
	return m_pipelineLayoutStorage.get_ptr(m_pipelineLayoutIds.at(layout));
}

Vulkan::Renderpass* Vulkan::LogicalDevice::request_render_pass(const RenderpassCreateInfo& info)
{
	auto [compatibleHash, actualHash] = info.get_hash();
	auto result = m_renderpassIds.find(actualHash);
	size_t renderpassId = 0;
	if (result == m_renderpassIds.end()) {
		renderpassId = m_renderpassStorage.emplace_intrusive(*this, info);
		m_renderpassIds[actualHash] = renderpassId;
		if (m_compatibleRenderpassIds.find(compatibleHash) == m_compatibleRenderpassIds.end())
			m_compatibleRenderpassIds[compatibleHash] = renderpassId;
	}
	else {
		renderpassId = result->second;
	}
	return m_renderpassStorage.get_ptr(renderpassId);
}

Vulkan::Renderpass* Vulkan::LogicalDevice::request_compatible_render_pass(const RenderpassCreateInfo& info)
{
	auto [compatibleHash, actualHash] = info.get_hash();

	auto result = m_compatibleRenderpassIds.find(compatibleHash);
	size_t renderpassId = 0;
	if (m_compatibleRenderpassIds.find(compatibleHash) == m_compatibleRenderpassIds.end()) {
		renderpassId = m_renderpassStorage.emplace_intrusive(*this, info);
		m_compatibleRenderpassIds[compatibleHash] = renderpassId;

		m_renderpassIds[actualHash] = renderpassId;
	}
	else {
		renderpassId = result->second;
	}
	return m_renderpassStorage.get_ptr(renderpassId);
}

VkPipeline Vulkan::LogicalDevice::request_pipeline(const PipelineCompile& compile) noexcept
{
	return m_pipelineStorage.request_pipeline(compile);
}

Vulkan::RenderpassCreateInfo Vulkan::LogicalDevice::request_swapchain_render_pass() noexcept
{
	RenderpassCreateInfo info;
	auto* swapchainView = get_swapchain_image_view();
	info.colorAttachmentsCount = 1;
	info.clearAttachments.set(0);
	info.storeAttachments.set(0);
	info.colorAttachmentsViews[0] = swapchainView;
	info.clearColors[0] = VkClearColorValue{
		.float32 = {0.0f, 0.0f, 0.0f, 1.0f}
	};
	info.clearDepthStencil = VkClearDepthStencilValue{
		.depth = 1.0f, .stencil = 0
	};
	uint32_t width = swapchainView->get_image()->get_width();
	uint32_t height = swapchainView->get_image()->get_height();
	info.depthStencilAttachment = request_render_target(width, height, VK_FORMAT_D16_UNORM);
	info.opFlags.set(static_cast<uint32_t>(RenderpassCreateInfo::OpFlags::DepthStencilClear));

	return info;
}

Vulkan::Framebuffer* Vulkan::LogicalDevice::request_framebuffer(const RenderpassCreateInfo& info)
{
	return m_framebufferAllocator.request_framebuffer(info);
}

VkSemaphore Vulkan::LogicalDevice::request_semaphore()
{
	return m_semaphoreManager.request_semaphore();
}

Vulkan::CommandBufferHandle Vulkan::LogicalDevice::request_command_buffer(CommandBuffer::Type type)
{
	auto cmd = get_pool(get_thread_index(),type).request_command_buffer();
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(cmd, &beginInfo);
	return m_commandBufferPool.emplace(*this, cmd, type, get_thread_index());
}

Vulkan::ImageView* Vulkan::LogicalDevice::request_render_target(uint32_t width, uint32_t height, VkFormat format, uint32_t index, VkSampleCountFlagBits sampleCount)
{
	return m_attachmentAllocator.request_attachment(width, height, format, index, sampleCount);
}

void Vulkan::LogicalDevice::resize_buffer(Buffer& buffer, VkDeviceSize newSize, bool copyData)
{
	auto info = buffer.get_info();
	info.size = newSize;
	auto stagingBuffer = create_buffer(info, nullptr);
	if (copyData) {
		auto cmd = request_command_buffer(CommandBuffer::Type::Transfer);
		cmd->copy_buffer(*stagingBuffer , buffer);
		submit_staging(cmd, info.usage, true);
	}
	buffer.swap_contents(*stagingBuffer);
}




VkSemaphore Vulkan::LogicalDevice::get_present_semaphore()
{
	auto tempSem = m_wsiState.present;
	m_wsiState.present = VK_NULL_HANDLE;
	frame().recycledSemaphores.push_back(tempSem);
	return tempSem;
}

bool Vulkan::LogicalDevice::swapchain_touched()  const noexcept
{
	return m_wsiState.swapchain_touched;
}

VkQueue Vulkan::LogicalDevice::get_graphics_queue()  const noexcept
{
	return m_graphics.queue;
}


void Vulkan::LogicalDevice::update_uniform_buffer()
{
	auto currentImage = get_swapchain_image_index();
	Ubo ubo[2]{ {
		Math::mat44::identity(),
		Math::mat44::identity(),
		Math::mat44::identity()
	} ,{
		Math::mat44::identity(),
		Math::mat44::identity(),
		Math::mat44::identity()
	} };
	static float degrees;
	ubo[0].model = Math::mat44(Math::mat33::rotation_matrix(0, 0, (degrees++) / 100));
	ubo[0].model(3, 3) = 1;
	ubo[0].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
	ubo[0].proj = Math::mat44::perspective(0.01f, 10.f, 45.0f, static_cast<float>(get_swapchain_image_view()->get_image()->get_width()) / static_cast<float>(get_swapchain_image_view()->get_image()->get_height()));
	ubo[1].model = Math::mat44(Math::mat33::rotation_matrix(0, 0, (degrees++) / 100));
	ubo[1].model(3, 3) = 1;
	ubo[1].model = ubo[1].model * Math::mat44::translation_matrix(Math::vec3({ 0.0f, 0.0f, -0.5f }));
	ubo[1].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
	ubo[1].proj = Math::mat44::perspective(0.01f, 10.f, 45.0f, static_cast<float>(get_swapchain_image_view()->get_image()->get_width()) / static_cast<float>(get_swapchain_image_view()->get_image()->get_height()));
	void* mappedData;
	m_vmaAllocator->map_memory(m_uniformBuffersAllocations[currentImage], &mappedData);
	memcpy(mappedData, &ubo, sizeof(ubo));
	m_vmaAllocator->unmap_memory(m_uniformBuffersAllocations[currentImage]);
	m_vmaAllocator->flush(m_uniformBuffersAllocations[currentImage], 0, sizeof(ubo));
}

void Vulkan::LogicalDevice::create_default_sampler()
{
	SamplerCreateInfo createInfo{
		//.
	};
	createInfo.maxLod = VK_LOD_CLAMP_NONE;
	createInfo.maxAnisotropy = 1.0f;
	for (int i = 0; i < static_cast<int>(DefaultSampler::Size); i++) {
		auto type = static_cast<DefaultSampler>(i);
		switch (type) {
		case DefaultSampler::NearestShadow:
		case DefaultSampler::LinearShadow:
			createInfo.compareEnable = VK_TRUE;
			createInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			break;
		default:
			createInfo.compareEnable = VK_FALSE;
			break;
		}

		switch (type) {
		case DefaultSampler::TrilinearClamp:
		case DefaultSampler::TrilinearWrap:
			createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		default:
			createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		}

		switch (type) {
		case DefaultSampler::LinearClamp:
		case DefaultSampler::LinearWrap:
		case DefaultSampler::TrilinearClamp:
		case DefaultSampler::TrilinearWrap:
		case DefaultSampler::LinearShadow:
			createInfo.magFilter = VK_FILTER_LINEAR;
			createInfo.minFilter = VK_FILTER_LINEAR;
			break;
		default:
			createInfo.magFilter = VK_FILTER_NEAREST;
			createInfo.minFilter = VK_FILTER_NEAREST;
			break;
		}
		switch (type) {
		case DefaultSampler::LinearShadow:
		case DefaultSampler::NearestShadow:
		case DefaultSampler::TrilinearClamp:
		case DefaultSampler::LinearClamp:
		case DefaultSampler::NearestClamp:
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case DefaultSampler::LinearWrap:
		case DefaultSampler::TrilinearWrap:
		case DefaultSampler::NearestWrap:
		default:
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		}
		m_defaultSampler[i] = std::make_unique<Sampler>(*this, createInfo);
	}
}

Vulkan::Sampler* Vulkan::LogicalDevice::get_default_sampler(DefaultSampler samplerType) const noexcept
{
	assert(samplerType != DefaultSampler::Size);
	return m_defaultSampler[static_cast<size_t>(samplerType)].get();
}

void Vulkan::LogicalDevice::wait_idle()
{
	vkDeviceWaitIdle(m_device);
}


Vulkan::LogicalDevice::FrameResource::~FrameResource()
{
	begin();
}

void Vulkan::LogicalDevice::FrameResource::begin()
{
	if (!waitForFences.empty()) {
		std::vector<VkFence> fences;
		fences.reserve(waitForFences.size());
		for (auto &fence : waitForFences)
			fences.push_back(fence);
		vkWaitForFences(r_device.get_device(), static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX);
		waitForFences.clear();
	}
	for (auto& pool : graphicsPool) {
		assert(submittedGraphicsCmds.empty());
		pool.reset();
	}
	for (auto& pool : computePool) {
		assert(submittedComputeCmds.empty());
		pool.reset();
	}
	for (auto& pool : transferPool) {
		assert(submittedTransferCmds.empty());
		pool.reset();
	}
	for (auto fb : deletedFramebuffer) {
		vkDestroyFramebuffer(r_device.get_device(), fb, r_device.get_allocator());
	}
	deletedFramebuffer.clear();

	for (auto semaphore : deletedSemaphores) {
		vkDestroySemaphore(r_device.get_device(), semaphore, r_device.get_allocator());
	}
	deletedSemaphores.clear();

	for (auto semaphore : recycledSemaphores) {
		r_device.m_semaphoreManager.recycle_semaphore(semaphore);
	}
	recycledSemaphores.clear();

	for (auto [buffer, allocation] : deletedBufferAllocations) {
		vmaDestroyBuffer(r_device.get_vma_allocator()->get_handle(), buffer, allocation);
	}
	deletedBufferAllocations.clear();
	for (auto [image, allocation] : deletedImageAllocations) {
		vmaDestroyImage(r_device.get_vma_allocator()->get_handle(), image, allocation);
	}
	deletedImageAllocations.clear();

	for (auto image : deletedImages) {
		vkDestroyImage(r_device.get_device(), image, r_device.get_allocator());
	}
	deletedImages.clear();
	for (auto sampler : deletedSampler) {
		vkDestroySampler(r_device.get_device(), sampler, r_device.get_allocator());
	}
	deletedSampler.clear();
	for (auto imageView : deletedImageViews) {
		vkDestroyImageView(r_device.get_device(), imageView, r_device.get_allocator());
	}
	deletedImageViews.clear();

	for (auto bufferView : deletedBufferViews) {
		vkDestroyBufferView(r_device.get_device(), bufferView, r_device.get_allocator());
	}
	deletedBufferViews.clear();
}
