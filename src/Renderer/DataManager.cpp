#include "Renderer/DataManager.h"
#include "VulkanWrapper/Buffer.h"
#include "Renderer/ShaderInterface.h"
#include "Renderer/MeshManager.h"

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
vulkan::BufferHandle nyan::DataManager<T>::create_buffer(size_t size)
{
	vulkan::BufferInfo info{
		.size = size * sizeof(T),
		.usage = m_usage,
		.offset = 0,
		.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU
	};
	return r_device.create_buffer(info, {});
}

template class nyan::DataManager<nyan::shaders::Mesh>;
template class nyan::DataManager<nyan::shaders::Material>;
template class nyan::DataManager<nyan::shaders::DDGIVolume>;
template class nyan::DataManager<nyan::InstanceData>;