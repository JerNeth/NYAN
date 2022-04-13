#ifndef VKPIPELINE_H
#define VKPIPELINE_H
#pragma once
#include "VulkanIncludes.h"
#include <Util>
#include "Shader.h"
#include "DescriptorSet.h"
#include "Renderpass.h"

namespace vulkan {
	class Renderpass;
	class LogicalDevice;
	constexpr unsigned BLEND_FACTOR_BITS = Utility::bit_width(VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);
	constexpr unsigned BLEND_OP_BITS = Utility::bit_width(VK_BLEND_OP_MAX);
	constexpr unsigned WRITE_MASK_BITS = Utility::bit_width(VK_COLOR_COMPONENT_A_BIT);
	constexpr unsigned CULL_MODE_BITS = Utility::bit_width(VK_CULL_MODE_FRONT_AND_BACK);
	constexpr unsigned FRONT_FACE_BITS = Utility::bit_width(VK_FRONT_FACE_CLOCKWISE);
	constexpr unsigned COMPARE_OP_BITS = Utility::bit_width(VK_COMPARE_OP_ALWAYS);
	constexpr unsigned STENCIL_OP_BITS = Utility::bit_width(VK_STENCIL_OP_DECREMENT_AND_WRAP);
	constexpr unsigned TOPOLOGY_BITS = Utility::bit_width(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
	constexpr unsigned LOGIC_OP_BITS = Utility::bit_width(VK_LOGIC_OP_SET);
	constexpr unsigned POLYGON_MODE_BITS = Utility::bit_width(VK_POLYGON_MODE_POINT);
	constexpr unsigned RASTERIZATION_SAMPLE_BITS = Utility::bit_width(VK_SAMPLE_COUNT_16_BIT);
	class Shader;
	struct BlendAttachment {
		VkBool32 blend_enable : 1;
		VkBlendFactor src_color_blend : BLEND_FACTOR_BITS;
		VkBlendFactor dst_color_blend : BLEND_FACTOR_BITS;
		VkBlendOp color_blend_op : BLEND_OP_BITS; 
		VkBlendFactor src_alpha_blend : BLEND_FACTOR_BITS;
		VkBlendFactor dst_alpha_blend : BLEND_FACTOR_BITS;
		VkBlendOp alpha_blend_op : BLEND_OP_BITS;
		VkColorComponentFlags color_write_mask : WRITE_MASK_BITS;
	};

	struct DynamicGraphicsPipelineState {
		VkBool32 depth_write_enable : 1;
		VkBool32 depth_test_enable : 1;
		VkBool32 depth_bias_enable : 1;
		VkBool32 depth_bounds_test_enable : 1;
		VkCompareOp depth_compare_op : COMPARE_OP_BITS;

		VkBool32 stencil_test_enable : 1;
		VkStencilOp stencil_front_fail : STENCIL_OP_BITS;
		VkStencilOp stencil_front_pass : STENCIL_OP_BITS;
		VkStencilOp stencil_front_depth_fail : STENCIL_OP_BITS;
		VkCompareOp stencil_front_compare_op : COMPARE_OP_BITS;

		VkStencilOp stencil_back_fail : STENCIL_OP_BITS;
		VkStencilOp stencil_back_pass : STENCIL_OP_BITS;
		VkStencilOp stencil_back_depth_fail : STENCIL_OP_BITS;
		VkCompareOp stencil_back_compare_op : COMPARE_OP_BITS;

		VkCullModeFlags cull_mode : CULL_MODE_BITS;
		VkFrontFace front_face : 2;
		VkBool32 primitive_restart_enable : 1;
		VkBool32 rasterizer_discard_enable : 1;
		VkPrimitiveTopology primitive_topology : TOPOLOGY_BITS;
	};

	struct GraphicsPipelineState {
		VkPolygonMode polygon_mode : POLYGON_MODE_BITS;

		VkSampleCountFlagBits rasterization_samples : RASTERIZATION_SAMPLE_BITS;
		VkBool32 alpha_to_coverage : 1;
		VkBool32 alpha_to_one : 1;
		VkBool32 sample_shading : 1;

		VkBool32 logic_op_enable : 1;
		VkLogicOp logic_op : LOGIC_OP_BITS;

		//All devices supporting tessellation use 32 as max value
		uint32_t patch_control_points : Utility::bit_width(32);

		std::array<BlendAttachment, MAX_ATTACHMENTS> blendAttachments{};

		friend bool operator==(const GraphicsPipelineState& left, const GraphicsPipelineState& right) {
			return memcmp(&left, &right, sizeof(GraphicsPipelineState)) == 0;
		}
	};
	constexpr DynamicGraphicsPipelineState defaultDynamicGraphicsPipelineState{
		.depth_write_enable {VK_TRUE},
		.depth_test_enable {VK_TRUE},
		.depth_bias_enable {VK_FALSE},
		.depth_bounds_test_enable {VK_FALSE},
		.depth_compare_op {VK_COMPARE_OP_LESS},

		.stencil_test_enable {VK_FALSE},
		.stencil_front_fail {},
		.stencil_front_pass {},
		.stencil_front_depth_fail {},
		.stencil_front_compare_op {},

		.stencil_back_fail {},
		.stencil_back_pass {},
		.stencil_back_depth_fail {},
		.stencil_back_compare_op {},

		.cull_mode {VK_CULL_MODE_BACK_BIT},
		.front_face {VK_FRONT_FACE_COUNTER_CLOCKWISE},
		.primitive_restart_enable {VK_FALSE},
		.rasterizer_discard_enable {VK_FALSE},
		.primitive_topology {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
	};
	constexpr GraphicsPipelineState defaultGraphicsPipelineState{
		.polygon_mode {VK_POLYGON_MODE_FILL},
		.rasterization_samples {VK_SAMPLE_COUNT_1_BIT},
		.logic_op_enable {VK_FALSE},
		.patch_control_points {0},
		.blendAttachments {
			BlendAttachment {
				.blend_enable {VK_FALSE},
				.src_color_blend {},
				.color_write_mask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
			}
		}
	};
	constexpr GraphicsPipelineState alphaBlendedGraphicsPipelineState{
		.polygon_mode {VK_POLYGON_MODE_FILL},
		.rasterization_samples {VK_SAMPLE_COUNT_1_BIT},
		.logic_op_enable {VK_FALSE},
		.patch_control_points {0},
		.blendAttachments {
			BlendAttachment {
				.blend_enable {VK_TRUE},
				.src_color_blend {VK_BLEND_FACTOR_SRC_ALPHA},
				.dst_color_blend {VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
				.color_blend_op {VK_BLEND_OP_ADD},
				.src_alpha_blend {VK_BLEND_FACTOR_SRC_ALPHA},
				.dst_alpha_blend {VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
				.alpha_blend_op {VK_BLEND_OP_ADD},
				.color_write_mask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
			}
		}
	};
	// Blend Logic Pseudo Code
	//if (blendEnable) {
	//	finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
	//	finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
	//}
	//else {
	//	finalColor = newColor;
	//}

	//finalColor = finalColor & colorWriteMask;
	struct RenderingCreateInfo {
		uint32_t colorAttachmentCount;
		uint32_t viewMask = 0;
		std::array<VkFormat, MAX_ATTACHMENTS> colorAttachmentFormats{ VK_FORMAT_UNDEFINED };
		VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	};
	struct GraphicsPipelineConfig {
		DynamicGraphicsPipelineState dynamicState;
		GraphicsPipelineState state;
		RenderingCreateInfo renderingCreateInfo;
		uint16_t vertexInputCount;
		uint16_t shaderCount;
		//Relevant vertex formats only go up to ~128
		std::array<uint8_t, MAX_VERTEX_INPUTS> vertexInputFormats;
		//Five 
		std::array<ShaderId, 5> shaderInstances;
		VkPipelineLayout pipelineLayout;
	};
	struct ComputePipelineConfig {
		ShaderId shaderInstance;
		VkPipelineLayout pipelineLayout;
	};
	struct PipelineState {
		unsigned depth_write : 1;
		unsigned depth_test : 1;
		unsigned blend_enable : 1;
		unsigned cull_mode : CULL_MODE_BITS;
		unsigned front_face : FRONT_FACE_BITS;
		unsigned depth_bias_enable : 1;
		//7
		
		unsigned dynamic_cull_mode : 1;
		unsigned dynamic_front_face : 1;
		unsigned dynamic_primitive_topology : 1;
		unsigned dynamic_vertex_input_binding_stride : 1;
		unsigned dynamic_depth_test : 1;
		unsigned dynamic_depth_write : 1;
		unsigned dynamic_depth_compare : 1;
		unsigned dynamic_depth_bounds_test : 1;
		unsigned dynamic_stencil_test : 1;
		unsigned dynamic_stencil_op : 1;
		unsigned dynamic_depth_bias_enable : 1;
		unsigned depth_compare : COMPARE_OP_BITS;
		//21
		
		unsigned stencil_test : 1;
		unsigned stencil_front_fail : STENCIL_OP_BITS;
		unsigned stencil_front_pass : STENCIL_OP_BITS;
		unsigned stencil_front_depth_fail : STENCIL_OP_BITS;
		unsigned alpha_to_coverage : 1;
		//32
		unsigned stencil_front_compare_op : COMPARE_OP_BITS;
		unsigned stencil_back_fail : STENCIL_OP_BITS;
		unsigned stencil_back_pass : STENCIL_OP_BITS;
		unsigned stencil_back_depth_fail : STENCIL_OP_BITS;
		unsigned stencil_back_compare_op : COMPARE_OP_BITS;
		
		unsigned alpha_to_one : 1;
		
		unsigned src_color_blend : BLEND_FACTOR_BITS;
		unsigned dst_color_blend : BLEND_FACTOR_BITS;
		unsigned color_blend_op : BLEND_OP_BITS;
		unsigned sample_shading : 1;
		//64

		unsigned src_alpha_blend : BLEND_FACTOR_BITS;
		unsigned dst_alpha_blend : BLEND_FACTOR_BITS;
		unsigned alpha_blend_op : BLEND_OP_BITS;		
		unsigned primitive_restart : 1;
		unsigned topology : TOPOLOGY_BITS;	

		
		unsigned polygon_mode : 2;
		unsigned subgroup_control_size : 1;
		unsigned subgroup_full_group : 1;
		unsigned subgroup_min_size_log2 : 3;
		unsigned subgroup_max_size_log2 : 3;
		unsigned conservative_raster : 1;

		//Put here because of alignment
		unsigned dynamic_primitive_restart : 1;
		unsigned dynamic_rasterizer_discard : 1;
		unsigned dynamic_vertex_input : 1;
		//96
		unsigned color_write_mask : WRITE_MASK_BITS * MAX_ATTACHMENTS;
		//
		friend bool operator==(const PipelineState& left, const PipelineState& right) {
			return memcmp(&left, &right, sizeof(PipelineState))== 0;
		}
	};
	static_assert(sizeof(PipelineState) == 16, "Something wrong with PipelineState");
	constexpr PipelineState defaultPipelineState {
		.depth_write = VK_TRUE,
		.depth_test = VK_TRUE,
		.blend_enable = VK_FALSE,
		.cull_mode = VK_CULL_MODE_BACK_BIT,
		//.cull_mode = VK_CULL_MODE_NONE,
		.front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depth_bias_enable = VK_FALSE,
		.dynamic_cull_mode = 0,
		.dynamic_front_face = 0,
		.dynamic_primitive_topology = 0,
		.dynamic_vertex_input_binding_stride = 0,
		.dynamic_depth_test = 0,
		.dynamic_depth_write = 0,
		.dynamic_depth_compare = 0,
		.dynamic_depth_bounds_test = 0,
		.dynamic_stencil_test = 0,
		.dynamic_stencil_op = 0,
		.dynamic_depth_bias_enable = 0,
		.depth_compare = VK_COMPARE_OP_LESS,
		.stencil_test = VK_FALSE,
		.alpha_to_coverage = VK_FALSE,
		.alpha_to_one = VK_FALSE,
		.src_color_blend = VK_BLEND_FACTOR_ONE,
		.dst_color_blend = VK_BLEND_FACTOR_ZERO,
		.color_blend_op = VK_BLEND_OP_ADD,
		.sample_shading = VK_TRUE,
		.src_alpha_blend = VK_BLEND_FACTOR_ONE,
		.dst_alpha_blend = VK_BLEND_FACTOR_ZERO,
		.alpha_blend_op = VK_BLEND_OP_ADD,
		.primitive_restart = VK_FALSE,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.polygon_mode = VK_POLYGON_MODE_FILL,
		.subgroup_control_size = VK_FALSE,
		.subgroup_full_group = VK_FALSE,
		.subgroup_min_size_log2 = 0,
		.subgroup_max_size_log2 = 0,
		.conservative_raster = 0,
		.dynamic_primitive_restart = 0,
		.dynamic_rasterizer_discard = 0,
		.dynamic_vertex_input = 0,
		.color_write_mask = (1ull<<(WRITE_MASK_BITS * MAX_ATTACHMENTS))-1ull,
	};
	struct VertexAttributes {
		std::array<VkFormat, MAX_VERTEX_ATTRIBUTES> formats;
		std::array<uint8_t, MAX_VERTEX_ATTRIBUTES> bindings;
		std::pair<VkFormat, uint8_t> operator[](size_t idx) {
			assert(idx < MAX_VERTEX_ATTRIBUTES);
			return {formats[idx], bindings[idx]};
		}

		std::pair<VkFormat, uint8_t> operator[](size_t idx) const {
			assert(idx < MAX_VERTEX_ATTRIBUTES);
			return { formats[idx], bindings[idx] };
		}
		friend bool operator==(const VertexAttributes& lhs, const VertexAttributes& rhs) {
			return lhs.formats == rhs.formats && lhs.bindings == rhs.bindings;
		}
	};
	struct InputRates {
		void set(size_t idx, VkVertexInputRate rate) {
			if(rate == VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE)
				rates.set(idx);
			else
				rates.reset(idx);
		}
		VkVertexInputRate operator[](size_t idx) const {
			return static_cast<VkVertexInputRate>(rates.test(idx));
		}
		friend bool operator==(const InputRates& lhs, const InputRates& rhs) {
			return lhs.rates == rhs.rates;
		}
	private:
		std::bitset<MAX_VERTEX_BINDINGS> rates;
	};
	struct PipelineCompile {
		PipelineState state;
		const Program* program = nullptr;
		Renderpass* compatibleRenderPass = nullptr;
		VertexAttributes attributes{};
		InputRates inputRates;

		uint32_t subpassIndex = 0;
		friend bool operator==(const PipelineCompile& lhs, const PipelineCompile& rhs) {
			if (!lhs.program || !rhs.program)
				return false;
			if (!lhs.compatibleRenderPass || !rhs.compatibleRenderPass)
				return false;
			return (lhs.program->get_hash() == rhs.program->get_hash()) &&
					(lhs.compatibleRenderPass->get_compatible_hash() == rhs.compatibleRenderPass->get_compatible_hash()) &&
					//(lhs.attributes == rhs.attributes) &&
					//(lhs.inputRates == rhs.inputRates) &&
					(lhs.subpassIndex == rhs.subpassIndex);
		}
	};
	struct PipelineCompileHasher {
		size_t operator()(const PipelineCompile& compile) const {
			Utility::Hasher h;
			h(compile.state);
			h(compile.program->get_hash());
			if(compile.compatibleRenderPass)
				h(compile.compatibleRenderPass->get_compatible_hash());
			h(compile.subpassIndex);
			if (!compile.state.dynamic_vertex_input) {
				h(compile.attributes);
				h(compile.inputRates);
			}
			//Dont't hash pipeline layout since it depends entirely on the shaders, which we already hashed

			return h();
		}
	};
	class PipelineLayout {
	public:
		PipelineLayout(LogicalDevice& parent, const ShaderLayout& layout);
		~PipelineLayout();
		const VkPipelineLayout& get_layout() const;
		const ShaderLayout& get_shader_layout() const;
		const DescriptorSetAllocator* get_allocator(size_t set) const;
		DescriptorSetAllocator* get_allocator(size_t set);
		const VkDescriptorUpdateTemplate& get_update_template(size_t set) const;
		Utility::HashValue get_hash() const noexcept {
			return m_hashValue;
		}
		
	private:
		void create_update_template();
		LogicalDevice& r_device;
		ShaderLayout m_shaderLayout;
		std::array<DescriptorSetAllocator*, MAX_DESCRIPTOR_SETS> m_descriptors{};
		std::array<VkDescriptorUpdateTemplate, MAX_DESCRIPTOR_SETS> m_updateTemplate{};
		VkPipelineLayout m_layout = VK_NULL_HANDLE;
		Utility::HashValue m_hashValue;
	};
	class PipelineLayout2 {
	public:
		PipelineLayout2(LogicalDevice& device, const std::vector<VkDescriptorSetLayout>& sets);
		~PipelineLayout2();
		PipelineLayout2(PipelineLayout2&) = delete;
		PipelineLayout2(PipelineLayout2&&) = delete;
		PipelineLayout2& operator=(PipelineLayout2&) = delete;
		PipelineLayout2& operator=(PipelineLayout2&&) = delete;

		operator VkPipelineLayout() const;
		VkPipelineLayout get_layout() const noexcept;

	private:
		LogicalDevice& r_device;
		VkPipelineLayout m_layout{ VK_NULL_HANDLE };
	};

	class Pipeline {
	public:
		//Implicit compute
		Pipeline(LogicalDevice& parent, const Program& program);
		//Implicit graphics pipeline
		Pipeline(LogicalDevice& parent, const PipelineCompile& compiled);

		Pipeline(Pipeline& other) = default;
		Pipeline(Pipeline&& other) = default;
		Pipeline& operator=(const Pipeline& other) = default;
		Pipeline& operator=(Pipeline&& other) = default;
		VkPipeline get_pipeline() const noexcept;

	private:
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_layout = VK_NULL_HANDLE;
		static PipelineState s_pipelineState;
	};

	class PipelineStorage {
	public:
		PipelineStorage(LogicalDevice& device);
		PipelineStorage(PipelineStorage&) = delete;
		PipelineStorage(PipelineStorage&&) = delete;
		PipelineStorage& operator=(PipelineStorage&) = delete;
		PipelineStorage& operator=(PipelineStorage&&) = delete;
		~PipelineStorage();
		VkPipeline request_pipeline(const PipelineCompile& compile);
		VkPipeline request_pipeline(const Program& program);
	private:
		LogicalDevice& r_device;
		std::unordered_map<PipelineCompile, Pipeline, PipelineCompileHasher> m_hashMap;
	};

	class PipelineCache {
	public:
		PipelineCache(LogicalDevice& device, const std::string& path);
		PipelineCache(PipelineCache&) = delete;
		PipelineCache(PipelineCache&&) = delete;
		PipelineCache& operator=(PipelineCache&) = delete;
		PipelineCache& operator=(PipelineCache&&) = delete;
		~PipelineCache() noexcept;
		VkPipelineCache get_handle() const noexcept;
	private:
		LogicalDevice& r_parent;
		std::string m_path;
		VkPipelineCache m_handle;
	};
	class Pipeline2 {
	public:
		Pipeline2(LogicalDevice& parent, const GraphicsPipelineConfig& config);
		Pipeline2(LogicalDevice& parent, const ComputePipelineConfig& config);
		VkPipeline get_pipeline() const noexcept;
		VkPipelineLayout get_layout() const noexcept;
		const DynamicGraphicsPipelineState& get_dynamic_state() const noexcept;
	private:
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_layout = VK_NULL_HANDLE;
		VkPipelineBindPoint m_type;
		DynamicGraphicsPipelineState m_initialDynamicState;
	};

	using PipelineId = uint32_t;
	class PipelineStorage2 {
	public:
		PipelineStorage2(LogicalDevice& device);
		~PipelineStorage2();
		Pipeline2* get_pipeline(PipelineId pipelineId);
		const Pipeline2* get_pipeline(PipelineId pipelineId) const;
		PipelineId add_pipeline(const ComputePipelineConfig& config);
		PipelineId add_pipeline(const GraphicsPipelineConfig& config);
	private:
		LogicalDevice& r_device;
		Utility::LinkedBucketList<Pipeline2> m_pipelines;
	};

	class PipelineBind {
	public:
		PipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint);
		void bind_descriptor_sets(uint32_t firstSet, const std::vector<VkDescriptorSet>& descriptorSets,
			const std::vector<uint32_t>& dynamicOffsets = {});
		//void bind_descriptor_set(DescriptorSet)
		template<typename T>
		void push_constants(const T& t) {
			vkCmdPushConstants(m_cmd, m_layout, VK_SHADER_STAGE_ALL, 0, sizeof(T), &t);
		}
	protected:
		VkCommandBuffer m_cmd;
		VkPipelineLayout m_layout;
		VkPipelineBindPoint m_bindPoint;
	};

	class GraphicsPipelineBind : public PipelineBind {
	public:
		GraphicsPipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint);
		void set_scissor(VkRect2D scissor);
		void set_viewport(VkViewport viewport);
		void bind_vertex_buffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers, const VkDeviceSize* offsets);
		void bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
		void draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0);
		//void draw();
		//
		//void set_something_dynamic();
	private:

	};
	class ComputePipelineBind : public PipelineBind {
	public:
		ComputePipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint);
		void dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
		//void dispatch
	private:

	};
	class RaytracingPipelineBind : public PipelineBind {
	public:
		RaytracingPipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint);
		//void traceRays
	private:
	};

}

#endif