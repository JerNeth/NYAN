#include "Renderer/DeferredLighting.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Sampler.h"
#include "Utility/Exceptions.h"
#include "entt/entt.hpp"
#include <stb_image_write.h>
#include <ctime>

nyan::DeferredLighting::DeferredLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass,
	nyan::RenderResource::Id albedoRead, nyan::RenderResource::Id normalRead, nyan::RenderResource::Id pbrRead, nyan::RenderResource::Id depthRead,
	nyan::RenderResource::Id stencilRead) :
	Renderer(device, registry, renderManager, pass),
	m_albedoRead (albedoRead),
	m_normalRead (normalRead),
	m_pbrRead (pbrRead),
	m_depthRead	(depthRead),
	m_stencilRead (stencilRead)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_deferredPipeline);

			auto [viewport, scissor] = r_device.get_swapchain_viewport_and_scissor();
			pipelineBind.set_scissor_with_count(1, &scissor);
			pipelineBind.set_viewport_with_count(1, &viewport);

			
			render(pipelineBind);
		}, true);
}

void nyan::DeferredLighting::render(vulkan::GraphicsPipelineBind& bind)
{
	const auto& ddgiManager = r_renderManager.get_ddgi_manager();
	PushConstants constants{
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
		.albedoBinding {r_pass.get_read_bind(m_albedoRead)},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.normalBinding {r_pass.get_read_bind(m_normalRead)},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.pbrBinding {r_pass.get_read_bind(m_pbrRead)},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.depthBinding {r_pass.get_read_bind(m_depthRead, nyan::Renderpass::Read::Type::ImageDepth)},
		.depthSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.stencilBinding {r_pass.get_read_bind(m_stencilRead, nyan::Renderpass::Read::Type::ImageStencil)},
		.stencilSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.ddgiBinding{ ddgiManager.get_binding() },
		.ddgiCount{ static_cast<uint32_t>(ddgiManager.slot_count()) },
		.ddgiIndex{ 0 },
	};
	bind.push_constants(constants);
	bind.draw(3, 1);
}

void nyan::DeferredLighting::create_pipeline()
{
	vulkan::GraphicsPipelineConfig pipelineConfig{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::GraphicsPipelineState{
		.polygon_mode {VK_POLYGON_MODE_FILL},
		.rasterization_samples {VK_SAMPLE_COUNT_1_BIT},
		.logic_op_enable {VK_FALSE},
		.patch_control_points {0},
		.blendAttachments {
			vulkan::BlendAttachment {
				.blend_enable {VK_FALSE},
				.color_write_mask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
			},
			vulkan::BlendAttachment {
				.blend_enable {VK_FALSE},
				.color_write_mask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
			},
		},
	},
	.vertexInputCount = 0,
	.shaderCount = 2,
	.vertexInputFormats {},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("fullscreen_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("deferredLighting_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	pipelineConfig.dynamicState.depth_write_enable = VK_FALSE;
	pipelineConfig.dynamicState.depth_test_enable = VK_FALSE;
	pipelineConfig.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	//pipelineConfig.dynamicState.stencil_test_enable = VK_TRUE;
	//pipelineConfig.dynamicState.stencil_front_reference = 0;
	//pipelineConfig.dynamicState.stencil_front_write_mask = 0xFF;
	//pipelineConfig.dynamicState.stencil_front_compare_mask = 0xFF;
	//pipelineConfig.dynamicState.stencil_front_compare_op = VK_COMPARE_OP_NOT_EQUAL;
	//pipelineConfig.dynamicState.stencil_front_fail = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_front_pass = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_front_depth_fail = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_back_reference = 0;
	//pipelineConfig.dynamicState.stencil_back_write_mask = 0xFF;
	//pipelineConfig.dynamicState.stencil_back_compare_mask = 0xFF;
	//pipelineConfig.dynamicState.stencil_back_compare_op = VK_COMPARE_OP_NOT_EQUAL;
	//pipelineConfig.dynamicState.stencil_back_fail = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_back_pass = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_back_depth_fail = VK_STENCIL_OP_KEEP;
	r_pass.add_pipeline(pipelineConfig, &m_deferredPipeline);
}

nyan::DeferredRayShadowsLighting::DeferredRayShadowsLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass,
	const GBuffer& gBuffer, const Lighting& lighting) :
	Renderer(device, registry, renderManager, pass),
	m_pipeline(device, generate_config()),
	m_gbuffer(gBuffer),
	m_lighting(lighting)
{
	r_pass.add_read(m_gbuffer.albedo);
	r_pass.add_read(m_gbuffer.normal);
	r_pass.add_read(m_gbuffer.pbr);
	r_pass.add_read(m_gbuffer.depth, nyan::Renderpass::Read::Type::ImageDepth);
	r_pass.add_read(m_gbuffer.stencil, nyan::Renderpass::Read::Type::ImageStencil);
	r_pass.add_write(m_lighting.diffuse, nyan::Renderpass::Write::Type::Compute);
	r_pass.add_write(m_lighting.specular, nyan::Renderpass::Write::Type::Compute);
	renderManager.get_ddgi_manager().add_read(pass.get_id());
	renderManager.get_ddgi_restir_manager().add_read(pass.get_id());
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_raytracing_pipeline(m_pipeline);

			render(pipelineBind);

		}, false);
}

