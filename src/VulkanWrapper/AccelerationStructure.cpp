#include "AccelerationStructure.h"
#include "Instance.h"
#include "LogicalDevice.h"
#include "CommandBuffer.h"


vulkan::AccelerationStructure::AccelerationStructure(LogicalDevice& device, VkAccelerationStructureKHR handle, BufferHandle buffer, VkAccelerationStructureCreateInfoKHR info) :
	VulkanObject(device, handle),
	m_buffer(buffer),
	m_info(info)
{

	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.pNext = nullptr,
		.accelerationStructure = m_handle
	};
	m_reference = vkGetAccelerationStructureDeviceAddressKHR(r_device, &addressInfo);
}

vulkan::AccelerationStructure::AccelerationStructure(AccelerationStructure&& other) :
	VulkanObject(other.r_device, other.m_handle),
	m_buffer(std::move(other.m_buffer)),
	m_info(std::move(other.m_info)),
	m_reference(other.m_reference)
{
	if (this != &other) {
		other.m_handle = VK_NULL_HANDLE;
	}
}

vulkan::AccelerationStructure& vulkan::AccelerationStructure::operator=(AccelerationStructure&& other)
{
	if (this != &other && &r_device == &other.r_device) {
		std::swap(m_handle, other.m_handle);
		std::swap(m_buffer, other.m_buffer);
		std::swap(m_reference, other.m_reference);
		std::swap(m_info, other.m_info);
	}
	return *this;
}

vulkan::AccelerationStructure::~AccelerationStructure()
{
	if (m_handle) {
		r_device.queue_acceleration_structure_deletion(m_handle);
		m_handle = VK_NULL_HANDLE;
		m_reference = 0;
	}
}

bool vulkan::AccelerationStructure::is_compactable() const noexcept
{
	return m_info.createFlags;
}

VkAccelerationStructureInstanceKHR vulkan::AccelerationStructure::create_instance(uint32_t sbtOffset) const noexcept
{
	VkAccelerationStructureInstanceKHR instance {
		.transform {
			.matrix{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0
			}
		},
		.instanceCustomIndex = 0, //gl_InstanceCustomIndex != gl_InstanceID (latter = idx into built tlas)
		.mask = 0xFFu,
		.instanceShaderBindingTableRecordOffset = sbtOffset,
		.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
		.accelerationStructureReference = m_reference
	};
	return instance;
}

uint64_t vulkan::AccelerationStructure::get_reference() const noexcept
{
	assert(m_reference);
	return m_reference;
}

void vulkan::AccelerationStructure::set_debug_label(const char* name) noexcept
{
	if constexpr (debugMarkers) {
		if (r_device.get_supported_extensions().debug_utils) {
			VkDebugUtilsObjectNameInfoEXT label{
				.sType {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT},
				.pNext {nullptr},
				.objectType {VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR},
				.objectHandle {reinterpret_cast<uint64_t>(m_handle)},
				.pObjectName {name},
			};
			vkSetDebugUtilsObjectNameEXT(r_device, &label);
		}
		else if (r_device.get_supported_extensions().debug_marker) {
			VkDebugMarkerObjectNameInfoEXT label{
				.sType {VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT},
				.pNext {nullptr},
				.objectType {VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT},
				.object {reinterpret_cast<uint64_t>(m_handle)},
				.pObjectName {name},
			};
			vkDebugMarkerSetObjectNameEXT(r_device, &label);
		}
	}
}

