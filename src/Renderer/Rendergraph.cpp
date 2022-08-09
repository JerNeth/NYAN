#include "Renderer/RenderGraph.h"
#include "CommandBuffer.h"

using namespace nyan;
//using namespace vulkan;

nyan::Renderpass::Renderpass(Rendergraph& graph, Type type, uint32_t id, const entt::hashed_string& name) :
	r_graph(graph),
	m_type(type),
	m_id(id),
	m_name(name)
{
}

void nyan::Renderpass::add_read(const entt::hashed_string& name, Renderpass::Read::Type readType)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	assert(std::find_if(m_reads.cbegin(), m_reads.cend(), [&resource, readType](const auto& read) { return read.id == resource.m_id && read.type == readType;  }) == m_reads.cend());
	m_reads.push_back(Read{ resource.m_id, readType, VK_NULL_HANDLE });

	if (resource.m_uses.size() <= m_id) {
		resource.m_uses.resize(m_id + 1ull);
	}

	resource.m_uses[m_id].set(RenderResource::UseType::Sample);
}

void nyan::Renderpass::add_attachment(const entt::hashed_string& name, ImageAttachment attachment, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	resource.attachment = attachment;
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_attachments.push_back(resource.m_id);

	if (resource.m_uses.size() <= m_id) 
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);
}

void nyan::Renderpass::add_attachment(const entt::hashed_string& name, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	assert(r_graph.resource_exists(name));
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_attachments.push_back(resource.m_id);

	if (resource.m_uses.size() <= m_id)
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);
}

void nyan::Renderpass::add_swapchain_attachment(Math::vec4 clearColor, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	//Currently only support hybrid queue for swapchain Synchronization
	nyan::ImageAttachment swap;
	swap.clearColor = clearColor;
	swap.format = r_graph.get_device().get_swapchain_image_view()->get_format();
	auto& resource = r_graph.get_resource("swap");
	resource.name = "swap";
	resource.attachment = swap;
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_attachments.push_back(resource.m_id);
	r_graph.set_swapchain(resource.name);
	m_rendersSwap = true;

	if (resource.m_uses.size() <= m_id) 
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);
}

void nyan::Renderpass::add_depth_attachment(const entt::hashed_string& name, ImageAttachment attachment, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	resource.attachment = attachment;
	resource.m_type = RenderResource::Type::Image;
	m_depth = resource.m_id;

	if (resource.m_uses.size() <= m_id) 
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);
}

void nyan::Renderpass::add_depth_attachment(const entt::hashed_string& name, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	m_depth = resource.m_id;

	if (resource.m_uses.size() <= m_id)
		resource.m_uses.resize(m_id + 1ull);
	
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);
}

void nyan::Renderpass::add_depth_stencil_attachment(const entt::hashed_string& name, ImageAttachment attachment, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	resource.attachment = attachment;
	resource.m_type = RenderResource::Type::Image;
	m_depth = m_stencil = resource.m_id;

	if (resource.m_uses.size() <= m_id)
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);
}

void nyan::Renderpass::add_depth_stencil_attachment(const entt::hashed_string& name, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	m_depth = m_stencil = resource.m_id;

	if (resource.m_uses.size() <= m_id)
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);
}

void nyan::Renderpass::add_stencil_attachment(const entt::hashed_string& name, ImageAttachment attachment, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	resource.attachment = attachment;
	resource.m_type = RenderResource::Type::Image;

	if (resource.m_uses.size() <= m_id)
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);

	m_stencil = resource.m_id;
}

void nyan::Renderpass::add_stencil_attachment(const entt::hashed_string& name, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	if (resource.m_uses.size() <= m_id)
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::Attachment);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);
	m_stencil = resource.m_id;
}

//void nyan::Renderpass::add_read_dependency(const std::string& name, bool storageImage)
//{
//	auto& resource = r_graph.get_resource(name);
//	resource.m_readIn.insert(std::upper_bound(resource.m_readIn.begin(), resource.m_readIn.end(), m_id), m_id);
//	if(storageImage)
//		resource.storageImage = true;
//}

void nyan::Renderpass::add_write(const entt::hashed_string& name, ImageAttachment attachment, Renderpass::Write::Type writeType, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	auto& resource = r_graph.get_resource(name);
	resource.name = name;
	resource.attachment = attachment;

	if (resource.m_uses.size() <= m_id)
		resource.m_uses.resize(m_id + 1ull);
	resource.m_uses[m_id].set(RenderResource::UseType::ImageStore);
	if (clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);

	assert(std::find_if(m_writes.cbegin(), m_writes.cend(), [&resource](const auto& write) { return write.id == resource.m_id; }) == m_writes.cend());
	m_writes.push_back(Write{ resource.m_id , writeType, VK_NULL_HANDLE});
}

