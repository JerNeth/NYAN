#include "Renderer/RenderGraph.h"

using namespace nyan;
//using namespace vulkan;

nyan::Renderpass::Renderpass(Rendergraph& graph, Type type, uint32_t id, const std::string& name) : r_graph(graph), m_type(type), m_id(id), m_name(name)
{
}

void nyan::Renderpass::add_input(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_readIn.insert(m_id);
	assert(std::find(m_reads.begin(), m_reads.end(), resource.m_id) == m_reads.end());
	m_reads.push_back(resource.m_id);
}

void nyan::Renderpass::add_output(const std::string& name, ImageAttachment attachment)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(m_id);
	resource.attachment = attachment;
	assert(std::find(m_writes.begin(), m_writes.end(), resource.m_id) == m_writes.end());
	m_writes.push_back(resource.m_id);
}

void nyan::Renderpass::add_swapchain_output()
{
	nyan::ImageAttachment swap;
	auto& resource = r_graph.get_resource("swap");
	resource.m_writeToIn.insert(m_id);
	resource.attachment = swap;
	assert(std::find(m_writes.begin(), m_writes.end(), resource.m_id) == m_writes.end());
	m_writes.push_back(resource.m_id);
	r_graph.set_swapchain("swap");
}

void nyan::Renderpass::add_depth_input(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_readIn.insert(m_id);
	m_depthStencilRead = resource.m_id;
}

void nyan::Renderpass::add_depth_output(const std::string& name, ImageAttachment attachment)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(m_id);
	resource.attachment = attachment;
	resource.m_type = RenderResource::Type::Image;
	m_depthStencilWrite = resource.m_id;
}

void nyan::Renderpass::add_read_dependency(const std::string& name, bool storageImage)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_readIn.insert(m_id);
	if(storageImage)
		resource.storageImage = true;
}

void nyan::Renderpass::add_write_dependency(const std::string& name, bool storageImage)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(m_id);
	if (storageImage)
		resource.storageImage = true;
}

