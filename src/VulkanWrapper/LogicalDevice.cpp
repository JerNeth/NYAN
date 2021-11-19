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
vulkan::LogicalDevice::LogicalDevice(const vulkan::Instance& parentInstance, VkDevice device, uint32_t graphicsFamilyQueueIndex,
					uint32_t computeFamilyQueueIndex, uint32_t transferFamilyQueueIndex, VkPhysicalDevice physicalDevice) :
	r_instance(parentInstance),
	m_device(device, nullptr),
	m_physicalDevice(physicalDevice),
	m_graphics(graphicsFamilyQueueIndex),
	m_compute(computeFamilyQueueIndex == ~0 ? graphicsFamilyQueueIndex : computeFamilyQueueIndex),
	m_transfer(transferFamilyQueueIndex == ~0 ? graphicsFamilyQueueIndex : transferFamilyQueueIndex),
	m_framebufferAllocator(*this),
	m_fenceManager(*this),
	m_semaphoreManager(*this),
	m_pipelineStorage(*this),
	m_attachmentAllocator(*this)
{
	volkLoadDevice(device);
	vkGetDeviceQueue(m_device, m_graphics.familyIndex, 0, &m_graphics.queue);
	vkGetDeviceQueue(m_device, m_compute.familyIndex, 0, &m_compute.queue);
	vkGetDeviceQueue(m_device, m_transfer.familyIndex, 0, &m_transfer.queue);
	create_vma_allocator();
	create_default_sampler();	
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalProperties);
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(physicalDevice,nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensions(count);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data());
	for (auto& extension : extensions) {
		#ifdef VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME
		if (strcmp(extension.extensionName, VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME) == 0) {
			m_supportedExtensions.descriptor_update_template = 1;
		}
		else 
		#endif
		#ifdef VK_KHR_SWAPCHAIN_EXTENSION_NAME
		if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
			m_supportedExtensions.swapchain = 1;
		}
		else 
		#endif
		#ifdef VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
		if (strcmp(extension.extensionName, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) == 0) {
			m_supportedExtensions.timeline_semaphore = 1;
		}
		else 
		#endif
		#ifdef VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME
		if (strcmp(extension.extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0) {
			m_supportedExtensions.fullscreen_exclusive = 1;
		}
		else 
		#endif
		#ifdef VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
		if (strcmp(extension.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0) {
			m_supportedExtensions.extended_dynamic_state = 1;
		}
		else 
		#endif
		#ifdef VK_EXT_DEBUG_MARKER_EXTENSION_NAME
		if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
			m_supportedExtensions.debug_marker = 1;
		}
		else 
		#endif
		#ifdef VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		if (strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
			m_supportedExtensions.debug_utils = 1;
		}
		#endif
	}
	assert(m_supportedExtensions.swapchain);
	m_frameResources.reserve(MAX_FRAMES_IN_FLIGHT);
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_frameResources.emplace_back(new FrameResource{*this});
	}
	m_physicalFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	m_physicalFeatures.pNext = nullptr;
	vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_physicalFeatures);
}
vulkan::LogicalDevice::~LogicalDevice()
{
	if (m_wsiState.aquire != VK_NULL_HANDLE) {
		vkDestroySemaphore(get_device(), m_wsiState.aquire, get_allocator());
	}
	if (m_wsiState.present != VK_NULL_HANDLE) {
		vkDestroySemaphore(get_device(), m_wsiState.present, get_allocator());
	}
}

void vulkan::LogicalDevice::set_acquire_semaphore(uint32_t index, VkSemaphore semaphore) noexcept
{
	if (m_wsiState.aquire != VK_NULL_HANDLE)
		frame().recycledSemaphores.push_back(m_wsiState.aquire);
	m_wsiState.aquire = semaphore;
	m_wsiState.swapchain_touched = false;
	m_wsiState.index = index;
}

void vulkan::LogicalDevice::init_swapchain(const std::vector<VkImage>& swapchainImages, uint32_t width, uint32_t height, VkFormat format)
{
	m_wsiState.swapchainImages.clear();
	wait_idle();
	auto info = ImageInfo::render_target(width, height, format);
	info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	info.isSwapchainImage = true;
	assert(m_wsiState.aquire == VK_NULL_HANDLE);
	assert(m_wsiState.present == VK_NULL_HANDLE);

	m_wsiState.swapchain_touched = false;
	m_wsiState.index = 0u;

	for (const auto image : swapchainImages) {
		std::vector<AllocationHandle> allocs;
		auto handle = m_imagePool.emplace(*this, image, info, allocs);
		handle->disown();
		handle->set_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		m_wsiState.swapchainImages.emplace_back(handle);
	}

}

const vulkan::ImageView* vulkan::LogicalDevice::get_swapchain_image_view() const noexcept
{
	return m_wsiState.swapchainImages[m_wsiState.index]->get_view();
}

vulkan::ImageView* vulkan::LogicalDevice::get_swapchain_image_view() noexcept
{
	return m_wsiState.swapchainImages[m_wsiState.index]->get_view();
}

