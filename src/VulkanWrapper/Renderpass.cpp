#include "Renderpass.h"
#include "Framebuffer.h"
#include "LogicalDevice.h"

vulkan::Renderpass::Renderpass(LogicalDevice& parent, const RenderpassCreateInfo& createInfo) : r_device(parent)
{
	std::tie(m_compatibleHashValue, m_hashValue) = createInfo.get_hash();
	uint32_t subpassCount = createInfo.subpassCount;
	auto s = createInfo.subpasses.data();
	const RenderpassCreateInfo::SubpassCreateInfo* subpasses = createInfo.subpasses.data();
	RenderpassCreateInfo::SubpassCreateInfo defaultPass;
	if (subpassCount == 0) {
		defaultPass.colorAttachmentsCount = createInfo.colorAttachmentsCount;
		if (createInfo.opFlags.test(RenderpassCreateInfo::OpFlags::DepthStencilReadOnly))
			defaultPass.depthStencil = RenderpassCreateInfo::DepthStencil::Read;
		else
			defaultPass.depthStencil = RenderpassCreateInfo::DepthStencil::ReadWrite;
		for (uint32_t i = 0; i < createInfo.colorAttachmentsCount; i++) {
			defaultPass.colorAttachments[i] = i;
		}
		subpassCount = 1;
		subpasses = &defaultPass;
	}
	std::bitset<MAX_ATTACHMENTS> implicitTransition;
	std::bitset<MAX_ATTACHMENTS> implicitBottomOfPipe;
	const unsigned attachmentCount = createInfo.colorAttachmentsCount + (createInfo.depthStencilAttachment ? 1: 0);
	assert(!(createInfo.loadAttachments & createInfo.clearAttachments).any());
	std::array<VkAttachmentDescription, MAX_ATTACHMENTS + 1> attachmentDescriptions{};

	VkAttachmentLoadOp depthStencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	VkAttachmentStoreOp depthStencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	
	if (createInfo.opFlags.test(RenderpassCreateInfo::OpFlags::DepthStencilClear))
		depthStencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	else if (createInfo.opFlags.test(RenderpassCreateInfo::OpFlags::DepthStencilLoad))
		depthStencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

	if (createInfo.opFlags.test(RenderpassCreateInfo::OpFlags::DepthStencilStore))
		depthStencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkImageLayout depthStencilLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (createInfo.depthStencilAttachment) {
		//TODO get depth layout
		bool test = createInfo.opFlags.test(RenderpassCreateInfo::OpFlags::DepthStencilReadOnly);
		depthStencilLayout = test ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	const auto get_color_load_op = [&createInfo](uint32_t idx) {
		if (createInfo.clearAttachments.test(idx))
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		else if (createInfo.loadAttachments.test(idx))
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		else
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	};
	const auto get_color_store_op = [&createInfo](uint32_t idx) {
		if (createInfo.storeAttachments.test(idx))
			return VK_ATTACHMENT_STORE_OP_STORE;
		else
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	};
	for (uint32_t i = 0; i < createInfo.colorAttachmentsCount; i++) {
		auto info = createInfo.colorAttachmentsViews[i];
		colorAttachments[i] = info->get_format();
		VkAttachmentDescription colorAttachment{
			.format = colorAttachments[i],
			.samples = info->get_image()->get_info().samples,
			.loadOp = get_color_load_op(i),
			.storeOp = get_color_store_op(i),
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		//TODO
		//Ignore transient for now
		if (info->get_image()->get_info().isSwapchainImage) {
			if (colorAttachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
				implicitBottomOfPipe.set(i);
				colorAttachment.initialLayout = info->get_image()->get_info().layout;
			}
			colorAttachment.finalLayout = info->get_image()->get_info().layout;
			implicitTransition.set(i);
		}
		else {
			if (colorAttachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
				implicitBottomOfPipe.set(i);
				colorAttachment.initialLayout = info->get_image()->get_info().layout;
			}
		}
		attachmentDescriptions[i] = colorAttachment;
	}
	depthStencilAttachment = createInfo.depthStencilAttachment ? createInfo.depthStencilAttachment->get_format() : VK_FORMAT_UNDEFINED;

	if (createInfo.depthStencilAttachment) {
		VkAttachmentDescription depthAttachment{
		.format = depthStencilAttachment,
		.samples = createInfo.depthStencilAttachment->get_image()->get_info().samples,
		.loadOp = depthStencilLoadOp,
		.storeOp = depthStencilStoreOp,
		//TODO Handle stencil
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = depthStencilLayout,
		.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		attachmentDescriptions[createInfo.colorAttachmentsCount] = depthAttachment;
	}
	std::vector<VkAttachmentReference> references;
	references.reserve(static_cast<size_t>(subpassCount) * static_cast<size_t>(MAX_ATTACHMENTS) * 3ull + static_cast<size_t>(subpassCount));
	std::vector<VkSubpassDescription> subpassDescriptions(subpassCount);
	std::vector<VkSubpassDependency> externalDependencies;
	for (uint32_t i = 0; i < subpassCount; i++) {

		uint32_t k;
		for (k = 0; k < subpasses[i].colorAttachmentsCount; k++) references.emplace_back();
		VkAttachmentReference* colors = nullptr;
		if(k) colors = &references[references.size() - k];
		for (k = 0; k < subpasses[i].inputAttachmentsCount; k++) references.emplace_back();
		VkAttachmentReference* inputs = nullptr;
		if (k) inputs = &references[references.size() - k];
		for (k = 0; k < subpasses[i].resolveAttachmentsCount; k++) references.emplace_back();
		VkAttachmentReference* resolves = nullptr;
		if(k) resolves = &references[references.size() - k];
		references.emplace_back();
		auto* depth = &references[references.size() - 1];

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = subpasses[i].inputAttachmentsCount,
			.pInputAttachments = inputs,
			.colorAttachmentCount = subpasses[i].colorAttachmentsCount,
			.pColorAttachments = colors,
			.pDepthStencilAttachment = depth
		};
		
		for (uint32_t j = 0; j < subpass.colorAttachmentCount; j++) {
			auto attachment = subpasses[i].colorAttachments[j];
			assert(attachment == VK_ATTACHMENT_UNUSED || (attachment < attachmentCount));
			colors[j].attachment = attachment;
			colors[j].layout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		for (uint32_t j = 0; j < subpass.inputAttachmentCount; j++) {
			auto attachment = subpasses[i].inputAttachments[j];
			assert(attachment == VK_ATTACHMENT_UNUSED || (attachment < attachmentCount));
			inputs[j].attachment = attachment;
			inputs[j].layout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		if (subpasses[i].resolveAttachmentsCount) {
			assert(subpasses[i].resolveAttachmentsCount == subpasses[i].colorAttachmentsCount);
			subpass.pResolveAttachments = resolves;
			for (uint32_t j = 0; j < subpass.colorAttachmentCount; j++) {
				auto attachment = subpasses[i].resolveAttachments[j];
				assert(attachment == VK_ATTACHMENT_UNUSED || (attachment < attachmentCount));
				resolves[j].attachment = attachment;
				resolves[j].layout = VK_IMAGE_LAYOUT_UNDEFINED;
			}
		}
		if (createInfo.depthStencilAttachment && subpasses[i].depthStencil != RenderpassCreateInfo::DepthStencil::None) {
			depth->attachment = createInfo.colorAttachmentsCount;
			depth->layout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else {
			depth->attachment = VK_ATTACHMENT_UNUSED;
			depth->layout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		subpassDescriptions[i] = subpass;
	}
	const auto find_color_attachment = [&](uint32_t subpass, uint32_t attachment) -> VkAttachmentReference*{
		auto* colors = subpassDescriptions[subpass].pColorAttachments;
		for (uint32_t i = 0; i < subpassDescriptions[subpass].colorAttachmentCount; i++)
			if (colors[i].attachment == attachment)
				return const_cast<VkAttachmentReference*>(&colors[i]);
		return nullptr;
	};
	const auto find_resolve_attachment = [&](uint32_t subpass, uint32_t attachment) -> VkAttachmentReference* {
		if (!subpassDescriptions[subpass].pResolveAttachments)
			return nullptr;
		auto* resolves = subpassDescriptions[subpass].pResolveAttachments;
		for (uint32_t i = 0; i < subpassDescriptions[subpass].colorAttachmentCount; i++)
			if (resolves[i].attachment == attachment)
				return const_cast<VkAttachmentReference*>(&resolves[i]);
		return nullptr;
	}; 
	const auto find_input_attachment = [&](uint32_t subpass, uint32_t attachment) -> VkAttachmentReference* {
		auto* inputs = subpassDescriptions[subpass].pInputAttachments;
		for (uint32_t i = 0; i < subpassDescriptions[subpass].inputAttachmentCount; i++)
			if (inputs[i].attachment == attachment)
				return const_cast<VkAttachmentReference*>(&inputs[i]);
		return nullptr;
	}; 
	const auto find_depth_attachment = [&](uint32_t subpass, uint32_t attachment) -> VkAttachmentReference* {
		auto* depthStencil = subpassDescriptions[subpass].pDepthStencilAttachment;
		if (depthStencil->attachment == attachment)
			return const_cast<VkAttachmentReference*>(depthStencil);
		else
			return nullptr;
	
	};
	std::array<std::bitset<32>, MAX_ATTACHMENTS + 1> preserves{};

	std::array<uint32_t, MAX_ATTACHMENTS + 1> lastSubpassForAttachment{};

	assert(subpassCount <= 32);

	std::bitset<32> colorSelfDependency;
	std::bitset<32> depthSelfDependency;

	std::bitset<32> inputAttachmentRead;
	std::bitset<32> colorAttachmentReadWrite;
	std::bitset<32> depthStencilAttachmentWrite;
	std::bitset<32> depthStencilAttachmentRead;

	std::bitset<32> externalColorDependency;
	std::bitset<32> externalDepthDependency;
	std::bitset<32> externalInputDependency;
	std::bitset<32> externalBottomOfPipeDependency;
	for (uint32_t attachment = 0; attachment < attachmentCount; attachment++) {
		bool used = false;
		auto currentLayout = attachmentDescriptions[attachment].initialLayout;
		for (uint32_t subpass = 0; subpass < subpassCount; subpass++) {
			auto* color = find_color_attachment(subpass, attachment);
			auto* resolve = find_resolve_attachment(subpass, attachment);
			auto* input = find_input_attachment(subpass, attachment);
			auto* depth = find_depth_attachment(subpass, attachment);
			
			//Debug checks
			if (color || resolve)
				assert(!depth);
			if (depth)
				assert(!color && !resolve);
			if (resolve)
				assert(!color && !depth);

			if (!used && implicitTransition.test(attachment)) {
				if (color)
					externalColorDependency.set(subpass);
				if (depth)
					externalDepthDependency.set(subpass);
				if (input)
					externalInputDependency.set(subpass);
			}

			if (!used && implicitBottomOfPipe.test(attachment)) {
				externalBottomOfPipeDependency.set(subpass);
			}

			if (resolve && input) {
				//If the same attachment is used as both resolve and input attachment, we need a general image layout
				currentLayout = VK_IMAGE_LAYOUT_GENERAL;
				resolve->layout = currentLayout;
				input->layout = currentLayout;

				if (!used && attachmentDescriptions[attachment].initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
					attachmentDescriptions[attachment].initialLayout = currentLayout;
				}

				if (!used && attachmentDescriptions[attachment].initialLayout != currentLayout) {
					externalColorDependency.set(subpass);
					externalInputDependency.set(subpass);
				}
				used = true;
				lastSubpassForAttachment[attachment] = subpass;

				colorAttachmentReadWrite.set(subpass);
				inputAttachmentRead.set(subpass);
			}
			else if (resolve) {
				if (currentLayout != VK_IMAGE_LAYOUT_GENERAL)
					currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				if (!used && attachmentDescriptions[attachment].initialLayout != currentLayout)
					externalColorDependency.set(subpass);
				resolve->layout = currentLayout;
				used = true;
				lastSubpassForAttachment[attachment] = subpass;
				colorAttachmentReadWrite.set(subpass);
			}
			else if (color && input) {
				currentLayout = VK_IMAGE_LAYOUT_GENERAL;
				color->layout = currentLayout;
				input->layout = currentLayout;
				if (!used && attachmentDescriptions[attachment].initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
					attachmentDescriptions[attachment].initialLayout = currentLayout;
				}
				if (!used && attachmentDescriptions[attachment].initialLayout != currentLayout) {
					externalColorDependency.set(subpass);
					externalInputDependency.set(subpass);
				}
				used = true;
				lastSubpassForAttachment[attachment] = subpass;
				colorSelfDependency.set(subpass);

				colorAttachmentReadWrite.set(subpass);
				inputAttachmentRead.set(subpass);
			}
			else if (color) {
				if (currentLayout != VK_IMAGE_LAYOUT_GENERAL)
					currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				color->layout = currentLayout;
				if (!used && attachmentDescriptions[attachment].initialLayout != currentLayout) {
					externalColorDependency.set(subpass);
				}

				used = true;
				lastSubpassForAttachment[attachment] = subpass;
				colorAttachmentReadWrite.set(subpass);
			}
			else if (depth && input) {
				assert(subpasses[subpass].depthStencil != RenderpassCreateInfo::DepthStencil::None);
				if (subpasses[subpass].depthStencil == RenderpassCreateInfo::DepthStencil::ReadWrite) {
					depthSelfDependency.set(subpass);
					currentLayout = VK_IMAGE_LAYOUT_GENERAL;
					depthStencilAttachmentWrite.set(subpass);
					if (!used && attachmentDescriptions[attachment].initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
						attachmentDescriptions[attachment].initialLayout = currentLayout;
				}
				else {
					if (currentLayout != VK_IMAGE_LAYOUT_GENERAL)
						currentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}

				if (!used && attachmentDescriptions[attachment].initialLayout != currentLayout) {
					externalInputDependency.set(subpass);
					externalDepthDependency.set(subpass);
				}

				depthStencilAttachmentRead.set(subpass);
				inputAttachmentRead.set(subpass);
				depth->layout = currentLayout;
				input->layout = currentLayout;
				used = true;
				lastSubpassForAttachment[attachment] = subpass;
			}
			else if (depth) {
				assert(subpasses[subpass].depthStencil != RenderpassCreateInfo::DepthStencil::None);
				if (subpasses[subpass].depthStencil == RenderpassCreateInfo::DepthStencil::ReadWrite) {
					depthStencilAttachmentWrite.set(subpass);
					if (currentLayout != VK_IMAGE_LAYOUT_GENERAL)
						currentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else {
					if (currentLayout != VK_IMAGE_LAYOUT_GENERAL)
						currentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}

				if (!used && attachmentDescriptions[attachment].initialLayout != currentLayout)
					externalDepthDependency.set(subpass);

				depthStencilAttachmentRead.set(subpass);
				depth->layout = currentLayout;
				used = true;
				lastSubpassForAttachment[attachment] = subpass;
			}
			else if (input) {
				if (currentLayout != VK_IMAGE_LAYOUT_GENERAL)
					currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (!used && attachmentDescriptions[attachment].initialLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
					attachmentDescriptions[attachment].initialLayout = currentLayout;

				if (!used && attachmentDescriptions[attachment].initialLayout != currentLayout)
					externalInputDependency.set(subpass);
				input->layout = currentLayout;
				used = true;
				lastSubpassForAttachment[attachment] = subpass;
			}
			else {
				assert(0);
			}
		}
		assert(used);
		if (attachmentDescriptions[attachment].finalLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
			assert(currentLayout != VK_IMAGE_LAYOUT_UNDEFINED);
			attachmentDescriptions[attachment].finalLayout = currentLayout;
		}
		assert(attachmentDescriptions[attachment].finalLayout != VK_IMAGE_LAYOUT_UNDEFINED);
	}
	std::vector<uint32_t> preserveAttachments;
	preserveAttachments.reserve(static_cast<size_t>(subpassCount)* MAX_ATTACHMENTS);
	for (uint32_t attachment = 0; attachment < attachmentCount; attachment++) {
		preserves[attachment]&= (1ull << lastSubpassForAttachment[attachment]) -1;
	}
	
	for (uint32_t subpass = 0; subpass < subpassCount; subpass++) {
		auto& pass = subpassDescriptions[subpass];
		uint32_t preserveCount = 0;
		for (uint32_t attachment = 0; attachment < attachmentCount; attachment++) {
			if (preserves[attachment].test(subpass)) {
				preserveAttachments.emplace_back(attachment);
				preserveCount++;
			}
		}
		uint32_t* preserve = nullptr;
		if (preserveCount) preserve = &preserveAttachments[preserveAttachments.size() - preserveCount];
		pass.pPreserveAttachments = preserve;
		pass.preserveAttachmentCount = preserveCount;
	}
	assert(subpassCount > 0);
	VkRenderPassCreateInfo renderPassCreateInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = attachmentCount,
		.pAttachments = attachmentDescriptions.data(),
		.subpassCount = subpassCount,
		.pSubpasses = subpassDescriptions.data(),
	};
	auto externalDependenciesMask = externalColorDependency | externalDepthDependency | externalInputDependency;
	for (uint32_t subpass = 0; subpass < externalDependenciesMask.size(); subpass++) {
		if (!externalDependenciesMask.test(subpass))
			continue;
		externalDependencies.emplace_back();
		auto& dependency = externalDependencies.back();
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = subpass;

		if (externalBottomOfPipeDependency.test(subpass))
			dependency.srcStageMask |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		if (externalColorDependency.test(subpass)) {
			dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
		if (externalDepthDependency.test(subpass)) {
			dependency.srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		if (externalInputDependency.test(subpass)) {
			dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		}
	}
	auto selfDependenciesMask = colorSelfDependency | depthSelfDependency;
	for (uint32_t subpass = 0; subpass < selfDependenciesMask.size(); subpass++) {
		if (!selfDependenciesMask.test(subpass))
			continue;
		externalDependencies.emplace_back();
		auto& dependency = externalDependencies.back();
		dependency.srcSubpass = subpass;
		dependency.dstSubpass = subpass;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		if (colorSelfDependency.test(subpass)) {
			dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		if (depthSelfDependency.test(subpass))
		{
			dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}
	//Cache invalidation between subpasses
	for (uint32_t subpass = 1; subpass < subpassCount; subpass++) {
		externalDependencies.emplace_back();
		auto& dependency = externalDependencies.back();
		dependency.srcSubpass = subpass - 1;
		dependency.dstSubpass = subpass;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		if (colorAttachmentReadWrite.test(subpass - 1u)) {
			dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		if (depthStencilAttachmentWrite.test(subpass - 1u)) {
			dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		if (colorAttachmentReadWrite.test(subpass)) {
			dependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
		if (depthStencilAttachmentRead.test(subpass)) {
			dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		if (depthStencilAttachmentWrite.test(subpass)) {
			dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		if (inputAttachmentRead.test(subpass)) {
			dependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		}
	}
	if (!externalDependencies.empty()) {
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(externalDependencies.size());
		renderPassCreateInfo.pDependencies = externalDependencies.data();
	}
	for (uint32_t i = 0; i < renderPassCreateInfo.subpassCount; i++) {
		auto& subpass = renderPassCreateInfo.pSubpasses[i];
		Subpass info{
			.depthStencilAttachmentReference = *subpass.pDepthStencilAttachment,
			.colorAttachmentCount = subpass.colorAttachmentCount,
			.inputAttachmentCount = subpass.inputAttachmentCount
		};
		memcpy(info.colorAttachmentReferences.data(), subpass.pColorAttachments, subpass.colorAttachmentCount * sizeof(VkAttachmentReference));
		memcpy(info.inputAttachmentReferences.data(), subpass.pInputAttachments, subpass.inputAttachmentCount * sizeof(VkAttachmentReference));
		m_subpasses.push_back(info);
	}
	if (auto result = vkCreateRenderPass(r_device.m_device, &renderPassCreateInfo, r_device.m_allocator, &m_renderPass); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create render pass, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create render pass, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	
}

//vulkan::Renderpass::Renderpass(LogicalDevice& parent, VkRenderPass renderPass): r_device(parent), m_renderPass(renderPass)
//{
//}

vulkan::Renderpass::~Renderpass() noexcept
{
	vkDestroyRenderPass(r_device.m_device, m_renderPass, r_device.m_allocator);
}

bool vulkan::Renderpass::has_depth_attachment(uint32_t subpass) const noexcept
{
	assert(m_subpasses.size() > subpass);
	return m_subpasses[subpass].depthStencilAttachmentReference.attachment != VK_ATTACHMENT_UNUSED;
}

uint32_t vulkan::Renderpass::get_num_input_attachments(uint32_t subpass) const noexcept
{
	assert(m_subpasses.size() > subpass);
	return m_subpasses[subpass].inputAttachmentCount;
}

uint32_t vulkan::Renderpass::get_num_color_attachments(uint32_t subpass) const noexcept
{
	assert(m_subpasses.size() > subpass);
	return m_subpasses[subpass].colorAttachmentCount;
}

VkRenderPass vulkan::Renderpass::get_render_pass() const noexcept
{
	return m_renderPass;
}

const VkAttachmentReference& vulkan::Renderpass::get_input_attachment(uint32_t idx, uint32_t subpass) const noexcept
{
	assert(m_subpasses.size() > subpass);
	assert(m_subpasses[subpass].inputAttachmentCount > idx);
	return m_subpasses[subpass].inputAttachmentReferences[idx];
}

const VkAttachmentReference& vulkan::Renderpass::get_color_attachment(uint32_t idx, uint32_t subpass) const noexcept
{
	assert(m_subpasses.size() > subpass);
	assert(m_subpasses[subpass].colorAttachmentCount > idx);
	return m_subpasses[subpass].colorAttachmentReferences[idx];
}
