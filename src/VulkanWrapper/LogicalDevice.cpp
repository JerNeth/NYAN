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
	m_graphicsFamilyQueueIndex(graphicsFamilyQueueIndex),
	m_computeFamilyQueueIndex(computeFamilyQueueIndex == ~0 ? graphicsFamilyQueueIndex : computeFamilyQueueIndex),
	m_transferFamilyQueueIndex(transferFamilyQueueIndex == ~0 ? graphicsFamilyQueueIndex : transferFamilyQueueIndex),
	m_physicalProperties(properties),
	m_framebufferAllocator(*this),
	m_fenceManager(*this),
	m_semaphoreManager(*this),
	m_pipelineStorage(*this)
{
	vkGetDeviceQueue(m_device, m_graphicsFamilyQueueIndex, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, m_computeFamilyQueueIndex, 0, &m_computeQueue);
	vkGetDeviceQueue(m_device, m_transferFamilyQueueIndex, 0, &m_transferQueue);
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

void Vulkan::LogicalDevice::demo_setup()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr
	};
	VkDescriptorSetLayoutBinding samplerLayoutBinding{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	};
	create_descriptor_set_layout(std::array< VkDescriptorSetLayoutBinding, 2>{uboLayoutBinding, samplerLayoutBinding});
	create_swapchain();
	create_depth_resources();
	create_program();
	//auto vertName = "basic_vert.spv";
	//auto fragName = "basic_frag.spv";
	//auto vertShader = request_shader(vertName);
	//auto fragShader = request_shader(fragName);

	//create_graphics_pipeline<Vertex, 2>({ vertShader, fragShader });
	create_command_pool();
	create_vertex_buffer();
	create_index_buffer();


	//create_texture_sampler();
	
	create_uniform_buffers();
	create_descriptor_pool();

}

void Vulkan::LogicalDevice::demo_teardown()
{
	cleanup_swapchain();
	vkDestroyImageView(m_device, m_imageView, m_allocator);
	vmaDestroyImage(m_vmaAllocator->get_handle(), m_image, m_imageAllocation);
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, m_allocator);
	m_vmaAllocator->destroy_buffer(m_vertexBuffer, m_vertexBufferAllocation);
	m_vmaAllocator->destroy_buffer(m_indexBuffer, m_indexBufferAllocation);
	for (uint32_t i = 0; i < m_swapchains[0]->get_image_count(); i++)
		vkDestroyCommandPool(m_device, m_commandPool[i], m_allocator);
}

void Vulkan::LogicalDevice::next_frame()
{

	m_currentFrame++;
	if (m_currentFrame >= m_frameResources.size())
		m_currentFrame = 0;
	frame().begin();
	m_wsiState.swapchain_touched = false;
	if (m_wsiState.aquire != VK_NULL_HANDLE)
		frame().recycledSemaphores.push_back(m_wsiState.aquire);
	m_wsiState.aquire = m_semaphoreManager.request_semaphore();
	m_demoImage = m_swapchains[0]->aquire_next_image(m_wsiState.aquire);
}

void Vulkan::LogicalDevice::end_frame()
{
	if (!frame().submittedTransferCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		submit_queue(CommandBuffer::Type::Transfer, &fence);
		frame().waitForFences.emplace_back(std::move(fence));
	}
	if (!frame().submittedGraphicsCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		submit_queue(CommandBuffer::Type::Generic, &fence);
		frame().waitForFences.emplace_back(std::move(fence));
	}
	if (!frame().submittedComputeCmds.empty()) {
		FenceHandle fence(m_fenceManager);
		submit_queue(CommandBuffer::Type::Compute,&fence);
		frame().waitForFences.emplace_back(std::move(fence));
	}
	m_swapchains[0]->present_queue();
}