void nyan::Renderpass::add_swapchain_write(Math::vec4 clearColor, Renderpass::Write::Type writeType, bool clear)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	assert(m_type == Renderpass::Type::Generic);
	//Currently only support hybrid queue for swapchain Synchronization
	nyan::ImageAttachment swap;
	swap.clearColor = clearColor;
	swap.format = r_graph.get_device().get_swapchain_image_view()->get_format();
	auto& resource = r_graph.get_resource("swap");
	resource.name = "swap";

	if (resource.m_uses.size() <= m_id)
		resource.m_uses.resize(m_id + 1ull);

	resource.m_uses[m_id].set(RenderResource::UseType::ImageStore);
	if(clear)
		resource.m_uses[m_id].set(RenderResource::UseType::Clear);

	resource.attachment = swap;
	assert(std::find(m_attachments.begin(), m_attachments.end(), resource.m_id) == m_attachments.end());
	m_writes.push_back(Write{ resource.m_id , writeType, VK_NULL_HANDLE });
	r_graph.set_swapchain(resource.name);
	m_rendersSwap = true;
}

void nyan::Renderpass::copy(const entt::hashed_string& source, const entt::hashed_string& target)
{
	assert(r_graph.m_state == Rendergraph::State::Setup);
	auto& sourceResource = r_graph.get_resource(source);
	auto& targetResource = r_graph.get_resource(target);
	sourceResource.name = source;
	targetResource.name = target;
	targetResource.attachment = sourceResource.attachment;
	if (targetResource.m_uses.size() <= m_id) {
		targetResource.m_uses.resize(m_id + 1ull);
	}
	if (sourceResource.m_uses.size() <= m_id) {
		sourceResource.m_uses.resize(m_id + 1ull);
	}
	targetResource.m_uses[m_id].set(RenderResource::UseType::CopyTarget);
	sourceResource.m_uses[m_id].set(RenderResource::UseType::CopySource);
	m_copies.push_back(Copy{.src = sourceResource.m_id, .dst = targetResource.m_id});
	//sourceResource.m_copiedIn.insert(m_id);
	//targetResource.m_copiedIntoIn.insert(m_id);
}

void nyan::Renderpass::do_binds() {


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
		auto& read = r_graph.get_resource(readId);
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
	if (m_globalPostBarrierIndex != m_globalPreBarrierIndex ||
		m_bufferPostBarrierIndex != m_bufferPreBarrierIndex ||
		m_imagePostBarrierIndex != m_imagePreBarrierIndex)
		cmd.barrier2(0, 
			static_cast<uint32_t>(m_globalPostBarrierIndex - m_globalPreBarrierIndex), m_globalBarriers2.barriers.data() + m_globalPreBarrierIndex,
			static_cast<uint32_t>(m_bufferPostBarrierIndex - m_bufferPreBarrierIndex), m_bufferBarriers2.barriers.data() + m_bufferPreBarrierIndex,
			static_cast<uint32_t>(m_imagePostBarrierIndex - m_imagePreBarrierIndex), m_imageBarriers2.barriers.data() + m_imagePreBarrierIndex);
}

void nyan::Renderpass::apply_copy_barriers(vulkan::CommandBuffer& cmd)
{
	if (m_globalPreBarrierIndex != m_globalCopyBarrierIndex ||
		m_bufferPreBarrierIndex != m_bufferCopyBarrierIndex ||
		m_imagePreBarrierIndex != m_imageCopyBarrierIndex)
		cmd.barrier2(0,
			static_cast<uint32_t>(m_globalPreBarrierIndex - m_globalCopyBarrierIndex), m_globalBarriers2.barriers.data() + m_globalCopyBarrierIndex,
			static_cast<uint32_t>(m_bufferPreBarrierIndex - m_bufferCopyBarrierIndex), m_bufferBarriers2.barriers.data() + m_bufferCopyBarrierIndex,
			static_cast<uint32_t>(m_imagePreBarrierIndex - m_imageCopyBarrierIndex), m_imageBarriers2.barriers.data() + m_imageCopyBarrierIndex);
}

void nyan::Renderpass::apply_post_barriers(vulkan::CommandBuffer& cmd)
{
	if (m_globalBarriers2.barriers.size() != m_globalPostBarrierIndex ||
		m_bufferBarriers2.barriers.size() != m_bufferPostBarrierIndex ||
		m_imageBarriers2.barriers.size() != m_imagePostBarrierIndex)
		cmd.barrier2(0,
			static_cast<uint32_t>(m_globalBarriers2.barriers.size() - m_globalPostBarrierIndex), m_globalBarriers2.barriers.data() + m_globalPostBarrierIndex,
			static_cast<uint32_t>(m_bufferBarriers2.barriers.size() - m_bufferPostBarrierIndex), m_bufferBarriers2.barriers.data() + m_bufferPostBarrierIndex,
			static_cast<uint32_t>(m_imageBarriers2.barriers.size() - m_imagePostBarrierIndex), m_imageBarriers2.barriers.data() + m_imagePostBarrierIndex);
}

