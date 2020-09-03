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
	vmaCreateBuffer(m_vmaAllocator, &bufferCreateInfo, &allocInfo, &buffer, &allocation, nullptr);
	return std::make_pair(buffer, allocation);
}


void Vulkan::LogicalDevice::cleanup_swapchain()
{

	vmaDestroyImage(m_vmaAllocator, m_depthImage, m_depthImageAllocation);
	vkDestroyImageView(m_device, m_depthImageView, m_allocator);
	for (size_t i = 0; i < m_swapChainImages.size(); i++) {
		vmaDestroyBuffer(m_vmaAllocator, m_uniformBuffers[i], m_uniformBuffersAllocations[i]);
	}
	vkDestroyDescriptorPool(m_device, m_descriptorPool, m_allocator);
	for (auto imageView : m_swapChainImageViews)
		vkDestroyImageView(m_device, imageView, m_allocator);

	for (auto framebuffer : m_swapChainFramebuffers)
		vkDestroyFramebuffer(m_device, framebuffer, m_allocator);

	vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
	
	//vkDestroyPipeline(m_device, m_graphicsPipeline, m_allocator);
	//vkDestroyPipelineLayout(m_device, m_pipelineLayout, m_allocator);
	//vkDestroyRenderPass(m_device, m_renderPass, m_allocator);
	
	vkDestroySwapchainKHR(m_device, m_swapChain, m_allocator);
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
void Vulkan::LogicalDevice::create_texture_sampler()
{
	VkSamplerCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = 16.0f,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	if (auto result = vkCreateSampler(m_device, &createInfo, m_allocator, &m_imageSampler); result != VK_SUCCESS) {
		throw std::runtime_error("Could not create sampler");
	}
}
void Vulkan::LogicalDevice::create_depth_resources()
{
	VkFormat format = VK_FORMAT_D16_UNORM;
	auto [image, allocation] = create_image(m_swapChainExtent.width, m_swapChainExtent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	m_depthImage = image;
	m_depthImageAllocation = allocation;
	m_depthImageView = create_image_view(format, m_depthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
}
void Vulkan::LogicalDevice::create_vertex_buffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	auto [stagingBuffer, stagingAllocation] = create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	void* data;

	vmaMapMemory(m_vmaAllocator, stagingAllocation, &data);
	std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vmaUnmapMemory(m_vmaAllocator, stagingAllocation);

	auto[buffer, allocation] = create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	m_vertexBuffer = buffer;
	m_vertexBufferAllocation = allocation;
	copy_buffer(stagingBuffer, m_vertexBuffer, bufferSize);

	vmaDestroyBuffer(m_vmaAllocator, stagingBuffer, stagingAllocation);
}
void Vulkan::LogicalDevice::create_index_buffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	auto [stagingBuffer, stagingAllocation] = create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	void* data;

	vmaMapMemory(m_vmaAllocator, stagingAllocation, &data);
	std::memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
	vmaUnmapMemory(m_vmaAllocator, stagingAllocation);

	auto [buffer, allocation] = create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	m_indexBuffer = buffer;
	m_indexBufferAllocation = allocation;
	copy_buffer(stagingBuffer, m_indexBuffer, bufferSize);

	vmaDestroyBuffer(m_vmaAllocator, stagingBuffer, stagingAllocation);
}

void Vulkan::LogicalDevice::create_uniform_buffers()
{
	VkDeviceSize bufferSize = sizeof(Ubo);
	m_uniformBuffers.resize(m_swapChainImages.size());
	m_uniformBuffersAllocations.resize(m_swapChainImages.size());
	for (size_t i = 0; i < m_swapChainImages.size(); i++) {
		auto[buffer, allocation ] = create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		m_uniformBuffers[i] = buffer;
		m_uniformBuffersAllocations[i] = allocation;
	}
}

void Vulkan::LogicalDevice::create_swapchain()
{
	VkSurfaceCapabilitiesKHR capabilities;

	uint32_t numFormats;
	uint32_t numModes;
	vkGetPhysicalDeviceSurfaceFormatsKHR(r_instance.m_physicalDevice, r_instance.m_surface, &numFormats, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(r_instance.m_physicalDevice, r_instance.m_surface, &numModes, nullptr);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(numFormats);
	std::vector<VkPresentModeKHR> presentModes(numModes);
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r_instance.m_physicalDevice, r_instance.m_surface, &capabilities);
	vkGetPhysicalDeviceSurfaceFormatsKHR(r_instance.m_physicalDevice, r_instance.m_surface, &numFormats, surfaceFormats.data());
	vkGetPhysicalDeviceSurfacePresentModesKHR(r_instance.m_physicalDevice, r_instance.m_surface, &numModes, presentModes.data());
	VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
	if (auto _surfaceFormat = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(),
		[](auto format) {return format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; });
		_surfaceFormat != surfaceFormats.cend())
		surfaceFormat = *_surfaceFormat;
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (std::find(presentModes.cbegin(), presentModes.cend(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.cend()) {
		presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}
	m_swapChainExtent = capabilities.currentExtent;
	if (m_swapChainExtent.width == UINT32_MAX) {
		m_swapChainExtent = capabilities.maxImageExtent;
	}
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = r_instance.m_surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = m_swapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, //TODO: For defered rendering switch to VK_IMAGE_USAGE_TRANSFER_DST_BIT
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};
	if (auto result = vkCreateSwapchainKHR(m_device, &createInfo, m_allocator, &m_swapChain); result != VK_SUCCESS) {
		if (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
			throw std::runtime_error("VK: could not create swapchain, native window in use");
		}
		if (result == VK_ERROR_INITIALIZATION_FAILED) {
			throw std::runtime_error("VK: could not create swapchain, initialization failed");
		}
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw std::runtime_error("VK: could not create swapchain, surface lost");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not create swapchain, device lost");
		}
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create swapchain, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create swapchain, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	m_swapChainImageFormat = surfaceFormat.format;
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
	
}
template<typename VertexType, size_t numShaders>
void Vulkan::LogicalDevice::create_graphics_pipeline(std::array<Shader*, numShaders> shaders)
{

	std::array<VkPipelineShaderStageCreateInfo, numShaders> shaderStages;
	for (size_t i = 0; i < numShaders; i++) {
		shaderStages[i] = shaders[i]->get_create_info();
	}

	auto bindingDescriptions = VertexType::get_binding_descriptions();
	auto attributeDescriptions = VertexType::get_attribute_descriptions();
	
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
		.pVertexBindingDescriptions = bindingDescriptions.data(),
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data()
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = (float) m_swapChainExtent.width,
		.height = (float) m_swapChainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor{
		.offset = {0,0},
		.extent = m_swapChainExtent
	};

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.f,
		.depthBiasClamp = 0.f,
		.depthBiasSlopeFactor = 0.f,
		.lineWidth = 1.0f,

	};

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_TRUE,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = nullptr, // Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE, // Optional
	};

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front{},
		.back{},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};
	VkPipelineColorBlendAttachmentState colorBlendAttachement{
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachement,
		.blendConstants{0.0f, 0.0f, 0.0f, 0.0f}
	};
	VkDynamicState dynamicStates[]{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates
	};

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &m_descriptorSetLayout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr
	};
	

	//if (auto result = vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, m_allocator, &m_pipelineLayout); result != VK_SUCCESS) {
	//	if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
	//		throw std::runtime_error("VK: could not create pipeline layout, out of host memory");
	//	}
	//	if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
	//		throw std::runtime_error("VK: could not create pipeline layout, out of device memory");
	//	}
	//	else {
	//		throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
	//	}
	//}
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = static_cast<uint32_t>(shaderStages.size()),
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputStateCreateInfo,
		.pInputAssemblyState = &inputAssemblyStateCreateInfo,
		.pViewportState = &viewportStateCreateInfo,
		.pRasterizationState = &rasterizationStateCreateInfo,
		.pMultisampleState = &multisampleStateCreateInfo,
		.pDepthStencilState = &depthStencilStateCreateInfo,
		.pColorBlendState = &colorBlendStateCreateInfo,
		.pDynamicState = nullptr, //&dynamicStateCreateInfo
		.layout = m_pipelineLayout,
		.renderPass = m_renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	if (auto result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, m_allocator, &m_graphicsPipeline); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create graphics pipeline, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create graphics pipeline, out of device memory");
		}
		if (result == VK_ERROR_INVALID_SHADER_NV) {
			throw std::runtime_error("VK: could not create graphics pipeline, invalid shader nv");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}

}
Vulkan::LogicalDevice::LogicalDevice(const Vulkan::Instance& parentInstance, VkDevice device, uint32_t graphicsFamilyQueueIndex, VkPhysicalDeviceProperties& properties) : 
	r_instance(parentInstance),
	m_device(device), 
	m_graphicsFamilyQueueIndex(graphicsFamilyQueueIndex),
	m_physicalProperties(properties)
{
	vkGetDeviceQueue(m_device, graphicsFamilyQueueIndex, 0, &m_graphicsQueue);
	create_vma_allocator();
	
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
	DescriptorSetLayout l;
	
	create_descriptor_set_layout(std::array< VkDescriptorSetLayoutBinding, 2>{uboLayoutBinding, samplerLayoutBinding});
	create_swapchain();
	create_image_views();
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
	
	
	create_texture_sampler();
	create_depth_resources();
	create_framebuffers();
	create_uniform_buffers();
	create_descriptor_pool();
	

	
}
Vulkan::LogicalDevice::~LogicalDevice()
{

	cleanup_swapchain();
	m_descriptorAllocatorsStorage.destroy();
	m_shaderStorage.destroy();
	m_programStorage.destroy();
	m_pipelineLayoutStorage.destroy();
	m_pipeline->~Pipeline();
	delete m_testRenderPass;
	vkDestroySampler(m_device, m_imageSampler, m_allocator);
	vkDestroyImageView(m_device, m_imageView, m_allocator);
	vmaDestroyImage(m_vmaAllocator, m_image, m_imageAllocation);
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, m_allocator);
	vmaDestroyBuffer(m_vmaAllocator, m_vertexBuffer, m_vertexBufferAllocation);
	vmaDestroyBuffer(m_vmaAllocator, m_indexBuffer, m_indexBufferAllocation);
	for(auto semaphore : m_imageAvailableSemaphores)
		vkDestroySemaphore(m_device, semaphore, m_allocator);
	for (auto semaphore : m_renderFinishedSemaphores)
		vkDestroySemaphore(m_device, semaphore, m_allocator);
	for (auto fence : m_inFlightFences)
		vkDestroyFence(m_device, fence, m_allocator);
	vkDestroyCommandPool(m_device, m_commandPool, m_allocator);
	vmaDestroyAllocator(m_vmaAllocator);
	vkDestroyDevice(m_device, nullptr);
}
void Vulkan::LogicalDevice::create_image_views()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());
	for (int i = 0; i < m_swapChainImages.size(); i++) {
		m_swapChainImageViews[i] = create_image_view(m_swapChainImageFormat, m_swapChainImages[i], VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

VkImageView Vulkan::LogicalDevice::create_image_view(VkFormat format, VkImage image, VkImageAspectFlags aspect)
{
	VkImageView imageView;
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
		delete m_testRenderPass;
	RenderpassCreateInfo createInfo;
	createInfo.colorAttachmentsCount = 1;
	createInfo.clearAttachments.set(0);
	createInfo.storeAttachments.set(0);
	createInfo.attachmentInfos[0].format = m_swapChainImageFormat;
	createInfo.attachmentInfos[0].isSwapchainImage = true;
	createInfo.attachmentInfos[0].layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	createInfo.usingDepth = true;
	createInfo.subpassCount = 1;
	createInfo.opFlags.set(static_cast<uint32_t>(RenderpassCreateInfo::OpFlags::DepthStencilClear));

	createInfo.subpasses[0].colorAttachmentsCount = 1;
	createInfo.subpasses[0].colorAttachments[0] = 0;
	createInfo.subpasses[0].depthStencil = RenderpassCreateInfo::DepthStencil::ReadWrite;

	m_testRenderPass = new Renderpass(*this, createInfo);
	m_renderPass = m_testRenderPass->get_render_pass();
	//VkAttachmentDescription colorAttachment{
	//	.format = m_swapChainImageFormat,
	//	.samples = VK_SAMPLE_COUNT_1_BIT,
	//	.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	//	.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	//	.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	//	.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//	.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	//	.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	//};

	//VkAttachmentReference colorAttachmentReference{
	//	.attachment = 0,
	//	.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	//};
	//
	//VkAttachmentDescription depthAttachment{
	//	.format = VK_FORMAT_D16_UNORM,
	//	.samples = VK_SAMPLE_COUNT_1_BIT,
	//	.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	//	.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//	.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	//	.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//	.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	//	.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	//};
	//VkAttachmentReference depthAttachmentReference{
	//	.attachment = 1,
	//	.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	//};
	//VkSubpassDescription subpassDescription{
	//	.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
	//	.colorAttachmentCount = 1,
	//	.pColorAttachments = &colorAttachmentReference,
	//	.pDepthStencilAttachment = &depthAttachmentReference
	//};
	////VkRenderPassMultiviewCreateInfo
	//VkSubpassDependency subpassDependency{
	//	.srcSubpass = VK_SUBPASS_EXTERNAL,
	//	.dstSubpass = 0,
	//	.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//	.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//	.srcAccessMask = 0,
	//	.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	//};
	//std::array< VkAttachmentDescription, 2> attachments{ colorAttachment ,depthAttachment };
	//VkRenderPassCreateInfo renderPassCreateInfo{
	//	.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	//	.attachmentCount = static_cast<uint32_t>(attachments.size()),
	//	.pAttachments = attachments.data(),
	//	.subpassCount = 1,
	//	.pSubpasses = &subpassDescription,
	//	.dependencyCount = 1,
	//	.pDependencies = &subpassDependency
	//};
	//if (auto result = vkCreateRenderPass(m_device, &renderPassCreateInfo, m_allocator, &m_renderPass); result != VK_SUCCESS) {
	//	if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
	//		throw std::runtime_error("VK: could not create render pass, out of host memory");
	//	}
	//	if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
	//		throw std::runtime_error("VK: could not create render pass, out of device memory");
	//	}
	//	else {
	//		throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
	//	}
	//}
}

void Vulkan::LogicalDevice::create_framebuffers()
{
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
	for (int i = 0; i < m_swapChainImageViews.size(); i++) {
		std::array<VkImageView, 2> attachments{m_swapChainImageViews[i], m_depthImageView};
		
		VkFramebufferCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = m_renderPass,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.width = m_swapChainExtent.width,
			.height = m_swapChainExtent.height,
			.layers = 1
		};
		if (auto result = vkCreateFramebuffer(m_device, &createInfo, m_allocator, &m_swapChainFramebuffers[i]); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not create framebuffer, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not create framebuffer, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
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
	m_commandBuffers.resize(m_swapChainFramebuffers.size());
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
	for (int i = 0; i < m_commandBuffers.size(); i++) {
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
		std::array<VkClearValue, 2> clearColors{
			VkClearValue{0.0f, 0.0f, 0.0f, 1.0f},
			VkClearValue{1.0f}
		};
		VkRenderPassBeginInfo renderPassBeginInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = m_renderPass,
			.framebuffer = m_swapChainFramebuffers[i],
			.renderArea{
				.offset {0,0},
				.extent = m_swapChainExtent
			},
			.clearValueCount = static_cast<uint32_t>(clearColors.size()),
			.pClearValues = clearColors.data(),
		};
		VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)m_swapChainExtent.width,
		.height = (float)m_swapChainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
		};

		VkRect2D scissor{
			.offset = {0,0},
			.extent = m_swapChainExtent
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
	m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		
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
				throw std::runtime_error("VK: error "+ result+ std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
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
	VmaAllocatorCreateInfo allocatorInfo{
		.physicalDevice = r_instance.m_physicalDevice,
		.device = m_device,
		.instance = r_instance.m_instance,
	};
	if (auto result = vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator); result != VK_SUCCESS) {
		throw std::runtime_error("VMA: could not create allocator");
	}
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
	std::vector<VkDescriptorSetLayout> layouts(m_swapChainImages.size(), m_descriptorSetLayout);
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
			.sampler = m_imageSampler,
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
			.descriptorCount = static_cast<uint32_t>(m_swapChainImages.size())
		},
		VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = static_cast<uint32_t>(m_swapChainImages.size())
		}
	};
	VkDescriptorPoolCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = static_cast<uint32_t>(m_swapChainImages.size()),
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
	VkImage image;
	VmaAllocation allocation;
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
	
	if (auto result = vmaCreateImage(m_vmaAllocator, &createInfo, &allocInfo, &image, &allocation, nullptr); result != VK_SUCCESS) {
		throw std::runtime_error("Vk: error creating image");
	}
	return std::make_pair(image, allocation);
}

