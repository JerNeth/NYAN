#include "Renderer/RenderGraph.h"
#include "CommandBuffer.h"

using namespace nyan;
//using namespace vulkan;

nyan::Renderpass::Renderpass(Rendergraph& graph, Type type, uint32_t id, const std::string& name) :
	r_graph(graph),
	m_type(type),
	m_id(id),
	m_name(name)
{
}

void nyan::Renderpass::add_read(const std::string& name, Renderpass::Read::Type readType)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_readIn.insert(std::upper_bound(resource.m_readIn.begin(), resource.m_readIn.end(), m_id), m_id);
	assert(std::find_if(m_reads.cbegin(), m_reads.cend(), [&resource, readType](const auto& read) { return read.id == resource.m_id && read.type == readType;  }) == m_reads.cend());
	m_reads.push_back(Read{ resource.m_id, readType, VK_NULL_HANDLE });
}

void nyan::Renderpass::add_attachment(const std::string& name, ImageAttachment attachment)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	resource.attachment = attachment;
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_attachments.push_back(resource.m_id);
}

void nyan::Renderpass::add_attachment(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_attachments.push_back(resource.m_id);
}

void nyan::Renderpass::add_swapchain_attachment(Math::vec4 clearColor)
{
	assert(m_type == Renderpass::Type::Graphics);
	//Currently only support hybrid queue for swapchain Synchronization
	nyan::ImageAttachment swap;
	swap.clearColor = clearColor;
	swap.format = r_graph.get_device().get_swapchain_image_view()->get_format();
	auto& resource = r_graph.get_resource("swap");
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	resource.attachment = swap;
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_attachments.push_back(resource.m_id);
	r_graph.set_swapchain("swap");
	m_rendersSwap = true;
}

void nyan::Renderpass::add_depth_attachment(const std::string& name, ImageAttachment attachment)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	resource.attachment = attachment;
	resource.m_type = RenderResource::Type::Image;
	m_depth = resource.m_id;
}

void nyan::Renderpass::add_depth_attachment(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	m_depth = resource.m_id;
}

void nyan::Renderpass::add_depth_stencil_attachment(const std::string& name, ImageAttachment attachment)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	resource.attachment = attachment;
	resource.m_type = RenderResource::Type::Image;
	m_depth = m_stencil = resource.m_id;
}

void nyan::Renderpass::add_depth_stencil_attachment(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	m_depth = m_stencil = resource.m_id;
}

void nyan::Renderpass::add_stencil_attachment(const std::string& name, ImageAttachment attachment)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	resource.attachment = attachment;
	resource.m_type = RenderResource::Type::Image;
	m_stencil = resource.m_id;
}

void nyan::Renderpass::add_stencil_attachment(const std::string& name)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	m_stencil = resource.m_id;
}

//void nyan::Renderpass::add_read_dependency(const std::string& name, bool storageImage)
//{
//	auto& resource = r_graph.get_resource(name);
//	resource.m_readIn.insert(std::upper_bound(resource.m_readIn.begin(), resource.m_readIn.end(), m_id), m_id);
//	if(storageImage)
//		resource.storageImage = true;
//}

void nyan::Renderpass::add_write(const std::string& name, ImageAttachment attachment, Renderpass::Write::Type writeType)
{
	auto& resource = r_graph.get_resource(name);
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	resource.attachment = attachment;
	resource.storageImage = true;
	assert(std::find_if(m_writes.cbegin(), m_writes.cend(), [&resource](const auto& write) { return write.id == resource.m_id; }) == m_writes.cend());
	m_writes.push_back(Write{ resource.m_id , writeType, VK_NULL_HANDLE});
}

void nyan::Renderpass::add_swapchain_write(Math::vec4 clearColor, Renderpass::Write::Type writeType)
{
	assert(m_type == Renderpass::Type::Graphics);
	//Currently only support hybrid queue for swapchain Synchronization
	nyan::ImageAttachment swap;
	swap.clearColor = clearColor;
	swap.format = r_graph.get_device().get_swapchain_image_view()->get_format();
	auto& resource = r_graph.get_resource("swap");
	resource.m_writeToIn.insert(std::upper_bound(resource.m_writeToIn.begin(), resource.m_writeToIn.end(), m_id), m_id);
	resource.attachment = swap;
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_writes.push_back(Write{ resource.m_id , writeType, VK_NULL_HANDLE });
	r_graph.set_swapchain("swap");
	m_rendersSwap = true;
}