void nyan::Renderpass::add_pre_barrier(const VkImageMemoryBarrier2& barrier, RenderResourceId image)
{
	m_imagePostBarrierIndex++;
	m_imageBarriers2.barriers.insert(m_imageBarriers2.barriers.begin() + m_imagePreBarrierIndex, barrier);
	m_imageBarriers2.images.insert(m_imageBarriers2.images.begin() + m_imagePreBarrierIndex, image);
}
void nyan::Renderpass::add_copy_barrier(const VkImageMemoryBarrier2& barrier, RenderResourceId image)
{

	m_imagePreBarrierIndex++;
	m_imagePostBarrierIndex++;
	m_imageBarriers2.barriers.insert(m_imageBarriers2.barriers.begin() + m_imageCopyBarrierIndex, barrier);
	m_imageBarriers2.images.insert(m_imageBarriers2.images.begin() + m_imageCopyBarrierIndex, image);
}
void nyan::Renderpass::add_post_barrier(const VkImageMemoryBarrier2& barrier, RenderResourceId image)
{
	m_imageBarriers2.barriers.insert(m_imageBarriers2.barriers.begin() + m_imagePostBarrierIndex, barrier);
	m_imageBarriers2.images.insert(m_imageBarriers2.images.begin() + m_imagePostBarrierIndex, image);
}

