#include "Shader.h"
#include "DescriptorSet.h"
#include "Instance.h"
#include "LogicalDevice.h"
#ifdef __clang__
#pragma clang diagnostic push
#endif
#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning( disable : 26812 )
#endif
#include "spirv_cross.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "Utility/Exceptions.h"

vulkan::Shader::Shader(LogicalDevice& parent, const std::vector<uint32_t>& shaderCode) :
	VulkanObject(parent, create_module(parent, shaderCode))
{
	Utility::VectorHash<uint32_t> hasher;
	m_hashValue = hasher(shaderCode);
	parse_shader( shaderCode);
}

vulkan::Shader::~Shader()
{
	if(m_handle != VK_NULL_HANDLE)
		vkDestroyShaderModule(r_device.get_device(), m_handle, r_device.get_allocator());
}

vulkan::ShaderStage vulkan::Shader::get_stage()
{
	return m_stage;
}

//static inline std::tuple<uint32_t, uint32_t, spirv_cross::SPIRType> get_values(const spirv_cross::Resource& resource, const spirv_cross::Compiler& comp)
//{
//	auto set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
//	auto binding = comp.get_decoration(resource.id, spv::DecorationBinding);
//	const auto& type = comp.get_type(resource.type_id);
//	return { set, binding, type };
//}

//static inline void array_info(std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS>& layouts, const spirv_cross::SPIRType& type, uint32_t set, uint32_t binding, vulkan::ShaderStage stage)
//{
//
//	layouts[set].descriptors[binding].stages.set(stage);
//	//layouts[set].stages[binding] |= 1u << static_cast<uint32_t>(m_stage);
//	auto& size = layouts[set].descriptors[binding].arraySize;
//	if (type.array.empty()) {
//		size = 1;
//	}
//	else {
//		//TODO Error handling
//		size = static_cast<uint8_t>(type.array.front());
//	}
//}
static vulkan::ShaderStage convert_spriv_execution_model(spv::ExecutionModel model) {
	switch (model) {
	case spv::ExecutionModel::ExecutionModelVertex:
		return vulkan::ShaderStage::Vertex;
	case spv::ExecutionModel::ExecutionModelTessellationControl:
		return vulkan::ShaderStage::TesselationControl;
	case spv::ExecutionModel::ExecutionModelTessellationEvaluation:
		return vulkan::ShaderStage::TesselationEvaluation;
	case spv::ExecutionModel::ExecutionModelGeometry:
		return vulkan::ShaderStage::Geometry;
	case spv::ExecutionModel::ExecutionModelFragment:
		return vulkan::ShaderStage::Fragment;
	case spv::ExecutionModel::ExecutionModelGLCompute:
		return vulkan::ShaderStage::Compute;
	case spv::ExecutionModel::ExecutionModelTaskNV:
		return vulkan::ShaderStage::Task;
	case spv::ExecutionModel::ExecutionModelMeshNV:
		return vulkan::ShaderStage::Mesh;
	case spv::ExecutionModel::ExecutionModelRayGenerationKHR:
		return vulkan::ShaderStage::Raygen;
	case spv::ExecutionModel::ExecutionModelIntersectionKHR:
		return vulkan::ShaderStage::Intersection;
	case spv::ExecutionModel::ExecutionModelAnyHitKHR:
		return vulkan::ShaderStage::AnyHit;
	case spv::ExecutionModel::ExecutionModelClosestHitKHR:
		return vulkan::ShaderStage::ClosestHit;
	case spv::ExecutionModel::ExecutionModelMissKHR:
		return vulkan::ShaderStage::Miss;
	case spv::ExecutionModel::ExecutionModelCallableKHR:
		return vulkan::ShaderStage::Callable;
	default:
		return vulkan::ShaderStage::Size;
	}
}

static vulkan::Shader::SpecializationConstantId::Type convertType(spirv_cross::SPIRType::BaseType type)
{
	using enum spirv_cross::SPIRType::BaseType;
	switch (type) {
	case SByte:
		return vulkan::Shader::SpecializationConstantId::Type::SByte;
	case UByte:
		return vulkan::Shader::SpecializationConstantId::Type::UByte;
	case Short:
		return vulkan::Shader::SpecializationConstantId::Type::Short;
	case UShort:
		return vulkan::Shader::SpecializationConstantId::Type::UShort;
	case Int:
		return vulkan::Shader::SpecializationConstantId::Type::Int;
	case UInt:
		return vulkan::Shader::SpecializationConstantId::Type::UInt;
	case Int64:
		return vulkan::Shader::SpecializationConstantId::Type::Int64;
	case UInt64:
		return vulkan::Shader::SpecializationConstantId::Type::UInt64;
	case Half:
		return vulkan::Shader::SpecializationConstantId::Type::Half;
	case Float:
		return vulkan::Shader::SpecializationConstantId::Type::Float;
	case Double:
		return vulkan::Shader::SpecializationConstantId::Type::Double;
	case Boolean:
		return vulkan::Shader::SpecializationConstantId::Type::Bool;
	default:
		assert(false);
		throw std::runtime_error("Unknown BaseType");
	}
}