void nyan::DeferredRayShadowsLighting::render(vulkan::RaytracingPipelineBind& bind)
{
	const auto& ddgiManager = r_renderManager.get_ddgi_manager();
	const auto& ddgiReSTIRManager = r_renderManager.get_ddgi_restir_manager();
	auto writeBindDiffuse = r_pass.get_write_bind(m_lighting.diffuse, nyan::Renderpass::Write::Type::Compute);
	auto writeBindSpecular = r_pass.get_write_bind(m_lighting.specular, nyan::Renderpass::Write::Type::Compute);
	assert(writeBindDiffuse != InvalidBinding);
	assert(writeBindSpecular != InvalidBinding);
	auto tlas = r_renderManager.get_instance_manager().get_tlas_bind();
	assert(tlas);
	if (!tlas)
		return;
	PushConstants constants{
		.accBinding {*tlas}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
		.ddgiBinding {ddgiManager.get_binding()},
		.ddgiCount {static_cast<uint32_t>(ddgiManager.slot_count())},
		.ddgiIndex {0},
		.ddgiReSTIRBinding {ddgiReSTIRManager.get_binding()},
		.ddgiReSTIRCount {static_cast<uint32_t>(ddgiReSTIRManager.slot_count())},
		.ddgiReSTIRIndex {0},
		.useDDGIReSTIR {m_useDDGIReSTIR},
		.albedoBinding {r_pass.get_read_bind(m_gbuffer.albedo)},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.normalBinding {r_pass.get_read_bind(m_gbuffer.normal)},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.pbrBinding {r_pass.get_read_bind(m_gbuffer.pbr)},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.depthBinding {r_pass.get_read_bind(m_gbuffer.depth, nyan::Renderpass::Read::Type::ImageDepth)},
		.depthSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.stencilBinding {r_pass.get_read_bind(m_gbuffer.stencil, nyan::Renderpass::Read::Type::ImageStencil)},
		.stencilSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.diffuseImageBinding {writeBindDiffuse},
		.specularImageBinding {writeBindSpecular},
		
	};
	const auto& writeSpecular = r_pass.get_graph().get_resource(m_lighting.specular);
	const auto& attachment = std::get<ImageAttachment>(writeSpecular.attachment);
	uint32_t width {0}, height {0};
	if (attachment.size == ImageAttachment::Size::Swapchain) {
		width = static_cast<uint32_t>(attachment.width * r_device.get_swapchain_width());
		height = static_cast<uint32_t>(attachment.height * r_device.get_swapchain_height());
	}
	else {
		width = static_cast<uint32_t>(attachment.width);
		height = static_cast<uint32_t>(attachment.height);
	}
	bind.push_constants(constants);
	bind.trace_rays(m_pipeline, width, height, 1);

}

void nyan::DeferredRayShadowsLighting::set_use_ddgi_restir(bool use)
{
	m_useDDGIReSTIR = use;
}

vulkan::RaytracingPipelineConfig nyan::DeferredRayShadowsLighting::generate_config()
{
	return vulkan::RaytracingPipelineConfig{
		.rgenGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_deferred_rgen")},
			},
		},
		.hitGroups {
			vulkan::Group
			{
				.anyHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_alpha_test_rahit")},
			},
		},
		.missGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_shadows_rmiss")},
			},
		},
		.recursionDepth {1},
		.pipelineLayout = r_device.get_bindless_pipeline_layout()
	};
}

