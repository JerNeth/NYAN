#include "Renderer/RenderGraph.h"
#include "VulkanWrapper/Image.h"
#include "CommandBuffer.h"

using namespace nyan;
//using namespace vulkan;

nyan::Renderpass::Renderpass(nyan::Rendergraph& graph, nyan::Renderpass::Type type, Id id, const std::string& name) :
	r_graph(graph),
	m_type(type),
	m_id(id),
	m_name(name)
{
}

void nyan::Renderpass::add_read(RenderResource::Id id, Renderpass::Read::Type readType)
{
	if (r_graph.m_state != Rendergraph::State::Setup)
		r_graph.m_state = Rendergraph::State::Dirty;
	auto& resource = r_graph.get_resource(id);
	assert(std::find_if(m_reads.cbegin(), m_reads.cend(), [&resource, readType](const auto& read) { return read.id == resource.m_id && read.type == readType;  }) == m_reads.cend());
	m_reads.push_back(Read{ resource.m_id, readType, VK_NULL_HANDLE });

	if (resource.uses.size() <= m_id.id) {
		resource.uses.resize(m_id.id + 1ull);
	}

	resource.uses[m_id.id].set(RenderResource::ImageUse::Sample);
}

void nyan::Renderpass::add_attachment(RenderResource::Id id, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	assert(r_graph.resource_exists(id));
	auto& resource = r_graph.get_resource(id);
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_attachments.push_back(resource.m_id);

	if (resource.uses.size() <= m_id.id)
		resource.uses.resize(m_id.id + 1ull);
	resource.uses[m_id.id].set(RenderResource::ImageUse::Attachment);
	if (clear)
		resource.uses[m_id.id].set(RenderResource::ImageUse::Clear);
}

void nyan::Renderpass::add_swapchain_attachment(Math::vec4 clearColor, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	//Currently only support hybrid queue for swapchain Synchronization

	if (r_graph.m_swapchainResource == InvalidResourceId) {
		nyan::ImageAttachment swap;
		swap.clearColor = clearColor;
		swap.format = r_graph.r_device.get_swapchain_image_view()->get_format();
		r_graph.m_swapchainResource = r_graph.add_ressource(swap);
	}
	auto& resource = r_graph.get_resource(r_graph.m_swapchainResource);
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_attachments.push_back(resource.m_id);
	m_rendersSwap = true;

	if (resource.uses.size() <= m_id.id)
		resource.uses.resize(m_id.id + 1ull);
	resource.uses[m_id.id].set(RenderResource::ImageUse::Attachment);
	if (clear)
		resource.uses[m_id.id].set(RenderResource::ImageUse::Clear);
}

void nyan::Renderpass::add_depth_attachment(RenderResource::Id id, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(id);
	m_depth = resource.m_id;

	if (resource.uses.size() <= m_id.id)
		resource.uses.resize(m_id.id + 1ull);
	
	resource.uses[m_id.id].set(RenderResource::ImageUse::Attachment);
	if (clear)
		resource.uses[m_id.id].set(RenderResource::ImageUse::Clear);
}

void nyan::Renderpass::add_depth_stencil_attachment(RenderResource::Id id, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(id);
	m_depth = m_stencil = resource.m_id;

	if (resource.uses.size() <= m_id.id)
		resource.uses.resize(m_id.id + 1ull);
	resource.uses[m_id.id].set(RenderResource::ImageUse::Attachment);
	if (clear)
		resource.uses[m_id.id].set(RenderResource::ImageUse::Clear);
}

void nyan::Renderpass::add_stencil_attachment(RenderResource::Id id, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(id);

	if (resource.uses.size() <= m_id.id)
		resource.uses.resize(m_id.id + 1ull);
	resource.uses[m_id.id].set(RenderResource::ImageUse::Attachment);
	if (clear)
		resource.uses[m_id.id].set(RenderResource::ImageUse::Clear);

	m_stencil = resource.m_id;
}

void nyan::Renderpass::add_write(RenderResource::Id id, Renderpass::Write::Type writeType, bool clear)
{
	if (r_graph.m_state != Rendergraph::State::Setup)
		r_graph.m_state = Rendergraph::State::Dirty;
	auto& resource = r_graph.get_resource(id);

	if (resource.uses.size() <= m_id.id)
		resource.uses.resize(m_id.id + 1ull);
	resource.uses[m_id.id].set(RenderResource::ImageUse::ImageStore);
	if (clear)
		resource.uses[m_id.id].set(RenderResource::ImageUse::Clear);

	assert(std::find_if(m_writes.cbegin(), m_writes.cend(), [&resource](const auto& write) { return write.id == resource.m_id; }) == m_writes.cend());
	m_writes.push_back(Write{ resource.m_id , writeType, VK_NULL_HANDLE});
}

void nyan::Renderpass::add_swapchain_write(Math::vec4 clearColor, Renderpass::Write::Type writeType, bool clear)
{
	if (r_graph.m_state != Rendergraph::State::Setup)
		r_graph.m_state = Rendergraph::State::Dirty;
	assert(m_type == Renderpass::Type::Generic);
	//Currently only support hybrid queue for swapchain Synchronization
	if (r_graph.m_swapchainResource == InvalidResourceId) {
		nyan::ImageAttachment swap;
		swap.clearColor = clearColor;
		swap.format = r_graph.r_device.get_swapchain_image_view()->get_format();
		r_graph.m_swapchainResource = r_graph.add_ressource(swap);
	}
	auto& resource = r_graph.get_resource(r_graph.m_swapchainResource);
	resource.name = "swapchain";

	if (resource.uses.size() <= m_id.id)
		resource.uses.resize(m_id.id + 1ull);

	resource.uses[m_id.id].set(RenderResource::ImageUse::ImageStore);
	if(clear)
		resource.uses[m_id.id].set(RenderResource::ImageUse::Clear);

	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_writes.push_back(Write{ resource.m_id , writeType, VK_NULL_HANDLE });
	m_rendersSwap = true;
}

void nyan::Renderpass::copy(RenderResource::Id source, RenderResource::Id target)
{
	if (r_graph.m_state != Rendergraph::State::Setup)
		r_graph.m_state = Rendergraph::State::Dirty;
	auto& sourceResource = r_graph.get_resource(source);
	auto& targetResource = r_graph.get_resource(target);
	targetResource.attachment = sourceResource.attachment;
	if (targetResource.uses.size() <= m_id.id) {
		targetResource.uses.resize(m_id.id + 1ull);
	}
	if (sourceResource.uses.size() <= m_id.id) {
		sourceResource.uses.resize(m_id.id + 1ull);
	}
	targetResource.uses[m_id.id].set(RenderResource::ImageUse::CopyTarget);
	sourceResource.uses[m_id.id].set(RenderResource::ImageUse::CopySource);
	m_copies.push_back(Copy{.src = sourceResource.m_id, .dst = targetResource.m_id});
	//sourceResource.m_copiedIn.insert(m_id);
	//targetResource.m_copiedIntoIn.insert(m_id);
}

