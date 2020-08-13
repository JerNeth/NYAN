#ifndef VKLOGICALDEVICE_H
#define VKLOGICALDEVICE_H
#pragma once
#include "VulkanIncludes.h"
#include <vector>
#include <array>
#include <fstream>
#include <assert.h>
#include <unordered_map>
#include "Utility.h"
#include "LinAlg.h"
#include "Shader.h"
#include "Instance.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
namespace Vulkan {
	class Instance; 
	class Program;
	struct ShaderLayout;
	class PipelineLayout;
	struct Vertex {
		std::array<float, 3> pos;
		std::array<float, 3> color;
		std::array<float, 2> texcoords;
		static std::array<VkVertexInputBindingDescription, 1> get_binding_descriptions() {
			std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{ 
				VkVertexInputBindingDescription{
					.binding = 0,
					.stride = sizeof(Vertex),
					.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
				}
			};
			return bindingDescriptions;
		}
		static std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{
				VkVertexInputAttributeDescription{
					.location = 0,
					.binding = 0,
					.format = VK_FORMAT_R32G32B32_SFLOAT,
					.offset = offsetof(Vertex, pos)
				},
				VkVertexInputAttributeDescription{
					.location = 1,
					.binding = 0,
					.format = VK_FORMAT_R32G32B32_SFLOAT,
					.offset = offsetof(Vertex, color)
				},
				VkVertexInputAttributeDescription{
					.location = 2,
					.binding = 0,
					.format = VK_FORMAT_R32G32_SFLOAT,
					.offset = offsetof(Vertex, texcoords)
				},
			};
			return attributeDescriptions;
		}
	};
	constexpr std::array<Vertex, 8> vertices{
		Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		Vertex{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		Vertex{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		Vertex{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		Vertex{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		Vertex{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};
	constexpr std::array<uint16_t, 12> indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
	struct Ubo {
		bla::mat44 model;
		bla::mat44 view;
		bla::mat44 proj;
	};
	constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
	class LogicalDevice {
		friend class PipelineLayout;
		friend class Shader;
		friend class Pipeline;
		friend class Renderpass;
		friend class DescriptorSetAllocator;
		
	public:

		LogicalDevice(const Instance& parentInstance, VkDevice device, uint32_t graphicsQueueFamilyIndex, VkPhysicalDeviceProperties& properties);
		~LogicalDevice();
		LogicalDevice(LogicalDevice&) = delete;
		LogicalDevice& operator=(LogicalDevice&) = delete;
		void draw_frame();
		void wait_idle();
		void create_swap_chain();
		void create_sync_objects();
		void recreate_swap_chain();
		void create_texture_image(uint32_t width, uint32_t height, uint32_t channels, char* imageData);
		DescriptorSetAllocator* request_descriptor_set_allocator(const DescriptorSetLayout& layout);
		void register_shader(const std::string& shaderName, const std::vector<uint32_t>& shaderCode);
		Shader* request_shader(const std::string& shaderName, const std::vector<uint32_t>& shaderCode);
		Shader* request_shader(const std::string& shaderName) const;
		Program* request_program(const std::vector<Shader*>& shaders);
		PipelineLayout* request_pipeline_layout(const ShaderLayout& layout);
		void create_program();
	private:
		std::pair<VkBuffer, VmaAllocation> create_buffer(VkDeviceSize size, VkBufferUsageFlags  usage, VmaMemoryUsage memoryUsage);
		void cleanup_swapchain();
		void copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		VkCommandBuffer begin_single_time_commands();
		void end_single_time_commands(VkCommandBuffer commandBuffer);
		void create_texture_image_view();
		void create_texture_sampler();
		void create_depth_resources();
		void create_vertex_buffer();
		void create_index_buffer();
		void create_uniform_buffers();
		void create_swapchain();
		template<typename VertexType, size_t numShaders>
		void create_graphics_pipeline(std::array<Shader*, numShaders> shaders);
		void create_image_views();
		VkImageView create_image_view(VkFormat format, VkImage image, VkImageAspectFlags aspect);
		void create_render_pass();
		void create_framebuffers();
		void create_command_pool();
		void create_command_buffers();
		void create_vma_allocator();
		template<size_t numBindings>
		void create_descriptor_set_layout(std::array<VkDescriptorSetLayoutBinding, numBindings> bindings);
		void create_descriptor_sets();
		void create_descriptor_pool();
		std::pair< VkImage, VmaAllocation> create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags flags, VmaMemoryUsage memoryUsage);
		
		void update_uniform_buffer(uint32_t currentImage);

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		

		VkDevice m_device;
		const Instance& r_instance;
		VkAllocationCallbacks* m_allocator = NULL;
		const uint32_t m_graphicsFamilyQueueIndex;
		VkPhysicalDeviceProperties m_physicalProperties;

		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_swapChainImages;
		VkExtent2D m_swapChainExtent;
		VkFormat m_swapChainImageFormat;
		std::vector<VkImageView> m_swapChainImageViews;
		VkPipelineLayout m_pipelineLayout;
		VkRenderPass m_renderPass;
		VkPipeline m_graphicsPipeline;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;
		VkCommandPool m_commandPool;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;

		std::vector<VkBuffer> m_uniformBuffers;
		std::vector<VmaAllocation> m_uniformBuffersAllocations;

		VkBuffer m_vertexBuffer;
		VmaAllocation m_vertexBufferAllocation;
		VkBuffer m_indexBuffer;
		VmaAllocation m_indexBufferAllocation;
		VkImage m_image;
		VmaAllocation m_imageAllocation;
		VkImageView m_imageView;
		VkSampler m_imageSampler;

		VkImage m_depthImage;
		VmaAllocation m_depthImageAllocation;
		VkImageView m_depthImageView;

		VmaAllocator m_vmaAllocator;

		std::vector<VkCommandBuffer> m_commandBuffers;
		std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
		std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;
		std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame = 0;

		Program* m_program;
		VkQueue m_graphicsQueue;

		std::unordered_map< DescriptorSetLayout, size_t, Utility::Hash<DescriptorSetLayout>> m_descriptorAllocatorIds;
		Utility::LinkedBucketList<DescriptorSetAllocator> m_descriptorAllocatorsStorage;

		std::unordered_map< std::string, size_t> m_shaderIds;
		Utility::LinkedBucketList<Shader> m_shaderStorage;

		std::unordered_map< std::vector<Shader*>, size_t, Utility::VectorHash<Shader*>> m_programIds;
		Utility::LinkedBucketList<Program> m_programStorage;

		std::unordered_map< ShaderLayout, size_t, Utility::Hash<ShaderLayout>> m_pipelineLayoutIds;
		Utility::LinkedBucketList<PipelineLayout> m_pipelineLayoutStorage;
	};
}
#endif // VKLOGICALDEVICE_H