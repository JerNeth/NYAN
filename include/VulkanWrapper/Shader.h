#ifndef VKSHADER_H
#define VKSHADER_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
#include <Util>
#include "LinAlg.h"
#include "MaxVals.h"
#include <typeinfo>

namespace vulkan {
	struct ResourceBinding {
		union {
			VkDescriptorBufferInfo buffer;
			VkDescriptorImageInfo image;
			VkBufferView bufferView; 
			VkAccelerationStructureKHR accelerationStructure;
		};
	};	
	enum class ShaderStage : uint32_t{
		Vertex = Utility::bit_pos(VK_SHADER_STAGE_VERTEX_BIT),// 0,
		TesselationControl = Utility::bit_pos(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT), //1,
		TesselationEvaluation = Utility::bit_pos(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT), //2,
		Geometry = Utility::bit_pos(VK_SHADER_STAGE_GEOMETRY_BIT), //3,
		Fragment = Utility::bit_pos(VK_SHADER_STAGE_FRAGMENT_BIT), //4,
		Compute = Utility::bit_pos(VK_SHADER_STAGE_COMPUTE_BIT),//5,
		Task = Utility::bit_pos(VK_SHADER_STAGE_TASK_BIT_NV), //6,
		Mesh = Utility::bit_pos(VK_SHADER_STAGE_MESH_BIT_NV), //7,
		Raygen = Utility::bit_pos(VK_SHADER_STAGE_RAYGEN_BIT_NV),//8
		AnyHit = Utility::bit_pos(VK_SHADER_STAGE_ANY_HIT_BIT_NV),//9
		ClosestHit = Utility::bit_pos(VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV),//10
		Miss = Utility::bit_pos(VK_SHADER_STAGE_MISS_BIT_NV),//11
		Intersection = Utility::bit_pos(VK_SHADER_STAGE_INTERSECTION_BIT_NV),//12
		Callable = Utility::bit_pos(VK_SHADER_STAGE_CALLABLE_BIT_NV),//13
		Size,
	};
	constexpr uint32_t NUM_SHADER_STAGES = static_cast<uint32_t>(ShaderStage::Size);

	//enum class DescriptorType : uint32_t {
	//	Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
	//	CombinedImageSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	//	SampledImage = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	//	StorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	//	UniformTexelBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
	//	StorageTexelBuffer = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
	//	UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	//	StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	//	UniformDynamicBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	//	StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	//	InputAttachment = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
	//	InlineUniformBlock = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
	//	AccelerationStructure = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
	//	Invalid
	//};
	//struct DescriptorLayout {
	//	uint32_t arraySize = 0;
	//	DescriptorType type = DescriptorType::Invalid;
	//	Utility::bitset <static_cast<size_t>(ShaderStage::Size), ShaderStage> stages {};
	//	friend bool operator==(const DescriptorLayout& left, const DescriptorLayout& right) {
	//		return left.type == right.type && 
	//			left.arraySize == right.arraySize &&
	//			left.stages == right.stages;
	//	}
	//};
	//struct DescriptorSetLayout {
	//	std::array<DescriptorLayout, MAX_BINDINGS> descriptors;
	//	friend bool operator==(const DescriptorSetLayout& left, const DescriptorSetLayout& right) {
	//		//if (left.descriptors.size() != right.descriptors.size())
	//		//	return false;
	//		for (size_t i = 0; i < left.descriptors.size(); i++)
	//			if (left.descriptors[i] != right.descriptors[i])
	//				return false;
	//		return true;
	//	}
	//};
	//struct ShaderLayout {
	//	std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS> descriptors;
	//	Utility::bitset<MAX_DESCRIPTOR_SETS> used;
	//	Utility::bitset<MAX_VERTEX_ATTRIBUTES> inputs;
	//	Utility::bitset<MAX_VERTEX_ATTRIBUTES> outputs;
	//	std::array<uint8_t, MAX_VERTEX_ATTRIBUTES> attributeElementCounts;
	//	VkPushConstantRange pushConstantRange{};
	//	friend bool operator==(ShaderLayout& left, ShaderLayout& right) {
	//		return left.descriptors == right.descriptors &&
	//			left.used == right.used &&
	//			left.inputs == right.inputs &&
	//			left.outputs == right.outputs &&
	//			left.attributeElementCounts == right.attributeElementCounts &&
	//			left.pushConstantRange.offset == right.pushConstantRange.offset &&
	//			left.pushConstantRange.size == right.pushConstantRange.size &&
	//			left.pushConstantRange.stageFlags == right.pushConstantRange.stageFlags;
	//	}
	//	friend bool operator==(const ShaderLayout& left, const ShaderLayout& right) {
	//		return left.descriptors == right.descriptors &&
	//			left.used == right.used &&
	//			left.inputs == right.inputs &&
	//			left.outputs == right.outputs &&
	//			left.attributeElementCounts == right.attributeElementCounts &&
	//			left.pushConstantRange.offset == right.pushConstantRange.offset &&
	//			left.pushConstantRange.size == right.pushConstantRange.size &&
	//			left.pushConstantRange.stageFlags == right.pushConstantRange.stageFlags;
	//	}
	//	void combine(const ShaderLayout& other) {
	//		used |= other.used;
	//		inputs |= other.inputs;
	//		outputs |= other.outputs;
	//		for (uint32_t i = 0; i < MAX_VERTEX_ATTRIBUTES; i++) {
	//			assert(!(attributeElementCounts[i] != 0) || (other.attributeElementCounts[i] == 0));
	//			if(attributeElementCounts[i] == 0)
	//				attributeElementCounts[i] = other.attributeElementCounts[i];
	//		}
	//		pushConstantRange.stageFlags |= other.pushConstantRange.stageFlags;
	//		pushConstantRange.size = Math::max(pushConstantRange.size, other.pushConstantRange.size);
	//		assert(pushConstantRange.size <= 128u);
	//		for (uint32_t descriptor = 0; descriptor < descriptors.size(); descriptor++) {
	//			auto& descriptorSetLayouts = descriptors[descriptor].descriptors;
	//			const auto& otherDescriptorSetLayouts = other.descriptors[descriptor].descriptors;