vulkan::AccelerationStructureBuilder::AccelerationStructureBuilder(LogicalDevice& device) :
	r_device(device)
{
}
std::optional<size_t> vulkan::AccelerationStructureBuilder::queue_item(const BLASInfo& info)
{
	assert(info.vertexBuffer && info.indexBuffer && "Must give valid buffers to AS build");
	if ((info.vertexBuffer == VK_NULL_HANDLE) || (info.indexBuffer == VK_NULL_HANDLE))
		return std::nullopt;
	VkBufferDeviceAddressInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext = nullptr,
		.buffer = info.vertexBuffer
	};
	VkDeviceAddress vertexAddress = vkGetBufferDeviceAddress(r_device.get_device(), &bufferInfo) + info.vertexOffset;
	bufferInfo.buffer = info.indexBuffer;
	VkDeviceAddress indexAddress = vkGetBufferDeviceAddress(r_device.get_device(), &bufferInfo) + info.indexOffset;
	VkDeviceAddress transformAddress = 0;
	if (info.transformBuffer != VK_NULL_HANDLE) {
		bufferInfo.buffer = info.transformBuffer;
		transformAddress = vkGetBufferDeviceAddress(r_device.get_device(), &bufferInfo);
	}
	auto idx = m_pendingBuilds.size();
	m_pendingBuilds.push_back(BLASBuildEntry{
		VkAccelerationStructureGeometryKHR{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.pNext = nullptr,
			.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
			.geometry {
				.triangles{
					.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
					.pNext = nullptr,
					.vertexFormat = info.vertexFormat,
					.vertexData {
						.deviceAddress = vertexAddress
					},
					.vertexStride = info.vertexStride,
					.maxVertex = info.vertexCount,
					.indexType = info.indexType,
					.indexData {
						.deviceAddress = indexAddress
					},
					.transformData = transformAddress,
				}
			},
			.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
		},
		VkAccelerationStructureBuildRangeInfoKHR{
			.primitiveCount = info.indexCount / 3,
			.primitiveOffset = 0,
			.firstVertex = 0,
			.transformOffset = info.transformOffset,
		},
		info.flags
		});
	return idx;
}
std::vector<vulkan::AccelerationStructureHandle> vulkan::AccelerationStructureBuilder::build_pending()
{
	VkDeviceSize totalSize{0};
	VkDeviceSize maxScratchSize{0};
	uint32_t compactionCount{0};
	for (auto& build : m_pendingBuilds) {
		build.buildInfo = VkAccelerationStructureBuildGeometryInfoKHR {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			.flags = build.flags,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.geometryCount = 1,
			.pGeometries = &build.geometry,
		};
		build.sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(r_device,
			VkAccelerationStructureBuildTypeKHR::VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&build.buildInfo, &build.buildRange.primitiveCount, &build.sizeInfo);
		totalSize += build.sizeInfo.accelerationStructureSize;
		maxScratchSize = Math::max(maxScratchSize, build.sizeInfo.buildScratchSize);
		compactionCount += (build.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR) == VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
	}
	if (!m_scratch || (*m_scratch)->get_size() < maxScratchSize)
		m_scratch = r_device.create_buffer(BufferInfo{
			.size = maxScratchSize,
			.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			.offset = 0,
			.memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY }, {});
	
	auto scratchAddress = (*m_scratch)->get_address();
	VkQueryPool queryPool{ VK_NULL_HANDLE };
	if (compactionCount) {
		assert(compactionCount == m_pendingBuilds.size() && "No mixing of compacted/uncompacted AS in one batch allowed");
		VkQueryPoolCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
			.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
			.queryCount = compactionCount,
		};
		vkCreateQueryPool(r_device, &info, r_device.get_allocator(), &queryPool);
	}
	//Use nvpro values for now
	std::vector<size_t> buildIndices;
	VkDeviceSize batchSize{ 0 };
	VkDeviceSize batchLimit{ 256'000'000 };

	std::vector< AccelerationStructureHandle> retVal;
	for (size_t idx = 0; idx < m_pendingBuilds.size(); ++idx) {
		buildIndices.push_back(idx);
		//Enforce 256 byte alignment
		batchSize += (((m_pendingBuilds[idx].sizeInfo.accelerationStructureSize) + 255) / 256) * 256;
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
		if (batchSize >= batchLimit || idx == m_pendingBuilds.size() - 1) {
			auto cmd = r_device.request_command_buffer(CommandBufferType::Compute);
			if (queryPool) 
				vkResetQueryPool(r_device, queryPool, 0, static_cast<uint32_t>(buildIndices.size()));

			auto batchBuffer =  r_device.create_buffer(BufferInfo{
					.size = batchSize,
					.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR ,
					.offset = 0,
					.memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY }, {});
			VkDeviceSize offset = 0;
			uint32_t queryCount = 0;
			std::vector< AccelerationStructure> tmpAccs;

			for (auto buildIndex : buildIndices) {
				VkAccelerationStructureKHR handle;
				auto& build = m_pendingBuilds[buildIndex];
				VkAccelerationStructureCreateInfoKHR info{
					.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
					.buffer = *batchBuffer,
					.offset = offset,
					.size = build.sizeInfo.accelerationStructureSize,
					.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
				};
				offset += Utility::align_up(build.sizeInfo.accelerationStructureSize, static_cast<VkDeviceSize>(256));
				vkCreateAccelerationStructureKHR(r_device, &info, r_device.get_allocator(), &handle);
				build.buildInfo.dstAccelerationStructure = handle;
				build.buildInfo.scratchData.deviceAddress = scratchAddress;
				if(queryPool)
					tmpAccs.emplace_back(r_device, handle, batchBuffer, info);
				else
					retVal.emplace_back(m_acclerationStructurePool.emplace(r_device, handle, batchBuffer, info));
				auto* range = &build.buildRange;
				vkCmdBuildAccelerationStructuresKHR(cmd->get_handle(), 1, &build.buildInfo, &range);
				cmd->barrier(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
					VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
				if (queryPool)
					vkCmdWriteAccelerationStructuresPropertiesKHR(cmd->get_handle(), 1, &handle,
						VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, queryPool, queryCount++);
			}
			auto fenceHandle = r_device.request_empty_fence();
			r_device.submit_flush(cmd, 0, nullptr, &fenceHandle);
			auto fence = fenceHandle.get_handle();
			vkWaitForFences(r_device, 1, &fence, VK_TRUE, UINT64_MAX);

			if (queryPool) {

				auto cmdCompact = r_device.request_command_buffer(CommandBufferType::Compute);

				std::vector<VkDeviceSize> compactSizes(queryCount);
				vkGetQueryPoolResults(r_device, queryPool, 0, queryCount, queryCount * sizeof(VkDeviceSize),
					compactSizes.data(), sizeof(VkDeviceSize), VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
				offset = 0;
				VkDeviceSize totalCompactSize{ 0 };
				for (auto compactSize : compactSizes)
					totalCompactSize += ((compactSize + 255) / 256) * 256;
				auto compactBuffer = r_device.create_buffer(BufferInfo{
						.size = totalCompactSize,
						.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR ,
						.offset = 0,
						.memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY }, {});
				for (int i = 0; i < tmpAccs.size(); i++) {
					VkAccelerationStructureKHR handle;
					VkAccelerationStructureCreateInfoKHR info{
						.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
						.buffer = *compactBuffer,
						.offset = offset,
						.size = compactSizes[i],
						.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
					};
					offset += Utility::align_up(compactSizes[i], 256ull);
					vkCreateAccelerationStructureKHR(r_device, &info, r_device.get_allocator(), &handle);
					VkCopyAccelerationStructureInfoKHR copyInfo{
						.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
						.src = tmpAccs[i],
						.dst = handle,
						.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR
					};
					vkCmdCopyAccelerationStructureKHR(cmdCompact->get_handle(), &copyInfo);
					retVal.emplace_back(m_acclerationStructurePool.emplace(r_device, handle, batchBuffer, info));
				}
				r_device.submit_flush(cmdCompact, 0, nullptr, &fenceHandle);
				auto fenceCompact = fenceHandle.get_handle();
				vkWaitForFences(r_device, 1, &fenceCompact, VK_TRUE, UINT64_MAX);
			}
			batchSize = 0;
			buildIndices.clear();
		}
	}
	m_pendingBuilds.clear();
	vkDestroyQueryPool(r_device, queryPool, r_device.get_allocator());
	return retVal;
}
vulkan::AccelerationStructureHandle vulkan::AccelerationStructureBuilder::build_tlas(const std::vector<VkAccelerationStructureInstanceKHR>& instances, VkBuildAccelerationStructureFlagsKHR flags)
{
	uint32_t instanceCount = static_cast<uint32_t>(instances.size());

	BufferInfo info{
		.size = sizeof(VkAccelerationStructureInstanceKHR) * instanceCount,
		.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		.offset = 0,
		.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU,
	};
	auto instanceDataBuffer = r_device.create_buffer(info, { InputData {reinterpret_cast<const void*>(instances.data()), info.size} });
	VkBufferDeviceAddressInfo addressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext = nullptr,
		.buffer = *instanceDataBuffer
	};
	auto instanceDataBufferAddr = vkGetBufferDeviceAddress(r_device, &addressInfo);
	auto cmd = r_device.request_command_buffer(CommandBufferType::Compute);
	cmd->barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR);


	VkAccelerationStructureGeometryKHR geometry{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = nullptr,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry {
			.instances {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
				.pNext = nullptr,
				.data = instanceDataBufferAddr,
			}
		},
	};
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = flags,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.dstAccelerationStructure = VK_NULL_HANDLE,
		.geometryCount = 1,
		.pGeometries = &geometry,
	};
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		.pNext = nullptr,
	};
	vkGetAccelerationStructureBuildSizesKHR(r_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
		&instanceCount, &sizeInfo);

	auto accelBuffer = r_device.create_buffer(BufferInfo{
			.size = sizeInfo.accelerationStructureSize,
			.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR ,
			.offset = 0,
			.memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY }, {});

	VkAccelerationStructureCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.buffer = *accelBuffer,
		.size = sizeInfo.accelerationStructureSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
	};
	VkAccelerationStructureKHR accelHandle {VK_NULL_HANDLE};
	vkCreateAccelerationStructureKHR(r_device, &createInfo, r_device.get_allocator(), &accelHandle);
	auto tlas = m_acclerationStructurePool.emplace(r_device, accelHandle, accelBuffer, createInfo);

	BufferInfo scratchInfo{
		.size = sizeInfo.buildScratchSize,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.offset = 0,
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
	};
	auto scratchBuffer = r_device.create_buffer(scratchInfo, {});

	addressInfo.buffer = *scratchBuffer;
	buildInfo.dstAccelerationStructure = accelHandle;
	buildInfo.scratchData.deviceAddress = vkGetBufferDeviceAddress(r_device, &addressInfo);
	VkAccelerationStructureBuildRangeInfoKHR buildRange{
		.primitiveCount = instanceCount,
		.primitiveOffset = 0,
		.firstVertex = 0,
		.transformOffset = 0,
	};
	const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildRange;

	vkCmdBuildAccelerationStructuresKHR(cmd->get_handle(), 1, &buildInfo, &pBuildOffsetInfo);


	auto fenceHandle = r_device.request_empty_fence();
	r_device.submit_flush(cmd, 0, nullptr, &fenceHandle);
	auto fence = fenceHandle.get_handle();
	vkWaitForFences(r_device, 1, &fence, VK_TRUE, UINT64_MAX);
	return tlas;
}