nyan::LightComposite::LightComposite(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass
		, const Lighting& lighting) :
	Renderer(device, registry, renderManager, pass),
	m_lighting(lighting)
{
	r_pass.add_read(m_lighting.diffuse);
	r_pass.add_read(m_lighting.specular);
	r_pass.add_swapchain_attachment(Math::vec4{}, true);
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_compositePipeline);

			auto [viewport, scissor] = r_device.get_swapchain_viewport_and_scissor();
			pipelineBind.set_scissor_with_count(1, &scissor);
			pipelineBind.set_viewport_with_count(1, &viewport);

			render(pipelineBind);
		}, true);
		pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
			{
				if (m_dumpToDisk) {
					m_dumpToDisk = false;
					auto* data = (*m_screenshotBuffer)->map_data();
					std::vector<uint32_t> convData(m_screenshotWidth * m_screenshotHeight);
					if (m_screenshotFormat == VK_FORMAT_A2B10G10R10_UNORM_PACK32) {
						for (size_t i = 0; i < convData.size(); ++i) {
							uint32_t A2BGR10 = reinterpret_cast<uint32_t*>(data)[i];
							uint32_t R8 = static_cast<uint8_t>(((A2BGR10 >> 0) & (0x3FF)) * (1.f / 1023.f) * 255.f);
							uint32_t G8 = static_cast<uint8_t>(((A2BGR10 >> 10) & (0x3FF)) * (1.f / 1023.f) * 255.f);
							uint32_t B8 = static_cast<uint8_t>(((A2BGR10 >> 20) & (0x3FF)) * (1.f / 1023.f) * 255.f);
							uint32_t A8 = static_cast<uint8_t>(((A2BGR10 >> 30) & (0x3)) * (1.f / 3.f) * 255.f);
							uint32_t RGBA8 = (A8 << (24)) |
								(B8 << (16)) |
								(G8 << (8)) |
								(R8 << (0));
							convData[i] = RGBA8;
						}
					}
					else if (m_screenshotFormat == VK_FORMAT_B8G8R8A8_UNORM){
						for (size_t i = 0; i < convData.size(); ++i) {
							uint32_t B8G8R8A8 = reinterpret_cast<uint32_t*>(data)[i];
							uint32_t R8 = (B8G8R8A8 >> 16) & (0xFF);
							uint32_t G8 = (B8G8R8A8 >> 8) & (0xFF);
							uint32_t B8 = (B8G8R8A8 >> 0) & (0xFF);
							uint32_t A8 = (B8G8R8A8 >> 24) & (0xFF);
							uint32_t RGBA8 = (A8 << (24)) |
								(B8 << (16)) |
								(G8 << (8)) |
								(R8 << (0));
							convData[i] = RGBA8;
						}
					}
					else {
						return;
					}
					time_t rawtime;
					struct tm* timeinfo;
					char buffer[80];

					time(&rawtime);
					timeinfo = localtime(&rawtime);

					//strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S.png", timeinfo);
					strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S.bmp", timeinfo);
					std::string str(buffer);
					if (m_seriesOffset <= (m_seriesCounter -1 ) && (m_seriesCounter - 1) < (m_seriesMax + m_seriesOffset))
						str = (m_seriesFolder / (std::to_string(m_seriesCounter - 1) + ".bmp")).string();
						//str = (m_seriesFolder / (std::to_string(m_seriesCounter - 1) + ".png")).string();
					//str = "Screenshot_" + str + ".png";
					//str = "Screenshot.png";
					//stbi_write_png(str.c_str(), m_screenshotWidth, m_screenshotHeight, 4, convData.data(), m_screenshotWidth * 4);
					stbi_write_bmp(str.c_str(), m_screenshotWidth, m_screenshotHeight, 4, convData.data());
				}
				if (m_screenshot || (m_seriesCounter <( m_seriesMax + m_seriesOffset)))
				{
					if (m_seriesCounter < (m_seriesMax + m_seriesOffset))
						m_seriesCounter++;
					else
						m_screenshot = false;
					if (m_seriesCounter < m_seriesOffset)
						return;

					m_dumpToDisk = true;
					auto& swapchain = r_pass.get_graph().get_resource(r_pass.get_graph().get_swapchain_resource());
					auto& img = *swapchain.handle;
					std::vector copy{ VkBufferImageCopy{
						.bufferOffset {0},
						.bufferRowLength {0},
						.bufferImageHeight {0},
						.imageSubresource {
							.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT},
							.mipLevel {0},
							.baseArrayLayer {0},
							.layerCount {1}
						},
						.imageOffset {0, 0, 0},
						.imageExtent {img.get_info().width, img.get_info().height, img.get_info().depth},
					} };
					cmd.barrier2(VkImageMemoryBarrier2{
						.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
						.pNext {nullptr},
						.srcStageMask {VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT},
						.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
						.dstStageMask {VK_PIPELINE_STAGE_2_TRANSFER_BIT},
						.dstAccessMask {VK_ACCESS_2_MEMORY_READ_BIT},
						.oldLayout{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
						.newLayout{VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
						.image{img},
						.subresourceRange{
							.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT},
							.baseMipLevel {0},
							.levelCount {VK_REMAINING_MIP_LEVELS},
							.baseArrayLayer {0},
							.layerCount {VK_REMAINING_ARRAY_LAYERS}
						}
						});
					m_screenshotFormat = img.get_format();
					m_screenshotWidth = img.get_info().width;
					m_screenshotHeight = img.get_info().height;
					auto bufferSize = vulkan::format_block_size(img.get_format()) * img.get_info().width * img.get_info().height * img.get_info().depth;
					if (!m_screenshotBuffer || (*m_screenshotBuffer)->get_size() < bufferSize) {
						m_screenshotBuffer = std::make_unique<vulkan::BufferHandle>(r_device.create_buffer(vulkan::BufferInfo{
							.size {bufferSize},
							.usage {VK_BUFFER_USAGE_TRANSFER_DST_BIT },
							.offset {0},
							.memoryUsage {VMA_MEMORY_USAGE_CPU_ONLY},
							}, {}));
					}
					cmd.copy_image_to_buffer(img, *m_screenshotBuffer, copy, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

					cmd.barrier2(VkImageMemoryBarrier2{
						.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
						.pNext {nullptr},
						.srcStageMask {VK_PIPELINE_STAGE_2_TRANSFER_BIT},
						.srcAccessMask {VK_ACCESS_2_MEMORY_READ_BIT},
						.dstStageMask {VK_PIPELINE_STAGE_2_TRANSFER_BIT},
						.dstAccessMask {VK_ACCESS_2_NONE},
						.oldLayout{VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
						.newLayout{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
						.image{img},
						.subresourceRange{
							.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT},
							.baseMipLevel {0},
							.levelCount {VK_REMAINING_MIP_LEVELS},
							.baseArrayLayer {0},
							.layerCount {VK_REMAINING_ARRAY_LAYERS}
						}
						});
				}
			}, false);
}