void nyan::Renderpass::update() 
{
	update_views();
	update_rendering_info();
	update_image_barriers();
	update_binds();
}

void nyan::Renderpass::execute(vulkan::CommandBuffer& cmd)
{
	if (m_rendersSwap)
		cmd.touch_swapchain();
	for (size_t i{ 0 }; i < m_renderFunctions.size(); i++) {
		const auto &renderFunction = m_renderFunctions[i];
		if (m_useRendering[i])
			begin_rendering(cmd);
		renderFunction(cmd, *this);
		if (m_useRendering[i])
			end_rendering(cmd);
	}
	do_copies(cmd);
}

void nyan::Renderpass::do_copies(vulkan::CommandBuffer& cmd)
{
	apply_copy_barriers(cmd);
	for (const auto& [srcId, dstId] : m_copies) {
		const auto& src = r_graph.get_resource(srcId);
		const auto& dst = r_graph.get_resource(dstId);
		cmd.copy_image(*src.handle, *dst.handle);
	}
}

void nyan::Renderpass::apply_pre_barriers(vulkan::CommandBuffer& cmd)
{
	if (m_globalBarriers2.postIndex != m_globalBarriers2.preIndex ||
		m_bufferBarriers2.postIndex != m_bufferBarriers2.preIndex ||
		m_imageBarriers2.postIndex != m_imageBarriers2.preIndex)
		cmd.barrier2(0, 
			static_cast<uint32_t>(m_globalBarriers2.postIndex - m_globalBarriers2.preIndex), m_globalBarriers2.barriers.data() + m_globalBarriers2.preIndex,
			static_cast<uint32_t>(m_bufferBarriers2.postIndex - m_bufferBarriers2.preIndex), m_bufferBarriers2.barriers.data() + m_bufferBarriers2.preIndex,
			static_cast<uint32_t>(m_imageBarriers2.postIndex - m_imageBarriers2.preIndex), m_imageBarriers2.barriers.data() + m_imageBarriers2.preIndex);
}

void nyan::Renderpass::apply_copy_barriers(vulkan::CommandBuffer& cmd)
{
	if (m_globalBarriers2.preIndex != m_globalBarriers2.copyIndex ||
		m_bufferBarriers2.preIndex != m_bufferBarriers2.copyIndex ||
		m_imageBarriers2.preIndex != m_imageBarriers2.copyIndex)
		cmd.barrier2(0,
			static_cast<uint32_t>(m_globalBarriers2.preIndex - m_globalBarriers2.copyIndex), m_globalBarriers2.barriers.data() + m_globalBarriers2.copyIndex,
			static_cast<uint32_t>(m_bufferBarriers2.preIndex - m_bufferBarriers2.copyIndex), m_bufferBarriers2.barriers.data() + m_bufferBarriers2.copyIndex,
			static_cast<uint32_t>(m_imageBarriers2.preIndex - m_imageBarriers2.copyIndex), m_imageBarriers2.barriers.data() + m_imageBarriers2.copyIndex);
}

void nyan::Renderpass::apply_post_barriers(vulkan::CommandBuffer& cmd)
{
	if (m_globalBarriers2.barriers.size() != m_globalBarriers2.postIndex ||
		m_bufferBarriers2.barriers.size() != m_bufferBarriers2.postIndex ||
		m_imageBarriers2.barriers.size() != m_imageBarriers2.postIndex)
		cmd.barrier2(0,
			static_cast<uint32_t>(m_globalBarriers2.barriers.size() - m_globalBarriers2.postIndex), m_globalBarriers2.barriers.data() + m_globalBarriers2.postIndex,
			static_cast<uint32_t>(m_bufferBarriers2.barriers.size() - m_bufferBarriers2.postIndex), m_bufferBarriers2.barriers.data() + m_bufferBarriers2.postIndex,
			static_cast<uint32_t>(m_imageBarriers2.barriers.size() - m_imageBarriers2.postIndex), m_imageBarriers2.barriers.data() + m_imageBarriers2.postIndex);
}

void nyan::Renderpass::add_pre_barrier(const VkImageMemoryBarrier2& barrier, RenderResource::Id image)
{
	m_imageBarriers2.postIndex++;
	m_imageBarriers2.barriers.insert(m_imageBarriers2.barriers.begin() + m_imageBarriers2.preIndex, barrier);
	m_imageBarriers2.images.insert(m_imageBarriers2.images.begin() + m_imageBarriers2.preIndex, image);
}
void nyan::Renderpass::add_copy_barrier(const VkImageMemoryBarrier2& barrier, RenderResource::Id image)
{

	m_imageBarriers2.preIndex++;
	m_imageBarriers2.postIndex++;
	m_imageBarriers2.barriers.insert(m_imageBarriers2.barriers.begin() + m_imageBarriers2.copyIndex, barrier);
	m_imageBarriers2.images.insert(m_imageBarriers2.images.begin() + m_imageBarriers2.copyIndex, image);
}
void nyan::Renderpass::add_post_barrier(const VkImageMemoryBarrier2& barrier, RenderResource::Id image)
{
	m_imageBarriers2.barriers.insert(m_imageBarriers2.barriers.begin() + m_imageBarriers2.postIndex, barrier);
	m_imageBarriers2.images.insert(m_imageBarriers2.images.begin() + m_imageBarriers2.postIndex, image);
}

void nyan::Renderpass::add_pre_barrier(const VkMemoryBarrier2& barrier)
{
	m_globalBarriers2.postIndex++;
	m_globalBarriers2.barriers.insert(m_globalBarriers2.barriers.begin() + m_globalBarriers2.preIndex, barrier);
}
void nyan::Renderpass::add_copy_barrier(const VkMemoryBarrier2& barrier)
{

	m_globalBarriers2.preIndex++;
	m_globalBarriers2.postIndex++;
	m_globalBarriers2.barriers.insert(m_globalBarriers2.barriers.begin() + m_globalBarriers2.copyIndex, barrier);
}
void nyan::Renderpass::add_post_barrier(const VkMemoryBarrier2& barrier)
{
	m_globalBarriers2.barriers.insert(m_globalBarriers2.barriers.begin() + m_globalBarriers2.postIndex, barrier);
}

void nyan::Renderpass::add_pipeline(vulkan::GraphicsPipelineConfig config, vulkan::PipelineId* id)
{
	m_queuedPipelineBuilds.emplace_back(config, id);
}