void nyan::Renderpass::add_post_barrier(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	Barrier barrier;
	if (resource.m_type == RenderResource::Type::Image) {
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		VkImageMemoryBarrier imageBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.newLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			//TODO
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = VK_NULL_HANDLE,
			.subresourceRange {
				.aspectMask = vulkan::ImageInfo::format_to_aspect_mask(attachment.format),
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
		if (m_type == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.src = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}
			else {
				barrier.src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
		}
		else {
			barrier.src = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		}
		if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
			barrier.dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else {
			barrier.dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		}

		barrier.imageBarrierCount = 1;
		assert(m_imageBarriers.size() <= USHRT_MAX);
		barrier.imageBarrierOffset = static_cast<uint16_t>(m_imageBarriers.size());
		barrier.resourceId = resource.m_id;

		m_imageBarriers.push_back(imageBarrier);
		m_postBarriers.push_back(barrier);
	}
	else {
		assert(false);
		//TODO
	}
}

void nyan::Renderpass::apply_pre_barriers(vulkan::CommandBufferHandle& cmd)
{
	for (auto& barrier : m_preBarriers) {
		//std::cout << "Prebarrier (" << barrier.imageBarrierOffset << ")" << " Ressource (" << barrier.resourceId << ")\n";
		cmd->barrier(barrier.src, barrier.dst, 0, nullptr, barrier.bufferBarrierCount, m_bufferBarriers.data() + barrier.bufferBarrierOffset
			, barrier.imageBarrierCount, m_imageBarriers.data() + barrier.imageBarrierOffset);
	}
}

void nyan::Renderpass::apply_post_barriers(vulkan::CommandBufferHandle& cmd)
{
	for (auto& barrier : m_postBarriers) {
		//std::cout << "Postbarrier (" << barrier.imageBarrierOffset << ")" << " Ressource (" << barrier.resourceId << ")\n";
		cmd->barrier(barrier.src, barrier.dst, 0, nullptr, barrier.bufferBarrierCount, m_bufferBarriers.data() + barrier.bufferBarrierOffset
			,barrier.imageBarrierCount, m_imageBarriers.data() + barrier.imageBarrierOffset);
	}
}

nyan::Rendergraph::Rendergraph(vulkan::LogicalDevice& device)
	: r_device(device)
{
	
}

Renderpass& nyan::Rendergraph::add_pass(const std::string& name, Renderpass::Type type)
{
	assert(m_state == State::Setup);
	return m_renderpasses.emplace(name, *this, type, m_renderpassCount++, name);
}

Renderpass& nyan::Rendergraph::get_pass(const std::string& name)
{
	return m_renderpasses.get(name);
}

void nyan::Rendergraph::build()
{
	assert(m_state == State::Setup);
	m_state = State::Build;
	m_renderresources.for_each([&](RenderResource& resource) {
		auto followRead = resource.m_readIn.begin();
		for (auto write = resource.m_writeToIn.begin(); write != resource.m_writeToIn.end(); write++) {
			for (; followRead != resource.m_readIn.end(); followRead++)
				if (*followRead > *write)
					break;

			auto followWrite = write;
			followWrite++;
			if (followWrite != resource.m_writeToIn.end() && followRead != resource.m_readIn.end()) {
				if (*followRead <= *followWrite) {
					set_up_RaW(*write, *followRead, resource);
				}
				else {
					set_up_WaW(*write, *followWrite, resource);
				}
			}
			else if (followWrite != resource.m_writeToIn.end()) {
				set_up_WaW(*write, *followWrite, resource);
			}
			else if (followRead != resource.m_readIn.end()) {
				set_up_RaW(*write, *followRead, resource);
			}
		}
		//Layout transitions Read -> Write
		if (!resource.m_writeToIn.empty() && resource.m_type == RenderResource::Type::Image) {
			auto followWrite = resource.m_writeToIn.begin();
			for (auto read = resource.m_readIn.begin(); read != resource.m_readIn.end(); read++) {
				for (; followWrite != resource.m_writeToIn.end(); followWrite++)
					if (*followWrite > *read)
						break;
				if (followWrite == resource.m_writeToIn.end())
					break;

				followRead = read;
				while (followRead != resource.m_readIn.end() && *followRead < *followWrite)
					read = followRead++;

				set_up_WaR(*read, *followWrite, resource);
			}
			if (!resource.m_readIn.empty() && (*resource.m_writeToIn.begin() < *resource.m_readIn.begin())) {
				//Necessary roll-over last Read -> first Write
				set_up_WaR(*resource.m_readIn.rbegin(), *resource.m_writeToIn.begin(), resource);
			}
		}
	});
	m_renderpasses.for_each([&](Renderpass& pass) {
		if (pass.get_type() == Renderpass::Type::Graphics) {
			assert(!pass.m_rpInfo);
			pass.m_rpInfo = std::make_unique< vulkan::RenderpassCreateInfo>();
			for (auto read : pass.m_reads) {
				auto& resource = m_renderresources.get_direct(read);
				if (resource.m_type == RenderResource::Type::Image) {
					//auto& attachment = std::get<ImageAttachment>(resource.attachment);
					auto* info = pass.m_rpInfo.get();
					auto& subpass = info->subpasses[0];
					info->subpassCount = 1;
					info->loadAttachments.set(info->colorAttachmentsCount);
					subpass.inputAttachments[subpass.inputAttachmentsCount++] = pass.m_rpInfo->colorAttachmentsCount++;
				}
			}
			for (auto write : pass.m_writes) {
				auto& resource = m_renderresources.get_direct(write);
				if (resource.m_type == RenderResource::Type::Image) {
					auto& attachment = std::get<ImageAttachment>(resource.attachment);
					auto* info = pass.m_rpInfo.get();
					auto& subpass = info->subpasses[0];
					info->subpassCount = 1;
					if (*resource.m_writeToIn.begin() == pass.m_id) {
						info->clearAttachments.set(info->colorAttachmentsCount);
						for(size_t i = 0u; i < 4u; i++)
							info->clearColors[info->colorAttachmentsCount].float32[i] = attachment.clearColor[i];
					}
					info->storeAttachments.set(info->colorAttachmentsCount);
					subpass.colorAttachments[subpass.colorAttachmentsCount++] = pass.m_rpInfo->colorAttachmentsCount++;
				}
			}
			//Can't have both
			assert(pass.m_depthStencilWrite == InvalidResourceId || pass.m_depthStencilRead == InvalidResourceId);
			if (pass.m_depthStencilRead != InvalidResourceId) {
				auto& resource = m_renderresources.get_direct(pass.m_depthStencilRead);
				assert(resource.m_type == RenderResource::Type::Image);
				//auto& attachment = std::get<ImageAttachment>(resource.attachment);
				auto* info = pass.m_rpInfo.get();
				info->opFlags.set(vulkan::RenderpassCreateInfo::OpFlags::DepthStencilReadOnly);
			}
			if (pass.m_depthStencilWrite != InvalidResourceId) {
				auto& resource = m_renderresources.get_direct(pass.m_depthStencilWrite);
				assert(resource.m_type == RenderResource::Type::Image);
				auto& attachment = std::get<ImageAttachment>(resource.attachment);
				auto* info = pass.m_rpInfo.get();
				info->opFlags.set(vulkan::RenderpassCreateInfo::OpFlags::DepthStencilStore);
				//Are we the first to read this
				assert(!resource.m_writeToIn.empty());
				if (*resource.m_writeToIn.begin() == pass.m_id) {
					info->opFlags.set(vulkan::RenderpassCreateInfo::OpFlags::DepthStencilClear);
					info->clearDepthStencil.depth = attachment.clearColor[0];
					info->clearDepthStencil.stencil = static_cast<uint32_t>(attachment.clearColor[1]);
				}
				else
					info->opFlags.set(vulkan::RenderpassCreateInfo::OpFlags::DepthStencilLoad);
			}
		}
	});

	assert(m_state == State::Build);
	m_state = State::Execute;
}

void nyan::Rendergraph::execute()
{
	assert(m_state == State::Execute);
	//Update Attachments
	m_renderresources.for_each([&](RenderResource& resource) {
		if (resource.m_type == RenderResource::Type::Image) {
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			uint32_t width = r_device.get_swapchain_width();
			uint32_t height = r_device.get_swapchain_height();
			if (attachment.size == ImageAttachment::Size::Absolute) {
				width = static_cast<uint32_t>(attachment.width);
				height = static_cast<uint32_t>(attachment.height);
			}
			else {
				width = static_cast<uint32_t>(width * attachment.width);
				height = static_cast<uint32_t>(height * attachment.height);
			}
			if (m_swapchainResource == resource.m_id) {
				resource.handle = r_device.get_swapchain_image_view();
			}
			else {
				VkImageUsageFlags usage = 0;
				if (!resource.m_readIn.empty())
					usage |=VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				if (resource.storageImage)
					usage |= VK_IMAGE_USAGE_STORAGE_BIT;
				resource.handle = r_device.request_render_target(width, height, attachment.format, resource.m_id, usage);
			}
		}
	});

	m_renderpasses.for_each([this](Renderpass& pass) {
		uint32_t attachmentId = 0;
		if (pass.get_type() == Renderpass::Type::Graphics) {
			for (auto read : pass.m_reads) {
				auto& resource = m_renderresources.get_direct(read);
				if (resource.m_type == RenderResource::Type::Image) {
					auto* info = pass.m_rpInfo.get();
					assert(resource.handle);
					info->colorAttachmentsViews[attachmentId++] = resource.handle;
				}
			}
			for (auto write : pass.m_writes) {
				auto& resource = m_renderresources.get_direct(write);
				auto& attachment = std::get<ImageAttachment>(resource.attachment);
				if (resource.m_type == RenderResource::Type::Image) {
					auto* info = pass.m_rpInfo.get();
					if (*resource.m_writeToIn.begin() == pass.m_id) {
						for (size_t i = 0u; i < 4u; i++)
							info->clearColors[attachmentId].float32[i] = attachment.clearColor[i];
					}
					assert(resource.handle);
					info->colorAttachmentsViews[attachmentId++] = resource.handle;
				}
			}
			assert(pass.m_depthStencilWrite == InvalidResourceId || pass.m_depthStencilRead == InvalidResourceId);
			if (pass.m_depthStencilRead != InvalidResourceId) {
				auto& resource = m_renderresources.get_direct(pass.m_depthStencilRead);
				assert(resource.m_type == RenderResource::Type::Image);
				auto* info = pass.m_rpInfo.get();
				assert(resource.handle);
				info->depthStencilAttachment = resource.handle;
			}
			if (pass.m_depthStencilWrite != InvalidResourceId) {
				auto& resource = m_renderresources.get_direct(pass.m_depthStencilWrite);
				auto& attachment = std::get<ImageAttachment>(resource.attachment);
				assert(resource.m_type == RenderResource::Type::Image);
				auto* info = pass.m_rpInfo.get();
				if (*resource.m_writeToIn.begin() == pass.m_id) {
					info->clearDepthStencil.depth = attachment.clearColor[0];
					info->clearDepthStencil.stencil = static_cast<uint32_t>(attachment.clearColor[1]);
				}
				assert(resource.handle);
				info->depthStencilAttachment = resource.handle;
			}
		}
		auto barrierUpdate = [this, &pass](const std::vector<Barrier>& barriers) {
			for (auto& barrier : barriers) {
				assert(barrier.resourceId != InvalidResourceId);
				auto& resource = m_renderresources.get_direct(barrier.resourceId);
				if (resource.m_type == RenderResource::Type::Image) {
					assert(resource.handle);
					for (size_t i = 0; i < barrier.imageBarrierCount; i++)
						pass.m_imageBarriers[barrier.imageBarrierOffset + i].image = resource.handle->get_image()->get_handle();
				}
				else {
					assert(false);
					for (size_t i = 0; i < barrier.bufferBarrierCount; i++)
						pass.m_bufferBarriers[barrier.bufferBarrierOffset + i].buffer = VK_NULL_HANDLE;
				}
			}
		};
		barrierUpdate(pass.m_postBarriers);
		barrierUpdate(pass.m_preBarriers);
		vulkan::CommandBuffer::Type commandBufferType = vulkan::CommandBuffer::Type::Generic;
		switch (pass.get_type()) {
		case Renderpass::Type::Compute:
			commandBufferType = vulkan::CommandBuffer::Type::Compute;
			break;
		case Renderpass::Type::Graphics:
			commandBufferType = vulkan::CommandBuffer::Type::Generic;
			break;
		}
		//std::cout << "Execute pass: "<< pass.get_id() << "\n";
		auto cmd = r_device.request_command_buffer(commandBufferType);
		cmd->begin_region(pass.m_name.c_str());
		pass.apply_pre_barriers(cmd);
		if (pass.get_type() == Renderpass::Type::Graphics)
			cmd->begin_render_pass(pass.get_info());
		pass.execute(cmd);
		if (pass.get_type() == Renderpass::Type::Graphics)
			cmd->end_render_pass();
		pass.apply_post_barriers(cmd);
		cmd->end_region();
		r_device.submit(cmd);
	});
}

RenderResource& nyan::Rendergraph::add_ressource(const std::string& name, Attachment attachment)
{
	auto& r =m_renderresources.emplace(name, m_resourceCount++);
	r.attachment = attachment;
	return r;
}

RenderResource& nyan::Rendergraph::get_resource(const std::string& name)
{
	if (!m_renderresources.contains(name))
		return m_renderresources.emplace(name, m_resourceCount++);
	return m_renderresources.get(name);
}

void nyan::Rendergraph::set_swapchain(const std::string& name)
{
	auto& resource = m_renderresources.get(name);
	m_swapchainResource = resource.m_id;
}

void nyan::Rendergraph::set_up_RaW(RenderpassId write, RenderpassId read, const RenderResource& resource)
{
	auto& src = m_renderpasses.get_direct(write);
	auto& dst = m_renderpasses.get_direct(read);
	//TODO Differentiate between Async Compute and Graphics for potential ownership transfer
	//assert(src.get_type() == dst.get_type());
	if (resource.m_type == RenderResource::Type::Image) {
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		ImageBarrier barrier{
			.src = write,
			.dst = read,
			.srcLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.dstLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
		if (src.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				barrier.srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				/*barrier.srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				barrier.srcAccess = VK_ACCESS_MEMORY_WRITE_BIT;*/
			}
			else {
				barrier.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				barrier.srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				//barrier.srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				//barrier.srcAccess = VK_ACCESS_MEMORY_WRITE_BIT;
			}
		}
		else {
			barrier.srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
		}
		if (dst.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrier.dstAccess = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				//barrier.dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				//barrier.dstAccess = VK_ACCESS_MEMORY_READ_BIT;
			}
			else {
				barrier.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrier.dstAccess = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
				//barrier.dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				//barrier.dstAccess = VK_ACCESS_MEMORY_READ_BIT;
			}
		}
		else {
			barrier.dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.dstAccess = VK_ACCESS_SHADER_READ_BIT;
		}
		//std::cout << "RaW ";
		set_up_barrier(barrier, resource);
	}
	else {
		assert(false);
		//TODO
	}
}