void nyan::Renderpass::add_pre_barrier(const VkMemoryBarrier2& barrier)
{
	m_globalPostBarrierIndex++;
	m_globalBarriers2.barriers.insert(m_globalBarriers2.barriers.begin() + m_globalPreBarrierIndex, barrier);
}
void nyan::Renderpass::add_copy_barrier(const VkMemoryBarrier2& barrier)
{

	m_globalPreBarrierIndex++;
	m_globalPostBarrierIndex++;
	m_globalBarriers2.barriers.insert(m_globalBarriers2.barriers.begin() + m_globalCopyBarrierIndex, barrier);
}
void nyan::Renderpass::add_post_barrier(const VkMemoryBarrier2& barrier)
{
	m_globalBarriers2.barriers.insert(m_globalBarriers2.barriers.begin() + m_globalPostBarrierIndex, barrier);
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
uint32_t nyan::Renderpass::get_write_bind(const entt::hashed_string& name, Write::Type type)
{
	const auto& resource = r_graph.get_resource(name);
	auto res = std::find_if(m_writes.cbegin(), m_writes.cend(), [&resource, type](const auto& write) { return resource.m_id == write.id && type == write.type; });
	assert(res != m_writes.cend());
	if (res != m_writes.cend())
		return res->binding;
	else
		return InvalidResourceId;
}

uint32_t nyan::Renderpass::get_read_bind(const entt::hashed_string& name, Read::Type type)
{
	const auto& resource = r_graph.get_resource(name);
	auto res = std::find_if(m_reads.cbegin(), m_reads.cend(), [&resource, type](const auto& read) { return resource.m_id == read.id && type == read.type; });
	assert(res != m_reads.cend());
	if (res != m_reads.cend())
		return res->binding;
	else
		return InvalidResourceId;

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

void nyan::Renderpass::add_signal(uint32_t passId, VkPipelineStageFlags2 stage)
{
	m_signals.push_back({ passId, stage });
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
	auto it = std::find_if(m_writes.cbegin(), m_writes.cend(), [id](const auto& write) { return write.id == id && write.type == Renderpass::Write::Type::Compute; });
	return it != m_writes.cend();
}

bool nyan::Renderpass::is_attachment(RenderResourceId id) const
{
	return std::find(m_attachments.cbegin(), m_attachments.cend(), id) != m_attachments.cend() || m_depth ==id || m_stencil == id;
}

bool nyan::Renderpass::is_write(const RenderResource& resource) const
{
	return is_write(resource.m_id);
}

void nyan::Renderpass::build()
{
	for (auto& [config, id] : m_queuedPipelineBuilds) {
		config.renderingCreateInfo = m_renderingCreateInfo;
		*id = r_graph.get_device().get_pipeline_storage().add_pipeline(config);
	}
	m_queuedPipelineBuilds.clear();
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

Renderpass& nyan::Rendergraph::add_pass(const entt::hashed_string& name, Renderpass::Type type)
{
	assert(m_state == State::Setup);
	return m_renderpasses.emplace(name.value(), *this, type, m_renderpassCount++, name);
}

Renderpass& nyan::Rendergraph::get_pass(const entt::hashed_string& name)
{
	return m_renderpasses.get(name.value());
}

void nyan::Rendergraph::build()
{
	assert(m_state == State::Setup);
	m_state = State::Build;
	m_renderresources.for_each([&](RenderResource& resource) {

		bool first = true;
			
		for (size_t i = 0; i < resource.m_uses.size(); i++) {
			const auto& srcUse = resource.m_uses[i];
			if (srcUse.none())
				continue;
			if (first) {
				first = false;
				//CreatePreBarrier
				size_t lastUse = i;
				for (size_t j = i; j < resource.m_uses.size(); j++) {
					if (!resource.m_uses[j].none())
						lastUse = j;
				}
				if (srcUse.test(RenderResource::UseType::Clear) || resource.m_id == m_swapchainResource)
					set_up_first_transition(i, resource);
				else
					set_up_transition(lastUse, i, resource);
			}
			set_up_copy(i, resource);
			size_t j = i + 1;
			for (; j < resource.m_uses.size(); j++) {
				const auto& dstUse = resource.m_uses[j];
				if (!dstUse.none())
					break;
			}
			if (j < resource.m_uses.size()) {
				set_up_transition(i, j, resource);
			}
			i = j - 1;
		}

		if (resource.m_id == m_swapchainResource) {

			for (int i = resource.m_uses.size() - 1; i > 0; i--) {
				const auto& srcUse = resource.m_uses[i];
				if (!srcUse.none()) {
					swapchain_present_transition(static_cast<RenderpassId>(i));
					break;
				}
			}
		}
		
	});
	m_renderpasses.for_each([&](Renderpass& pass) {
		//pass.m_attachmentPool = std::make_unique<vulkan::DescriptorPool>(r_device, );
		//VkMemoryBarrier2 barrier{
		//	.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
		//	.pNext {nullptr},
		//	.srcStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT },
		//	.srcAccessMask {0},
		//	.dstStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT },
		//	.dstAccessMask {0 },
		//};
		//pass.add_pre_barrier(barrier);
		//pass.add_post_barrier(barrier);
		if (pass.get_type() == Renderpass::Type::Generic) {
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
					if (resource.m_uses[pass.m_id].test(RenderResource::UseType::Clear)) {
						renderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						for (size_t i = 0u; i < 4u; i++)
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
				if (resource.m_uses[pass.m_id].test(RenderResource::UseType::Clear)) {
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
				if (resource.m_uses[pass.m_id].test(RenderResource::UseType::Clear)) {
					renderingAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					renderingAttachment.clearValue.depthStencil.stencil = std::bit_cast<uint32_t>(attachment.clearColor[1]);
				}
				pass.m_renderingCreateInfo.stencilAttachmentFormat = attachment.format;
				pass.m_renderInfo.pStencilAttachment = &pass.m_stencilAttachment;
				pass.m_stencilAttachment = renderingAttachment;
			}
		}
		pass.build();
		//m_lastPass = pass.m_id;
		});
	//if (m_lastPass != invalidRenderpassId) {
	//	auto& lastPass = m_renderpasses.get_direct(static_cast<size_t>(m_lastPass));
	//	lastPass.add_post_barrier(
	//	VkMemoryBarrier2{ 
	//			.sType{VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
	//		.srcStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT},
	//		.dstStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT} 
	//		});
	//}
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
				resource.handle = r_device.get_swapchain_image();
			}
			else {
				VkImageUsageFlags usage = 0;
				VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				bool first = true;
				Utility::bitset<static_cast<size_t>(RenderResource::UseType::Size), RenderResource::UseType> totalUses;
				
				for (const auto& use : resource.m_uses) {
					totalUses |= use;
					//TODO add transfer dst for clear depth stencil
					//if (use.test(RenderResource::UseType::Clear) 
					//	&& vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)
					//	&& ) {
					//	usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
					//}
					if (use.any_of(RenderResource::UseType::BlitTarget, RenderResource::UseType::CopyTarget)) {
						usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
						if (first) {
							first = false;
							initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
						}
					}
					if (use.test(RenderResource::UseType::Sample)) {
						usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
						if (first) {
							first = false;
							initialLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
							Utility::log().format("First usage of image %d is sampling, is this intended?", resource.m_id);
						}
					}
					if (use.any_of(RenderResource::UseType::ImageLoad, RenderResource::UseType::ImageStore)) {
						usage |= VK_IMAGE_USAGE_STORAGE_BIT;
						if (first) {
							first = false;
							initialLayout = VK_IMAGE_LAYOUT_GENERAL;
						}
					}
					if (use.test(RenderResource::UseType::Attachment) && vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
						usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
						if (first) {
							first = false;
							initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						}
					}
					if (use.test(RenderResource::UseType::Attachment) && !vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
						usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
						if (first) {
							first = false;
							initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						}
					}
					if (use.any_of(RenderResource::UseType::BlitSource, RenderResource::UseType::CopySource)) {
						usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
						if (first) {
							first = false;
							initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
							Utility::log().format("First usage of image %d is as copy source, is this intended?", resource.m_id);
						}
					}
				}
				if(!totalUses.none())
					resource.handle = r_device.request_render_target(width, height, attachment.format, resource.m_id, usage, initialLayout);
			}
			resource.handle->set_debug_label(resource.name.data());
			resource.handle->get_view()->set_debug_label((std::string(resource.name.data()) + "_view").c_str());
		}
	});
	m_renderpasses.for_each([this](Renderpass& pass) {

		for (auto& [readId, readType, readView, readBinding] : pass.m_reads) {
			auto& resource = m_renderresources.get_direct(readId);
			if (resource.m_type == RenderResource::Type::Image) {
				assert(resource.handle);
				if (readType == Renderpass::Read::Type::ImageColor) {
					readView = resource.handle->get_view()->get_image_view();

				}
				else if (readType == Renderpass::Read::Type::ImageDepth) {
					auto* tmp = resource.handle->get_depth_view();
					assert(tmp);
					if (!tmp)
						continue;
					readView = tmp->get_image_view();
				}
				else if (readType == Renderpass::Read::Type::ImageStencil) {
					auto* tmp = resource.handle->get_stencil_view();
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
				writeView = resource.handle->get_view()->get_image_view();
			}
		}
		uint32_t attachmentId = 0;
		for (auto attachmentRessourceId : pass.m_attachments) {
			auto& resource = m_renderresources.get_direct(attachmentRessourceId);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			if (resource.m_type == RenderResource::Type::Image) {
				if (resource.m_uses[pass.m_id].test(RenderResource::UseType::Clear)) {
					for (size_t i = 0u; i < 4u; i++)
						pass.m_colorAttachments[attachmentId].clearValue.color.float32[i] = attachment.clearColor[i];
				}
				assert(resource.handle);
				pass.m_colorAttachments[attachmentId++].imageView = resource.handle->get_view()->get_image_view();
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
				pass.m_renderInfo.layerCount = Math::max(pass.m_renderInfo.layerCount, resource.handle->get_info().arrayLayers);
			}
		}
		if (pass.m_depth != InvalidResourceId) {
			auto& resource = m_renderresources.get_direct(pass.m_depth);
			assert(resource.m_type == RenderResource::Type::Image);
			assert(resource.handle);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			if (pass.m_stencil == InvalidResourceId) {
				pass.m_depthAttachment.imageView = resource.handle->get_depth_view()->get_image_view();
			}
			else {
				pass.m_depthAttachment.imageView = resource.handle->get_view()->get_image_view();
			}
			if (resource.m_uses[pass.m_id].test(RenderResource::UseType::Clear)) {
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
				pass.m_stencilAttachment.imageView = resource.handle->get_stencil_view()->get_image_view();
			}
			else {
				pass.m_stencilAttachment.imageView = resource.handle->get_view()->get_image_view();
			}
			if (resource.m_uses[pass.m_id].test(RenderResource::UseType::Clear)) {
				pass.m_stencilAttachment.clearValue.depthStencil.stencil = static_cast<uint32_t>(attachment.clearColor[1]);
			}
		}


		for (size_t i = 0; i < pass.m_imageBarriers2.barriers.size(); i++) {
			auto& barrier = pass.m_imageBarriers2.barriers[i];
			auto& image = pass.m_imageBarriers2.images[i];
			assert(image != InvalidResourceId);
			auto& resource = m_renderresources.get_direct(image);
			assert(resource.m_type == RenderResource::Type::Image);
			assert(resource.handle);
			barrier.image = resource.handle->get_handle();
		}
		pass.do_binds();
		});

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
			auto& resource = m_renderresources.get_direct(id);
			auto& attachment = std::get<ImageAttachment>(resource.attachment);
			if (resource.m_type == RenderResource::Type::Image && !vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format)) {
				if (resource.m_uses[pass.m_id].test(RenderResource::UseType::Clear)) {
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
		r_device.submit(cmdHandle, static_cast<uint32_t>(signals.size()), signals.data());
		for (size_t i{ 0 }; i < pass.m_signals.size(); i++) {
			auto& waitPass = m_renderpasses.get_direct(pass.m_signals[i].passId);
			waitPass.add_wait(signals[i], pass.m_signals[i].stage);
		}
	});
	r_device.wait_idle(); //Synchronization issues, guessing view matrix gets updated while frame in flight
}