	//			assert(descriptorSetLayouts.size() == otherDescriptorSetLayouts.size());

	//			for (uint32_t binding = 0; binding < descriptorSetLayouts.size(); binding++) {
	//				auto& descriptorSetLayout = descriptorSetLayouts[binding];
	//				const auto& otherDescriptorSetLayout = otherDescriptorSetLayouts[binding];
	//				if (descriptorSetLayout.type == otherDescriptorSetLayout.type &&
	//					descriptorSetLayout.arraySize == otherDescriptorSetLayout.arraySize) {
	//					descriptorSetLayout.stages |= otherDescriptorSetLayout.stages;
	//				}
	//				else if(descriptorSetLayout.arraySize == 0) {
	//					descriptorSetLayout = otherDescriptorSetLayout;
	//				}
	//			}
	//		}
	//	}
	//};

	static constexpr uint32_t invalidSpecializationConstant {std::numeric_limits<uint32_t>::max()};
	class Shader : public VulkanObject< VkShaderModule>
	{
	public:
		template<typename T>
		struct SpecializationConstant {
			uint32_t id;
			T value;
		};
		struct SpecializationConstantId {
			enum class Type : uint32_t {
				SByte,
				UByte,
				Short,
				UShort,
				Int,
				UInt,
				Int64,
				UInt64,
				Half,
				Float,
				Double,
				Bool
			} type;
			uint32_t id;
		};
	public:
		Shader(LogicalDevice& parent, const std::vector<uint32_t>& shaderCode);
		~Shader();
		ShaderStage get_stage();
		void parse_shader(const std::vector<uint32_t>& shaderCode);
		//const ShaderLayout& get_layout() const {
		//	return m_layout;
		//}
		VkPipelineShaderStageCreateInfo get_create_info();
		Utility::HashValue get_hash();
		SpecializationConstantId get_specialization_constant_id(const std::string& name) const;
	private:
		static VkShaderModule create_module(const LogicalDevice& device, const std::vector<uint32_t>& shaderCode);

		ShaderStage m_stage;
		//ShaderLayout m_layout;
		Utility::HashValue m_hashValue;
		std::unordered_map<std::string, SpecializationConstantId> m_specializationConstants;
	}; 