void Vulkan::LogicalDevice::submit_queue(CommandBuffer::Type type, FenceHandle* fence)
{
	auto& submissions = get_current_submissions(type);
	//Split commands into pre and post swapchain commands
	std::array<VkSubmitInfo, 2> submitInfos;
	uint32_t submitCounts = 0;
	std::vector<VkCommandBuffer> commands;
	std::array<std::vector<VkSemaphore>, 2> waitSemaphores;
	std::array<std::vector<VkFlags>, 2> waitStages;
	std::array<std::vector<VkSemaphore>, 2> signalSemaphores;
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
	for (uint32_t i = 0; i < submitCounts; i++) {
		submitInfos[i].waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores[i].size());
		submitInfos[i].pWaitSemaphores = waitSemaphores[i].data();
		submitInfos[i].pWaitDstStageMask = waitStages[i].data(); 

		submitInfos[i].signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores[i].size());
		submitInfos[i].pSignalSemaphores = signalSemaphores[i].data();

	}
	VkFence localFence = fence ? *fence : VK_NULL_HANDLE;
	if (auto result = vkQueueSubmit(m_graphicsQueue, submitCounts, submitInfos.data(), localFence); result != VK_SUCCESS) {
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
		frame().deletedFramebuffer.push_back(framebuffer);
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

void Vulkan::LogicalDevice::queue_buffer_deletion(VkBuffer buffer, VmaAllocation allocation) noexcept
{
	frame().deletedBufferAllocations.push_back({ buffer, allocation });
}

void Vulkan::LogicalDevice::submit(CommandBufferHandle cmd)
{
	auto type = cmd->get_type();
	auto& submissions = get_current_submissions(type);
	cmd->end();
	submissions.push_back(cmd);
}

Vulkan::LogicalDevice::FrameResource& Vulkan::LogicalDevice::frame()
{
	return *m_frameResources[m_currentFrame];
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

std::pair<VkBuffer, VmaAllocation> Vulkan::LogicalDevice::create_buffer(VkDeviceSize size, VkBufferUsageFlags  usage, VmaMemoryUsage memoryUsage)
{
	VkBuffer buffer;
	VmaAllocation allocation;
	VkBufferCreateInfo bufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	VmaAllocationCreateInfo allocInfo{
		.usage = memoryUsage
	};
	vmaCreateBuffer(m_vmaAllocator->get_handle(), &bufferCreateInfo, &allocInfo, &buffer, &allocation, nullptr);
	return std::make_pair(buffer, allocation);
}


void Vulkan::LogicalDevice::cleanup_swapchain()
{
	for (size_t i = 0; i < m_swapchains[0]->get_image_count(); i++) {
		vmaDestroyBuffer(m_vmaAllocator->get_handle(), m_uniformBuffers[i], m_uniformBuffersAllocations[i]);
	}
	vkDestroyDescriptorPool(m_device, m_descriptorPool, m_allocator);

	
	//vkDestroyPipeline(m_device, m_graphicsPipeline, m_allocator);
	//vkDestroyPipelineLayout(m_device, m_pipelineLayout, m_allocator);
	//vkDestroyRenderPass(m_device, m_renderPass, m_allocator);
	
	//vkDestroySwapchainKHR(m_device, m_swapChain, m_allocator);
}
void Vulkan::LogicalDevice::copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = begin_single_time_commands();
	VkBufferCopy copyRegion{
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size
	};
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	end_single_time_commands(commandBuffer);
}
void Vulkan::LogicalDevice::transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands();
	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
	};
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::runtime_error("unsupported layout transition!");
	}
	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	end_single_time_commands(commandBuffer);
}
void Vulkan::LogicalDevice::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands();
	VkBufferImageCopy region{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.imageOffset {0,0,0},
		.imageExtent{
			.width = width,
			.height = height,
			.depth = 1
		}
	};
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	end_single_time_commands(commandBuffer);
}
VkCommandBuffer Vulkan::LogicalDevice::begin_single_time_commands()
{
	VkCommandBufferAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_commandPool[0],
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocateInfo, &commandBuffer);
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}
void Vulkan::LogicalDevice::end_single_time_commands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer
	};
	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);
	vkFreeCommandBuffers(m_device, m_commandPool[0], 1, &commandBuffer);
}
void Vulkan::LogicalDevice::create_texture_image_view()
{
	m_imageView = create_image_view(VK_FORMAT_R8G8B8A8_SRGB, m_image, VK_IMAGE_ASPECT_COLOR_BIT);
}
void Vulkan::LogicalDevice::create_depth_resources()
{
	ImageInfo info{
		.format = VK_FORMAT_D16_UNORM,
		.width = m_swapchains[0]->get_width(),
		.height = m_swapchains[0]->get_height(),
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.layout = VK_IMAGE_LAYOUT_GENERAL
	};
	m_depth = std::make_unique<Image>(*this, info, VMA_MEMORY_USAGE_GPU_ONLY);
	ImageViewCreateInfo viewInfo{
		.image =m_depth.get(),
		.format = info.format,
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT
	};
	m_depthView = std::make_unique<ImageView>(*this, viewInfo);
}
void Vulkan::LogicalDevice::create_vertex_buffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	auto [stagingBuffer, stagingAllocation] = create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	void* data;

	m_vmaAllocator->map_memory(stagingAllocation, &data);
	std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	m_vmaAllocator->unmap_memory(stagingAllocation);

	auto[buffer, allocation] = create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	m_vertexBuffer = buffer;
	m_vertexBufferAllocation = allocation;
	copy_buffer(stagingBuffer, m_vertexBuffer, bufferSize);

	m_vmaAllocator->destroy_buffer(stagingBuffer, stagingAllocation);
}
void Vulkan::LogicalDevice::create_index_buffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	auto [stagingBuffer, stagingAllocation] = create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	void* data;

	m_vmaAllocator->map_memory(stagingAllocation, &data);
	std::memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	m_vmaAllocator->unmap_memory(stagingAllocation);

	auto [buffer, allocation] = create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	m_indexBuffer = buffer;
	m_indexBufferAllocation = allocation;
	copy_buffer(stagingBuffer, m_indexBuffer, bufferSize);

	m_vmaAllocator->destroy_buffer(stagingBuffer, stagingAllocation);
	
}

