#include "Renderer/DataManager.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "Renderer/ShaderInterface.h"
#include "Renderer/MeshManager.h"

template<typename T>
bool nyan::DataManager<T>::upload(vulkan::CommandBuffer& cmd)
{
	if (!m_slot->dirty)
		return false;
	if (m_slot->slotCapacity < m_slot->data.size()) {
		m_slot->stagingBuffer = create_buffer(m_slot->data.capacity(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		m_slot->deviceBuffer = create_buffer(m_slot->data.capacity(), m_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		rebind_buffer(m_slot->deviceBuffer);
		m_slot->slotCapacity = static_cast<uint32_t>(m_slot->data.capacity());
	}
	auto size = sizeof(T) * m_slot->data.size();
	auto* map = m_slot->stagingBuffer->map_data();
	std::memcpy(map, m_slot->data.data(), size);
	m_slot->stagingBuffer->flush(0, static_cast<uint32_t>(size));
	m_slot->stagingBuffer->invalidate(0, static_cast<uint32_t>(size));
	cmd.copy_buffer(m_slot->deviceBuffer, m_slot->stagingBuffer, 0, 0, size);
	m_slot->dirty = false;
	return true;
}

template<typename T>
uint32_t nyan::DataManager<T>::bind_buffer(vulkan::BufferHandle& buffer)
{
	return r_device.get_bindless_set().set_storage_buffer(VkDescriptorBufferInfo{ .buffer = buffer->get_handle(), .offset = 0, .range = buffer->get_size() });
}

template<typename T>
inline void nyan::DataManager<T>::rebind_buffer(vulkan::BufferHandle& buffer)
{
	r_device.get_bindless_set().set_storage_buffer(m_slot->binding, VkDescriptorBufferInfo{ .buffer = buffer->get_handle(), .offset = 0, .range = buffer->get_size() });
} 
template<typename T>
vulkan::BufferHandle nyan::DataManager<T>::create_buffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	vulkan::BufferInfo info{
		.size = size * sizeof(T),
		.usage = usage,
		.offset = 0,
		.memoryUsage = memoryUsage
	};
	return r_device.create_buffer(info, {});
}

template class nyan::DataManager<nyan::shaders::Mesh>;
template class nyan::DataManager<nyan::shaders::Material>;
template class nyan::DataManager<nyan::shaders::DDGIVolume>;
template class nyan::DataManager<nyan::shaders::Scene>;
template class nyan::DataManager<nyan::InstanceData>;