void Vulkan::LogicalDevice::create_texture_image(uint32_t width, uint32_t height, uint32_t channels, char* imageData)
{
	VkDeviceSize imageSize = width * height * channels;

	auto [stagingBuffer, stagingAllocation] = create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	void* data;
	vmaMapMemory(m_vmaAllocator, stagingAllocation, &data);
	std::memcpy(data, imageData, imageSize);
	vmaUnmapMemory(m_vmaAllocator, stagingAllocation);

	auto [image, imageAllocation] = create_image(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	m_image = image;
	m_imageAllocation = imageAllocation;
	transition_image_layout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copy_buffer_to_image(stagingBuffer, m_image, width, height);
	transition_image_layout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vmaDestroyBuffer(m_vmaAllocator, stagingBuffer, stagingAllocation);
	
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
	
	return nullptr;
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
	m_pipeline = new(pipelineStorage)Pipeline(Pipeline::request_pipeline(*this, program, m_testRenderPass, 0));
	m_graphicsPipeline = m_pipeline->get_pipeline();
}




void Vulkan::LogicalDevice::update_uniform_buffer(uint32_t currentImage)
{

	Ubo ubo{
		bla::mat44::identity(),
		bla::mat44::identity(),
		bla::mat44::identity()
	};
	static float degrees;
	ubo.model = bla::mat44(bla::mat33::rotation_matrix(0, 0, (degrees++) / 100));
	ubo.model(3, 3) = 1;
	ubo.view = bla::mat44::look_at(bla::vec3({ 2.0f, 2.0f, 2.0f }), bla::vec3({ 0.0f, 0.0f, 0.0f }), bla::vec3({0.0f, 0.0f, 1.0f}));
	ubo.proj = bla::mat44::perspective(0.01f, 10.f, 45.0f, static_cast<float>(m_swapChainExtent.width)/ static_cast<float>(m_swapChainExtent.height));
	void* mappedData;
	vmaMapMemory(m_vmaAllocator, m_uniformBuffersAllocations[currentImage], &mappedData);
	memcpy(mappedData, &ubo, sizeof(Ubo));
	vmaUnmapMemory(m_vmaAllocator, m_uniformBuffersAllocations[currentImage]);
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
	uint32_t imageIndex;
	if (auto result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not acquire next image, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not acquire next image, out of device memory");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not acquire next image, device lost");
		}
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate_swap_chain();
			return;
		}
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw std::runtime_error("VK: could not acquire next image, surface lost");
		}
		if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) {
			throw std::runtime_error("VK: could not acquire next image, fullscreen exclusive mode lost");
		}
		if (result == VK_SUBOPTIMAL_KHR) {

		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
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
	VkSwapchainKHR swapChains[]{ m_swapChain };
	VkResult results = VK_SUCCESS;
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = &m_swapChain,
		.pImageIndices = &imageIndex,
		.pResults = &results
	};

	if (auto result = vkQueuePresentKHR(m_graphicsQueue, &presentInfo); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not present, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not present, out of device memory");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not present, device lost");
		}
		
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw std::runtime_error("VK: could not present, surface lost");
		}
		if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) {
			throw std::runtime_error("VK: could not present, fullscreen exclusive mode lost");
		}
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreate_swap_chain();
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	m_currentFrame++;
	if (m_currentFrame == MAX_FRAMES_IN_FLIGHT)
		m_currentFrame = 0;
	vkQueueWaitIdle(m_graphicsQueue);
}

void Vulkan::LogicalDevice::wait_idle()
{
	vkDeviceWaitIdle(m_device);
}

void Vulkan::LogicalDevice::create_swap_chain()
{
	create_swapchain();
	create_image_views();
	create_render_pass();
	//auto vertShaderCode = read_binary_file("basic_vert.spv");
	//auto fragShaderCode = read_binary_file("basic_frag.spv");
	//ShaderLayout l;
	//Shader vert(*this, vertShaderCode);
	//Shader frag(*this, fragShaderCode);

	//create_graphics_pipeline<Vertex, 2>({&frag, &vert });
	create_depth_resources();
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
	create_swap_chain();

}