void nyan::Renderpass::begin_rendering(vulkan::CommandBuffer& cmd)
{
	if (m_type == Renderpass::Type::Generic)
		cmd.begin_rendering(m_renderInfo);
}

void nyan::Renderpass::end_rendering(vulkan::CommandBuffer& cmd)
{
	if (m_type == Renderpass::Type::Generic)
		cmd.end_rendering();
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
uint32_t nyan::Renderpass::get_write_bind(RenderResource::Id id, Write::Type type)
{
	assert(id != InvalidResourceId);
	const auto& resource = r_graph.get_resource(id);
	auto res = std::find_if(m_writes.cbegin(), m_writes.cend(), [&resource, type](const auto& write) { return resource.m_id == write.id && type == write.type; });
	assert(res != m_writes.cend());
	if (res != m_writes.cend())
		return res->binding;
	else
		return InvalidResourceId.id;
}

uint32_t nyan::Renderpass::get_read_bind(RenderResource::Id id, Read::Type type)
{
	assert(id != InvalidResourceId);
	const auto& resource = r_graph.get_resource(id);
	auto res = std::find_if(m_reads.cbegin(), m_reads.cend(), [&resource, type](const auto& read) { return resource.m_id == read.id && type == read.type; });
	assert(res != m_reads.cend());
	if (res != m_reads.cend())
		return res->binding;
	else
		return InvalidResourceId.id;

}

void nyan::Renderpass::add_wait(VkSemaphore wait, VkPipelineStageFlags2 stage)
{
	m_waitInfos.push_back(VkSemaphoreSubmitInfo{
			.sType {VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO },
			.pNext {nullptr},
			.semaphore {wait},
			.value {0},
			.stageMask {stage}
		});
}

void nyan::Renderpass::add_signal(Renderpass::Id passId, VkPipelineStageFlags2 stage)
{
	m_signals.push_back({ passId, stage });
}

void nyan::Renderpass::build()
{
	build_rendering_info();
	build_pipelines();
}

void nyan::Renderpass::clear_dependencies()
{
	m_bufferBarriers2.clear();
	m_imageBarriers2.clear();
	m_bufferBarriers2.clear();
	m_signals.clear();
}

void nyan::Renderpass::clear_resource_references(RenderResource::Id id)
{
	for (auto it = m_writes.begin(); it != m_writes.end();) {
		if (it->id == id) {
			m_writes.erase(it);
			break;
		}
	}
	for (auto it = m_reads.begin(); it != m_reads.end();) {
		if (it->id == id) {
			m_reads.erase(it);
			break;
		}
	}
	for (auto it = m_copies.begin(); it != m_copies.end();) {
		if (it->dst == id || it->src == id) {
			m_copies.erase(it);
			break;
		}
	}
	for (auto it = m_attachments.begin(); it != m_attachments.end();) {
		if (it->id == id.id) {
			assert(false && "Attachment deletion not supported yet");
			return;
			//m_attachments.erase(it);
			//break;
		}
	}
	if (m_depth == id) {
		assert(false && "Attachment deletion not supported yet");
	}
	if (m_stencil == id) {
		assert(false && "Attachment deletion not supported yet");
	}
}

void nyan::Renderpass::build_rendering_info()
{
	if (m_type == Renderpass::Type::Generic) {
		m_renderingCreateInfo = {};
		m_renderInfo = { VK_STRUCTURE_TYPE_RENDERING_INFO };
		m_renderingCreateInfo.colorAttachmentCount = 0;
		m_renderInfo.colorAttachmentCount = 0;
		for (auto& [readId, readType, readView, readBinding] : m_reads) {
			auto& resource = r_graph.m_renderresources.get(readId);
			if (resource.m_type == RenderResource::Type::Image) {
				readView = VK_NULL_HANDLE;
			}
		}
		for (auto& [writeId, writeType, writeView, writeBinding] : m_writes) {
			auto& resource = r_graph.m_renderresources.get(writeId);
			if (resource.m_type == RenderResource::Type::Image) {
				writeView = VK_NULL_HANDLE;
			}
		}
		for (auto attachmentId : m_attachments) {
			auto& resource = r_graph.m_renderresources.get(attachmentId);
			if (resource.m_type == RenderResource::Type::Image) {
				auto& attachment = std::get<ImageAttachment>(resource.attachment);
				auto& info = m_renderingCreateInfo;
				info.colorAttachmentFormats[info.colorAttachmentCount++] = attachment.format;

				m_renderInfo.pColorAttachments = m_colorAttachments.data();
				VkRenderingAttachmentInfo renderingAttachment{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.resolveMode = VK_RESOLVE_MODE_NONE,
					.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				};
				if (resource.uses[m_id.id].test(RenderResource::ImageUse::Clear)) {
					renderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					for (size_t i = 0u; i < 4u; i++)
						renderingAttachment.clearValue.color.float32[i] = attachment.clearColor[i];
				}
				m_colorAttachments[m_renderInfo.colorAttachmentCount++] = renderingAttachment;
			}
		}
		if (m_depth != InvalidResourceId) {
			auto& resource = r_graph.m_renderresources.get(m_depth);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			assert(resource.m_type == RenderResource::Type::Image);
			VkRenderingAttachmentInfo renderingAttachment{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					.resolveMode = VK_RESOLVE_MODE_NONE,
					.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			};
			if (resource.uses[m_id.id].test(RenderResource::ImageUse::Clear)) {
				renderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				renderingAttachment.clearValue.depthStencil.depth = attachment.clearColor[0];
			}
			m_renderingCreateInfo.depthAttachmentFormat = attachment.format;
			m_renderInfo.pDepthAttachment = &m_depthAttachment;
			m_depthAttachment = renderingAttachment;
		}
		if (m_stencil != InvalidResourceId) {
			assert(m_depth == InvalidResourceId || m_depth == m_stencil);
			auto& resource = r_graph.m_renderresources.get(m_stencil);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			assert(resource.m_type == RenderResource::Type::Image);
			VkRenderingAttachmentInfo renderingAttachment{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					.resolveMode = VK_RESOLVE_MODE_NONE,
					.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			};
			if (resource.uses[m_id.id].test(RenderResource::ImageUse::Clear)) {
				renderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				renderingAttachment.clearValue.depthStencil.stencil = std::bit_cast<uint32_t>(attachment.clearColor[1]);
			}
			m_renderingCreateInfo.stencilAttachmentFormat = attachment.format;
			m_renderInfo.pStencilAttachment = &m_stencilAttachment;
			m_stencilAttachment = renderingAttachment;
		}
	}
}

void nyan::Renderpass::build_pipelines()
{
	for (auto& [config, id] : m_queuedPipelineBuilds) {
		config.renderingCreateInfo = m_renderingCreateInfo;
		*id = r_graph.r_device.get_pipeline_storage().add_pipeline(config);
	}
	//m_queuedPipelineBuilds.clear();
}


void nyan::Renderpass::update_binds() {


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
		const auto& write = r_graph.get_resource(writeId);
		if (write.m_type == RenderResource::Type::Image) {
			assert(is_write(writeId));
			if (writeBinding != ~0u) {
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
		const auto& read = r_graph.get_resource(readId);
		if (read.m_type == RenderResource::Type::Image) {
			assert(is_read(readId));
			if (readBinding != ~0u) {
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
}

void nyan::Renderpass::update_image_barriers()
{
	assert(m_imageBarriers2.barriers.size() == m_imageBarriers2.images.size());
	for (size_t i = 0; i < m_imageBarriers2.barriers.size(); i++) {
		auto& barrier = m_imageBarriers2.barriers[i];
		auto& image = m_imageBarriers2.images[i];
		assert(image != InvalidResourceId);
		auto& resource = r_graph.m_renderresources.get(image);
		assert(resource.m_type == RenderResource::Type::Image);
		assert(resource.handle);
		barrier.image = resource.handle->get_handle();
	}
}

void nyan::Renderpass::update_rendering_info()
{
	uint32_t attachmentId = 0;
	for (auto attachmentRessourceId : m_attachments) {
		auto& resource = r_graph.m_renderresources.get(attachmentRessourceId);
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		if (resource.m_type == RenderResource::Type::Image) {
			if (resource.uses[m_id.id].test(RenderResource::ImageUse::Clear)) {
				for (size_t i = 0u; i < 4u; i++)
					m_colorAttachments[attachmentId].clearValue.color.float32[i] = attachment.clearColor[i];
			}
			assert(resource.handle);
			m_colorAttachments[attachmentId++].imageView = *resource.handle->get_view();
			uint32_t width = r_graph.r_device.get_swapchain_width();
			uint32_t height = r_graph.r_device.get_swapchain_height();
			if (attachment.size == ImageAttachment::Size::Absolute) {
				width = static_cast<uint32_t>(attachment.width);
				height = static_cast<uint32_t>(attachment.height);
			}
			else {
				width = static_cast<uint32_t>(width * attachment.width);
				height = static_cast<uint32_t>(height * attachment.height);
			}
			m_renderInfo.renderArea.extent.width = Math::max(m_renderInfo.renderArea.extent.width, width);
			m_renderInfo.renderArea.extent.height = Math::max(m_renderInfo.renderArea.extent.height, height);
			m_renderInfo.layerCount = Math::max(m_renderInfo.layerCount, resource.handle->get_info().arrayLayers);
		}
	}
	if (m_depth != InvalidResourceId) {
		auto& resource = r_graph.m_renderresources.get(m_depth);
		assert(resource.m_type == RenderResource::Type::Image);
		assert(resource.handle);
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		if (m_stencil == InvalidResourceId) {
			m_depthAttachment.imageView = *resource.handle->get_depth_view();
		}
		else {
			m_depthAttachment.imageView = *resource.handle->get_view();
		}
		if (resource.uses[m_id.id].test(RenderResource::ImageUse::Clear)) {
			m_depthAttachment.clearValue.depthStencil.depth = attachment.clearColor[0];
		}
	}
	if (m_stencil != InvalidResourceId) {
		auto& resource = r_graph.m_renderresources.get(m_stencil);
		assert(m_depth == InvalidResourceId || m_depth == m_stencil);
		assert(resource.m_type == RenderResource::Type::Image);
		assert(resource.handle);
		auto& attachment = std::get<ImageAttachment>(resource.attachment);
		if (m_depth == InvalidResourceId) {
			m_stencilAttachment.imageView = *resource.handle->get_stencil_view();
		}
		else {
			m_stencilAttachment.imageView = *resource.handle->get_view();
		}
		if (resource.uses[m_id.id].test(RenderResource::ImageUse::Clear)) {
			m_stencilAttachment.clearValue.depthStencil.stencil = static_cast<uint32_t>(attachment.clearColor[1]);
		}
	}
}

void nyan::Renderpass::update_views()
{
	for (auto& [readId, readType, readView, readBinding] : m_reads) {
		auto& resource = r_graph.m_renderresources.get(readId);
		if (resource.m_type == RenderResource::Type::Image) {
			assert(resource.handle);
			if (readType == Renderpass::Read::Type::ImageColor) {
				readView = *resource.handle->get_view();

			}
			else if (readType == Renderpass::Read::Type::ImageDepth) {
				auto* tmp = resource.handle->get_depth_view();
				assert(tmp);
				if (!tmp)
					continue;
				readView = *tmp;
			}
			else if (readType == Renderpass::Read::Type::ImageStencil) {
				auto* tmp = resource.handle->get_stencil_view();
				assert(tmp);
				if (!tmp)
					continue;
				readView = *tmp;
			}

		}
	}
	for (auto& [writeId, writeType, writeView, writeBinding] : m_writes) {
		auto& resource = r_graph.m_renderresources.get(writeId);
		if (resource.m_type == RenderResource::Type::Image) {
			assert(resource.handle);
			if (!resource.handle)
				continue;
			writeView = *resource.handle->get_view();
		}
	}
}


bool nyan::Renderpass::is_read(RenderResource::Id id) const
{
	return std::find_if(m_reads.cbegin(), m_reads.cend(), [id](const auto& read) { return read.id == id; }) != m_reads.cend();
}

bool nyan::Renderpass::is_write(RenderResource::Id id) const
{
	return std::find_if(m_writes.cbegin(), m_writes.cend(), [id](const auto& write) { return write.id == id; }) != m_writes.cend();
}

bool nyan::Renderpass::is_compute_write(RenderResource::Id id) const
{
	auto it = std::find_if(m_writes.cbegin(), m_writes.cend(), [id](const auto& write) { return write.id == id && write.type == Renderpass::Write::Type::Compute; });
	return it != m_writes.cend();
}

bool nyan::Renderpass::is_attachment(RenderResource::Id id) const
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

Renderpass::Id nyan::Rendergraph::add_pass(const std::string& name, Renderpass::Type type)
{
	Renderpass::Id id = Renderpass::Id{ m_renderpassCount++ };
	assert(m_state == State::Setup);
	if (type == Renderpass::Type::Generic)
		m_lastGeneric = id;
	else if(type == Renderpass::Type::AsyncCompute)
		m_lastCompute = id;
	m_renderpasses.emplace(id, *this, type, id, name);
	return id;
}

Renderpass& nyan::Rendergraph::get_pass(Renderpass::Id id)
{
	return m_renderpasses.get(id);
}

void nyan::Rendergraph::build()
{
	assert(m_state == State::Setup);
	m_state = State::Build;
	m_renderresources.for_each([&](RenderResource& resource) {
		setup_render_resource_barriers(resource);
	});
	m_renderpasses.for_each([&](Renderpass& pass) {

		pass.build();
		});

	assert(m_state == State::Build);
	m_state = State::Execute;
}
void nyan::Rendergraph::begin_frame()
{
	if (m_state == State::Dirty)
	{
		clear_dependencies();
		remove_queued_resources();
		m_renderresources.for_each([&](RenderResource& resource) {
			setup_render_resource_barriers(resource);
			});

		m_state = State::Execute;
	}
	//Update Attachments
	m_renderresources.for_each([&](RenderResource& resource) {
		update_render_resource(resource);
		});
	m_renderpasses.for_each([this](Renderpass& pass) {
		pass.update();
		});
}

void nyan::Rendergraph::end_frame()
{

	assert(m_state == State::Execute);

	m_renderpasses.for_each([this](Renderpass& pass) {

		vulkan::CommandBufferType commandBufferType = vulkan::CommandBufferType::Generic;
		switch (pass.get_type()) {
		case Renderpass::Type::AsyncCompute:
			commandBufferType = vulkan::CommandBufferType::Compute;
			break;
		case Renderpass::Type::Generic:
			commandBufferType = vulkan::CommandBufferType::Generic;
			break;
		}
		//std::cout << "Execute pass: "<< pass.get_id() << "\n";
		auto cmdHandle = r_device.request_command_buffer(commandBufferType);
		auto& cmd = *cmdHandle;
		cmd.begin_region(pass.m_name.data());
		pass.apply_pre_barriers(cmd);
		for (auto [id, type, view, binding] : pass.m_writes) {
			auto& resource = m_renderresources.get(id);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			if (resource.m_type == RenderResource::Type::Image && !vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				if (resource.uses[pass.m_id.id].test(RenderResource::ImageUse::Clear)) {
					VkClearColorValue clearValue{
						.float32 {
							attachment.clearColor.x(),
							attachment.clearColor.y(),
							attachment.clearColor.z(),
							attachment.clearColor.w()
						}
					};
					cmd.clear_color_image(*resource.handle, VK_IMAGE_LAYOUT_GENERAL, &clearValue);
				}
			}
		}
		pass.execute(cmd);
		pass.apply_post_barriers(cmd);
		cmd.end_region();

		r_device.add_wait_semaphores(commandBufferType, pass.m_waitInfos);
		pass.m_waitInfos.clear();
		std::vector<VkSemaphore> signals(pass.m_signals.size(), VK_NULL_HANDLE);
		if (pass.m_id == m_lastCompute || pass.m_id == m_lastGeneric)
			signals.push_back(VK_NULL_HANDLE);
		r_device.submit(cmdHandle, static_cast<uint32_t>(signals.size()), signals.data());
		for (size_t i{ 0 }; i < pass.m_signals.size(); i++) {
			auto& waitPass = m_renderpasses.get(pass.m_signals[i].passId);
			waitPass.add_wait(signals[i], pass.m_signals[i].stage);
		}
		if (pass.m_id == m_lastCompute || pass.m_id == m_lastGeneric)
			r_device.add_wait_semaphore(vulkan::CommandBufferType::Transfer, signals.back(), VK_PIPELINE_STAGE_2_COPY_BIT);
	});
}

RenderResource::Id nyan::Rendergraph::add_ressource(const std::string& name, Attachment attachment)
{
	RenderResource::Id id{ m_resourceCount++ };
	auto& r = m_renderresources.emplace(id, id);
	r.name = name;
	r.attachment = attachment;
	if (attachment.index() == 0) {
		r.m_type = RenderResource::Type::Image;
	}
	else {
		r.m_type = RenderResource::Type::Buffer;
	}
	return id;
}

RenderResource::Id nyan::Rendergraph::add_ressource(Attachment attachment)
{
	RenderResource::Id id{ m_resourceCount++ };
	auto& r = m_renderresources.emplace(id, id);
	r.name = std::format("UnnamedRenderResource {}", static_cast<uint32_t>(id.id));
	r.attachment = attachment;
	if (attachment.index() == 0) {
		r.m_type = RenderResource::Type::Image;
	}
	else {
		r.m_type = RenderResource::Type::Buffer;
	}
	return id;
}

RenderResource& nyan::Rendergraph::get_resource(RenderResource::Id id)
{
	assert(id != InvalidResourceId);
	return m_renderresources.get(id);
}

const RenderResource& nyan::Rendergraph::get_resource(RenderResource::Id id) const
{
	assert(id != InvalidResourceId);
	return m_renderresources.get(id);
}

void nyan::Rendergraph::remove_resource(RenderResource::Id id)
{
	if(resource_exists(id))
		m_queuedResourceDeletion.push_back(id);
}

bool nyan::Rendergraph::resource_exists(RenderResource::Id id)
{
	return m_renderresources.contains(id);
}

void nyan::Rendergraph::swapchain_present_transition(Renderpass::Id src_const)
{
	if (m_swapchainResource == InvalidResourceId) {
		Utility::log().format("Renderpass: %d tries to write to an invalid swapchain, this shouldn't happen", static_cast<uint32_t>(src_const.id));
		assert(false);
		return;
	}
	auto& src = m_renderpasses.get(src_const);
	assert(src.get_type() == Renderpass::Type::Generic);

	auto& resource = m_renderresources.get(m_swapchainResource);
	Utility::log().format("Swap Present Ressource ({}) Renderpass ({})", resource.name.data(), src.m_name.data());
	assert(resource.uses.size() > src_const.id);
	auto usage = resource.uses[src_const.id];

	if (resource.m_type != RenderResource::Type::Image) {
		Utility::log().format("Renderpass: %d thinks the swapchain image is a buffer", static_cast<uint32_t>(src_const.id));
		assert(false);
		return;
	}
	VkImageMemoryBarrier2 imageBarrier{
		.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask {},
		.srcAccessMask {},
		.dstStageMask { VK_PIPELINE_STAGE_2_NONE },  //Use semaphore instead
		.dstAccessMask { 0 }, //Use semaphore instead
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image {VK_NULL_HANDLE},
		.subresourceRange {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS,
		}
	};
	if (usage.test(RenderResource::ImageUse::CopyTarget)) {
		assert(!usage.test(RenderResource::ImageUse::BlitTarget));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::BlitTarget)) {
		assert(!usage.test(RenderResource::ImageUse::CopyTarget));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::Sample)) {
		assert(!usage.test(RenderResource::ImageUse::ImageStore));
		assert(!usage.test(RenderResource::ImageUse::Attachment));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.srcAccessMask = 0;//VK_ACCESS_2_SHADER_SAMPLED_READ_BIT; No purpose in making reads available
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::Attachment)) {
		assert(!usage.test(RenderResource::ImageUse::ImageLoad));
		assert(!usage.test(RenderResource::ImageUse::ImageStore));
		assert(!usage.test(RenderResource::ImageUse::Sample));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::ImageLoad)) {
		//if(src.is_compute_write(resource))
		assert(false);
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.srcAccessMask = 0;//VK_ACCESS_2_SHADER_STORAGE_READ_BIT; No purpose in making reads available
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::ImageUse::ImageStore)) {
		assert(src.is_write(resource));
		if (src.is_compute_write(resource)) {
			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		}
		else {
			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		}
		imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	}

	//Transfer should have happened now
	if (usage.test(RenderResource::ImageUse::CopySource)) {
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		imageBarrier.srcAccessMask = 0; // VK_ACCESS_2_TRANSFER_READ_BIT; No purpose in making reads available
		//if(imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::BlitSource)) {
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		imageBarrier.srcAccessMask = 0; // VK_ACCESS_2_TRANSFER_READ_BIT; No purpose in making reads available
		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	}
	

	assert(imageBarrier.oldLayout != VK_IMAGE_LAYOUT_UNDEFINED);

	src.add_post_barrier(imageBarrier, m_swapchainResource);

}