	class ShaderInstance {
	public:
		ShaderInstance(VkShaderModule module, VkShaderStageFlagBits stage);
		template<typename... Args>
		ShaderInstance(VkShaderModule module, VkShaderStageFlagBits stage, Args... args) :
			m_module(module),
			m_entryPoint("main"),
			m_offset(0),
			m_stage(stage)
		{
			handle_constants(std::forward<Args>(args)...);
			m_specializationInfo = VkSpecializationInfo{
				.mapEntryCount = static_cast<uint32_t>(m_specialization.size()),
				.pMapEntries = m_specialization.data(),
				.dataSize = m_dataStorage.size(),
				.pData = m_dataStorage.data()
			};
		}
		VkPipelineShaderStageCreateInfo get_stage_info() const;
		VkShaderStageFlagBits get_stage() const;
	private:
		template<typename T>
		void handle_constants(const Shader::SpecializationConstant<T>& constant)
		{
			m_offset = create_constant(constant.id, constant.value, m_offset);
		}
		template<typename... Args>
		void handle_constants(Args... args)
		{
			(handle_constants(args), ...);
		}
		template<typename T>
		uint32_t create_constant(uint32_t constantId, T value, uint32_t offset) {
			if (constantId != invalidSpecializationConstant) {
				auto size = sizeof(T);
				m_specialization.push_back(VkSpecializationMapEntry{ .constantID {constantId},.offset{offset},.size {size} });
				m_dataStorage.resize(offset + size);
				T* data = reinterpret_cast<T*>(&m_dataStorage[offset]);
				*data = value;
			}
			else {
				assert(false);
			}
			return offset;
		}

		VkShaderModule m_module;
		std::vector< VkSpecializationMapEntry> m_specialization;
		std::vector<std::byte> m_dataStorage;
		std::string m_entryPoint;
		uint32_t m_offset;
		VkSpecializationInfo m_specializationInfo;
		VkShaderStageFlagBits m_stage;
	};
	class ShaderStorage {
	public:
		template<typename T>
		struct SpecializationConstant {
			const std::string& name;
			T value;
		};
	public:
		ShaderStorage(LogicalDevice& device);

		ShaderId add_instance(ShaderId shaderId);
		ShaderId add_instance(ShaderId shaderId, uint32_t workGroupSizeX, uint32_t workGroupSizeY, uint32_t workGroupSizeZ);
		template<typename... Args>
		ShaderId add_instance_with_constants(ShaderId shaderId, Args... args) noexcept
		{
			m_currentShader = get_shader(shaderId);
			return static_cast<ShaderId>(m_instanceStorage.emplace_intrusive(ShaderInstance{ m_currentShader->get_handle()
				, static_cast<VkShaderStageFlagBits>(1ull << static_cast<uint32_t>(m_currentShader->get_stage()))
				, handle_constant(args)... }) );
		}
		ShaderId add_shader(const std::vector<uint32_t>& shaderCode);
		Shader* get_shader(ShaderId shaderId);
		ShaderInstance* get_instance(ShaderId instanceId);
		void clear();
	private:
		template<typename T>
		Shader::SpecializationConstant<T> handle_constant(SpecializationConstant<T> constant)
		{
			auto id = m_currentShader->get_specialization_constant_id(constant.name);
			switch (id.type) {
				case Shader::SpecializationConstantId::Type::SByte:
					assert(typeid(T) == typeid(int8_t));
						break;
				case Shader::SpecializationConstantId::Type::UByte:
					assert(typeid(T) == typeid(uint8_t));
						break;
				case Shader::SpecializationConstantId::Type::Short:
					assert(typeid(T) == typeid(int16_t));
						break;
				case Shader::SpecializationConstantId::Type::UShort:
					assert(typeid(T) == typeid(uint16_t));
						break;
				case Shader::SpecializationConstantId::Type::Int:
					assert(typeid(T) == typeid(int32_t));
						break;
				case Shader::SpecializationConstantId::Type::UInt:
					assert(typeid(T) == typeid(uint32_t));
						break;
				case Shader::SpecializationConstantId::Type::Int64:
					assert(typeid(T) == typeid(int64_t));
						break;
				case Shader::SpecializationConstantId::Type::UInt64:
					assert(typeid(T) == typeid(uint64_t));
						break;
				case Shader::SpecializationConstantId::Type::Float:
					assert(typeid(T) == typeid(float));
						break;
				case Shader::SpecializationConstantId::Type::Double:
					assert(typeid(T) == typeid(double));
						break;
				case Shader::SpecializationConstantId::Type::Half:
					assert(typeid(T) == typeid(Math::half));
					break;
				case Shader::SpecializationConstantId::Type::Bool:
					assert(typeid(T) == typeid(VkBool32));
					break;
				default:
					assert(false && "TODO");
					break;
			}
			return { id.id, constant.value};
		}

		LogicalDevice& r_device;
		Utility::LinkedBucketList<ShaderInstance> m_instanceStorage;
		Utility::LinkedBucketList<Shader> m_shaderStorage;
		Shader* m_currentShader = nullptr;
	};

};

#endif //VKSHADER_H