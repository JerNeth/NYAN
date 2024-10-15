module;

//#include <cassert>
//#include <expected>
//#include <span>

#include "volk.h"
#include "spirv_cross.hpp"

module NYANVulkan;
import std;

import NYANLog;

using namespace nyan::vulkan;


Shader::Shader(Shader&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	m_shaderData(other.m_shaderData)
{

}


Shader& Shader::operator=(Shader&& other) noexcept
{
	assert(ptr_device == other.ptr_device);
	if (this != std::addressof(other))
	{
		std::swap(m_handle, other.m_handle);
		std::swap(m_shaderData, other.m_shaderData);

	}
	return *this;
}

Shader::~Shader() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		ptr_device->vkDestroyShaderModule(m_handle);
}

Shader::Stage Shader::get_stage() const noexcept
{
	return m_shaderData.stage;
}

ShaderInstance nyan::vulkan::Shader::create_shader_instance() const noexcept
{
	return ShaderInstance{m_shaderData, m_handle};
}

constexpr std::optional<Shader::Stage> convert_spriv_execution_model(const spv::ExecutionModel model) noexcept
{
	switch (model) {
	case spv::ExecutionModel::ExecutionModelVertex:
		return Shader::Stage::Vertex;
	case spv::ExecutionModel::ExecutionModelTessellationControl:
		return Shader::Stage::TesselationControl;
	case spv::ExecutionModel::ExecutionModelTessellationEvaluation:
		return Shader::Stage::TesselationEvaluation;
	case spv::ExecutionModel::ExecutionModelGeometry:
		return Shader::Stage::Geometry;
	case spv::ExecutionModel::ExecutionModelFragment:
		return Shader::Stage::Fragment;
	case spv::ExecutionModel::ExecutionModelGLCompute:
		return Shader::Stage::Compute;
	case spv::ExecutionModel::ExecutionModelTaskEXT:
		return Shader::Stage::Task;
	case spv::ExecutionModel::ExecutionModelMeshEXT:
		return Shader::Stage::Mesh;
	case spv::ExecutionModel::ExecutionModelRayGenerationKHR:
		return Shader::Stage::Raygen;
	case spv::ExecutionModel::ExecutionModelIntersectionKHR:
		return Shader::Stage::Intersection;
	case spv::ExecutionModel::ExecutionModelAnyHitKHR:
		return Shader::Stage::AnyHit;
	case spv::ExecutionModel::ExecutionModelClosestHitKHR:
		return Shader::Stage::ClosestHit;
	case spv::ExecutionModel::ExecutionModelMissKHR:
		return Shader::Stage::Miss;
	case spv::ExecutionModel::ExecutionModelCallableKHR:
		return Shader::Stage::Callable;
	default:
		assert(false);
		return std::nullopt;
	}
}

constexpr Shader::SpecializationConstant::Type convert_type(const spirv_cross::SPIRType::BaseType type) noexcept
{
	using enum spirv_cross::SPIRType::BaseType;
	switch (type) {
	case SByte:
		return Shader::SpecializationConstant::Type::SByte;
	case UByte:
		return Shader::SpecializationConstant::Type::UByte;
	case Short:
		return Shader::SpecializationConstant::Type::Short;
	case UShort:
		return Shader::SpecializationConstant::Type::UShort;
	case Int:
		return Shader::SpecializationConstant::Type::Int;
	case UInt:
		return Shader::SpecializationConstant::Type::UInt;
	case Int64:
		return Shader::SpecializationConstant::Type::Int64;
	case UInt64:
		return Shader::SpecializationConstant::Type::UInt64;
	case Half:
		return Shader::SpecializationConstant::Type::Half;
	case Float:
		return Shader::SpecializationConstant::Type::Float;
	case Double:
		return Shader::SpecializationConstant::Type::Double;
	case Boolean:
		return Shader::SpecializationConstant::Type::Bool;
	default:
		assert(false);
	}
	return Shader::SpecializationConstant::Type::UInt;
}