void vulkan::LogicalDevice::next_frame()
{
	end_frame();

	if (!m_fenceCallbacks.empty()) {
		auto range = m_fenceCallbacks.equal_range(m_fenceCallbacks.begin()->first);
		std::vector<VkFence> clear;
		for (auto it = m_fenceCallbacks.begin(); it != m_fenceCallbacks.end(); it = range.second) {
			range = m_fenceCallbacks.equal_range(it->first);
			if (vkGetFenceStatus(m_device, range.first->first) == VK_SUCCESS) {
				for (auto cb = range.first; cb != range.second; cb++) {
					cb->second();
				}
				clear.push_back(range.first->first);
			}
		}
		for (auto fence : clear) {
			auto range = m_fenceCallbacks.equal_range(fence);
			m_fenceCallbacks.erase(range.first, range.second);
			m_fenceManager.reset_fence(fence);
		}
	}

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

void vulkan::LogicalDevice::end_frame()
{
	frame().deletedSemaphores.insert(frame().deletedSemaphores.end(), frame().signalSemaphores.cbegin(), frame().signalSemaphores.cend());
	frame().signalSemaphores.clear();
	if (m_transfer.needsFence|| !frame().submittedTransferCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		VkSemaphore sem = VK_NULL_HANDLE;
		submit_queue(CommandBuffer::Type::Transfer, &fence, 1, &sem);
		frame().waitForFences.emplace_back(std::move(fence));
		frame().signalSemaphores.push_back(sem);
		m_transfer.needsFence = false;
	}
	if (m_graphics.needsFence || !frame().submittedGraphicsCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		VkSemaphore sem = VK_NULL_HANDLE;
		submit_queue(CommandBuffer::Type::Generic, &fence, 1, &sem);
		frame().waitForFences.emplace_back(std::move(fence));
		frame().signalSemaphores.push_back(sem);
		m_graphics.needsFence = false;
	}
	if (m_compute.needsFence || !frame().submittedComputeCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		VkSemaphore sem = VK_NULL_HANDLE;
		submit_queue(CommandBuffer::Type::Compute, &fence, 1, &sem);
		frame().waitForFences.emplace_back(std::move(fence));
		frame().signalSemaphores.push_back(sem);
		m_compute.needsFence = false;
	}
	//m_swapchains[0]->present_queue();
}

void vulkan::LogicalDevice::submit_queue(CommandBuffer::Type type, FenceHandle* fence, uint32_t semaphoreCount, VkSemaphore* semaphores)
{
	auto& queue = get_queue(type);
	auto& submissions = get_current_submissions(type);
	if (submissions.empty()) {
		if (fence || semaphoreCount)
			submit_empty(type, fence, semaphoreCount, semaphores);
		return;
	}
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
#ifdef DEBUGSUBMISSIONS
	for (auto i = 0; i < submitCounts; i++) {
		std::cout << "Submitted :";
		for (auto cmd = 0; cmd < submitInfos[i].commandBufferCount; cmd++) {
			std::cout << submitInfos[i].pCommandBuffers[cmd] << " ";
		}
		std::cout << "\n\tWaits:";
		for (auto j = 0; j < submitInfos[i].waitSemaphoreCount; j++) {
			std::cout << submitInfos[i].pWaitSemaphores[j] << " ";
		}
		std::cout << "\n\tSignals:";
		for (auto j = 0; j < submitInfos[i].signalSemaphoreCount; j++) {
			std::cout << submitInfos[i].pSignalSemaphores[j] << " ";
		}
		std::cout << '\n';
	}
#endif //  0
	VkFence localFence = ((fence != nullptr) ? fence->get_handle() : VK_NULL_HANDLE);
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

void vulkan::LogicalDevice::queue_framebuffer_deletion(VkFramebuffer framebuffer) noexcept
{
	auto& curFrame = frame();
	if (auto res = std::find(curFrame.deletedFramebuffer.cbegin(), curFrame.deletedFramebuffer.cend(), framebuffer); res == curFrame.deletedFramebuffer.cend())
		curFrame.deletedFramebuffer.push_back(framebuffer);
}

void vulkan::LogicalDevice::queue_image_deletion(VkImage image) noexcept
{
	frame().deletedImages.push_back(image);
}

void vulkan::LogicalDevice::queue_image_view_deletion(VkImageView imageView) noexcept
{
	frame().deletedImageViews.push_back(imageView);
}

void vulkan::LogicalDevice::queue_buffer_view_deletion(VkBufferView bufferView) noexcept
{
	frame().deletedBufferViews.push_back(bufferView);
}

void vulkan::LogicalDevice::queue_image_sampler_deletion(VkSampler sampler) noexcept
{
	frame().deletedSampler.push_back(sampler);
}

void vulkan::LogicalDevice::queue_buffer_deletion(VkBuffer buffer) noexcept
{
	frame().deletedBuffer.push_back(buffer);
}

void vulkan::LogicalDevice::queue_allocation_deletion(VmaAllocation allocation) noexcept
{
	frame().deletedAllocations.push_back(allocation);
}

void vulkan::LogicalDevice::add_wait_semaphore(CommandBuffer::Type type, VkSemaphore semaphore, VkPipelineStageFlags stages, bool flush)
{
	assert(stages != 0);
	if (flush)
		submit_queue(type, nullptr);
	auto& queue = get_queue(type);
	queue.waitSemaphores.push_back(semaphore);
	queue.waitStages.push_back(stages);
	queue.needsFence = true;
}

void vulkan::LogicalDevice::submit_empty(CommandBuffer::Type type, FenceHandle* fence, uint32_t semaphoreCount, VkSemaphore* semaphores)
{
	auto& queue = get_queue(type);
	
	std::vector<VkSemaphore> signalSemaphores;
	std::vector<VkSemaphore> waitSemaphores;
	auto waitStages = queue.waitStages;
	queue.waitStages.clear();
	for (auto semaphore : queue.waitSemaphores) {
		frame().recycledSemaphores.push_back(semaphore);
		waitSemaphores.push_back(semaphore);
	}
	queue.waitSemaphores.clear();
	for (uint32_t i = 0; i < semaphoreCount; i++) {
		auto semaphore = m_semaphoreManager.request_semaphore();
		assert(semaphores[i] == VK_NULL_HANDLE);
		semaphores[i] = semaphore;
		signalSemaphores.push_back(semaphore);
	}

	if (fence)
		*fence = m_fenceManager.request_fence();
	VkFence localFence = ((fence != nullptr) ? fence->get_handle() : VK_NULL_HANDLE);
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
		.pWaitSemaphores = waitSemaphores.data(),
		.pWaitDstStageMask = waitStages.data(),
		.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
		.pSignalSemaphores = signalSemaphores.data()
	};
	vkQueueSubmit(queue.queue, 1, &submitInfo, localFence);
}

void vulkan::LogicalDevice::submit_staging(CommandBufferHandle cmd, VkBufferUsageFlags usage, bool flush)
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

void vulkan::LogicalDevice::submit(CommandBufferHandle cmd, uint32_t semaphoreCount, VkSemaphore* semaphores, vulkan::FenceHandle* fence)
{
	auto type = cmd->get_type();
	auto& submissions = get_current_submissions(type);
	cmd->end();
	submissions.push_back(cmd);
	if (semaphoreCount || fence) {
		submit_queue(type, fence, semaphoreCount, semaphores);
	}
}

void vulkan::LogicalDevice::wait_no_lock() noexcept
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

void vulkan::LogicalDevice::clear_semaphores() noexcept
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

void vulkan::LogicalDevice::add_fence_callback(VkFence fence, std::function<void(void)> callback)
{
	m_fenceCallbacks.emplace(fence, callback);
}

vulkan::LogicalDevice::FrameResource& vulkan::LogicalDevice::frame()
{
	return *m_frameResources[m_currentFrame];
}
vulkan::LogicalDevice::ImageBuffer vulkan::LogicalDevice::create_staging_buffer(const ImageInfo& info, InitialImageData* initialData, uint32_t baseMipLevel)
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
		.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY
	};
	auto buffer = create_buffer(bufferInfo);
	auto map = buffer->map_data();
	std::vector<VkBufferImageCopy> blits;
	blits.reserve(copyLevels-Math::min(baseMipLevel, copyLevels));
	for (uint32_t level = baseMipLevel; level < copyLevels; level++) {
		const auto& mip = mipInfo.mipLevels[level];
		blits.push_back(VkBufferImageCopy{
			.bufferOffset = mip.offset - mipInfo.mipLevels[baseMipLevel].offset,
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
		size_t dstLayerSize = mip.blockCountY * rowSize;

		uint32_t srcRowStride = (mip.width + blockSizeX - 1) / blockSizeX * blockStride;
		uint32_t srcLayerStride = (mip.height + blockSizeY - 1) / blockSizeY * srcRowStride;
		for (uint32_t arrayLayer = 0; arrayLayer < info.arrayLayers; arrayLayer++) {
			uint8_t* dst = static_cast<uint8_t*>(map) + (mip.offset - mipInfo.mipLevels[baseMipLevel].offset) + arrayLayer*dstLayerSize;
			const uint8_t* src = static_cast<const uint8_t*>(initialData[arrayLayer].data) + initialData[arrayLayer].mipOffsets[level];
			//std::cout << "Mip offset: " << mip.offset  << " initial offset: " << initialData[arrayLayer].mipOffsets[level] <<"\n";
			for(uint32_t z = 0; z < mip.depth; z++)
				for (uint32_t y = 0; y < mip.blockCountY; y++)
					memcpy(dst + z* dstLayerSize+ y * rowSize, src + z * srcLayerStride + y * srcRowStride, rowSize);
		}
		
	}
	return { buffer, blits };
}
vulkan::ImageHandle vulkan::LogicalDevice::create_image(const ImageInfo& info, VkImageUsageFlags usage)
{

	std::array<uint32_t, 3> queueFamilyIndices;
	assert(!(info.flags & (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT)));
	VkImageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = info.flags,
		.imageType = info.type,
		.format = info.format,
		.extent {
			.width = info.width,
			.height = info.height,
			.depth = info.depth
		},
		.mipLevels = info.generate_mips() ? calculate_mip_levels(info.width, info.height, info.depth) : info.mipLevels,
		.arrayLayers = info.arrayLayers,
		.samples = info.samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = info.usage | usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = queueFamilyIndices.data(),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
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
		if (createInfo.queueFamilyIndexCount == 1)
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	VmaAllocation allocation;
	VkImage image;
	VmaAllocationCreateInfo allocInfo{
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
	};
	if (auto result = vmaCreateImage(get_vma_allocator()->get_handle(), &createInfo, &allocInfo, &image, &allocation, nullptr); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could create image, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could create image, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	auto tmp = info;
	tmp.mipLevels = createInfo.mipLevels;
	std::vector<AllocationHandle> allocs{ m_allocationPool.emplace(*this, allocation) };
	auto handle(m_imagePool.emplace(*this, image, tmp, allocs));
	handle->set_stage_flags(Image::possible_stages_from_image_usage(createInfo.usage));
	handle->set_access_flags(Image::possible_access_from_image_usage(createInfo.usage));


	return handle;
}
vulkan::ImageHandle vulkan::LogicalDevice::create_sparse_image(const ImageInfo& info, VkImageUsageFlags usage)
{
	std::array<uint32_t, 3> queueFamilyIndices;
	assert(supports_sparse_textures());
	assert(info.flags & (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT));
	VkImageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = info.flags ,
		.imageType = info.type,
		.format = info.format,
		.extent {
			.width = info.width,
			.height = info.height,
			.depth = info.depth
		},
		.mipLevels = info.mipLevels,
		.arrayLayers = info.arrayLayers,
		.samples = info.samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = info.usage | usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = queueFamilyIndices.data(),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	assert(createInfo.mipLevels);
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
		if (createInfo.queueFamilyIndexCount == 1)
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	VkImage image;
	if (auto result = vkCreateImage(m_device, &createInfo, m_allocator, &image); result != VK_SUCCESS) {
		throw std::runtime_error("Vk: error creating image");
	}

	VmaAllocationCreateInfo allocCreateInfo{
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
	};
	VkSparseImageMemoryRequirements sparseMemoryRequirement = get_sparse_memory_requirements(image, ImageInfo::format_to_aspect_mask(createInfo.format));
	uint32_t mipTailBegin = sparseMemoryRequirement.imageMipTailFirstLod;
	std::vector< VkSparseMemoryBind> mipTailBinds;
	bool singleMipTail = (sparseMemoryRequirement.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT);
	
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_device, image, &memoryRequirements);
	assert(memoryRequirements.size <= m_physicalProperties.limits.sparseAddressSpaceSize);
	assert((memoryRequirements.size % memoryRequirements.alignment) == 0);

	std::vector<AllocationHandle> allocs;
	if (sparseMemoryRequirement.imageMipTailFirstLod < info.mipLevels) {
		if (singleMipTail) {
			VmaAllocation allocation;
			VmaAllocationInfo allocInfo;
			VkMemoryRequirements requirements{
				.size = sparseMemoryRequirement.imageMipTailSize,
				.alignment = memoryRequirements.alignment,
				.memoryTypeBits = memoryRequirements.memoryTypeBits,
			};
			vmaAllocateMemory(m_vmaAllocator->get_handle(), &requirements, &allocCreateInfo, &allocation, &allocInfo);
			mipTailBinds.reserve(1);
			VkSparseMemoryBind mipTailBind{
				.resourceOffset = sparseMemoryRequirement.imageMipTailOffset,
				.size = sparseMemoryRequirement.imageMipTailSize,
				.memory = allocInfo.deviceMemory,
				.memoryOffset = allocInfo.offset,
			};
			mipTailBinds.push_back(mipTailBind);
			allocs.push_back(m_allocationPool.emplace(*this, allocation));
		}
		else {
			std::vector<VmaAllocation> allocations;
			std::vector<VmaAllocationInfo> allocInfos;
			VkMemoryRequirements requirements{
				.size = sparseMemoryRequirement.imageMipTailSize,
				.alignment = memoryRequirements.alignment,
				.memoryTypeBits = memoryRequirements.memoryTypeBits,
			};
			allocations.resize(info.arrayLayers);
			allocInfos.resize(info.arrayLayers);
			mipTailBinds.reserve(info.arrayLayers);
			vmaAllocateMemoryPages(m_vmaAllocator->get_handle(), &requirements, &allocCreateInfo, info.arrayLayers, allocations.data(), allocInfos.data());
			for (uint32_t layer = 0; layer < info.arrayLayers; layer++) {

				VkSparseMemoryBind mipTailBind{
					.resourceOffset = sparseMemoryRequirement.imageMipTailOffset + layer*sparseMemoryRequirement.imageMipTailStride,
					.size = sparseMemoryRequirement.imageMipTailSize,
					.memory = allocInfos[layer].deviceMemory,
					.memoryOffset = allocInfos[layer].offset,
				};
				mipTailBinds.push_back(mipTailBind);
				allocs.push_back(m_allocationPool.emplace(*this, allocations[layer]));
			}
		}
		VkSparseImageOpaqueMemoryBindInfo mipTailBindInfo{
			.image = image,
			.bindCount = static_cast<uint32_t>(mipTailBinds.size()),
			.pBinds = mipTailBinds.data()
		};
		VkBindSparseInfo bindInfo{
			.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.bufferBindCount = 0,
			.pBufferBinds = 0,
			.imageOpaqueBindCount = 1,
			.pImageOpaqueBinds = &mipTailBindInfo,
			.imageBindCount = 0,
			.pImageBinds = nullptr,
		};
		if (m_graphics.queue != m_transfer.queue) {
			std::array<VkSemaphore, 2> semaphores;
			semaphores[0] = request_semaphore();
			semaphores[1] = request_semaphore();
			bindInfo.pSignalSemaphores = semaphores.data();
			bindInfo.signalSemaphoreCount = 2;
			add_wait_semaphore(CommandBuffer::Type::Generic, semaphores[0], VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, true);
			add_wait_semaphore(CommandBuffer::Type::Transfer, semaphores[1], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, true);
#if DEBUGSUBMISSIONS
			std::cout << "Submitted Sparse\n";
			std::cout << "\tWaits:";
			for (auto j = 0; j < bindInfo.waitSemaphoreCount; j++) {
				std::cout << bindInfo.pWaitSemaphores[j] << " ";
			}
			std::cout << "\n\tSignals:";
			for (auto j = 0; j < bindInfo.signalSemaphoreCount; j++) {
				std::cout << bindInfo.pSignalSemaphores[j] << " ";
			}
			std::cout << '\n';
			
#endif //  0
			if (m_transfer.supportsSparse)
				vkQueueBindSparse(m_transfer.queue, 1, &bindInfo, nullptr);
			else
				vkQueueBindSparse(m_graphics.queue, 1, &bindInfo, nullptr);
		}
		else {
			VkSemaphore sem = request_semaphore();
			//Mip Tail is assumed to be available
			bindInfo.pSignalSemaphores = &sem;
			bindInfo.signalSemaphoreCount = 1;
			add_wait_semaphore(CommandBuffer::Type::Generic, sem, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
#if DEBUGSUBMISSIONS
			std::cout << "Submitted Sparse\n";
			std::cout << "\tWaits:";
			for (auto j = 0; j < bindInfo.waitSemaphoreCount; j++) {
				std::cout << bindInfo.pWaitSemaphores[j] << " ";
			}
			std::cout << "\n\tSignals:";
			for (auto j = 0; j < bindInfo.signalSemaphoreCount; j++) {
				std::cout << bindInfo.pSignalSemaphores[j] << " ";
			}
			std::cout << '\n';

#endif //  0
			vkQueueBindSparse(m_graphics.queue, 1, &bindInfo, nullptr);
		}
	}
	
	auto handle(m_imagePool.emplace(*this, image, info, allocs, mipTailBegin));
	handle->set_stage_flags(Image::possible_stages_from_image_usage(createInfo.usage));
	handle->set_access_flags(Image::possible_access_from_image_usage(createInfo.usage));
	handle->set_single_mip_tail(singleMipTail);
	handle->set_optimal(false);
	handle->set_available_mip(sparseMemoryRequirement.imageMipTailFirstLod);
	return handle;
}
void vulkan::LogicalDevice::transition_image(ImageHandle& handle, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	auto graphicsCmd = request_command_buffer(CommandBuffer::Type::Generic);

	graphicsCmd->image_barrier(*handle, oldLayout, newLayout,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0);
	submit(graphicsCmd);
}
void vulkan::LogicalDevice::update_image_with_buffer(const ImageInfo& info, Image& image, const ImageBuffer& buffer, vulkan::FenceHandle* fence)
{
	VkAccessFlags finalTransitionSrcAccess = info.generate_mips() ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_TRANSFER_WRITE_BIT;
	VkAccessFlags prepareSrcAccess = m_graphics.queue == m_transfer.queue ? VK_ACCESS_TRANSFER_WRITE_BIT : 0;

	bool needMipBarrier = true;
	bool needInitialBarrier = true;

	auto graphicsCmd = request_command_buffer(CommandBuffer::Type::Generic);
	auto transferCmd = graphicsCmd;
	if (m_transfer.queue != m_graphics.queue)
		transferCmd = request_command_buffer(CommandBuffer::Type::Transfer);

	transferCmd->image_barrier(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
	transferCmd->copy_buffer_to_image(image, *buffer.buffer, buffer.blits.size(), buffer.blits.data());

	if (m_transfer.queue != m_graphics.queue) {
		VkPipelineStageFlags dstStages = info.generate_mips() ? VK_PIPELINE_STAGE_TRANSFER_BIT : image.get_stage_flags();
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
				.image = image.get_handle(),
				.subresourceRange {
					.aspectMask = ImageInfo::format_to_aspect_mask(info.format),
					.levelCount = info.generate_mips() ? 1 : info.mipLevels,
					.layerCount = info.arrayLayers,
				}
			};
			needInitialBarrier = info.generate_mips();

			auto acquire = release;
			acquire.srcAccessMask = 0;
			acquire.dstAccessMask = info.generate_mips() ? VK_ACCESS_TRANSFER_READ_BIT :
				image.get_access_flags() & Image::possible_access_from_image_layout(info.layout);
			transferCmd->barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, nullptr, 0, nullptr, 1, &release);
			graphicsCmd->barrier(dstStages, dstStages,
				0, nullptr, 0, nullptr, 1, &acquire);
		}
		VkSemaphore sem = VK_NULL_HANDLE;
		submit(transferCmd, 1, &sem);
		add_wait_semaphore(graphicsCmd->get_type(), sem, dstStages, true);
	}
	if (info.generate_mips()) {
		graphicsCmd->mip_barrier(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, needMipBarrier);
		graphicsCmd->generate_mips(image);
	}
	if (needInitialBarrier) {
		graphicsCmd->image_barrier(image, info.generate_mips() ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			info.layout, VK_PIPELINE_STAGE_TRANSFER_BIT, finalTransitionSrcAccess, image.get_stage_flags(),
			image.get_stage_flags() & Image::possible_access_from_image_layout(info.layout));
	}
	submit(graphicsCmd, 0 ,nullptr, fence);
	image.set_optimal(true);
}
void vulkan::LogicalDevice::update_sparse_image_with_buffer(const ImageInfo& info, Image& image, const ImageBuffer& buffer, vulkan::FenceHandle* fence, uint32_t mipLevel)
{
	auto graphicsCmd = request_command_buffer(CommandBuffer::Type::Generic);
	auto transferCmd = graphicsCmd;
	if (m_transfer.queue != m_graphics.queue)
		transferCmd = request_command_buffer(CommandBuffer::Type::Transfer);
	bool needInitialBarrier = true;

	transferCmd->image_barrier(image, image.is_optimal()?VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
	image.set_optimal(true);
	transferCmd->copy_buffer_to_image(image, *buffer.buffer, buffer.blits.size(), buffer.blits.data());

	if (m_transfer.queue != m_graphics.queue) {
		VkPipelineStageFlags dstStages = image.get_stage_flags();
		if (!info.concurrent_queue() && m_transfer.familyIndex != m_graphics.familyIndex) {
			VkImageMemoryBarrier release{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask = 0,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.srcQueueFamilyIndex = m_transfer.familyIndex,
				.dstQueueFamilyIndex = m_graphics.familyIndex,
				.image = image.get_handle(),
				.subresourceRange {
					.aspectMask = ImageInfo::format_to_aspect_mask(info.format),
					.baseMipLevel = 0,
					.levelCount = info.mipLevels ,
					.layerCount = info.arrayLayers,
				}
			};
			needInitialBarrier = false;
			auto acquire = release;
			acquire.srcAccessMask = 0;
			acquire.dstAccessMask = info.generate_mips() ? VK_ACCESS_TRANSFER_READ_BIT :
				image.get_access_flags() & Image::possible_access_from_image_layout(info.layout);
			transferCmd->barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, nullptr, 0, nullptr, 1, &release);
			graphicsCmd->barrier(dstStages, dstStages,
				0, nullptr, 0, nullptr, 1, &acquire);
		}
		std::array<VkSemaphore, 2> sem{};
		submit(transferCmd, 2, sem.data());
		add_wait_semaphore(graphicsCmd->get_type(), sem[0], dstStages);
		add_wait_semaphore(transferCmd->get_type(), sem[1], dstStages);
	}
	if (needInitialBarrier) {
		graphicsCmd->image_barrier(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, image.get_stage_flags(),
			image.get_stage_flags() & Image::possible_access_from_image_layout(info.layout));
	}
	submit(graphicsCmd, 0, nullptr, fence);
}
bool vulkan::LogicalDevice::resize_sparse_image_up(Image& image, uint32_t baseMipLevel)
{
	VmaAllocationCreateInfo allocInfo{
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
	};
	VkImage imageHandle = image.get_handle();
	VkSparseImageMemoryRequirements sparseMemoryRequirement = get_sparse_memory_requirements(imageHandle, ImageInfo::format_to_aspect_mask(image.get_format()));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_device, imageHandle, &memoryRequirements);
	assert(memoryRequirements.size <= m_physicalProperties.limits.sparseAddressSpaceSize);
	assert((memoryRequirements.size % memoryRequirements.alignment) == 0);
	assert((sparseMemoryRequirement.imageMipTailSize % memoryRequirements.alignment) == 0);

	bool singleMipTail = (sparseMemoryRequirement.formatProperties.flags & VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT);
	uint32_t allocationSize = memoryRequirements.alignment;
	uint32_t mipTailSize = sparseMemoryRequirement.imageMipTailSize;
	if (!singleMipTail)
		mipTailSize *= image.get_info().arrayLayers;

	VmaAllocationCreateInfo allocCreateInfo{
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
	};

	uint32_t allocationCount = 0;
	for (uint32_t mipLevel = image.get_available_mip(); mipLevel-- > baseMipLevel;) {
		VkExtent3D extent{
			.width = image.get_width(mipLevel) ,
			.height = image.get_height(mipLevel),
			.depth = image.get_depth(mipLevel),
		};
		VkExtent3D granularity = sparseMemoryRequirement.formatProperties.imageGranularity;

		uint32_t xCount = (extent.width + granularity.width - 1) / granularity.width;
		uint32_t yCount = (extent.height + granularity.height - 1) / granularity.height;
		uint32_t zCount = (extent.depth + granularity.depth - 1) / granularity.depth;
		allocationCount += xCount * yCount * zCount;
	}

	std::vector<VmaAllocation> allocations;
	std::vector<VmaAllocationInfo> allocationInfos;

	std::vector<AllocationHandle> allocationHandles;
	std::vector<VkSparseImageMemoryBind> imageBinds;
	VkMemoryRequirements requirements{
		.size = allocationSize,
		.alignment = memoryRequirements.alignment,
		.memoryTypeBits = memoryRequirements.memoryTypeBits,
	};
	allocations.resize(allocationCount);
	allocationInfos.resize(allocationCount);
	vmaAllocateMemoryPages(m_vmaAllocator->get_handle(), &requirements, &allocCreateInfo, allocationCount, allocations.data(), allocationInfos.data());
	allocationHandles.reserve(allocationCount);
	imageBinds.reserve(allocationCount);
	
	uint32_t current = 0;
	//Iterate reversly because we want the image allocations to be in that order
	for (uint32_t mipLevel = image.get_available_mip(); mipLevel --> baseMipLevel;) {
		VkExtent3D extent{
			.width = image.get_width(mipLevel) ,
			.height = image.get_height(mipLevel),
			.depth = image.get_depth(mipLevel),
		};
		VkExtent3D granularity = sparseMemoryRequirement.formatProperties.imageGranularity;

		uint32_t xCount = (extent.width + granularity.width -1) / granularity.width;
		uint32_t yCount = (extent.height + granularity.height - 1) / granularity.height;
		uint32_t zCount = (extent.depth + granularity.depth - 1) / granularity.depth;

		VkExtent3D blockExtent = granularity;
		for (uint32_t arrayLayer = 0; arrayLayer < image.get_info().arrayLayers; arrayLayer++) {
			for (uint32_t z = 0; z < zCount; z++) {
				if (z == zCount - 1)
					blockExtent.depth = extent.depth - granularity.depth * z;
				for (uint32_t y = 0; y < yCount; y++) {
					if (y == yCount - 1)
						blockExtent.height = extent.height - granularity.height * y;
					for (uint32_t x = 0; x < xCount; x++, current++) {
						if (x == xCount - 1)
							blockExtent.width = extent.width - granularity.width * x;
						VkOffset3D offset{
							.x = static_cast<int32_t>(x*granularity.width),
							.y = static_cast<int32_t>(y*granularity.height),
							.z = static_cast<int32_t>(z*granularity.depth)
						};
						VmaAllocationInfo allocationInfo = allocationInfos[current];
						VkSparseImageMemoryBind imageBind{
							.subresource {
								.aspectMask = ImageInfo::format_to_aspect_mask(image.get_format()),
								.mipLevel = mipLevel,
								.arrayLayer = arrayLayer,
							},
							.offset = offset,
							.extent = blockExtent,
							.memory = allocationInfo.deviceMemory,
							.memoryOffset = allocationInfo.offset
						};
						imageBinds.push_back(imageBind);
						allocationHandles.push_back(m_allocationPool.emplace(*this, allocations[current]));
					}
					blockExtent.width = granularity.width;
				}
				blockExtent.height = granularity.height;
			}
			blockExtent.depth = granularity.depth;
		}
	}
	

	VkSparseImageMemoryBindInfo imageBindInfo{
		.image = imageHandle,
		.bindCount = static_cast<uint32_t>(imageBinds.size()),
		.pBinds = imageBinds.data()
	};
	VkSemaphore sem = request_semaphore();
	VkBindSparseInfo bindInfo{
		.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = nullptr,
		.bufferBindCount = 0,
		.pBufferBinds = 0,
		.imageOpaqueBindCount = 0,
		.pImageOpaqueBinds = nullptr,
		.imageBindCount = 1,
		.pImageBinds = &imageBindInfo,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &sem,
	};