void nyan::LightComposite::render(vulkan::GraphicsPipelineBind& bind)
{
	PushConstants constants{
		.specularBinding {r_pass.get_read_bind(m_lighting.specular)},
		.specularSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.diffuseBinding {r_pass.get_read_bind(m_lighting.diffuse)},
		.diffuseSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.tonemapping {static_cast<uint32_t>(m_tonemappingType)}
	};
	bind.push_constants(constants);
	bind.draw(3, 1);
}

void nyan::LightComposite::set_tonemapping(ToneMapping type)
{
	m_tonemappingType = type;
}

void nyan::LightComposite::queue_screenshot()
{
	m_screenshot = true;
}

void nyan::LightComposite::queue_recording(const std::filesystem::path& folder, uint32_t count, uint32_t recordingOffset)
{
	m_seriesFolder = folder;
	m_seriesMax = count;
	m_seriesCounter = 0;
	m_seriesOffset = recordingOffset;
}

void nyan::LightComposite::create_pipeline()
{
	vulkan::GraphicsPipelineConfig pipelineConfig{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::defaultGraphicsPipelineState,
	.vertexInputCount = 0,
	.shaderCount = 2,
	.vertexInputFormats {},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("fullscreen_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("composite_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	pipelineConfig.dynamicState.depth_write_enable = VK_FALSE;
	pipelineConfig.dynamicState.depth_test_enable = VK_FALSE;
	pipelineConfig.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	r_pass.add_pipeline(pipelineConfig, &m_compositePipeline);
}
