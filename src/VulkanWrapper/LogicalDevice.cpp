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
	create_render_pass();
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
	
	create_framebuffers();
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
	for (auto semaphore : m_imageAvailableSemaphores)
		vkDestroySemaphore(m_device, semaphore, m_allocator);
	for (auto semaphore : m_renderFinishedSemaphores)
		vkDestroySemaphore(m_device, semaphore, m_allocator);
	for (auto fence : m_inFlightFences)
		vkDestroyFence(m_device, fence, m_allocator);
	vkDestroyCommandPool(m_device, m_commandPool, m_allocator);
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
	m_swapChainFramebuffers.clear();

	vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
	
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
Utility::HashValue Vulkan::LogicalDevice::hash_compatible_renderpass(const RenderpassCreateInfo& info)
{
	Utility::Hasher hasher;
	//std::bitset<MAX_ATTACHMENTS + 1> optimalLayouts;

	hasher(info.colorAttachmentsCount);
	for (uint32_t i = 0; i < info.colorAttachmentsCount; i++) {
		auto attachment = info.colorAttachmentsViews[i];
		hasher(attachment->get_format());
		//Ignore Optimal Layout for now
		//if (attachment->get_image()->get_info().layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		//	optimalLayouts.set(i);

		hasher(attachment->get_image()->get_info().layout);

	}
	if (info.usingDepth) {
		//Ignore Optimal Layout for now
		//if (info.depthStencilAttachment->get_image()->get_info().layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		//	optimalLayouts.set(info.colorAttachmentsCount);
		hasher(info.depthStencilAttachment->get_format());
	}
	else {
		hasher(VK_FORMAT_UNDEFINED);
	}


	//Ignore Multiview for now
	hasher(info.subpassCount);
	for (uint32_t i = 0; i < info.subpassCount; i++) {
		const auto& subpass = info.subpasses[i];
		hasher(subpass.colorAttachmentsCount);
		hasher(subpass.inputAttachmentsCount);
		hasher(subpass.resolveAttachmentsCount);
		hasher(subpass.depthStencil);
		for (uint32_t j = 0; j < subpass.colorAttachmentsCount; j++)
			hasher(subpass.colorAttachments[j]);
		for (uint32_t j = 0; j < subpass.inputAttachmentsCount; j++)
			hasher(subpass.inputAttachments[j]);
		for (uint32_t j = 0; j < subpass.resolveAttachmentsCount; j++)
			hasher(subpass.resolveAttachments[j]);
	}
	return hasher();
}
VkCommandBuffer Vulkan::LogicalDevice::begin_single_time_commands()
{
	VkCommandBufferAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_commandPool,
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
	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
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

Vulkan::LogicalDevice::LogicalDevice(const Vulkan::Instance& parentInstance, VkDevice device, uint32_t graphicsFamilyQueueIndex, uint32_t transferFamilyQueueIndex, VkPhysicalDeviceProperties& properties) : 
	r_instance(parentInstance),
	m_device(device, nullptr), 
	m_graphicsFamilyQueueIndex(graphicsFamilyQueueIndex),
	m_transferFamilyQueueIndex(transferFamilyQueueIndex == ~0 ? graphicsFamilyQueueIndex : transferFamilyQueueIndex),
	m_physicalProperties(properties)
{
	vkGetDeviceQueue(m_device, m_graphicsFamilyQueueIndex, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, m_transferFamilyQueueIndex, 0, &m_transferQueue);
	create_vma_allocator();
	create_default_sampler();		
}
Vulkan::LogicalDevice::~LogicalDevice()
{
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

void Vulkan::LogicalDevice::create_render_pass()
{
	if (m_testRenderPass)
		m_testRenderPass.reset(nullptr);
	m_createInfo.colorAttachmentsCount = 1;
	m_createInfo.clearAttachments.set(0);
	m_createInfo.storeAttachments.set(0);
	m_createInfo.colorAttachmentsViews[0] = m_swapchains[0]->get_swapchain_image_view(0);

	m_createInfo.usingDepth = true;
	m_createInfo.depthStencilAttachment = m_depthView.get();
	m_createInfo.subpassCount = 1;
	m_createInfo.opFlags.set(static_cast<uint32_t>(RenderpassCreateInfo::OpFlags::DepthStencilClear));

	//createInfo.subpasses[0].colorAttachmentsCount = 1;
	//createInfo.subpasses[0].colorAttachments[0] = 0;
	//createInfo.subpasses[0].depthStencil = RenderpassCreateInfo::DepthStencil::ReadWrite;
	m_createInfo.subpasses[0] = RenderpassCreateInfo::SubpassCreateInfo{
		.colorAttachments {0},
		.depthStencil = RenderpassCreateInfo::DepthStencil::ReadWrite,
		.colorAttachmentsCount = 1,
		
	};
	m_testRenderPass = std::make_unique<Renderpass>(*this, m_createInfo);
	m_renderPass = m_testRenderPass->get_render_pass();

}

void Vulkan::LogicalDevice::create_framebuffers()
{
	for (size_t i = 0; i < m_swapchains[0]->get_image_count(); i++) {
		m_createInfo.colorAttachmentsViews[0] = m_swapchains[0]->get_swapchain_image_view(static_cast<uint32_t>(i));
		m_swapChainFramebuffers.emplace_back(new Framebuffer(*this, m_createInfo));
	}
}

void Vulkan::LogicalDevice::create_command_pool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = 0,
		.queueFamilyIndex = m_graphicsFamilyQueueIndex,
	};
	if (auto result = vkCreateCommandPool(m_device, &commandPoolCreateInfo, m_allocator, &m_commandPool); result != VK_SUCCESS) {
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

void Vulkan::LogicalDevice::create_command_buffers()
{
	m_commandBuffers.resize(m_swapchains[0]->get_image_count());
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t> (m_commandBuffers.size()),
	};

	if (auto result = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, m_commandBuffers.data()); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not allocate command buffers, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not allocate command buffers, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	for (size_t i = 0; i < m_commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo commandBufferBeginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};
		if (auto result = vkBeginCommandBuffer(m_commandBuffers[i], &commandBufferBeginInfo); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not begin command buffer, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not begin command buffer, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
		
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
		VkRenderPassBeginInfo renderPassBeginInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = m_renderPass,
			.framebuffer = m_swapChainFramebuffers[i]->get_handle(),
			.renderArea = scissor,
			.clearValueCount = static_cast<uint32_t>(clearColors.size()),
			.pClearValues = clearColors.data(),
		};
		vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
		vkCmdSetViewport(m_commandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(m_commandBuffers[i], 0, 1, &scissor);
		VkBuffer vertexBuffers[]{ m_vertexBuffer };
		VkDeviceSize offsets[]{ 0 };
		vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		uint32_t offset[] = { 0,0 };
		vkCmdBindDescriptorSets(m_commandBuffers[i],VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i],1, offset);
		//vkCmdPushConstants(m_commandBuffers[i], m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Math::mat44), &model);
		vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		offset[0] = sizeof(Ubo);
		vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 1, offset);
		vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(m_commandBuffers[i]);
		
		if (auto result = vkEndCommandBuffer(m_commandBuffers[i]); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not end command buffer, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not end command buffer, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
	}
}