#if DEBUGSUBMISSIONS
	std::cout << "Submitted Sparse\n";
	std::cout << "\tWaits:";
	for (auto j = 0; j < bindInfo.waitSemaphoreCount; j++) {
		std::cout << bindInfo.pWaitSemaphores[j] << " ";
	}
	std::cout << "\n\tSignals:";
	for (auto j = 0; j < bindInfo.signalSemaphoreCount; j++) {
		std::cout << bindInfo.pSignalSemaphores[j] << " ";
	}
	std::cout << '\n';

#endif //  0
	if(m_transfer.supportsSparse)
		vkQueueBindSparse(m_transfer.queue, 1, &bindInfo, nullptr);
	else
		vkQueueBindSparse(m_graphics.queue, 1, &bindInfo, nullptr);
	add_wait_semaphore(CommandBuffer::Type::Transfer, sem, VK_PIPELINE_STAGE_TRANSFER_BIT);
	image.append_allocations(allocationHandles);
	image.set_available_mip(baseMipLevel);
	return true;
}
void vulkan::LogicalDevice::resize_sparse_image_down(Image& image, uint32_t baseMipLevel)
{
	VkImage imageHandle = image.get_handle();
	VkSparseImageMemoryRequirements sparseMemoryRequirement = get_sparse_memory_requirements(imageHandle, ImageInfo::format_to_aspect_mask(image.get_format()));

	uint32_t allocCount = 0;
	std::vector<VkSparseImageMemoryBind> imageBinds;
	assert(baseMipLevel <= image.get_mip_tail());
	for (uint32_t mipLevel = image.get_available_mip(); mipLevel < baseMipLevel; mipLevel++) {
		VkExtent3D extent{
			.width = image.get_width(mipLevel) ,
			.height = image.get_height(mipLevel),
			.depth = image.get_depth(mipLevel),
		};
		VkExtent3D granularity = sparseMemoryRequirement.formatProperties.imageGranularity;

		uint32_t xCount = (extent.width + granularity.width - 1) / granularity.width;
		uint32_t yCount = (extent.height + granularity.height - 1) / granularity.height;
		uint32_t zCount = (extent.depth + granularity.depth - 1) / granularity.depth;

		VkExtent3D blockExtent = granularity;
		for (uint32_t arrayLayer = 0; arrayLayer < image.get_info().arrayLayers; arrayLayer++) {
			for (uint32_t z = 0; z < zCount; z++) {
				if (z == zCount - 1)
					blockExtent.depth = extent.depth - granularity.depth * z;
				for (uint32_t y = 0; y < yCount; y++) {
					if (y == yCount - 1)
						blockExtent.height = extent.height - granularity.height * y;
					for (uint32_t x = 0; x < xCount; x++, allocCount++) {
						if (x == xCount - 1)
							blockExtent.width = extent.width - granularity.width * x;
						VkOffset3D offset{
							.x = static_cast<int32_t>(x * granularity.width),
							.y = static_cast<int32_t>(y * granularity.height),
							.z = static_cast<int32_t>(z * granularity.depth)
						};
						VkSparseImageMemoryBind imageBind{
							.subresource {
								.aspectMask = ImageInfo::format_to_aspect_mask(image.get_format()),
								.mipLevel = mipLevel,
								.arrayLayer = arrayLayer,
							},
							.offset = offset,
							.extent = blockExtent,
							.memory = VK_NULL_HANDLE,
							.memoryOffset = 0
						};
						imageBinds.push_back(imageBind);
					}
					blockExtent.width = granularity.width;
				}
				blockExtent.height = granularity.height;
			}
			blockExtent.depth = granularity.depth;
		}
	}
	VkSparseImageMemoryBindInfo imageBindInfo{
		.image = imageHandle,
		.bindCount = static_cast<uint32_t>(imageBinds.size()),
		.pBinds = imageBinds.data()
	};
	image.set_available_mip(baseMipLevel);
	VkBindSparseInfo bindInfo{
		.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = static_cast<uint32_t>(frame().signalSemaphores.size()),
		.pWaitSemaphores = frame().signalSemaphores.data(),
		.bufferBindCount = 0,
		.pBufferBinds = 0,
		.imageOpaqueBindCount = 0,
		.pImageOpaqueBinds = nullptr,
		.imageBindCount = 1,
		.pImageBinds = &imageBindInfo,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = nullptr,
	};
	VkFence fence = m_fenceManager.request_raw_fence();
#if DEBUGSUBMISSIONS
	std::cout << "Submitted Sparse\n";
	std::cout << "\tWaits:";
	for (auto j = 0; j < bindInfo.waitSemaphoreCount; j++) {
		std::cout << bindInfo.pWaitSemaphores[j] << " ";
	}
	std::cout << "\n\tSignals:";
	for (auto j = 0; j < bindInfo.signalSemaphoreCount; j++) {
		std::cout << bindInfo.pSignalSemaphores[j] << " ";
	}
	std::cout << '\n';

#endif //  0
	if (m_transfer.supportsSparse)
		vkQueueBindSparse(m_transfer.queue, 1, &bindInfo, fence);
	else
		vkQueueBindSparse(m_graphics.queue, 1, &bindInfo, fence);
	add_fence_callback(fence, [&image, allocCount]() {
		image.drop_allocations(allocCount);
		image.set_being_resized(false);
	});
	
}
std::vector<vulkan::CommandBufferHandle>& vulkan::LogicalDevice::get_current_submissions(CommandBuffer::Type type)
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