bool debugBarriers = false;


void nyan::Rendergraph::set_up_transition(Renderpass::Id from, Renderpass::Id to, const RenderResource& resource)
{
	auto& src = m_renderpasses.get(from);
	auto& dst = m_renderpasses.get(to);

	if (resource.m_type != RenderResource::Type::Image) {
		Utility::log().format("Renderpass ({}) -> ({}) wants to transition something which isn't an image ({}), which is not supported yet",
			src.m_name, dst.m_name, resource.name);
		assert(false);
		return;
	}
	auto srcUsage = resource.uses[from.id];
	auto dstUsage = resource.uses[to.id];

	auto& attachment = std::get<ImageAttachment>(resource.attachment);
	
	VkImageMemoryBarrier2 imageBarrier{
		.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask {},
		.srcAccessMask {},
		.dstStageMask {},  //Use semaphore instead
		.dstAccessMask {}, //Use semaphore instead
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image {VK_NULL_HANDLE},
		.subresourceRange {
			.aspectMask = vulkan::ImageInfo::format_to_aspect_mask(attachment.format),
			.baseMipLevel = 0,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS,
		}
	};
	auto setBarrierSource = [&imageBarrier](VkPipelineStageFlags2 stage, VkAccessFlags2 access, VkImageLayout layout) 
	{
		imageBarrier.srcStageMask |= stage;
		imageBarrier.srcAccessMask |= access;
		if (imageBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			imageBarrier.oldLayout = layout;
		else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			imageBarrier.oldLayout = layout;
		else if (imageBarrier.oldLayout != layout)
			assert(false);
	};
	if (srcUsage.test(RenderResource::ImageUse::Sample)) {
		assert(!srcUsage.test(RenderResource::ImageUse::ImageStore));
		assert(!srcUsage.test(RenderResource::ImageUse::Attachment));
		if (src.get_type() == Renderpass::Type::Generic)
			setBarrierSource(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, 0 /*VK_ACCESS_2_SHADER_SAMPLED_READ_BIT*/, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else
			setBarrierSource(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, 0 /*VK_ACCESS_2_SHADER_SAMPLED_READ_BIT*/, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	}
	if (srcUsage.test(RenderResource::ImageUse::Attachment)) {
		assert(!srcUsage.test(RenderResource::ImageUse::ImageLoad));
		assert(!srcUsage.test(RenderResource::ImageUse::ImageStore));
		assert(!srcUsage.test(RenderResource::ImageUse::Sample));
		if(vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format))
			setBarrierSource(VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
		else
			setBarrierSource(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
	}
	if (srcUsage.test(RenderResource::ImageUse::ImageLoad)) {
		//if(src.is_compute_write(resource))
		assert(false);
		setBarrierSource(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, 0 /*VK_ACCESS_2_SHADER_STORAGE_READ_BIT*/, VK_IMAGE_LAYOUT_GENERAL);

	}
	if (srcUsage.test(RenderResource::ImageUse::ImageStore)) {
		assert(src.is_write(resource));
		auto stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		if (src.is_compute_write(resource)) {
			stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		}
		setBarrierSource(stage, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);
	}

	if (srcUsage.test(RenderResource::ImageUse::CopySource)) {
		setBarrierSource(VK_PIPELINE_STAGE_2_COPY_BIT, 0/*VK_ACCESS_2_TRANSFER_READ_BIT*/, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}

	if (srcUsage.test(RenderResource::ImageUse::BlitSource)) {
		setBarrierSource(VK_PIPELINE_STAGE_2_BLIT_BIT, 0/*VK_ACCESS_2_TRANSFER_READ_BIT*/, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}

	if (srcUsage.test(RenderResource::ImageUse::CopyTarget)) {
		assert(!srcUsage.test(RenderResource::ImageUse::BlitTarget));
		setBarrierSource(VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	if (srcUsage.test(RenderResource::ImageUse::BlitTarget)) {
		assert(!srcUsage.test(RenderResource::ImageUse::CopyTarget));
		setBarrierSource(VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}


	auto setBarrierDestination= [&imageBarrier](VkPipelineStageFlags2 stage, VkAccessFlags2 access, VkImageLayout layout)
	{
		imageBarrier.dstStageMask |= stage;
		imageBarrier.dstAccessMask |= access;
		if (imageBarrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			imageBarrier.newLayout = layout;
		else if(imageBarrier.newLayout != layout)
			assert(false);
	};

	if (dstUsage.test(RenderResource::ImageUse::CopyTarget)) {
		setBarrierDestination(VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}
	if (dstUsage.test(RenderResource::ImageUse::BlitTarget)) {
		setBarrierDestination(VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	if (dstUsage.test(RenderResource::ImageUse::Sample)) {
		assert(!dstUsage.test(RenderResource::ImageUse::ImageStore));
		assert(!dstUsage.test(RenderResource::ImageUse::Attachment));
		if(dst.get_type() == Renderpass::Type::Generic)
			setBarrierDestination(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_SHADER_READ_BIT_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else
			setBarrierDestination(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_SHADER_READ_BIT_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	if (dstUsage.test(RenderResource::ImageUse::Attachment)) {
		assert(!dstUsage.test(RenderResource::ImageUse::ImageLoad));
		assert(!dstUsage.test(RenderResource::ImageUse::ImageStore));
		assert(!dstUsage.test(RenderResource::ImageUse::Sample));
		setBarrierDestination(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
	}
	if (dstUsage.test(RenderResource::ImageUse::ImageLoad)) {
		//if(src.is_compute_write(resource))
		assert(false);
		setBarrierDestination(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);
	}
	if (dstUsage.test(RenderResource::ImageUse::ImageStore)) {
		assert(dst.is_write(resource));
		auto stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		if (dst.is_compute_write(resource)) {
			stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		}
		setBarrierDestination(stage, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);
	}

	if (src.m_type != dst.m_type && r_device.get_graphics_family() != r_device.get_compute_family()) {
		if (src.m_type == Renderpass::Type::Generic) {
			imageBarrier.srcQueueFamilyIndex = r_device.get_graphics_family();
			imageBarrier.dstQueueFamilyIndex = r_device.get_compute_family();
		}
		else {
			imageBarrier.srcQueueFamilyIndex = r_device.get_compute_family();
			imageBarrier.dstQueueFamilyIndex = r_device.get_graphics_family();
		}
		assert(imageBarrier.newLayout != VK_IMAGE_LAYOUT_UNDEFINED);
		assert(imageBarrier.newLayout != VK_IMAGE_LAYOUT_PREINITIALIZED);
		auto copyBarrier = imageBarrier;
		copyBarrier.srcStageMask = 0;
		copyBarrier.srcAccessMask = 0;
		imageBarrier.dstStageMask = 0;
		imageBarrier.dstAccessMask = 0;
		assert(copyBarrier.dstStageMask);
		dst.add_pre_barrier(copyBarrier, resource.m_id);
		src.add_signal(to, copyBarrier.dstStageMask);

	}

	if (debugBarriers) {
		Utility::log().format("Ressource ({}) Renderpass ({}) -> ({})", resource.name, src.m_name, dst.m_name);
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	}
	if (imageBarrier.oldLayout != imageBarrier.newLayout && imageBarrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
		Utility::log().format("Renderpass {} -> {}, Possibly no Barrier needed here", src.m_name, dst.m_name);
		return;
	}

	if (imageBarrier.oldLayout == imageBarrier.newLayout &&
		imageBarrier.srcQueueFamilyIndex == imageBarrier.dstQueueFamilyIndex &&
		srcUsage.only(RenderResource::ImageUse::Attachment) && dstUsage.only(RenderResource::ImageUse::Attachment)) {
		Utility::log().format("Renderpass {} -> {}, Barrier for attachment only usage without layout transition or Queue Ownership transfer", src.m_name, dst.m_name);
	}
	else {
		src.add_post_barrier(imageBarrier, resource.m_id);
	}
	//src.m_imageBarriers2.postIndex++;
	//src.m_imageBarriers2.copyIndex++;
	//src.m_imageBarriers2.preIndex++;


}

void nyan::Rendergraph::update_render_resource(RenderResource& resource)
{
	if (resource.m_type == RenderResource::Type::Image) {
		update_render_resource_image(resource);
	}
	else {
		assert(false);
	}
}

void nyan::Rendergraph::update_render_resource_image(RenderResource& resource)
{
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
		resource.handle = r_device.get_swapchain_image();
	}
	else {
		VkImageUsageFlags usage = 0;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		bool first = true;
		Utility::bitset<static_cast<size_t>(RenderResource::ImageUse::Size), RenderResource::ImageUse> totalUses;

		for (const auto& use : resource.uses) {
			totalUses |= use;
			//TODO add transfer dst for clear depth stencil
			//if (use.test(RenderResource::ImageUse::Clear) 
			//	&& vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)
			//	&& ) {
			//	usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			//}
			if (use.any_of(RenderResource::ImageUse::BlitTarget, RenderResource::ImageUse::CopyTarget)) {
				usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				if (first) {
					first = false;
					initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				}
			}
			if (use.test(RenderResource::ImageUse::Sample)) {
				usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
				if (first) {
					first = false;
					initialLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
					Utility::log().format("First usage of image %d is sampling, is this intended?", resource.name);
				}
			}
			if (use.any_of(RenderResource::ImageUse::ImageLoad, RenderResource::ImageUse::ImageStore)) {
				usage |= VK_IMAGE_USAGE_STORAGE_BIT;
				if (first) {
					first = false;
					initialLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
			}
			if (use.test(RenderResource::ImageUse::Attachment) && vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				if (first) {
					first = false;
					initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
			}
			if (use.test(RenderResource::ImageUse::Attachment) && !vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				if (first) {
					first = false;
					initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
			}
			if (use.any_of(RenderResource::ImageUse::BlitSource, RenderResource::ImageUse::CopySource)) {
				usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				if (first) {
					first = false;
					initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					Utility::log().format("First usage of image %d is as copy source, is this intended?", resource.name);
				}
			}
		}
		if (!totalUses.none())
			resource.handle = r_device.request_render_target(width, height, attachment.format, resource.m_id.id, usage, initialLayout);
	}
	resource.handle->set_debug_label(resource.name.data());
	resource.handle->get_view()->set_debug_label((resource.name + "_view").c_str());
}

void nyan::Rendergraph::setup_render_resource_barriers(RenderResource& resource)
{
	bool first = true;

	for (Renderpass::Id::Type i = 0; i < static_cast<Renderpass::Id::Type>(resource.uses.size()); i++) {
		const auto& srcUse = resource.uses[i];
		if (srcUse.none())
			continue;
		if (first) {
			first = false;
			//CreatePreBarrier
			auto lastUse = i;
			for (auto j = i; j < static_cast<Renderpass::Id::Type>(resource.uses.size()); j++) {
				if (!resource.uses[j].none())
					lastUse = j;
			}
			if (srcUse.test(RenderResource::ImageUse::Clear) || resource.m_id == m_swapchainResource)
				set_up_first_transition(Renderpass::Id{ i }, resource);
			else
				set_up_transition(Renderpass::Id{ lastUse }, Renderpass::Id{ i }, resource);
		}
		set_up_copy(Renderpass::Id{ i }, resource);
		auto j = i + 1;
		for (; j < static_cast<Renderpass::Id::Type>(resource.uses.size()); j++) {
			const auto& dstUse = resource.uses[j];
			if (!dstUse.none())
				break;
		}
		if (j < static_cast<Renderpass::Id::Type>(resource.uses.size())) {
			set_up_transition(Renderpass::Id{ i }, Renderpass::Id{ j }, resource);
		}
		i = j - 1;
	}

	if (resource.m_id == m_swapchainResource) {

		for (int64_t i = static_cast<int64_t>(resource.uses.size()) - 1; i > 0; i--) {
			const auto& srcUse = resource.uses[i];
			if (!srcUse.none()) {
				swapchain_present_transition(Renderpass::Id{ static_cast<Renderpass::Id::Type>(i) });
				break;
			}
		}
	}
}

void nyan::Rendergraph::set_up_first_transition(Renderpass::Id dst_const, const RenderResource& resource) {
	auto& dst = m_renderpasses.get(dst_const);
	assert(dst.get_type() == Renderpass::Type::Generic);

	assert(resource.uses.size() > dst_const.id);
	auto usage = resource.uses[dst_const.id];

	if (resource.m_type != RenderResource::Type::Image) {
		Utility::log().format("Renderpass: {} tries not supported resource transition", dst.m_name);
		assert(false);
		return;
	}
	auto& attachment = std::get<ImageAttachment>(resource.attachment);
	VkImageMemoryBarrier2 imageBarrier{
		.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask { VK_PIPELINE_STAGE_2_NONE }, //Use semaphore instead
		.srcAccessMask { 0 }, //Use semaphore instead
		.dstStageMask { },
		.dstAccessMask { },
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image {VK_NULL_HANDLE},
		.subresourceRange {
			.aspectMask = vulkan::ImageInfo::format_to_aspect_mask(attachment.format),
			.baseMipLevel = 0,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS,
		}
	};

	if (usage.test(RenderResource::ImageUse::Sample)) {
		assert(!usage.test(RenderResource::ImageUse::Attachment));
		assert(!usage.test(RenderResource::ImageUse::ImageLoad)); //Technically possible, but unreasonable?
		assert(!usage.test(RenderResource::ImageUse::ImageStore));
		if (!usage.any_of(RenderResource::ImageUse::BlitTarget, RenderResource::ImageUse::CopyTarget)) {
			Utility::log().format("Renderpass: {} tries to sample an undefined target, this shouldn't happen", dst.m_name);
			assert(false);
		}
		//TODO: Compute Shader
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::ImageLoad)) {
		assert(!usage.test(RenderResource::ImageUse::Attachment));
		assert(!usage.test(RenderResource::ImageUse::Sample));  //Technically possible, but unreasonable?
		if (!usage.any_of(RenderResource::ImageUse::BlitTarget, RenderResource::ImageUse::CopyTarget)) {
			Utility::log().format("Renderpass: {} tries to load an undefined target, this shouldn't happen", dst.m_name);
			assert(false);
		}
		assert(false);
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::ImageUse::Attachment)) {
		assert(!usage.test(RenderResource::ImageUse::ImageLoad));
		assert(!usage.test(RenderResource::ImageUse::ImageStore));
		assert(!usage.test(RenderResource::ImageUse::Sample));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::ImageStore)) {
		assert(dst.is_write(resource));
		if (dst.is_compute_write(resource)) {
			imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		}
		else {
			imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		}
		imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (imageBarrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;

	if (debugBarriers) {
		Utility::log().format("Ressource ({}) Renderpass ({})", resource.name.data(), dst.m_name);
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	}

	dst.add_pre_barrier(imageBarrier, resource.m_id);

}

void nyan::Rendergraph::set_up_copy(Renderpass::Id dst_const, const RenderResource& resource) {
	auto& dst = m_renderpasses.get(dst_const);

	assert(resource.uses.size() > dst_const.id);
	auto usage = resource.uses[dst_const.id];

	if (resource.m_type != RenderResource::Type::Image) {
		Utility::log().format("Renderpass: {} tries not supported resource copy", dst.m_name);
		assert(false);
		return;
	}
	auto& attachment = std::get<ImageAttachment>(resource.attachment);
	VkImageMemoryBarrier2 imageBarrier{
		.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask { VK_PIPELINE_STAGE_2_NONE }, //Use semaphore instead
		.srcAccessMask { 0 }, //Use semaphore instead
		.dstStageMask { },
		.dstAccessMask { },
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image {VK_NULL_HANDLE},
		.subresourceRange {
			.aspectMask = vulkan::ImageInfo::format_to_aspect_mask(attachment.format),
			.baseMipLevel = 0,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS,
		}
	};

	if (usage.test(RenderResource::ImageUse::Sample)) {
		assert(!usage.test(RenderResource::ImageUse::Attachment));
		assert(!usage.test(RenderResource::ImageUse::ImageLoad)); //Technically possible, but unreasonable?
		assert(!usage.test(RenderResource::ImageUse::ImageStore));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::ImageLoad)) {
		assert(!usage.test(RenderResource::ImageUse::Attachment));
		assert(!usage.test(RenderResource::ImageUse::Sample));  //Technically possible, but unreasonable?
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::ImageUse::Attachment)) {
		assert(!usage.test(RenderResource::ImageUse::ImageLoad));
		assert(!usage.test(RenderResource::ImageUse::ImageStore));
		assert(!usage.test(RenderResource::ImageUse::Sample));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	}
	if (usage.test(RenderResource::ImageUse::ImageStore)) {
		assert(dst.is_write(resource));
		if (dst.is_compute_write(resource)) {
			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		}
		else {
			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		}
		imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	}

	if (usage.test(RenderResource::ImageUse::CopyTarget)) {
		assert(!usage.test(RenderResource::ImageUse::CopySource));
		assert(!usage.test(RenderResource::ImageUse::BlitSource));
		assert(!usage.test(RenderResource::ImageUse::BlitTarget));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		//else
		//	imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::ImageUse::BlitTarget)) {
		assert(!usage.test(RenderResource::ImageUse::CopyTarget));
		assert(!usage.test(RenderResource::ImageUse::CopySource));
		assert(!usage.test(RenderResource::ImageUse::BlitSource));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		//else
		//	imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::ImageUse::CopySource)) {
		assert(!usage.test(RenderResource::ImageUse::CopyTarget));
		assert(!usage.test(RenderResource::ImageUse::BlitSource));
		assert(!usage.test(RenderResource::ImageUse::BlitTarget));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		//else
		//	imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::ImageUse::BlitSource)) {
		assert(!usage.test(RenderResource::ImageUse::CopyTarget));
		assert(!usage.test(RenderResource::ImageUse::CopySource));
		assert(!usage.test(RenderResource::ImageUse::BlitTarget));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;

		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		//else
		//	imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}

	//We actually need an execution barrier here
	if (imageBarrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;

	assert(dst.get_type() == Renderpass::Type::Generic);

	if (debugBarriers) {
		Utility::log().format("Copy Ressource ({}) Renderpass ({})", resource.name, dst.m_name);
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	}

	dst.add_copy_barrier(imageBarrier, resource.m_id);

}

void nyan::Rendergraph::clear_dependencies()
{
	m_renderpasses.for_each([this](Renderpass& pass) {
		pass.clear_dependencies();
		});
}

void nyan::Rendergraph::remove_queued_resources()
{
	m_renderpasses.for_each([this](Renderpass& pass) {
		for (auto id : m_queuedResourceDeletion)
			pass.clear_resource_references(id);
		
		});

	for (auto id : m_queuedResourceDeletion)
		m_renderresources.remove(id);

	m_queuedResourceDeletion.clear();
}