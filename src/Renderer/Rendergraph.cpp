#include "Renderer/Rendergraph.h"

using namespace nyan;
//using namespace vulkan;

nyan::Renderpass::Renderpass(Rendergraph& graph, Type type, uint32_t id, const std::string& name) : r_graph(graph), m_type(type), m_id(id), m_name(name)
{
}

void nyan::Renderpass::add_input(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_readIn.insert(m_id);
	m_reads.insert(resource.m_id);
}

void nyan::Renderpass::add_output(const std::string& name, ImageAttachment attachment)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(m_id);
	resource.attachment = attachment;
	m_writes.insert(resource.m_id);
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

void nyan::Renderpass::add_read_dependency(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_readIn.insert(m_id);
}

void nyan::Renderpass::add_write_dependency(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(m_id);
	m_writes.insert(resource.m_id);
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
		barrier.imageBarrierOffset = m_postImageBarriers.size();

		m_postImageBarriers.push_back(imageBarrier);
		m_postResource.push_back(resource.m_id);
		m_postBarriers.push_back(barrier);
	}
	else {
		assert(false);
		//TODO
	}
}

void nyan::Renderpass::apply_post_barriers(vulkan::CommandBufferHandle& cmd)
{
	for (auto& barrier : m_postBarriers) {
		cmd->barrier(barrier.src, barrier.dst, 0, nullptr, barrier.bufferBarrierCount, m_postBufferBarriers.data() + barrier.bufferBarrierOffset
			,barrier.imageBarrierCount, m_postImageBarriers.data() + barrier.imageBarrierOffset);
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
	});
	m_renderpasses.for_each([&](Renderpass& pass) {
		if (pass.get_type() == Renderpass::Type::Graphics) {
			assert(!pass.m_rpInfo);
			pass.m_rpInfo = std::make_unique< vulkan::RenderpassCreateInfo>();
			for (auto read : pass.m_reads) {
				auto& resource = m_renderresources.get_direct(read);
				if (resource.m_type == RenderResource::Type::Image) {
					auto& attachment = std::get<ImageAttachment>(resource.attachment);
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
				auto& attachment = std::get<ImageAttachment>(resource.attachment);
				auto* info = pass.m_rpInfo.get();
				auto& subpass = info->subpasses[0];
				info->opFlags.set(vulkan::RenderpassCreateInfo::OpFlags::DepthStencilReadOnly);
			}
			if (pass.m_depthStencilWrite != InvalidResourceId) {
				auto& resource = m_renderresources.get_direct(pass.m_depthStencilWrite);
				assert(resource.m_type == RenderResource::Type::Image);
				auto& attachment = std::get<ImageAttachment>(resource.attachment);
				auto* info = pass.m_rpInfo.get();
				auto& subpass = info->subpasses[0];
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
	//check if attachments are still valid?
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
				resource.handle = r_device.request_render_target(width, height, attachment.format, resource.m_id);
			}
		}
	});

	m_renderpasses.for_each([&](Renderpass& pass) {
		uint32_t attachmentId = 0;
		//Update Attachments
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
			info->depthStencilAttachment =resource.handle;
		}
		uint32_t bufferBarrierIdx = 0;
		uint32_t imageBarrierIdx = 0;
		for (auto& resourceId : pass.m_postResource) {
			auto& resource = m_renderresources.get_direct(resourceId);
			if (resource.m_type == RenderResource::Type::Image) {
				assert(resource.handle);
				pass.m_postImageBarriers[imageBarrierIdx++].image = resource.handle->get_image()->get_handle();
			}
			else {
				bufferBarrierIdx++;
				assert(false);
			}
		}
		vulkan::CommandBuffer::Type commandBufferType = vulkan::CommandBuffer::Type::Generic;
		switch (pass.get_type()) {
		case Renderpass::Type::Compute:
			commandBufferType = vulkan::CommandBuffer::Type::Compute;
			break;
		case Renderpass::Type::Graphics:
			commandBufferType = vulkan::CommandBuffer::Type::Generic;
			break;
		}
		auto cmd = r_device.request_command_buffer(commandBufferType);
		cmd->begin_region(pass.m_name.c_str());
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

void nyan::Rendergraph::add_ressource(const std::string& name, Attachment attachment)
{
	auto& r =m_renderresources.emplace(name, m_resourceCount++);
	r.attachment = attachment;
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
	assert(src.get_type() == dst.get_type());
	Barrier barrier;
	if (resource.m_type == RenderResource::Type::Image) {
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		VkImageMemoryBarrier imageBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.oldLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, //TODO
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, //TODO
			.image = VK_NULL_HANDLE, //This gets updated each frame
			.subresourceRange {
				.aspectMask = vulkan::ImageInfo::format_to_aspect_mask(attachment.format),
				.baseMipLevel = 0,
				.levelCount = 1, //TODO
				.baseArrayLayer = 0, 
				.layerCount = 1, //TODO
			}
		};
		if (src.get_type() == Renderpass::Type::Graphics) {
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
		if (dst.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			}
			else {
				barrier.dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			}
		}
		else {
			barrier.dst = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		}
		barrier.imageBarrierCount = 1;
		barrier.imageBarrierOffset = src.m_postImageBarriers.size();

		src.m_postImageBarriers.push_back(imageBarrier);
		src.m_postResource.push_back(resource.m_id);
		src.m_postBarriers.push_back(barrier);
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
	assert(src.get_type() == dst.get_type());
	Barrier barrier;
	if (resource.m_type == RenderResource::Type::Image) {
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		VkImageMemoryBarrier imageBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.oldLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, //TODO
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, //TODO
			.image = VK_NULL_HANDLE, //This gets updated each frame
			.subresourceRange {
				.aspectMask = vulkan::ImageInfo::format_to_aspect_mask(attachment.format),
				.baseMipLevel = 0,
				.levelCount = 1, //TODO
				.baseArrayLayer = 0,
				.layerCount = 1, //TODO
			}
		};
		if (src.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.src = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				imageBarrier.srcAccessMask =  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}
			else {
				barrier.src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			}
		}
		else {
			barrier.src = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		}
		if (dst.get_type() == Renderpass::Type::Graphics) {
			if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				barrier.dst = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			}
			else {
				barrier.dst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			}
		}
		else {
			barrier.dst = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		}
		barrier.imageBarrierCount = 1;
		barrier.imageBarrierOffset = src.m_postImageBarriers.size();

		src.m_postImageBarriers.push_back(imageBarrier);
		src.m_postResource.push_back(resource.m_id);
		src.m_postBarriers.push_back(barrier);
	}
	else {
		assert(false);
		//TODO
	}
}