void vulkan::Shader::parse_shader(const std::vector<uint32_t>& shaderCode)
{
	using namespace spirv_cross;
	//bool usesBinding = false;
	Compiler comp(shaderCode); 
	m_stage = convert_spriv_execution_model(comp.get_execution_model());
	if (m_stage == vulkan::ShaderStage::Size)
		throw std::runtime_error("Unsupported Shadertype");
	ShaderResources resources = comp.get_shader_resources();
	
	if (m_stage == vulkan::ShaderStage::Compute) {
		spirv_cross::SpecializationConstant x, y, z;
		comp.get_work_group_size_specialization_constants(x, y, z);
		if (x.constant_id != 0 && x.id != 0) {
			const auto& constant = comp.get_constant(x.id);
			const auto& type = comp.get_type(constant.constant_type);
			m_specilizationConstants.emplace("local_size_x", SpecializationConstantId{ convertType(type.basetype), x.constant_id });
		}
		if (y.constant_id != 0 && y.id != 0) {
			const auto& constant = comp.get_constant(y.id);
			const auto& type = comp.get_type(constant.constant_type);
			m_specilizationConstants.emplace("local_size_y", SpecializationConstantId{ convertType(type.basetype), y.constant_id });
		}
		if (z.constant_id != 0 && z.id != 0) {
			const auto& constant = comp.get_constant(z.id);
			const auto& type = comp.get_type(constant.constant_type);
			m_specilizationConstants.emplace("local_size_z", SpecializationConstantId{ convertType(type.basetype), z.constant_id });
		}
		auto specs = comp.get_specialization_constants();
		for (auto& spec : specs) {
			const auto& name = comp.get_name(spec.id);
			if (!name.empty()) {
				const auto& constant = comp.get_constant(spec.id);
				const auto& type = comp.get_type(constant.constant_type);
				m_specilizationConstants.emplace(name, SpecializationConstantId{ convertType(type.basetype), spec.constant_id });
			}
		}
	}
	//auto specs = comp.get_specialization_constants();
	//for (auto spec : specs) {

	//	const auto& val = comp.get_constant(spec.id);
	//	comp.set_execution_mode()
	//		auto t = val.constant_type;
	//	comp.get_name(spec.id);
	//}
	//for (auto& uniformBuffer : resources.uniform_buffers) {
	//	auto [set, binding, type] = get_values(uniformBuffer, comp);
	//	m_layout.used.set(set);
	//	m_layout.descriptors[set].descriptors[binding].type = DescriptorType::UniformBuffer;
	//	array_info(m_layout.descriptors, type, set, binding);
	//}
	//for (auto& sampledImage : resources.sampled_images) {
	//	auto [set, binding, type] = get_values(sampledImage, comp);
	//	m_layout.used.set(set);
	//	if (type.image.dim == spv::DimBuffer) {
	//		m_layout.descriptors[set].descriptors[binding].type = DescriptorType::UniformTexelBuffer;
	//	}
	//	else {
	//		m_layout.descriptors[set].descriptors[binding].type = DescriptorType::CombinedImageSampler;
	//	}
	//	array_info(m_layout.descriptors, type, set, binding);
	//}
	//for (auto& subpassInput : resources.subpass_inputs) {
	//	auto [set, binding, type] = get_values(subpassInput, comp);
	//	m_layout.used.set(set);
	//	m_layout.descriptors[set].descriptors[binding].type = DescriptorType::InputAttachment;
	//	array_info(m_layout.descriptors, type, set, binding);
	//}
	//for (auto& separateImage : resources.separate_images) {
	//	auto [set, binding, type] = get_values(separateImage, comp);
	//	m_layout.used.set(set);
	//	if (type.image.dim == spv::DimBuffer) {
	//		m_layout.descriptors[set].descriptors[binding].type = DescriptorType::UniformTexelBuffer;
	//	}
	//	else {
	//		m_layout.descriptors[set].descriptors[binding].type = DescriptorType::SampledImage;
	//	}
	//	array_info(m_layout.descriptors, type, set, binding);
	//}
	//for (auto& separateSampler : resources.separate_samplers) {
	//	auto [set, binding, type] = get_values(separateSampler, comp);
	//	m_layout.used.set(set); 
	//	m_layout.descriptors[set].descriptors[binding].type = DescriptorType::Sampler;
	//	array_info(m_layout.descriptors, type, set, binding);
	//}
	//for (auto& storageImage : resources.storage_images) {
	//	auto [set, binding, type] = get_values(storageImage, comp);
	//	m_layout.used.set(set);
	//	m_layout.descriptors[set].descriptors[binding].type = DescriptorType::StorageImage;
	//	array_info(m_layout.descriptors, type, set, binding);
	//}
	//for (auto& storageImage : resources.storage_buffers) {
	//	auto [set, binding, type] = get_values(storageImage, comp);
	//	m_layout.used.set(set); 
	//	m_layout.descriptors[set].descriptors[binding].type = DescriptorType::StorageBuffer;
	//	array_info(m_layout.descriptors, type, set, binding);
	//}
	//for (auto& attrib : resources.stage_inputs) {
	//	if (m_stage == vulkan::ShaderStage::Vertex) {
	//		auto& type = comp.get_type(attrib.type_id);
	//		auto location = comp.get_decoration(attrib.id, spv::DecorationLocation);
	//		assert(type.vecsize <= 255);
	//		m_layout.attributeElementCounts[location] = static_cast<uint8_t>(type.vecsize);
	//		m_layout.inputs.set(location);
	//	}
	//}
	//for (auto& attrib : resources.stage_outputs) {
	//	if (m_stage == vulkan::ShaderStage::Fragment) {
	//		auto location = comp.get_decoration(attrib.id, spv::DecorationLocation);
	//		m_layout.outputs.set(location);
	//	}
	//}
	//if (!resources.push_constant_buffers.empty()) {
	//	m_layout.pushConstantRange.stageFlags |= static_cast<VkShaderStageFlagBits>(1u << static_cast<uint32_t>(m_stage));
	//	m_layout.pushConstantRange.size =  static_cast<uint32_t>(comp.get_declared_struct_size(comp.get_type(resources.push_constant_buffers.front().base_type_id)));
	//}
}