void nyan::Renderpass::execute(vulkan::CommandBufferHandle& cmd)
{

	//for (auto readId : m_reads) {
	//	const auto& read = r_graph.get_resource(readId);
	//	if (read.binding != ~0u) {
	//		if(is_attachment(readId))
	//			r_graph.r_device.get_bindless_set().set_storage_image(read.binding, VkDescriptorImageInfo{ .imageView = read.handle->get_image_view(), .imageLayout = VK_IMAGE_LAYOUT_GENERAL });
	//	}
	//	else {
	//		assert(false);
	//	}
	//}
	for (auto& [writeId, writeType, writeView, writeBinding] : m_writes) {
		auto& write = r_graph.get_resource(writeId);
		if (write.m_type == RenderResource::Type::Image) {
			if (writeBinding != ~0u) {
				assert(is_write(writeId));
				assert(writeView != VK_NULL_HANDLE);
				if (writeView != VK_NULL_HANDLE)
					r_graph.r_device.get_bindless_set().set_storage_image(writeBinding, VkDescriptorImageInfo{ .imageView = writeView, .imageLayout = VK_IMAGE_LAYOUT_GENERAL });
			}
			else {
				assert(writeView != VK_NULL_HANDLE);
				if (writeView != VK_NULL_HANDLE)
					writeBinding = r_graph.r_device.get_bindless_set().set_storage_image(VkDescriptorImageInfo{ .imageView = writeView, .imageLayout = VK_IMAGE_LAYOUT_GENERAL });
			}
		}
		else {
			assert(false);
		}
	}
	for (auto& [readId, readType, readView, readBinding] : m_reads) {
		auto& read = r_graph.get_resource(readId);
		if (read.m_type == RenderResource::Type::Image) {
			if (readBinding != ~0u) {
				assert(is_read(readId));
				assert(readView != VK_NULL_HANDLE);
				if (readView != VK_NULL_HANDLE)
					r_graph.r_device.get_bindless_set().set_sampled_image(readBinding, VkDescriptorImageInfo{ .imageView = readView, .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL });
			}
			else {
				assert(readView != VK_NULL_HANDLE);
				if (readView != VK_NULL_HANDLE)
					readBinding = r_graph.r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ .imageView = readView, .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL });
			}
		}
		else {
			assert(false);
		}
	}
	if (m_rendersSwap)
		cmd->touch_swapchain();
	for (size_t i{ 0 }; i < m_renderFunctions.size(); i++) {
		const auto &renderFunction = m_renderFunctions[i];
		if (m_useRendering[i])
			begin_rendering(cmd);
		renderFunction(cmd, *this);
		if (m_useRendering[i])
			end_rendering(cmd);
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

vulkan::PipelineId nyan::Renderpass::add_pipeline(vulkan::GraphicsPipelineConfig config)
{
	config.renderingCreateInfo = m_renderingCreateInfo;
	return r_graph.get_device().get_pipeline_storage().add_pipeline(config);
}

void nyan::Renderpass::begin_rendering(vulkan::CommandBufferHandle& cmd)
{
	if (m_type != Renderpass::Type::Compute)
		cmd->begin_rendering(m_renderInfo);
}

void nyan::Renderpass::end_rendering(vulkan::CommandBufferHandle& cmd)
{
	if (m_type != Renderpass::Type::Compute)
		cmd->end_rendering();
}

uint32_t nyan::Renderpass::get_write_bind(uint32_t idx)
{
	assert(m_writes.size() > idx);
	return m_writes[idx].binding;
}

uint32_t nyan::Renderpass::get_read_bind(uint32_t idx)
{
	assert(m_reads.size() > idx);
	return m_reads[idx].binding;
}
uint32_t nyan::Renderpass::get_write_bind(std::string_view v, Write::Type type)
{
	const auto& resource = r_graph.get_resource(v);
	auto res = std::find_if(m_writes.cbegin(), m_writes.cend(), [&resource, type](const auto& write) { return resource.m_id == write.id && type == write.type; });
	assert(res != m_writes.cend());
	if (res != m_writes.cend())
		return res->binding;
	else
		return InvalidResourceId;
}

uint32_t nyan::Renderpass::get_read_bind(std::string_view v, Read::Type type)
{
	const auto& resource = r_graph.get_resource(v);
	auto res = std::find_if(m_reads.cbegin(), m_reads.cend(), [&resource, type](const auto& read) { return resource.m_id == read.id && type == read.type; });
	assert(res != m_reads.cend());
	if (res != m_reads.cend())
		return res->binding;
	else
		return InvalidResourceId;

}

void nyan::Renderpass::add_wait(VkSemaphore wait, VkPipelineStageFlags stage)
{
	m_waitSemaphores.push_back(wait);
	m_waitStages.push_back(stage);
}

void nyan::Renderpass::add_signal(uint32_t passId, VkPipelineStageFlags stage)
{
	m_signalPassIds.push_back(passId);
	m_signalStages.push_back(stage);
}

bool nyan::Renderpass::is_read(RenderResourceId id) const
{
	return std::find_if(m_reads.cbegin(), m_reads.cend(), [id](const auto& read) { return read.id == id; }) != m_reads.cend();
}

bool nyan::Renderpass::is_write(RenderResourceId id) const
{
	return std::find_if(m_writes.cbegin(), m_writes.cend(), [id](const auto& write) { return write.id == id; }) != m_writes.cend();
}

bool nyan::Renderpass::is_compute_write(RenderResourceId id) const
{
	auto it = std::find_if(m_writes.cbegin(), m_writes.cend(), [id](const auto& write) { return write.id == id; });
	if (it == m_writes.cend())
		return false;
	return it->type == Renderpass::Write::Type::Compute;
}

bool nyan::Renderpass::is_attachment(RenderResourceId id) const
{
	return std::find(m_attachments.cbegin(), m_attachments.cend(), id) != m_attachments.cend() || m_depth ==id || m_stencil == id;
}

bool nyan::Renderpass::is_write(const RenderResource& resource) const
{
	return is_write(resource.m_id);
}

bool nyan::Renderpass::is_compute_write(const RenderResource& resource) const
{
	return is_compute_write(resource.m_id);
}

bool nyan::Renderpass::is_attachment(const RenderResource& resource) const
{
	return is_attachment(resource.m_id);
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
		if (resource.m_id == m_swapchainResource && !resource.m_writeToIn.empty()) {
			swapchain_present_transition(resource.m_writeToIn.back());
		}
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
		if (resource.m_id == m_swapchainResource && !resource.m_writeToIn.empty()) {
			swapchain_write_transition(resource.m_writeToIn.front());
		}
	});
	m_renderpasses.for_each([&](Renderpass& pass) {
		//pass.m_attachmentPool = std::make_unique<vulkan::DescriptorPool>(r_device, );
		if (pass.get_type() == Renderpass::Type::Graphics) {
			for (auto& [readId, readType, readView, readBinding] : pass.m_reads) {
				auto& resource = m_renderresources.get_direct(readId);
				if (resource.m_type == RenderResource::Type::Image) {
					readView = VK_NULL_HANDLE;
				}
			}
			for (auto& [writeId, writeType, writeView, writeBinding]: pass.m_writes) {
				auto& resource = m_renderresources.get_direct(writeId);
				if (resource.m_type == RenderResource::Type::Image) {
					writeView = VK_NULL_HANDLE;
				}
			}
			for (auto attachmentId : pass.m_attachments) {
				auto& resource = m_renderresources.get_direct(attachmentId);
				if (resource.m_type == RenderResource::Type::Image) {
					auto& attachment = std::get<ImageAttachment>(resource.attachment);
					auto& info = pass.m_renderingCreateInfo;
					info.colorAttachmentFormats[info.colorAttachmentCount++] = attachment.format;
					
					pass.m_renderInfo.pColorAttachments = pass.m_colorAttachments.data();
					VkRenderingAttachmentInfo renderingAttachment{
						.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
						.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.resolveMode = VK_RESOLVE_MODE_NONE,
						.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
						.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					};
					if (*resource.m_writeToIn.begin() == pass.m_id) {
						renderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						for(size_t i = 0u; i < 4u; i++)
							renderingAttachment.clearValue.color.float32[i] = attachment.clearColor[i];
					}
					pass.m_colorAttachments[pass.m_renderInfo.colorAttachmentCount++] = renderingAttachment;
				}
			}
			if (pass.m_depth != InvalidResourceId) {
				auto& resource = m_renderresources.get_direct(pass.m_depth);
				auto& attachment = std::get<ImageAttachment>(resource.attachment);
				assert(resource.m_type == RenderResource::Type::Image);
				VkRenderingAttachmentInfo renderingAttachment{
						.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
						.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						.resolveMode = VK_RESOLVE_MODE_NONE,
						.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
						.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				};
				if (*resource.m_writeToIn.begin() == pass.m_id) {
					renderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					renderingAttachment.clearValue.depthStencil.depth = attachment.clearColor[0];
				}
				pass.m_renderingCreateInfo.depthAttachmentFormat = attachment.format;
				pass.m_renderInfo.pDepthAttachment = &pass.m_depthAttachment;
				pass.m_depthAttachment = renderingAttachment;
			}
			if (pass.m_stencil != InvalidResourceId) {
				assert(pass.m_depth == InvalidResourceId || pass.m_depth == pass.m_stencil);
				auto& resource = m_renderresources.get_direct(pass.m_stencil);
				auto& attachment = std::get<ImageAttachment>(resource.attachment);
				assert(resource.m_type == RenderResource::Type::Image);
				VkRenderingAttachmentInfo renderingAttachment{
						.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
						.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						.resolveMode = VK_RESOLVE_MODE_NONE,
						.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
						.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				};
				if (*resource.m_writeToIn.begin() == pass.m_id) {
					renderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					renderingAttachment.clearValue.depthStencil.stencil = std::bit_cast<uint32_t>(attachment.clearColor[1]);
				}
				pass.m_renderingCreateInfo.stencilAttachmentFormat = attachment.format;
				pass.m_renderInfo.pStencilAttachment = &pass.m_stencilAttachment;
				pass.m_stencilAttachment = renderingAttachment;
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
					usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
				if (resource.storageImage)
					usage |= VK_IMAGE_USAGE_STORAGE_BIT;
				resource.handle = r_device.request_render_target(width, height, attachment.format, resource.m_id, usage);
			}
		}
	});

	m_renderpasses.for_each([this](Renderpass& pass) {
		for (auto& [readId, readType, readView, readBinding] : pass.m_reads) {
			auto& resource = m_renderresources.get_direct(readId);
			if (resource.m_type == RenderResource::Type::Image) {
				assert(resource.handle);
				if (readType == Renderpass::Read::Type::ImageColor) {
					readView = resource.handle->get_image_view();
				}
				else if (readType == Renderpass::Read::Type::ImageDepth) {
					auto* tmp = resource.handle->get_image()->get_depth_view();
					assert(tmp);
					if (!tmp)
						continue;
					readView = tmp->get_image_view();
				}
				else if (readType == Renderpass::Read::Type::ImageStencil) {
					auto* tmp = resource.handle->get_image()->get_stencil_view();
					assert(tmp);
					if (!tmp)
						continue;
					readView = tmp->get_image_view();
				}

			}
		}
		for (auto& [writeId, writeType, writeView, writeBinding] : pass.m_writes) {
			auto& resource = m_renderresources.get_direct(writeId);
			if (resource.m_type == RenderResource::Type::Image) {
				assert(resource.handle);
				if (!resource.handle)
					continue;
				writeView = resource.handle->get_image_view();
			}
		}
		uint32_t attachmentId = 0;
		for (auto attachmentRessourceId : pass.m_attachments) {
			auto& resource = m_renderresources.get_direct(attachmentRessourceId);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			if (resource.m_type == RenderResource::Type::Image) {
				if (*resource.m_writeToIn.begin() == pass.m_id) {
					for (size_t i = 0u; i < 4u; i++)
						pass.m_colorAttachments[attachmentId].clearValue.color.float32[i] = attachment.clearColor[i];
				}
				assert(resource.handle);
				pass.m_colorAttachments[attachmentId++].imageView = resource.handle->get_image_view();
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
				pass.m_renderInfo.renderArea.extent.width = Math::max(pass.m_renderInfo.renderArea.extent.width, width);
				pass.m_renderInfo.renderArea.extent.height = Math::max(pass.m_renderInfo.renderArea.extent.height, height);
				pass.m_renderInfo.layerCount = Math::max(pass.m_renderInfo.layerCount, resource.handle->get_image()->get_info().arrayLayers);
			}
		}
		if (pass.m_depth != InvalidResourceId) {
			auto& resource = m_renderresources.get_direct(pass.m_depth);
			assert(resource.m_type == RenderResource::Type::Image);
			assert(resource.handle);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			if (pass.m_stencil == InvalidResourceId) {
				pass.m_depthAttachment.imageView = resource.handle->get_image()->get_depth_view()->get_image_view();
			}
			else {
				pass.m_depthAttachment.imageView = resource.handle->get_image_view();
			}
			if (*resource.m_writeToIn.begin() == pass.m_id) {
				pass.m_depthAttachment.clearValue.depthStencil.depth = attachment.clearColor[0];
			}
		}
		if (pass.m_stencil != InvalidResourceId) {
			auto& resource = m_renderresources.get_direct(pass.m_stencil);
			assert(pass.m_depth == InvalidResourceId || pass.m_depth == pass.m_stencil);
			assert(resource.m_type == RenderResource::Type::Image);
			assert(resource.handle);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			if (pass.m_depth == InvalidResourceId) {
				pass.m_stencilAttachment.imageView = resource.handle->get_image()->get_stencil_view()->get_image_view();
			}
			else {
				pass.m_stencilAttachment.imageView = resource.handle->get_image_view();
			}
			if (*resource.m_writeToIn.begin() == pass.m_id) {
				pass.m_stencilAttachment.clearValue.depthStencil.stencil = static_cast<uint32_t>(attachment.clearColor[1]);
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
		vulkan::CommandBufferType commandBufferType = vulkan::CommandBufferType::Generic;
		switch (pass.get_type()) {
		case Renderpass::Type::Compute:
			commandBufferType = vulkan::CommandBufferType::Compute;
			break;
		case Renderpass::Type::Graphics:
			commandBufferType = vulkan::CommandBufferType::Generic;
			break;
		}
		//std::cout << "Execute pass: "<< pass.get_id() << "\n";
		auto cmd = r_device.request_command_buffer(commandBufferType);
		cmd->begin_region(pass.m_name.c_str());
		pass.apply_pre_barriers(cmd);
		pass.execute(cmd);
		pass.apply_post_barriers(cmd);
		cmd->end_region();
		for (size_t i{ 0 }; i < pass.m_waitSemaphores.size(); i++) {
			r_device.add_wait_semaphores(commandBufferType, pass.m_waitSemaphores, pass.m_waitStages);
		}
		pass.m_waitSemaphores.clear();
		pass.m_waitStages.clear();
		std::vector<VkSemaphore> signals(pass.m_signalPassIds.size(), VK_NULL_HANDLE);
		r_device.submit(cmd, static_cast<uint32_t>(signals.size()), signals.data());
		for (size_t i{ 0 }; i < pass.m_signalPassIds.size(); i++) {
			auto& waitPass = m_renderpasses.get_direct(pass.m_signalPassIds[i]);
			waitPass.add_wait(signals[i], pass.m_signalStages[i]);
		}
	});
}

RenderResource& nyan::Rendergraph::add_ressource(const std::string& name, Attachment attachment)
{
	auto& r =m_renderresources.emplace(name, m_resourceCount++);
	r.attachment = attachment;
	return r;
}


RenderResource& nyan::Rendergraph::get_resource(std::string_view v)
{
	if (!m_renderresources.contains(std::string(v)))
		return m_renderresources.emplace(std::string(v), m_resourceCount++);
	return m_renderresources.get(std::string(v));
}

RenderResource& nyan::Rendergraph::get_resource(RenderResourceId id)
{
	return m_renderresources.get_direct(id);
}

const RenderResource& nyan::Rendergraph::get_resource(std::string_view v) const
{
	assert(m_renderresources.contains(std::string(v)));
	return m_renderresources.get(std::string(v));
}

const RenderResource& nyan::Rendergraph::get_resource(RenderResourceId id)const 
{
	return m_renderresources.get_direct(id);
}

void nyan::Rendergraph::set_swapchain(const std::string& name)
{
	auto& resource = m_renderresources.get(name);
	m_swapchainResource = resource.m_id;
}

vulkan::LogicalDevice& nyan::Rendergraph::get_device() const
{
	return r_device;
}

void nyan::Rendergraph::swapchain_present_transition(RenderpassId src_)
{
	if (m_swapchainResource == InvalidResourceId)
		return;
	auto& src = m_renderpasses.get_direct(src_);
	//TODO Differentiate between Async Compute and Graphics for potential ownership transfer
	//assert(src.get_type() == dst.get_type());
	auto& resource = m_renderresources.get_direct(m_swapchainResource);

	Barrier barrier;
	assert(resource.m_type == RenderResource::Type::Image);
	//auto& attachment = std::get<ImageAttachment>(resource.attachment);
	VkImageMemoryBarrier imageBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = VK_NULL_HANDLE, //This gets updated each frame
		.subresourceRange {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS,
		}
	};
	if (src.is_attachment(resource)) {
		barrier.src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else {
		if (src.is_compute_write(resource)) {
			imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier.src = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		else {
			imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier.src = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
	}

	barrier.resourceId = resource.m_id;
	barrier.dst = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	barrier.imageBarrierCount = static_cast<uint16_t>(1);
	assert(src.m_imageBarriers.size() <= USHRT_MAX);
	barrier.imageBarrierOffset = static_cast<uint16_t>(src.m_imageBarriers.size());
	//std::cout << "\n\t" << vulkan::ImageLayoutNames[imageBarrier.oldLayout] << " -> " << vulkan::ImageLayoutNames[imageBarrier.newLayout];
	//std::cout << "\n";
	src.m_imageBarriers.push_back(imageBarrier);
	src.m_postBarriers.push_back(barrier);

}

void nyan::Rendergraph::swapchain_write_transition(RenderpassId dst_)
{
	if (m_swapchainResource == InvalidResourceId)
		return;
	auto& dst = m_renderpasses.get_direct(dst_);
	//TODO Differentiate between Async Compute and Graphics for potential ownership transfer
	//assert(src.get_type() == dst.get_type());
	auto& resource = m_renderresources.get_direct(m_swapchainResource);

	Barrier barrier;
	assert(resource.m_type == RenderResource::Type::Image);
	auto& attachment = std::get<ImageAttachment>(resource.attachment);
	VkImageMemoryBarrier imageBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = VK_NULL_HANDLE, //This gets updated each frame
		.subresourceRange {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = VK_REMAINING_MIP_LEVELS, 
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS, 
		}
	};

	if (dst.is_attachment(resource)) {
		imageBarrier.newLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.dst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else {
		if (dst.is_compute_write(resource)) {
			imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier.dst = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		else {
			imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier.dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
	}
	barrier.resourceId = resource.m_id;
	barrier.src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	barrier.imageBarrierCount = static_cast<uint16_t>(1);
	assert(dst.m_imageBarriers.size() <= USHRT_MAX);
	barrier.imageBarrierOffset = static_cast<uint16_t>(dst.m_imageBarriers.size());
	//std::cout << "\n\t" << vulkan::ImageLayoutNames[imageBarrier.oldLayout] << " -> " << vulkan::ImageLayoutNames[imageBarrier.newLayout];
	//std::cout << "\n";
	dst.m_imageBarriers.push_back(imageBarrier);
	dst.m_preBarriers.push_back(barrier);

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
			.dstLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
		if (src.is_attachment(resource))
			barrier.srcLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (src.is_write(resource)) {
			barrier.srcLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		if (src.get_type() == Renderpass::Type::Graphics) {

			if (src.is_attachment(resource)) {
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
			if (src.is_write(resource)) {
				barrier.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrier.srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
			}
		}
		else {
			barrier.srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
		}
		if (dst.get_type() == Renderpass::Type::Graphics) {
			barrier.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			barrier.dstAccess = VK_ACCESS_SHADER_READ_BIT;
			//barrier.dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			//barrier.dstAccess = VK_ACCESS_MEMORY_READ_BIT;
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
		if (src.is_attachment(resource))
			barrier.srcLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (src.is_write(resource))
			barrier.srcLayout = VK_IMAGE_LAYOUT_GENERAL;


		if (dst.is_attachment(resource))
			barrier.dstLayout = vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format) ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (dst.is_write(resource))
			barrier.dstLayout = VK_IMAGE_LAYOUT_GENERAL;

		if (src.get_type() == Renderpass::Type::Graphics) {

			if (src.is_attachment(resource)) {
				if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
					barrier.srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					barrier.srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}
				else {
					barrier.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					barrier.srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				}
			}
			if (src.is_write(resource)) {
				barrier.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrier.srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
			}
		}
		else {
			barrier.srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
		}
		if (dst.get_type() == Renderpass::Type::Graphics) {
			if (dst.is_attachment(resource)) {
				if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
					barrier.dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					barrier.dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				}
				else {
					barrier.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					barrier.dstAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				}
			}
			if (dst.is_write(resource)) {
				barrier.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrier.srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
			}
		}
		else {
			barrier.dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			barrier.dstAccess = VK_ACCESS_SHADER_WRITE_BIT;
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
		};

		if (dst.is_attachment(resource))
		if (dst.is_write(resource))

		barrier.srcAccess = VK_ACCESS_SHADER_READ_BIT;
		if (src.get_type() == Renderpass::Type::Graphics) {
			barrier.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			barrier.srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		if (dst.get_type() == Renderpass::Type::Graphics) {
			if (dst.is_attachment(resource)) {
				if (vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
					barrier.dstStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					barrier.dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					barrier.dstLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					//barrier.dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					//barrier.srcAccess = VK_ACCESS_MEMORY_WRITE_BIT;
				}
				else {
					barrier.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					barrier.dstAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 
					barrier.dstLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					//barrier.dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
					//barrier.srcAccess = VK_ACCESS_MEMORY_WRITE_BIT;
				}
			}
			if (dst.is_write(resource)) {
				barrier.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				barrier.srcAccess = VK_ACCESS_SHADER_WRITE_BIT;
				barrier.dstLayout = VK_IMAGE_LAYOUT_GENERAL;
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
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS,
		}
	};
	barrier.resourceId = resource.m_id;
	barrier.src = imageBarrier_.srcStage;
	barrier.dst = imageBarrier_.dstStage;
	barrier.imageBarrierCount = static_cast<uint16_t>(1);
	assert(src.m_imageBarriers.size() <= USHRT_MAX);
	barrier.imageBarrierOffset = static_cast<uint16_t>(src.m_imageBarriers.size());
	assert((!(src.get_type() == Renderpass::Type::Compute) || (imageBarrier_.srcStage != VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)));
	assert((!(dst.get_type() == Renderpass::Type::Compute) || (imageBarrier_.dstStage != VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)));
	if (src.get_type() != dst.get_type() && r_device.get_graphics_family() != r_device.get_compute_family()) {
		if (src.get_type() == Renderpass::Type::Graphics) {
			imageBarrier.srcQueueFamilyIndex = r_device.get_graphics_family();
			imageBarrier.dstQueueFamilyIndex = r_device.get_compute_family();
		}
		else {
			imageBarrier.srcQueueFamilyIndex = r_device.get_compute_family();
			imageBarrier.dstQueueFamilyIndex = r_device.get_graphics_family();
		}
		auto queueTransferImageBarrier = imageBarrier;
		auto queueBarrier = barrier;

		queueBarrier.imageBarrierOffset = static_cast<uint16_t>(dst.m_imageBarriers.size());
		barrier.dst = 0;
		queueBarrier.src = 0;
		imageBarrier.dstAccessMask = 0;
		queueTransferImageBarrier.srcAccessMask = 0;
		dst.m_imageBarriers.push_back(queueTransferImageBarrier);
		dst.m_preBarriers.push_back(queueBarrier);
		src.add_signal(imageBarrier_.dst, imageBarrier_.dstStage);
	}
	else {
		//std::cout << "Resource: " << resource.m_id << " Barrier (" << barrier.imageBarrierOffset  << "): "<< imageBarrier_.src << " -> " << imageBarrier_.dst;
	}
	//std::cout << "\n\t" << vulkan::ImageLayoutNames[imageBarrier.oldLayout] << " -> " << vulkan::ImageLayoutNames[imageBarrier.newLayout];
	//std::cout << "\n";
	src.m_imageBarriers.push_back(imageBarrier);
	src.m_postBarriers.push_back(barrier);
}