void Vulkan::LogicalDevice::create_sync_objects()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	VkFenceCreateInfo fenceCreateInfo{
		.sType =VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	m_imagesInFlight.resize(m_swapchains[0]->get_image_count(), VK_NULL_HANDLE);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		
		if (auto result = vkCreateSemaphore(m_device, &semaphoreCreateInfo, m_allocator, &m_imageAvailableSemaphores[i]); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not create Semaphore, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not create Semaphore, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
		if (auto result = vkCreateSemaphore(m_device, &semaphoreCreateInfo, m_allocator, &m_renderFinishedSemaphores[i]); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not create Semaphore, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not create Semaphore, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error "+ std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
		if (auto result = vkCreateFence(m_device, &fenceCreateInfo, m_allocator, &m_inFlightFences[i]); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not create fence, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not create fence, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
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
	auto compatibleHash = hash_compatible_renderpass(info);
	Utility::Hasher hasher(compatibleHash);
	
	hasher(info.opFlags.to_ulong());
	hasher(info.loadAttachments.to_ulong());
	hasher(info.clearAttachments.to_ulong());
	hasher(info.storeAttachments.to_ulong());

	auto result = m_renderpassIds.find(hasher());
	size_t renderpassId = 0;
	if (result == m_renderpassIds.end()) {
		renderpassId = m_renderpassStorage.emplace(*this, info);
		m_renderpassIds[hasher()] = renderpassId;
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
	auto compatibleHash = hash_compatible_renderpass(info);

	auto result = m_compatibleRenderpassIds.find(compatibleHash);
	size_t renderpassId = 0;
	if (m_compatibleRenderpassIds.find(compatibleHash) == m_compatibleRenderpassIds.end()) {
		renderpassId = m_renderpassStorage.emplace(*this, info);
		m_compatibleRenderpassIds[compatibleHash] = renderpassId;

		Utility::Hasher hasher(compatibleHash);
		hasher(info.opFlags.to_ulong());
		hasher(info.loadAttachments.to_ulong());
		hasher(info.clearAttachments.to_ulong());
		hasher(info.storeAttachments.to_ulong());
		m_renderpassIds[hasher()] = renderpassId;
	}
	else {
		renderpassId = result->second;
	}
	return m_renderpassStorage.get(renderpassId);
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
	m_pipeline = std::make_unique<Pipeline>(Pipeline::request_pipeline(*this, program, m_testRenderPass.get(), 0));
	m_graphicsPipeline = m_pipeline->get_pipeline();
}




void Vulkan::LogicalDevice::update_uniform_buffer(uint32_t currentImage)
{

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

void Vulkan::LogicalDevice::draw_frame()
{
	if (auto result = vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not wait for fence(s), out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not wait for fence(s), out of device memory");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not wait for fence(s), device lost");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	uint32_t imageIndex = m_swapchains[0]->aquire_next_image(m_imageAvailableSemaphores[m_currentFrame]);
	if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		if (auto result = vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not wait for fence(s), out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not wait for fence(s), out of device memory");
			}
			if (result == VK_ERROR_DEVICE_LOST) {
				throw std::runtime_error("VK: could not wait for fence(s), device lost");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
	}
	m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

	VkSemaphore waitSemaphores[]{ m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[]{ m_renderFinishedSemaphores[m_currentFrame] };

	update_uniform_buffer(imageIndex);
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_commandBuffers[imageIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores
	};

	if (auto result = vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not reset fence(s), out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not reset fence(s), out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}

	if (auto result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not submit Queue, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not submit Queue, out of device memory");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not submit Queue, device lost");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	m_swapchains[0]->present_to_queue(m_graphicsQueue, signalSemaphores, 1);
	
	m_currentFrame++;
	if (m_currentFrame == MAX_FRAMES_IN_FLIGHT)
		m_currentFrame = 0;
	//vkQueueWaitIdle(m_graphicsQueue);
}

void Vulkan::LogicalDevice::wait_idle()
{
	vkDeviceWaitIdle(m_device);
}

void Vulkan::LogicalDevice::create_swap_chain()
{
	

	create_depth_resources();
	create_render_pass();
	create_framebuffers();
	create_uniform_buffers();
	create_descriptor_pool();
	create_descriptor_sets();
	create_command_buffers();
}

void Vulkan::LogicalDevice::recreate_swap_chain()
{
	vkDeviceWaitIdle(m_device);

	cleanup_swapchain();
	m_swapchains[0]->recreate_swapchain();
	create_swap_chain();

}