RenderResource& nyan::Rendergraph::add_ressource(const entt::hashed_string& name, Attachment attachment)
{
	auto& r =m_renderresources.emplace(name.value(), m_resourceCount++);
	r.attachment = attachment;
	return r;
}


RenderResource& nyan::Rendergraph::get_resource(const entt::hashed_string& name)
{
	if (!m_renderresources.contains(name.value()))
		return m_renderresources.emplace(name.value(), m_resourceCount++);
	return m_renderresources.get(name.value());
}

bool nyan::Rendergraph::resource_exists(const entt::hashed_string& name)
{
	return m_renderresources.contains(name.value());
}

RenderResource& nyan::Rendergraph::get_resource(RenderResourceId id)
{
	return m_renderresources.get_direct(id);
}

const RenderResource& nyan::Rendergraph::get_resource(const entt::hashed_string& name) const
{
	assert(m_renderresources.contains(name.value()));
	return m_renderresources.get(name.value());
}

const RenderResource& nyan::Rendergraph::get_resource(RenderResourceId id)const 
{
	return m_renderresources.get_direct(id);
}

void nyan::Rendergraph::set_swapchain(const entt::hashed_string& name)
{
	auto& resource = m_renderresources.get(name);
	m_swapchainResource = resource.m_id;
}

