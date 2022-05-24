#pragma once
#ifndef VKACCELERATIONSTRUCTURE_H
#define VKACCELERATIONSTRUCTURE_H
#include <Util>
#include <Math/LinAlg.h>
#include "VulkanIncludes.h"
#include "Buffer.h"
#include "Shader.h"

namespace vulkan {
	class Buffer;
	class LogicalDevice;

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
	private:
		LogicalDevice& r_device;
		VkAccelerationStructureKHR m_handle;
		BufferHandle m_buffer;
		VkAccelerationStructureCreateInfoKHR m_info;
		bool m_isCompactable = false;
	};
	using AccelerationStructureHandle = Utility::ObjectHandle<AccelerationStructure, Utility::LinkedBucketList<AccelerationStructure>>;
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
			const Buffer* vertexBuffer = nullptr;
			uint32_t vertexCount = 0;
			VkDeviceSize vertexOffset = 0;
			const Buffer* indexBuffer = nullptr;
			uint32_t indexCount = 0;
			VkDeviceSize indexOffset = 0;
			const Buffer* transformBuffer = nullptr;
			uint32_t transformOffset = 0;
			VkIndexType indexType = VK_INDEX_TYPE_UINT32;
			VkBuildAccelerationStructureFlagsKHR flags = 0;
		};
		AccelerationStructureBuilder(LogicalDevice& device);
		void queue_item(const BLASInfo& info, VkFormat positionFormat, VkDeviceSize vertexSize, VkDeviceSize positionOffset = 0);
		std::vector<AccelerationStructureHandle> build_pending();
		AccelerationStructureHandle build_tlas(const std::vector< VkAccelerationStructureInstanceKHR>& instances,
			VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
		AccelerationStructureHandle build_tlas(const std::vector<uint32_t>& sizes, const std::vector<VkDeviceAddress>& addresses,
			VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
	private:
		LogicalDevice& r_device;
		std::vector<BLASBuildEntry> m_pendingBuilds;
		std::optional<BufferHandle> m_scratch;
		Utility::LinkedBucketList<AccelerationStructure> m_acclerationStructurePool;
	};

}

#endif !VKACCELERATIONSTRUCTURE_H