VkPipelineShaderStageCreateInfo vulkan::Shader::get_create_info()
{
	assert(m_handle != VK_NULL_HANDLE);
	VkPipelineShaderStageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = static_cast<VkShaderStageFlagBits>(1u <<static_cast<uint32_t>(m_stage)),
		.module = m_handle,
		.pName = "main",
		.pSpecializationInfo = nullptr,
	};
	return createInfo;
}

Utility::HashValue vulkan::Shader::get_hash()
{
	return m_hashValue;
}

vulkan::Shader::SpecializationConstantId vulkan::Shader::get_specialization_constant_id(const std::string& name) const
{
	auto it = m_specilizationConstants.find(name);
	assert(it != m_specilizationConstants.end());
	return it->second;
}

VkShaderModule vulkan::Shader::create_module(LogicalDevice& device, const std::vector<uint32_t>& shaderCode)
{
	VkShaderModule module{VK_NULL_HANDLE};
	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shaderCode.size() * sizeof(uint32_t),
		.pCode = shaderCode.data(),
	};
	if (auto result = vkCreateShaderModule(device.get_device(), &createInfo, device.get_allocator(), &module); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
	return module;
}

vulkan::ShaderInstance::ShaderInstance(VkShaderModule module, VkShaderStageFlagBits stage) :
	m_module(module),
	m_specialization(),
	m_dataStorage(),
	m_entryPoint("main"),
	m_stage(stage),
	m_specializationInfo(VkSpecializationInfo{
		.mapEntryCount = static_cast<uint32_t>(m_specialization.size()),
		.pMapEntries = m_specialization.data(),
		.dataSize = m_dataStorage.size(),
		.pData = m_dataStorage.data()
	})
{
	
}

VkPipelineShaderStageCreateInfo vulkan::ShaderInstance::get_stage_info() const
{
	VkPipelineShaderStageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = m_stage,
		.module = m_module,
		.pName = m_entryPoint.c_str(),
		.pSpecializationInfo = &m_specializationInfo
	};
	return createInfo;
}

VkShaderStageFlagBits vulkan::ShaderInstance::get_stage() const
{
	return m_stage;
}

vulkan::ShaderStorage::ShaderStorage(LogicalDevice& device) : 
	r_device(device)
{

}


vulkan::ShaderId vulkan::ShaderStorage::add_instance(ShaderId shaderId) 
{
	auto* shader = get_shader(shaderId);
	return static_cast<ShaderId>(m_instanceStorage.emplace_intrusive(shader->get_handle()
		,static_cast<VkShaderStageFlagBits>(1ull << static_cast<uint32_t>(shader->get_stage()))));
}

vulkan::ShaderId vulkan::ShaderStorage::add_shader(const std::vector<uint32_t>& shaderCode) 
{
	return static_cast<ShaderId>(m_shaderStorage.emplace_intrusive(r_device, shaderCode));
}

vulkan::Shader* vulkan::ShaderStorage::get_shader(ShaderId shaderId)
{
	return m_shaderStorage.get_ptr(shaderId);
}

vulkan::ShaderInstance* vulkan::ShaderStorage::get_instance(ShaderId instanceId)
{
	return m_instanceStorage.get_ptr(instanceId);
}

void vulkan::ShaderStorage::clear()
{
	m_instanceStorage.clear();
	m_shaderStorage.clear();
}