vulkan::LogicalDevice& nyan::Rendergraph::get_device() const
{
	return r_device;
}

void nyan::Rendergraph::swapchain_present_transition(RenderpassId src_const)
{
	if (m_swapchainResource == InvalidResourceId) {
		Utility::log().format("Renderpass: %d tries to write to an invalid swapchain, this shouldn't happen", src_const);
		assert(false);
		return;
	}
	auto& src = m_renderpasses.get_direct(src_const);
	assert(src.get_type() == Renderpass::Type::Generic);

	auto& resource = m_renderresources.get_direct(m_swapchainResource);
	Utility::log().format("Swap Present Ressource ({}) Renderpass ({})", resource.name.data(), src.m_name.data());
	assert(resource.m_uses.size() > src_const);
	auto usage = resource.m_uses[src_const];

	if (resource.m_type != RenderResource::Type::Image) {
		Utility::log().format("Renderpass: %d thinks the swapchain image is a buffer", src_const);
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
	if (usage.test(RenderResource::UseType::CopyTarget)) {
		assert(!usage.test(RenderResource::UseType::BlitTarget));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::BlitTarget)) {
		assert(!usage.test(RenderResource::UseType::CopyTarget));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::Sample)) {
		assert(!usage.test(RenderResource::UseType::ImageStore));
		assert(!usage.test(RenderResource::UseType::Attachment));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.srcAccessMask = 0;//VK_ACCESS_2_SHADER_SAMPLED_READ_BIT; No purpose in making reads available
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::Attachment)) {
		assert(!usage.test(RenderResource::UseType::ImageLoad));
		assert(!usage.test(RenderResource::UseType::ImageStore));
		assert(!usage.test(RenderResource::UseType::Sample));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::ImageLoad)) {
		//if(src.is_compute_write(resource))
		assert(false);
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.srcAccessMask = 0;//VK_ACCESS_2_SHADER_STORAGE_READ_BIT; No purpose in making reads available
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::UseType::ImageStore)) {
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
	if (usage.test(RenderResource::UseType::CopySource)) {
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		imageBarrier.srcAccessMask = 0; // VK_ACCESS_2_TRANSFER_READ_BIT; No purpose in making reads available
		//if(imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::BlitSource)) {
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		imageBarrier.srcAccessMask = 0; // VK_ACCESS_2_TRANSFER_READ_BIT; No purpose in making reads available
		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	}
	

	assert(imageBarrier.oldLayout != VK_IMAGE_LAYOUT_UNDEFINED);

	src.add_post_barrier(imageBarrier, m_swapchainResource);

}

bool debugBarriers = false;

