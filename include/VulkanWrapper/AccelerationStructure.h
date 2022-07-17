#pragma once
#ifndef VKACCELERATIONSTRUCTURE_H
#define VKACCELERATIONSTRUCTURE_H
#include <Util>
#include <Math/LinAlg.h>
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
#include "Buffer.h"
#include "Shader.h"

namespace vulkan {

	class AccelerationStructure : public Utility::UIDC {
	public:
		AccelerationStructure(LogicalDevice& device, VkAccelerationStructureKHR handle, BufferHandle buffer, VkAccelerationStructureCreateInfoKHR info);
		AccelerationStructure(AccelerationStructure&) = delete;
		AccelerationStructure(AccelerationStructure&& other);
		AccelerationStructure& operator=(AccelerationStructure&) = delete;
		AccelerationStructure& operator=(AccelerationStructure&& other);
		~AccelerationStructure();
		VkAccelerationStructureKHR get_handle() const noexcept;
		operator VkAccelerationStructureKHR() const noexcept;
		bool is_compactable() const noexcept;
		VkAccelerationStructureInstanceKHR create_instance() const noexcept;
		uint64_t get_reference() const noexcept;
		void set_debug_label(const char* name) noexcept;
	private:
		LogicalDevice& r_device;
		VkAccelerationStructureKHR m_handle = VK_NULL_HANDLE;
		BufferHandle m_buffer;
		VkAccelerationStructureCreateInfoKHR m_info;
		uint64_t m_reference;
		bool m_isCompactable = false;
	};
	class AccelerationStructureBuilder {
	private:
		struct BLASBuildEntry {
			VkAccelerationStructureGeometryKHR geometry;
			VkAccelerationStructureBuildRangeInfoKHR buildRange;
			VkBuildAccelerationStructureFlagsKHR flags;
			VkAccelerationStructureBuildGeometryInfoKHR buildInfo;
			VkAccelerationStructureBuildSizesInfoKHR sizeInfo;
		};
	public:
		struct BLASInfo {
			VkBuffer vertexBuffer = VK_NULL_HANDLE;
			uint32_t vertexCount = 0;
			VkDeviceSize vertexOffset = 0;
			VkFormat vertexFormat = VK_FORMAT_UNDEFINED;
			VkDeviceSize vertexStride = 0;
			VkBuffer indexBuffer = VK_NULL_HANDLE;
			uint32_t indexCount = 0;
			VkDeviceSize indexOffset = 0;
			VkBuffer transformBuffer = VK_NULL_HANDLE;
			uint32_t transformOffset = 0;
			VkIndexType indexType = VK_INDEX_TYPE_UINT32;
			VkBuildAccelerationStructureFlagsKHR flags = 0;
		};
		AccelerationStructureBuilder(LogicalDevice& device);
		std::optional<size_t> queue_item(const BLASInfo& info);
		std::vector<AccelerationStructureHandle> build_pending();
		AccelerationStructureHandle build_tlas(const std::vector< VkAccelerationStructureInstanceKHR>& instances,
			VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
		AccelerationStructureHandle build_tlas(uint32_t size, VkDeviceAddress address,
			VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
	private:
		LogicalDevice& r_device;
		std::vector<BLASBuildEntry> m_pendingBuilds;
		std::optional<BufferHandle> m_scratch;
		Utility::LinkedBucketList<AccelerationStructure> m_acclerationStructurePool;
	};

}

#endif !VKACCELERATIONSTRUCTURE_H