void Vulkan::LogicalDevice::create_uniform_buffers()
{
	VkDeviceSize bufferSize = alignof(Ubo) * 2;
	m_uniformBuffers.resize(m_swapchains[0]->get_image_count());
	m_uniformBuffersAllocations.resize(m_swapchains[0]->get_image_count());
	for (size_t i = 0; i < m_swapchains[0]->get_image_count(); i++) {
		auto[buffer, allocation ] = create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		m_uniformBuffers[i] = buffer;
		m_uniformBuffersAllocations[i] = allocation;
	}
}

void Vulkan::LogicalDevice::create_swapchain()
{
	m_swapchains.emplace_back(new Swapchain(*this, m_swapchains.size()));
}


VkImageView Vulkan::LogicalDevice::create_image_view(VkFormat format, VkImage image, VkImageAspectFlags aspect)
{
	VkImageView imageView{};
	VkImageViewCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange{
				.aspectMask = aspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
	};
	if (auto result = vkCreateImageView(m_device, &createInfo, m_allocator, &imageView); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create image view, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create image view, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	return imageView;
}

void Vulkan::LogicalDevice::create_command_pool()
{
	m_commandPool.resize(m_swapchains[0]->get_image_count());
	for (uint32_t i = 0; i < m_swapchains[0]->get_image_count(); i++) {
		VkCommandPoolCreateInfo commandPoolCreateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = 0,
			.queueFamilyIndex = m_graphicsFamilyQueueIndex,
		};
		if (auto result = vkCreateCommandPool(m_device, &commandPoolCreateInfo, m_allocator, &m_commandPool[i]); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not create command pool, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not create command pool, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
	}
}

void Vulkan::LogicalDevice::demo_create_command_buffer(VkCommandBuffer buf)
{
	
		
	VkViewport viewport{
	.x = 0.0f,
	.y = 0.0f,
	.width = static_cast<float>(m_swapchains[0]->get_width()),
	.height = static_cast<float>(m_swapchains[0]->get_height()),
	.minDepth = 0.0f,
	.maxDepth = 1.0f
	};

	VkRect2D scissor{
		.offset = {0,0},
		.extent = m_swapchains[0]->get_swapchain_extent()
	};
	static float degrees;
	Math::mat44 model = Math::mat44(Math::mat33::rotation_matrix(0, 0, (degrees++) / 100));
	model(3, 3) = 1;
	std::array<VkClearValue, 2> clearColors{
		VkClearValue{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
		VkClearValue{.depthStencil = {.depth = 1.0f, .stencil = 0}}
	};
	const auto& info = request_swapchain_render_pass();
	auto framebuffer = request_framebuffer(info);
	auto renderpass = request_render_pass(info);
	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderpass->get_render_pass(),
		.framebuffer = framebuffer->get_handle(),
		.renderArea = scissor,
		.clearValueCount = static_cast<uint32_t>(clearColors.size()),
		.pClearValues = clearColors.data(),
	};
	vkCmdBindIndexBuffer(buf, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	VkBuffer vertexBuffers[]{ m_vertexBuffer };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(buf, 0, 1, vertexBuffers, offsets);
	vkCmdBeginRenderPass(buf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
	vkCmdSetViewport(buf, 0, 1, &viewport);
	vkCmdSetScissor(buf, 0, 1, &scissor);
	uint32_t offset[] = { 0,0 };
	vkCmdBindDescriptorSets(buf,VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_demoImage],1, offset);
	//vkCmdPushConstants(m_commandBuffers[i], m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Math::mat44), &model);
	vkCmdDrawIndexed(buf, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	offset[0] = sizeof(Ubo);
	vkCmdBindDescriptorSets(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_demoImage], 1, offset);
	vkCmdDrawIndexed(buf, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	vkCmdEndRenderPass(buf);
		
	
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
template<size_t numBindings>
void Vulkan::LogicalDevice::create_descriptor_set_layout(std::array<VkDescriptorSetLayoutBinding, numBindings> bindings)
{
	
	//std::array<VkDescriptorSetLayoutBinding, 2> bindings{uboLayoutBinding, samplerLayoutBinding};
	VkDescriptorSetLayoutCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};
	
	if (auto result = vkCreateDescriptorSetLayout(m_device, &createInfo, m_allocator, &m_descriptorSetLayout); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorSetLayout, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorSetLayout, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}

void Vulkan::LogicalDevice::create_descriptor_sets()
{
	std::vector<VkDescriptorSetLayout> layouts(m_swapchains[0]->get_image_count(), m_descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data()
	};
	m_descriptorSets.resize(layouts.size());
	if (auto result = vkAllocateDescriptorSets(m_device, &allocateInfo, m_descriptorSets.data()); result != VK_SUCCESS) {
		throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
	}
	for (size_t i = 0; i < layouts.size(); i++) {
		VkDescriptorBufferInfo bufferInfo{
			.buffer = m_uniformBuffers[i],
			.offset = 0,
			.range = sizeof(Ubo)
		};
		VkDescriptorImageInfo imageInfo{
			.sampler = get_default_sampler(DefaultSampler::TrilinearClamp)->get_handle(),
			.imageView = m_imageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,		
		};
		std::array< VkWriteDescriptorSet, 2> descriptorWrites{
			VkWriteDescriptorSet {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				.pImageInfo = nullptr,
				.pBufferInfo = &bufferInfo,
				.pTexelBufferView = nullptr
			},
			VkWriteDescriptorSet {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = m_descriptorSets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &imageInfo,
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
			} 
		};
		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Vulkan::LogicalDevice::create_descriptor_pool()
{
	std::array< VkDescriptorPoolSize, 2> poolSizes{
		VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = static_cast<uint32_t>(m_swapchains[0]->get_image_count())
		},
		VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = static_cast<uint32_t>(m_swapchains[0]->get_image_count())
		}
	};
	VkDescriptorPoolCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = m_swapchains[0]->get_image_count(),
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data(),
	};

	if (auto result = vkCreateDescriptorPool(m_device, &createInfo, m_allocator, &m_descriptorPool); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorPool, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorPool, out of device memory");
		}
		if (result == VK_ERROR_FRAGMENTATION_EXT) {
			throw std::runtime_error("VK: could not create DescriptorPool, fragmentation error");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}

std::pair< VkImage, VmaAllocation> Vulkan::LogicalDevice::create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags flags, VmaMemoryUsage memoryUsage)
{
	VkImage image{};
	VmaAllocation allocation{};
	VkImageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent {
			.width = width,
			.height = height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = tiling,
		.usage = flags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	
	VmaAllocationCreateInfo allocInfo{
		.usage = memoryUsage,
	};
	
	if (auto result = vmaCreateImage(m_vmaAllocator->get_handle(), &createInfo, &allocInfo, &image, &allocation, nullptr); result != VK_SUCCESS) {
		throw std::runtime_error("Vk: error creating image");
	}
	return std::make_pair(image, allocation);
}

void Vulkan::LogicalDevice::create_texture_image(uint32_t width, uint32_t height, uint32_t channels, char* imageData)
{
	VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * static_cast<VkDeviceSize>(channels);

	auto [stagingBuffer, stagingAllocation] = create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	void* data;
	m_vmaAllocator->map_memory(stagingAllocation, &data);
	std::memcpy(data, imageData, imageSize);
	m_vmaAllocator->unmap_memory(stagingAllocation);

	auto [image, imageAllocation] = create_image(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	m_image = image;
	m_imageAllocation = imageAllocation;
	transition_image_layout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copy_buffer_to_image(stagingBuffer, m_image, width, height);
	transition_image_layout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_vmaAllocator->destroy_buffer(stagingBuffer, stagingAllocation);
	
	create_texture_image_view();
}

Vulkan::DescriptorSetAllocator* Vulkan::LogicalDevice::request_descriptor_set_allocator(const DescriptorSetLayout& layout)
{

	//std::hash <std::pair < DescriptorSetLayout, std::array<uint32_t, MAX_BINDINGS>>> h{};
	
	
	if (!m_descriptorAllocatorIds.contains(layout)) {
		m_descriptorAllocatorIds.emplace(layout, m_descriptorAllocatorsStorage.emplace(*this, layout));
	}
	return m_descriptorAllocatorsStorage.get(m_descriptorAllocatorIds.at(layout));
}

void Vulkan::LogicalDevice::register_shader(const std::string& shaderName, const std::vector<uint32_t>& shaderCode)
{
	if (!m_shaderIds.contains(shaderName)) {
		m_shaderIds.emplace(shaderName, m_shaderStorage.emplace(*this, shaderCode));
	}
}

Vulkan::Shader* Vulkan::LogicalDevice::request_shader(const std::string& shaderName) const
{
	return m_shaderStorage.get(m_shaderIds.at(shaderName));
}

Vulkan::Shader* Vulkan::LogicalDevice::request_shader(const std::string& shaderName, const std::vector<uint32_t>& shaderCode)
{
	register_shader(shaderName, shaderCode);
	return request_shader(shaderName);
}

Vulkan::Program* Vulkan::LogicalDevice::request_program(const std::vector<Shader*>& shaders)
{
	
	if (!m_programIds.contains(shaders)) {
		m_programIds.emplace(shaders, m_programStorage.emplace(shaders));
	}
	return m_programStorage.get(m_programIds.at(shaders));
}

Vulkan::PipelineLayout* Vulkan::LogicalDevice::request_pipeline_layout(const ShaderLayout& layout)
{
	
	if (!m_pipelineLayoutIds.contains(layout)) {
		m_pipelineLayoutIds.emplace(layout, m_pipelineLayoutStorage.emplace(*this, layout));
	}
	return m_pipelineLayoutStorage.get(m_pipelineLayoutIds.at(layout));
}

Vulkan::Renderpass* Vulkan::LogicalDevice::request_render_pass(const RenderpassCreateInfo& info)
{
	auto [compatibleHash, actualHash] = info.get_hash();
	auto result = m_renderpassIds.find(actualHash);
	size_t renderpassId = 0;
	if (result == m_renderpassIds.end()) {
		renderpassId = m_renderpassStorage.emplace(*this, info);
		m_renderpassIds[actualHash] = renderpassId;
		if (m_compatibleRenderpassIds.find(compatibleHash) == m_compatibleRenderpassIds.end())
			m_compatibleRenderpassIds[compatibleHash] = renderpassId;
	}
	else {
		renderpassId = result->second;
	}
	return m_renderpassStorage.get(renderpassId);
}

Vulkan::Renderpass* Vulkan::LogicalDevice::request_compatible_render_pass(const RenderpassCreateInfo& info)
{
	auto [compatibleHash, actualHash] = info.get_hash();

	auto result = m_compatibleRenderpassIds.find(compatibleHash);
	size_t renderpassId = 0;
	if (m_compatibleRenderpassIds.find(compatibleHash) == m_compatibleRenderpassIds.end()) {
		renderpassId = m_renderpassStorage.emplace(*this, info);
		m_compatibleRenderpassIds[compatibleHash] = renderpassId;

		m_renderpassIds[actualHash] = renderpassId;
	}
	else {
		renderpassId = result->second;
	}
	return m_renderpassStorage.get(renderpassId);
}

VkPipeline Vulkan::LogicalDevice::request_pipeline(const PipelineCompile& compile) noexcept
{
	return m_pipelineStorage.request_pipeline(compile);
}

const Vulkan::RenderpassCreateInfo& Vulkan::LogicalDevice::request_swapchain_render_pass() noexcept
{
	m_swapChainRenderPassInfo.colorAttachmentsCount = 1;
	m_swapChainRenderPassInfo.clearAttachments.set(0);
	m_swapChainRenderPassInfo.storeAttachments.set(0);
	m_swapChainRenderPassInfo.colorAttachmentsViews[0] = m_swapchains[0]->get_swapchain_image_view();

	m_swapChainRenderPassInfo.depthStencilAttachment = m_depthView.get();
	m_swapChainRenderPassInfo.opFlags.set(static_cast<uint32_t>(RenderpassCreateInfo::OpFlags::DepthStencilClear));

	return m_swapChainRenderPassInfo;
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
	return m_graphicsQueue;
}

void Vulkan::LogicalDevice::create_program()
{
	auto vertName = "basic_vert.spv";
	auto fragName = "basic_frag.spv";
	auto vertShaderCode = read_binary_file(vertName);
	auto fragShaderCode = read_binary_file(fragName);
	ShaderLayout layout;
	auto vertShader = request_shader(vertName, vertShaderCode);
	auto fragShader = request_shader(fragName, fragShaderCode);
	std::vector<Shader*> shaders{ vertShader, fragShader };
	vertShader->parse_shader(layout, vertShaderCode);
	fragShader->parse_shader(layout, fragShaderCode);
	auto program = request_program(shaders);
	program->set_pipeline_layout(request_pipeline_layout(layout));
	m_pipelineLayout = program->get_pipeline_layout()->get_layout();
	
	//Renderpass dummy(*this,m_renderPass);
	Attributes attributes{};
	attributes.formats[0] = get_format<Math::vec3>();
	attributes.formats[1] = get_format<Math::vec3>();
	attributes.formats[2] = get_format<Math::vec2>();
	InputRates inputRates{};
	inputRates.set(0, VK_VERTEX_INPUT_RATE_VERTEX);
	PipelineCompile compile{
		.state = defaultPipelineState,
		.program = program,
		.compatibleRenderPass = request_compatible_render_pass(request_swapchain_render_pass()),
		.attributes = attributes,
		.inputRates = inputRates,
		.subpassIndex = 0
	};
	m_graphicsPipeline = m_pipelineStorage.request_pipeline(compile);

}




void Vulkan::LogicalDevice::update_uniform_buffer()
{
	auto currentImage = m_demoImage;
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
	ubo[0].proj = Math::mat44::perspective(0.01f, 10.f, 45.0f, static_cast<float>(m_swapchains[0]->get_width()) / static_cast<float>(m_swapchains[0]->get_height()));
	ubo[1].model = Math::mat44(Math::mat33::rotation_matrix(0, 0, (degrees++) / 100));
	ubo[1].model(3, 3) = 1;
	ubo[1].model = ubo[1].model *Math::mat44::translation_matrix(Math::vec3({0.0f, 0.0f, -0.5f}));
	ubo[1].view = Math::mat44::look_at(Math::vec3({ 2.0f, 2.0f, 2.0f }), Math::vec3({ 0.0f, 0.0f, 0.0f }), Math::vec3({ 0.0f, 0.0f, 1.0f }));
	ubo[1].proj = Math::mat44::perspective(0.01f, 10.f, 45.0f, static_cast<float>(m_swapchains[0]->get_width()) / static_cast<float>(m_swapchains[0]->get_height()));
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

Vulkan::Sampler* Vulkan::LogicalDevice::get_default_sampler(DefaultSampler samplerType)
{
	assert(samplerType != DefaultSampler::Size);
	return m_defaultSampler[static_cast<size_t>(samplerType)].get();
}

void Vulkan::LogicalDevice::wait_idle()
{
	vkDeviceWaitIdle(m_device);
}

void Vulkan::LogicalDevice::create_swap_chain()
{
	

	create_depth_resources();
	create_uniform_buffers();
	create_descriptor_pool();
	create_descriptor_sets();
}

void Vulkan::LogicalDevice::recreate_swap_chain()
{
	vkDeviceWaitIdle(m_device);

	cleanup_swapchain();
	m_swapchains[0]->recreate_swapchain();
	create_swap_chain();

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
		pool.reset();
	}
	for (auto& pool : computePool) {
		pool.reset();
	}
	for (auto& pool : transferPool) {
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
		vmaDestroyBuffer(r_device.get_vma_allocator(), buffer, allocation);
	}
	deletedBufferAllocations.clear();
	for (auto [image, allocation] : deletedImageAllocations) {
		vmaDestroyImage(r_device.get_vma_allocator(), image, allocation);
	}
	deletedImageAllocations.clear();

	for (auto image : deletedImages) {
		vkDestroyImage(r_device.get_device(), image, r_device.get_allocator());
	}
	deletedImages.clear();

	for (auto imageView : deletedImageViews) {
		vkDestroyImageView(r_device.get_device(), imageView, r_device.get_allocator());
	}
	deletedImageViews.clear();

	for (auto bufferView : deletedBufferViews) {
		vkDestroyBufferView(r_device.get_device(), bufferView, r_device.get_allocator());
	}
	deletedBufferViews.clear();
}