std::expected<Shader, Error> Shader::create(const LogicalDevice& device, std::span<uint32_t> shaderCode) noexcept
{

	using namespace spirv_cross;
	//bool usesBinding = false;
	Compiler comp(shaderCode.data(), shaderCode.size());
	const auto stage = convert_spriv_execution_model(comp.get_execution_model());
	assert(stage);
	if (!stage) [[unlikely]]
		return std::unexpected{ VK_ERROR_INVALID_SHADER_NV };
	
	const auto& capabilities = comp.get_declared_capabilities();
	ShaderResources resources = comp.get_shader_resources();
	ShaderData shaderData{
		.stage = *stage
	};
	auto fill_storage = [&shaderData]<typename T0>(T0 value, const auto & spec, const auto & type, const auto & name)
	{
		auto prevSize = shaderData.specConstantStorage.size();

		shaderData.specConstantStorage.resize(prevSize + sizeof(T0));

		new(reinterpret_cast<T0*>(shaderData.specConstantStorage.data() + prevSize)) T0{ value };
		shaderData.specConstants.push_back(SpecializationConstant{
			.type = {convert_type(type.basetype)},
			.offset{static_cast<uint32_t>(prevSize)},
			.name{name}
			});

		shaderData.specConstantEntries.push_back(VkSpecializationMapEntry{
			.constantID {spec.constant_id},
			.offset{static_cast<uint32_t>(prevSize) },
			.size{sizeof(T0)}
			});
	};
	if (*stage == Stage::Compute) {
		spirv_cross::SpecializationConstant x, y, z;
		comp.get_work_group_size_specialization_constants(x, y, z);
		auto fill_fixed = [&](auto spec, auto name) {
				if (spec.constant_id != 0 && spec.id != 0) {
					const auto& constant = comp.get_constant(spec.id);
					const auto& type = comp.get_type(constant.constant_type);
					fill_storage(constant.scalar_i32(), spec, type, name);
				}
			};
		fill_fixed(x, "local_size_x");
		fill_fixed(y, "local_size_y");
		fill_fixed(z, "local_size_z");
	}

	for (auto specs = comp.get_specialization_constants(); auto & spec : specs) {
		if (const auto& name = comp.get_name(spec.id); !name.empty()) {
			const auto& constant = comp.get_constant(spec.id);
			const auto& type = comp.get_type(constant.constant_type);



			using enum spirv_cross::SPIRType::BaseType;
			switch (type.basetype) {
			case SByte:
				fill_storage(constant.scalar_i8(), spec, type, name);
				break;
			case UByte:
				fill_storage(constant.scalar_u8(), spec, type, name);
				break;
			case Short:
				fill_storage(constant.scalar_i16(), spec, type, name);
				break;
			case UShort:
				fill_storage(constant.scalar_u16(), spec, type, name);
				break;
			case Int:
				fill_storage(constant.scalar_i32(), spec, type, name);
				break;
			case UInt:
				fill_storage(constant.scalar(), spec, type, name);
				break;
			case Int64:
				fill_storage(constant.scalar_i64(), spec, type, name);
				break;
			case UInt64:
				fill_storage(constant.scalar_u64(), spec, type, name);
				break;
			case Half:
				fill_storage(constant.scalar_f16(), spec, type, name);
				break;
			case Float:
				fill_storage(constant.scalar_f32(), spec, type, name);
				break;
			case Double:
				fill_storage(constant.scalar_f64(), spec, type, name);
				break;
			case Boolean:
				fill_storage(constant.scalar(), spec, type, name);
			default:
				assert(false);
			}

			//m_specializationConstants.emplace(name, SpecializationConstantId{ *convType, spec.constant_id });
		}
	}

	const VkShaderModuleCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO},
		.pNext  {nullptr},
		.flags {0},
		.codeSize {shaderCode.size() * sizeof(uint32_t)},
		.pCode {shaderCode.data()},
	};

	VkShaderModule handle {VK_NULL_HANDLE};
	const auto& deviceWrapper = device.get_device();

	if(const auto result = deviceWrapper.vkCreateShaderModule(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return Shader{ deviceWrapper, handle, shaderData };
}

Shader::Shader(const LogicalDeviceWrapper& device, const VkShaderModule handle, ShaderData shaderData) noexcept :
	Object(device, handle),
	m_shaderData(std::move(shaderData))
{
	assert(m_handle != VK_NULL_HANDLE);
}

ShaderInstance::ShaderInstance(Shader::ShaderData data, VkShaderModule shaderModule) noexcept :
	m_data(std::move(data)),
	m_module(shaderModule),
	m_specializationInfo(VkSpecializationInfo{
		.mapEntryCount = static_cast<uint32_t>(m_data.specConstantEntries.size()),
		.pMapEntries = m_data.specConstantEntries.data(),
		.dataSize = m_data.specConstantStorage.size(),
		.pData = m_data.specConstantStorage.data()
		})
{
}

size_t nyan::vulkan::ShaderInstance::get_num_spec_constants() const noexcept
{
	return m_data.specConstantEntries.size();
}

VkPipelineShaderStageCreateInfo nyan::vulkan::ShaderInstance::get_shader_stage_create_info() const noexcept
{
	return VkPipelineShaderStageCreateInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.flags { 0},
		.stage = static_cast<VkShaderStageFlagBits>(1u << static_cast<uint32_t>(m_data.stage)),
		.module = m_module,
		.pName = "main",
		.pSpecializationInfo = &m_specializationInfo
	};
}