vulkan::AccelerationStructureHandle vulkan::AccelerationStructureBuilder::build_tlas(uint32_t size, VkDeviceAddress address, VkBuildAccelerationStructureFlagsKHR flags)
{
	auto cmd = r_device.request_command_buffer(CommandBufferType::Compute);


	VkAccelerationStructureGeometryKHR geometry{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.pNext = nullptr,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry {
			.instances {
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
				.pNext = nullptr,
				.arrayOfPointers = VK_FALSE,
				.data {.deviceAddress = address},
			}
		},
	};
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = flags,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.dstAccelerationStructure = VK_NULL_HANDLE,
		.geometryCount = 1,
		.pGeometries = &geometry,
	};
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		.pNext = nullptr,
	};
	vkGetAccelerationStructureBuildSizesKHR(r_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
		&size, &sizeInfo);

	auto accelBuffer = r_device.create_buffer(BufferInfo{
			.size = sizeInfo.accelerationStructureSize,
			.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR ,
			.offset = 0,
			.memoryUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY }, {});

	VkAccelerationStructureCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.buffer = *accelBuffer,
		.size = sizeInfo.accelerationStructureSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
	};
	VkAccelerationStructureKHR accelHandle{ VK_NULL_HANDLE };
	vkCreateAccelerationStructureKHR(r_device, &createInfo, r_device.get_allocator(), &accelHandle);
	auto tlas = m_acclerationStructurePool.emplace(r_device, accelHandle, accelBuffer, createInfo);

	BufferInfo scratchInfo{
		.size = sizeInfo.buildScratchSize,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.offset = 0,
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
	};
	auto scratchBuffer = r_device.create_buffer(scratchInfo, {});

	VkBufferDeviceAddressInfo addressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.pNext = nullptr,
		.buffer = *scratchBuffer
	};
	buildInfo.dstAccelerationStructure = accelHandle;
	buildInfo.scratchData.deviceAddress = vkGetBufferDeviceAddress(r_device, &addressInfo);

	VkAccelerationStructureBuildRangeInfoKHR buildRange {
		.primitiveCount = size,
		.primitiveOffset = 0,
		.firstVertex = 0,
		.transformOffset = 0,
	};
	const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildRange;

	vkCmdBuildAccelerationStructuresKHR(cmd->get_handle(), 1, &buildInfo, &pBuildOffsetInfo);


	auto fenceHandle = r_device.request_empty_fence();
	r_device.submit_flush(cmd, 0, nullptr, &fenceHandle);
	auto fence = fenceHandle.get_handle();
	vkWaitForFences(r_device, 1, &fence, VK_TRUE, UINT64_MAX);
	return tlas;
}
