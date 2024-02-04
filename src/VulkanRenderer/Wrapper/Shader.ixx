module;

#include <bit>
#include <expected>
#include <span>

#include "volk.h"

#include "magic_enum.hpp"

export module NYANVulkanWrapper:Shader;
import :LogicalDeviceWrapper;
import :Object;
import :Error;

export namespace nyan::vulkan::wrapper
{
	class LogicalDevice;
	class ShaderInstance;
	class Shader : public Object <VkShaderModule>
	{
	public:
		enum class Stage : uint32_t {
			Vertex = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_VERTEX_BIT)),// 0,
			TesselationControl = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)), //1,
			TesselationEvaluation = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)), //2,
			Geometry = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_GEOMETRY_BIT)), //3,
			Fragment = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_FRAGMENT_BIT)), //4,
			Compute = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_COMPUTE_BIT)),//5,
			Task = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_TASK_BIT_NV)), //6,
			Mesh = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_MESH_BIT_NV)), //7,
			Raygen = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_RAYGEN_BIT_NV)),//8
			AnyHit = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_ANY_HIT_BIT_NV)),//9
			ClosestHit = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV)),//10
			Miss = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_MISS_BIT_NV)),//11
			Intersection = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_INTERSECTION_BIT_NV)),//12
			Callable = std::countr_zero(static_cast<uint64_t>(VK_SHADER_STAGE_CALLABLE_BIT_NV))//13
		};

		struct SpecializationConstant {
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
			uint32_t offset;
			std::string name;
			template<typename T>
			static bool validate_type(Type type) noexcept {
				switch (type) {
				case Shader::SpecializationConstant::Type::SByte:
					assert(typeid(T) == typeid(int8_t));
					return typeid(T) == typeid(int8_t);
				case Shader::SpecializationConstant::Type::UByte:
					assert(typeid(T) == typeid(uint8_t));
					return typeid(T) == typeid(uint8_t);
				case Shader::SpecializationConstant::Type::Short:
					assert(typeid(T) == typeid(int16_t));
					return typeid(T) == typeid(int16_t);
				case Shader::SpecializationConstant::Type::UShort:
					assert(typeid(T) == typeid(uint16_t));
					return typeid(T) == typeid(uint16_t);
				case Shader::SpecializationConstant::Type::Int:
					assert(typeid(T) == typeid(int32_t));
					return typeid(T) == typeid(int32_t);
				case Shader::SpecializationConstant::Type::UInt:
					assert(typeid(T) == typeid(uint32_t));
					return typeid(T) == typeid(uint32_t);
				case Shader::SpecializationConstant::Type::Int64:
					assert(typeid(T) == typeid(int64_t));
					return typeid(T) == typeid(int64_t);
				case Shader::SpecializationConstant::Type::UInt64:
					assert(typeid(T) == typeid(uint64_t));
					return typeid(T) == typeid(uint64_t);
				case Shader::SpecializationConstant::Type::Float:
					assert(typeid(T) == typeid(float));
					return typeid(T) == typeid(float);
				case Shader::SpecializationConstant::Type::Double:
					assert(typeid(T) == typeid(double));
					return typeid(T) == typeid(double);
				case Shader::SpecializationConstant::Type::Half:
					//assert(typeid(T) == typeid(Math::half));
					return true;
				case Shader::SpecializationConstant::Type::Bool:
					assert(typeid(T) == typeid(VkBool32));
					return typeid(T) == typeid(VkBool32);
				default:
					assert(false && "TODO");
					return false;
				}
			}
		};
	private:
		struct ShaderData {
			Stage stage;
			std::optional<uint32_t> workGroupSizeX;
			std::optional<uint32_t> workGroupSizeY;
			std::optional<uint32_t> workGroupSizeZ;
			std::vector<Shader::SpecializationConstant> specConstants;
			std::vector<VkSpecializationMapEntry> specConstantEntries;
			std::vector<std::byte> specConstantStorage;
		};
	public:
		Shader(Shader&) = delete;
		Shader(Shader&& other) noexcept;

		Shader& operator=(Shader&) = delete;
		Shader& operator=(Shader&& other) noexcept;

		~Shader() noexcept;

		[[nodiscard]] Stage get_stage() const noexcept;

		[[nodiscard]] ShaderInstance create_shader_instance() const noexcept;

		[[nodiscard]] static std::expected<Shader, Error> create(const LogicalDevice& device, std::span<uint32_t> shaderCode) noexcept;
	private:
		Shader(const LogicalDeviceWrapper& device, VkShaderModule handle, ShaderData shaderData) noexcept;

		ShaderData m_shaderData;
		friend class ShaderInstance;
	};

	class ShaderInstance
	{
	public:
		ShaderInstance(Shader::ShaderData data, VkShaderModule shaderModule) noexcept;
		[[nodiscard]] size_t get_num_spec_constants() const noexcept;
		template<typename T>
		[[nodiscard]] bool set_spec_constant(std::string_view name, T val) noexcept;
		template<typename T>
		[[nodiscard]] bool set_spec_constant(uint32_t id, T val) noexcept;

		[[nodiscard]] VkPipelineShaderStageCreateInfo get_shader_stage_create_info() const noexcept;
	private:

		template<typename T>
		[[nodiscard]] T& spec_constant(uint32_t id) noexcept;


		Shader::ShaderData m_data;
		VkShaderModule m_module;
		VkSpecializationInfo m_specializationInfo;
		VkPipelineShaderStageCreateInfo m_shaderStageCreateInfo;
	};


	template<typename T>
	bool ShaderInstance::set_spec_constant(std::string_view name, T val) noexcept
	{
		for (size_t i = 0; i < m_data.specConstants.size(); ++i) {
			const auto& spec = m_data.specConstants[i];
			if (name.compare(spec.name))
				continue;
			if (!Shader::SpecializationConstant::validate_type<T>(spec.type))
				return false;
			spec_constant<T>(i) = val;
			return true;
		}
		return false;
	}
	template<typename T>
	bool ShaderInstance::set_spec_constant(uint32_t id, T val) noexcept
	{

		for (size_t i = 0; i < m_data.specConstantEntries.size(); ++i) {
			const auto& spec = m_data.specConstantEntries[i];
			if (id != spec.constantID)
				continue;
			if (!Shader::SpecializationConstant::validate_type<T>(m_data.specConstants[i].type))
				return false;
			spec_constant<T>(i) = val;
			return true;
		}
		return false;
	}
	template<typename T>
	T& ShaderInstance::spec_constant(uint32_t id) noexcept
	{
		assert(id < m_data.specConstants.size());
		return *reinterpret_cast<T*>(m_data.specConstantStorage.data() + m_data.specConstants[id].offset);
	}
}