void nyan::Rendergraph::set_up_transition(RenderpassId from, RenderpassId to, const RenderResource& resource)
{
	auto& src = m_renderpasses.get_direct(from);
	auto& dst = m_renderpasses.get_direct(to);

	if (resource.m_type != RenderResource::Type::Image) {
		Utility::log().format("Renderpass ({}) -> ({}) wants to transition something which isn't an image ({}), which is not supported yet", from, to, resource.m_id);
		assert(false);
		return;
	}
	auto srcUsage = resource.m_uses[from];
	auto dstUsage = resource.m_uses[to];

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
	if (srcUsage.test(RenderResource::UseType::Sample)) {
		assert(!srcUsage.test(RenderResource::UseType::ImageStore));
		assert(!srcUsage.test(RenderResource::UseType::Attachment));
		if (src.get_type() == Renderpass::Type::Generic)
			setBarrierSource(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, 0 /*VK_ACCESS_2_SHADER_SAMPLED_READ_BIT*/, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else
			setBarrierSource(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, 0 /*VK_ACCESS_2_SHADER_SAMPLED_READ_BIT*/, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	}
	if (srcUsage.test(RenderResource::UseType::Attachment)) {
		assert(!srcUsage.test(RenderResource::UseType::ImageLoad));
		assert(!srcUsage.test(RenderResource::UseType::ImageStore));
		assert(!srcUsage.test(RenderResource::UseType::Sample));
		if(vulkan::ImageInfo::is_depth_or_stencil_format(attachment.format))
			setBarrierSource(VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
		else
			setBarrierSource(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
	}
	if (srcUsage.test(RenderResource::UseType::ImageLoad)) {
		//if(src.is_compute_write(resource))
		assert(false);
		setBarrierSource(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, 0 /*VK_ACCESS_2_SHADER_STORAGE_READ_BIT*/, VK_IMAGE_LAYOUT_GENERAL);

	}
	if (srcUsage.test(RenderResource::UseType::ImageStore)) {
		assert(src.is_write(resource));
		auto stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		if (src.is_compute_write(resource)) {
			stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		}
		setBarrierSource(stage, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);
	}

	if (srcUsage.test(RenderResource::UseType::CopySource)) {
		setBarrierSource(VK_PIPELINE_STAGE_2_COPY_BIT, 0/*VK_ACCESS_2_TRANSFER_READ_BIT*/, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}

	if (srcUsage.test(RenderResource::UseType::BlitSource)) {
		setBarrierSource(VK_PIPELINE_STAGE_2_BLIT_BIT, 0/*VK_ACCESS_2_TRANSFER_READ_BIT*/, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}

	if (srcUsage.test(RenderResource::UseType::CopyTarget)) {
		assert(!srcUsage.test(RenderResource::UseType::BlitTarget));
		setBarrierSource(VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	if (srcUsage.test(RenderResource::UseType::BlitTarget)) {
		assert(!srcUsage.test(RenderResource::UseType::CopyTarget));
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

	if (dstUsage.test(RenderResource::UseType::CopyTarget)) {
		setBarrierDestination(VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}
	if (dstUsage.test(RenderResource::UseType::BlitTarget)) {
		setBarrierDestination(VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	if (dstUsage.test(RenderResource::UseType::Sample)) {
		assert(!dstUsage.test(RenderResource::UseType::ImageStore));
		assert(!dstUsage.test(RenderResource::UseType::Attachment));
		if(dst.get_type() == Renderpass::Type::Generic)
			setBarrierDestination(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_SHADER_READ_BIT_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else
			setBarrierDestination(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_SHADER_READ_BIT_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	if (dstUsage.test(RenderResource::UseType::Attachment)) {
		assert(!dstUsage.test(RenderResource::UseType::ImageLoad));
		assert(!dstUsage.test(RenderResource::UseType::ImageStore));
		assert(!dstUsage.test(RenderResource::UseType::Sample));
		setBarrierDestination(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
	}
	if (dstUsage.test(RenderResource::UseType::ImageLoad)) {
		//if(src.is_compute_write(resource))
		assert(false);
		setBarrierDestination(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);
	}
	if (dstUsage.test(RenderResource::UseType::ImageStore)) {
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
		Utility::log().format("Ressource ({}) Renderpass ({}) -> ({})", resource.name.data(), src.m_name.data(), dst.m_name.data());
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	}
	if (imageBarrier.oldLayout != imageBarrier.newLayout && imageBarrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
		Utility::log().format("Renderpass {} -> {}, Possibly no Barrier needed here", src.m_name.data(), dst.m_name.data());
		return;
	}

	if (imageBarrier.oldLayout == imageBarrier.newLayout &&
		imageBarrier.srcQueueFamilyIndex == imageBarrier.dstQueueFamilyIndex &&
		srcUsage.only(RenderResource::UseType::Attachment) && dstUsage.only(RenderResource::UseType::Attachment)) {
		Utility::log().format("Renderpass {} -> {}, Barrier for attachment only usage without layout transition or Queue Ownership transfer", src.m_name.data(), dst.m_name.data());
	}
	else {
		src.add_post_barrier(imageBarrier, resource.m_id);
	}
	//src.m_imagePostBarrierIndex++;
	//src.m_imageCopyBarrierIndex++;
	//src.m_imagePreBarrierIndex++;


}

void nyan::Rendergraph::set_up_first_transition(RenderpassId dst_const, const RenderResource& resource) {
	auto& dst = m_renderpasses.get_direct(dst_const);
	assert(dst.get_type() == Renderpass::Type::Generic);

	assert(resource.m_uses.size() > dst_const);
	auto usage = resource.m_uses[dst_const];

	if (resource.m_type != RenderResource::Type::Image) {
		Utility::log().format("Renderpass: {} tries not supported resource transition", dst_const);
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

	if (usage.test(RenderResource::UseType::Sample)) {
		assert(!usage.test(RenderResource::UseType::Attachment));
		assert(!usage.test(RenderResource::UseType::ImageLoad)); //Technically possible, but unreasonable?
		assert(!usage.test(RenderResource::UseType::ImageStore));
		if (!usage.any_of(RenderResource::UseType::BlitTarget, RenderResource::UseType::CopyTarget)) {
			Utility::log().format("Renderpass: {} tries to sample an undefined target, this shouldn't happen", dst.m_name.data());
			assert(false);
		}
		//TODO: Compute Shader
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::ImageLoad)) {
		assert(!usage.test(RenderResource::UseType::Attachment));
		assert(!usage.test(RenderResource::UseType::Sample));  //Technically possible, but unreasonable?
		if (!usage.any_of(RenderResource::UseType::BlitTarget, RenderResource::UseType::CopyTarget)) {
			Utility::log().format("Renderpass: {} tries to load an undefined target, this shouldn't happen", dst.m_name.data());
			assert(false);
		}
		assert(false);
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::UseType::Attachment)) {
		assert(!usage.test(RenderResource::UseType::ImageLoad));
		assert(!usage.test(RenderResource::UseType::ImageStore));
		assert(!usage.test(RenderResource::UseType::Sample));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::ImageStore)) {
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
		Utility::log().format("Ressource ({}) Renderpass ({})", resource.name.data(), dst.m_name.data());
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	}

	dst.add_pre_barrier(imageBarrier, resource.m_id);

}

void nyan::Rendergraph::set_up_copy(RenderpassId dst_const, const RenderResource& resource) {
	auto& dst = m_renderpasses.get_direct(dst_const);

	assert(resource.m_uses.size() > dst_const);
	auto usage = resource.m_uses[dst_const];

	if (resource.m_type != RenderResource::Type::Image) {
		Utility::log().format("Renderpass: {} tries not supported resource copy", dst_const);
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

	if (usage.test(RenderResource::UseType::Sample)) {
		assert(!usage.test(RenderResource::UseType::Attachment));
		assert(!usage.test(RenderResource::UseType::ImageLoad)); //Technically possible, but unreasonable?
		assert(!usage.test(RenderResource::UseType::ImageStore));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::ImageLoad)) {
		assert(!usage.test(RenderResource::UseType::Attachment));
		assert(!usage.test(RenderResource::UseType::Sample));  //Technically possible, but unreasonable?
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::UseType::Attachment)) {
		assert(!usage.test(RenderResource::UseType::ImageLoad));
		assert(!usage.test(RenderResource::UseType::ImageStore));
		assert(!usage.test(RenderResource::UseType::Sample));
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	}
	if (usage.test(RenderResource::UseType::ImageStore)) {
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

	if (usage.test(RenderResource::UseType::CopyTarget)) {
		assert(!usage.test(RenderResource::UseType::CopySource));
		assert(!usage.test(RenderResource::UseType::BlitSource));
		assert(!usage.test(RenderResource::UseType::BlitTarget));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		//else
		//	imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::UseType::BlitTarget)) {
		assert(!usage.test(RenderResource::UseType::CopyTarget));
		assert(!usage.test(RenderResource::UseType::CopySource));
		assert(!usage.test(RenderResource::UseType::BlitSource));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		//else
		//	imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::UseType::CopySource)) {
		assert(!usage.test(RenderResource::UseType::CopyTarget));
		assert(!usage.test(RenderResource::UseType::BlitSource));
		assert(!usage.test(RenderResource::UseType::BlitTarget));
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		//if (imageBarrier.oldLayout != VK_IMAGE_LAYOUT_GENERAL)
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		//else
		//	imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	if (usage.test(RenderResource::UseType::BlitSource)) {
		assert(!usage.test(RenderResource::UseType::CopyTarget));
		assert(!usage.test(RenderResource::UseType::CopySource));
		assert(!usage.test(RenderResource::UseType::BlitTarget));
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
		Utility::log().format("Copy Ressource ({}) Renderpass ({})", resource.name.data(), dst.m_name.data());
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	}

	dst.add_copy_barrier(imageBarrier, resource.m_id);

}