void nyan::Rendergraph::set_up_WaW(RenderpassId src_, RenderpassId dst_, const RenderResource& resource)
{
	auto& src = m_renderpasses.get_direct(src_);
	auto& dst = m_renderpasses.get_direct(dst_);
	//TODO Differentiate between Async Compute and Graphics for potential ownership transfer
	//assert(src.get_type() == dst.get_type());
	if (resource.m_type == RenderResource::Type::Image) {
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		ImageBarrier barrier{
			.src = src_,
			.dst = dst_,
		};
		barrier.srcLayout = barrier.dstLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (src.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				barrier.srcAccess =  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}
			else {
				barrier.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				barrier.srcAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			}
		}
		else {
			barrier.srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
		}
		if (dst.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				barrier.dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			}
			else {
				barrier.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				barrier.dstAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			}
		}
		else {
			barrier.dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.dstAccess = VK_ACCESS_SHADER_READ_BIT;
		}

		//std::cout << "WaW ";
		set_up_barrier(barrier, resource);
	}
	else {
		assert(false);
		//TODO
	}
}

void nyan::Rendergraph::set_up_WaR(RenderpassId read, RenderpassId write, const RenderResource& resource)
{
	auto& src = m_renderpasses.get_direct(read);
	auto& dst = m_renderpasses.get_direct(write);
	//TODO Differentiate between Async Compute and Graphics for potential ownership transfer
	//assert(src.get_type() == dst.get_type());
	if (resource.m_type == RenderResource::Type::Image) {
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		ImageBarrier barrier{
			.src = read,
			.dst = write,
			.srcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.dstLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		if (src.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrier.srcAccess = VK_ACCESS_SHADER_READ_BIT;
				//barrier.srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				//barrier.srcAccess = VK_ACCESS_MEMORY_READ_BIT;
			}
			else {
				barrier.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrier.srcAccess = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				//barrier.srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				//barrier.srcAccess = VK_ACCESS_MEMORY_READ_BIT;
			}
		}
		else {
			barrier.srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.srcAccess = VK_ACCESS_SHADER_READ_BIT;
		}
		if (dst.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.dstStage =  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				barrier.dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				//barrier.dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				//barrier.srcAccess = VK_ACCESS_MEMORY_WRITE_BIT;
			}
			else {
				barrier.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ;
				barrier.dstAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ;
				//barrier.dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				//barrier.srcAccess = VK_ACCESS_MEMORY_WRITE_BIT;
			}
		}
		else {
			barrier.dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.dstAccess = VK_ACCESS_SHADER_WRITE_BIT;
		}
		//std::cout << "WaR ";
		set_up_barrier(barrier, resource);
	}
	else {
		assert(false);
		//TODO
	}
}

