#include "Allocator.h"
#include "LogicalDevice.h"
#include "Image.h"

vulkan::Allocator::Allocator(VmaAllocator handle) :
	m_VmaHandle(handle)
{
}

vulkan::Allocator::Allocator(Allocator&& other) noexcept :
	m_VmaHandle(other.m_VmaHandle)
{
	other.m_VmaHandle = VK_NULL_HANDLE;
}

vulkan::Allocator::~Allocator() noexcept
{
	if(m_VmaHandle != VK_NULL_HANDLE)
		vmaDestroyAllocator(m_VmaHandle);
}

void vulkan::Allocator::map_memory(VmaAllocation allocation, void** data)
{
	vmaMapMemory(m_VmaHandle, allocation, data);
}

void vulkan::Allocator::unmap_memory(VmaAllocation allocation)
{
	vmaUnmapMemory(m_VmaHandle, allocation);
}

void vulkan::Allocator::destroy_buffer(VkBuffer buffer, VmaAllocation allocation)
{
	vmaDestroyBuffer(m_VmaHandle, buffer, allocation);
}

void vulkan::Allocator::flush(VmaAllocation allocation, uint32_t offset, uint32_t size)
{

	vmaFlushAllocation(m_VmaHandle, allocation, offset, size);
}
void vulkan::Allocator::invalidate(VmaAllocation allocation, uint32_t offset, uint32_t size)
{
	vmaInvalidateAllocation(m_VmaHandle, allocation, offset, size);
}
vulkan::AttachmentAllocator::AttachmentAllocator(LogicalDevice& parent) :
	r_device(parent),
	m_attachmentIds(new std::unordered_map<Utility::HashValue, ImageHandle>{})
{
}
void vulkan::AttachmentAllocator::clear() noexcept {
	m_attachmentIds->clear();
}
vulkan::Image* vulkan::AttachmentAllocator::request_attachment(uint32_t width, uint32_t height, VkFormat format, uint32_t index, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageLayout initialLayout, uint32_t arrayLayers)
{
	Utility::Hasher hasher;
	hasher(width);
	hasher(height);
	hasher(format);
	hasher(index);
	hasher(usage);
	hasher(arrayLayers);
	auto hash = hasher(samples);
	if (auto res = m_attachmentIds->find(hash); res != m_attachmentIds->end())
		return res->second;
	ImageInfo info = ImageInfo::render_target(width, height, format, arrayLayers);
	info.usage |= usage;
	info.layout = initialLayout;
	return m_attachmentIds->emplace(hash, r_device.create_image(info)).first->second;
}

vulkan::Allocation::Allocation(LogicalDevice& device, VmaAllocation handle)
	:r_device(device),
	m_VmaHandle(handle)
{
}

vulkan::Allocation::Allocation(Allocation&& other) noexcept :
	r_device(other.r_device),
	m_VmaHandle(other.m_VmaHandle)
{
	other.m_VmaHandle = VK_NULL_HANDLE;
}

vulkan::Allocation& vulkan::Allocation::operator=(Allocation&& other)
{
	assert(&r_device == &other.r_device);
	if (this != &other) {
		m_VmaHandle = other.m_VmaHandle;
		other.m_VmaHandle = VK_NULL_HANDLE;
	}
	return *this;
}

VkDeviceSize vulkan::Allocation::get_size() const noexcept {
	VmaAllocationInfo info;
	vmaGetAllocationInfo(r_device.get_vma_allocator()->get_handle(), m_VmaHandle, &info);
	return info.size;
}
VkDeviceSize vulkan::Allocation::get_offset() const noexcept {
	VmaAllocationInfo info;
	vmaGetAllocationInfo(r_device.get_vma_allocator()->get_handle(), m_VmaHandle, &info);
	return info.offset;
}
VkDeviceMemory vulkan::Allocation::get_memory() const noexcept {
	VmaAllocationInfo info;
	vmaGetAllocationInfo(r_device.get_vma_allocator()->get_handle(), m_VmaHandle, &info);
	return info.deviceMemory;
}
vulkan::Allocation::~Allocation()
{
	if (m_VmaHandle != VK_NULL_HANDLE)
		r_device.queue_allocation_deletion(m_VmaHandle);
}