vulkan::CommandPool& vulkan::LogicalDevice::get_pool(uint32_t threadId, CommandBuffer::Type type)
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


void vulkan::LogicalDevice::create_vma_allocator()
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

vulkan::BufferHandle vulkan::LogicalDevice::create_buffer(const BufferInfo& info,const void* initialData)
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
		tmp.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		auto stagingBuffer = create_buffer(tmp,initialData);
		auto cmd = request_command_buffer(CommandBuffer::Type::Transfer);
		cmd->copy_buffer(*handle, *stagingBuffer);
		submit_staging(cmd, info.usage, true);
	}
	else if (initialData != nullptr) {
		//No staging needed
		auto map = handle->map_data();
		memcpy(reinterpret_cast<char*>(map), initialData, info.size);
	}
	return handle;
}


vulkan::ImageViewHandle vulkan::LogicalDevice::create_image_view(const ImageViewCreateInfo& info)
{
	return m_imageViewPool.emplace(*this,info);
}

vulkan::ImageHandle vulkan::LogicalDevice::create_image(const ImageInfo& info, InitialImageData* initialData)
{
	if (initialData) {
		auto handle = create_image(info, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		auto buf = create_staging_buffer(info, initialData);
		update_image_with_buffer(info, *handle, buf);
		return handle;
	}
	else {
		auto handle = create_image(info, static_cast<VkImageUsageFlags>(0));
		if (info.layout != VK_IMAGE_LAYOUT_UNDEFINED)
			transition_image(handle, VK_IMAGE_LAYOUT_UNDEFINED, info.layout);
		return handle;
	}
}

vulkan::ImageHandle vulkan::LogicalDevice::create_sparse_image(const ImageInfo& info, InitialImageData* initialData)
{
	if (initialData) {
		auto handle = create_sparse_image(info, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		auto buf = create_staging_buffer(info, initialData, handle->get_available_mip());
		update_sparse_image_with_buffer(info, *handle, buf, nullptr, handle->get_available_mip());
		return handle;
	}
	else {
		return create_sparse_image(info, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	}
}

void vulkan::LogicalDevice::downsize_sparse_image(Image& image, uint32_t targetMipLevel)
{
	//Always keep mipTail resident
	if (targetMipLevel > image.get_mip_tail())
		targetMipLevel = image.get_mip_tail();
	//Only downsize
	if (targetMipLevel <= image.get_available_mip())
		return;
	image.set_being_resized(true);
	image.change_view_mip_level(targetMipLevel);
	resize_sparse_image_down(image, targetMipLevel);

}

bool vulkan::LogicalDevice::upsize_sparse_image(Image& image, InitialImageData* initialData, uint32_t targetMipLevel)
{
	if (!initialData)
		return false;
	if (targetMipLevel >= image.get_available_mip() )
		return false;
	image.set_being_resized(true);
	auto buf = create_staging_buffer(image.get_info(), initialData, targetMipLevel);
	resize_sparse_image_up(image, targetMipLevel);
	vulkan::FenceHandle fence(m_fenceManager);
	update_sparse_image_with_buffer(image.get_info(), image, buf, &fence, targetMipLevel);
	VkFence localFence = fence.release_handle();

	add_fence_callback(localFence, [&image, targetMipLevel]() {
		image.change_view_mip_level(targetMipLevel);
		image.set_being_resized(false);
	});

}



vulkan::DescriptorSetAllocator* vulkan::LogicalDevice::request_descriptor_set_allocator(const DescriptorSetLayout& layout)
{

	//std::hash <std::pair < DescriptorSetLayout, std::array<uint32_t, MAX_BINDINGS>>> h{};
	
	
	if (!m_descriptorAllocatorIds.contains(layout)) {
		m_descriptorAllocatorIds.emplace(layout, m_descriptorAllocatorsStorage.emplace_intrusive(*this, layout));
	}
	return m_descriptorAllocatorsStorage.get_ptr(m_descriptorAllocatorIds.at(layout));
}

size_t vulkan::LogicalDevice::register_shader(const std::vector<uint32_t>& shaderCode)
{
	return m_shaderStorage.emplace_intrusive(*this, shaderCode);
}


vulkan::Shader* vulkan::LogicalDevice::request_shader(size_t id)
{
	return m_shaderStorage.get_ptr(id);
}

vulkan::Program* vulkan::LogicalDevice::request_program(const std::vector<Shader*>& shaders)
{
	
	if (!m_programIds.contains(shaders)) {
		m_programIds.emplace(shaders, m_programStorage.emplace_intrusive(shaders));
	}
	return m_programStorage.get_ptr(m_programIds.at(shaders));
}

vulkan::PipelineLayout* vulkan::LogicalDevice::request_pipeline_layout(const ShaderLayout& layout)
{
	
	if (!m_pipelineLayoutIds.contains(layout)) {
		m_pipelineLayoutIds.emplace(layout, m_pipelineLayoutStorage.emplace_intrusive(*this, layout));
	}
	return m_pipelineLayoutStorage.get_ptr(m_pipelineLayoutIds.at(layout));
}

vulkan::Renderpass* vulkan::LogicalDevice::request_render_pass(const RenderpassCreateInfo& info)
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

vulkan::Renderpass* vulkan::LogicalDevice::request_compatible_render_pass(const RenderpassCreateInfo& info)
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

VkPipeline vulkan::LogicalDevice::request_pipeline(const PipelineCompile& compile) noexcept
{
	return m_pipelineStorage.request_pipeline(compile);
}

VkPipeline vulkan::LogicalDevice::request_pipeline(const Program& program) noexcept
{
	return m_pipelineStorage.request_pipeline(program);
}

vulkan::RenderpassCreateInfo vulkan::LogicalDevice::request_swapchain_render_pass(SwapchainRenderpassType type) noexcept
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
	if (type == SwapchainRenderpassType::Depth) {
		info.clearDepthStencil = VkClearDepthStencilValue{
			.depth = 1.0f, .stencil = 0
		};
		uint32_t width = swapchainView->get_image()->get_width();
		uint32_t height = swapchainView->get_image()->get_height();
		info.depthStencilAttachment = request_render_target(width, height, VK_FORMAT_D16_UNORM);
		info.opFlags.set(RenderpassCreateInfo::OpFlags::DepthStencilClear);
	}

	return info;
}

vulkan::Framebuffer* vulkan::LogicalDevice::request_framebuffer(const RenderpassCreateInfo& info)
{
	return m_framebufferAllocator.request_framebuffer(info);
}

VkSemaphore vulkan::LogicalDevice::request_semaphore()
{
	return m_semaphoreManager.request_semaphore();
}

vulkan::CommandBufferHandle vulkan::LogicalDevice::request_command_buffer(CommandBuffer::Type type)
{
	auto cmd = get_pool(get_thread_index(),type).request_command_buffer();
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(cmd, &beginInfo);
	return m_commandBufferPool.emplace(*this, cmd, type, get_thread_index());
}

vulkan::ImageView* vulkan::LogicalDevice::request_render_target(uint32_t width, uint32_t height, VkFormat format, uint32_t index, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount)
{
	return m_attachmentAllocator.request_attachment(width, height, format, index, sampleCount, usage);
}

void vulkan::LogicalDevice::resize_buffer(Buffer& buffer, VkDeviceSize newSize, bool copyData)
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




VkSemaphore vulkan::LogicalDevice::get_present_semaphore()
{
	auto tempSem = m_wsiState.present;
	m_wsiState.present = VK_NULL_HANDLE;
	frame().recycledSemaphores.push_back(tempSem);
	return tempSem;
}

bool vulkan::LogicalDevice::swapchain_touched()  const noexcept
{
	return m_wsiState.swapchain_touched;
}

VkQueue vulkan::LogicalDevice::get_graphics_queue()  const noexcept
{
	return m_graphics.queue;
}



void vulkan::LogicalDevice::create_default_sampler()
{
	SamplerCreateInfo createInfo{
		//.
	};
	createInfo.maxLod = VK_LOD_CLAMP_NONE;
	//createInfo.anisotropyEnable = VK_TRUE;
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

vulkan::Sampler* vulkan::LogicalDevice::get_default_sampler(DefaultSampler samplerType) const noexcept
{
	assert(samplerType != DefaultSampler::Size);
	return m_defaultSampler[static_cast<size_t>(samplerType)].get();
}

void vulkan::LogicalDevice::wait_idle()
{
	wait_no_lock();
}


vulkan::LogicalDevice::FrameResource::~FrameResource()
{
	begin();
}

void vulkan::LogicalDevice::FrameResource::begin()
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

	for (auto buffer : deletedBuffer) {
		vkDestroyBuffer(r_device.get_device(), buffer, r_device.get_allocator());
	}
	deletedBuffer.clear();
	for (auto allocation : deletedAllocations) {
		r_device.get_vma_allocator()->free_allocation(allocation);
	}
	deletedAllocations.clear();
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