void nyan::Rendergraph::set_up_barrier(const ImageBarrier& imageBarrier_, const RenderResource& resource)
{
	auto& src = m_renderpasses.get_direct(imageBarrier_.src);
	auto& dst = m_renderpasses.get_direct(imageBarrier_.dst);
	//TODO Differentiate between Async Compute and Graphics for potential ownership transfer
	//assert(src.get_type() == dst.get_type());
	Barrier barrier;
	assert(resource.m_type == RenderResource::Type::Image);
	auto& attachment = std::get<ImageAttachment>(resource.attachment);
	VkImageMemoryBarrier imageBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = imageBarrier_.srcAccess,
		.dstAccessMask = imageBarrier_.dstAccess,
		.oldLayout = imageBarrier_.srcLayout,
		.newLayout = imageBarrier_.dstLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = VK_NULL_HANDLE, //This gets updated each frame
		.subresourceRange {
			.aspectMask = vulkan::ImageInfo::format_to_aspect_mask(attachment.format),
			.baseMipLevel = 0,
			.levelCount = 1, //TODO
			.baseArrayLayer = 0,
			.layerCount = 1, //TODO
		}
	};
	barrier.resourceId = resource.m_id;
	barrier.src = imageBarrier_.srcStage;
	barrier.dst = imageBarrier_.dstStage;
	barrier.imageBarrierCount = static_cast<uint16_t>(1);
	assert(src.m_imageBarriers.size() <= USHRT_MAX);
	barrier.imageBarrierOffset = static_cast<uint16_t>(src.m_imageBarriers.size());
	if (src.get_type() != dst.get_type() && r_device.get_graphics_family() != r_device.get_compute_family()) {
		if (src.get_type() == Renderpass::Type::Graphics) {
			imageBarrier.srcQueueFamilyIndex = r_device.get_graphics_family();
			imageBarrier.dstQueueFamilyIndex = r_device.get_compute_family();
		}
		else {
			imageBarrier.srcQueueFamilyIndex = r_device.get_compute_family();
			imageBarrier.dstQueueFamilyIndex = r_device.get_graphics_family();
		}
		dst.m_preBarriers.push_back(barrier);
		//std::cout << "Resource: " << resource.m_id << "Barrier (QueueFamilyChange) (" << barrier.imageBarrierOffset << "): " << imageBarrier_.src << " -> " << imageBarrier_.dst;
	}
	else {
		//std::cout << "Resource: " << resource.m_id << " Barrier (" << barrier.imageBarrierOffset  << "): "<< imageBarrier_.src << " -> " << imageBarrier_.dst;
	}
	//std::cout << "\n\t" << vulkan::ImageLayoutNames[imageBarrier.oldLayout] << " -> " << vulkan::ImageLayoutNames[imageBarrier.newLayout];
	//std::cout << "\n";
	src.m_imageBarriers.push_back(imageBarrier);
	src.m_postBarriers.push_back(barrier);
}

