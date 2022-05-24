#include "Renderer/ImguiRenderer.h"
#include "entt/entt.hpp"
using namespace vulkan;

namespace MM {
	template <>
	void ComponentEditorWidget<nyan::Transform>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& t = reg.get<nyan::Transform>(e);
		ImGui::DragFloat("x", &t.position.x(), 0.1f);
		ImGui::DragFloat("y", &t.position.y(), 0.1f);
		ImGui::DragFloat("z", &t.position.z(), 0.1f);
		ImGui::DragFloat("pitch", &t.orientation.x(), 0.1f);
		ImGui::DragFloat("yaw", &t.orientation.y(), 0.1f);
		ImGui::DragFloat("roll", &t.orientation.z(), 0.1f);
	}
}
nyan::ImguiRenderer::ImguiRenderer(LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, glfww::Window* window) :
	r_device(device),
	r_registry(registry),
	ptr_window(window)
{
	start = std::chrono::high_resolution_clock::now();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = static_cast<float>(r_device.get_swapchain_width());
	io.DisplaySize.y = static_cast<float>(r_device.get_swapchain_height());
	io.BackendRendererName = "imgui_custom_vulkan";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	set_up_pipeline(renderManager.get_shader_manager(), pass);
	set_up_font();
	pass.add_renderfunction([this](vulkan::CommandBufferHandle & cmd, nyan::Renderpass & pass)
	{
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		if (drawData->TotalVtxCount > 0)
		{
			prep_buffer(drawData);
			create_cmds(drawData, cmd);
		}
	}, true);
	ptr_window->configure_imgui();
	m_editor.registerComponent<Transform>("Transform");
	//if (r_registry.data()) {
	//	m_entity = *r_registry.data();
	//}
}

nyan::ImguiRenderer::~ImguiRenderer()
{
	ImGui::DestroyContext();
}

void nyan::ImguiRenderer::update(std::chrono::nanoseconds dt)
{

}

void nyan::ImguiRenderer::next_frame()
{
	std::chrono::duration<double> fp_ms = std::chrono::steady_clock::now() - start;
	start = std::chrono::steady_clock::now();
	ptr_window->imgui_update_mouse_keyboard();
	values[values_offset] = static_cast<float>(fp_ms.count());
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = static_cast<float>(fp_ms.count());
	io.DisplaySize.x = static_cast<float>(r_device.get_swapchain_width());
	io.DisplaySize.y = static_cast<float>(r_device.get_swapchain_height());
	ImGui::NewFrame();
	ImGui::Begin("Metrics");

	values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);

	// Plots can display overlay texts
	// (in this example, we will display an average value)
	{
		float average = 0.0f;
		float max = 0;
		for (int n = 0; n < IM_ARRAYSIZE(values); n++) {
			average += values[n];
			if (values[n] > max)
				max = values[n];
		}
		average /= (float)IM_ARRAYSIZE(values);
		char overlay[] = "fps";
		//sprintf_s(overlay, "avg %f", average);
		ImGui::PlotLines("Frame Times", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, max * 1.2f, ImVec2(0, 80.0f));
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	//ImGui::Begin("Interaction");
	//ImGui::AlignTextToFramePadding();
	//ImGui::Text("Mip Level:");
	//ImGui::PopButtonRepeat();
	//ImGui::SameLine();
	//ImGui::End();

	m_editor.renderSimpleCombo(r_registry, m_entity);

	//ImGui::ShowDemoWindow();
}

void nyan::ImguiRenderer::end_frame()
{

}

void nyan::ImguiRenderer::create_cmds(ImDrawData* draw_data, CommandBufferHandle& cmd)
{
	auto pipelineBind = cmd->bind_graphics_pipeline(m_pipeline);
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	pipelineBind.bind_index_buffer((*m_dataBuffer)->get_handle() , m_bufferOffsets[3], sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
	std::array buffers{
		(*m_dataBuffer)->get_handle(),
		(*m_dataBuffer)->get_handle(),
		(*m_dataBuffer)->get_handle(),
	};
	pipelineBind.bind_vertex_buffers(0, 3, buffers.data(), m_bufferOffsets.data());
	struct PushConstants {
		float scale[2];
		float translate[2];
		int texId;
		int samplerId;
	} push {
		.scale {
			2.0f / static_cast<float>(draw_data->DisplaySize.x),
			2.0f / static_cast<float>(draw_data->DisplaySize.y)
		},
		.translate {
			-1.0f - draw_data->DisplayPos.x * push.scale[0],
			-1.0f - draw_data->DisplayPos.y * push.scale[1]
		},
		.texId {static_cast<int>(m_fontBind)},
		.samplerId {static_cast<int>(vulkan::DefaultSampler::LinearWrap)}
	};
	//push.scale[0] = 2.0f / draw_data->DisplaySize.x;
	//push.scale[1] = 2.0f / draw_data->DisplaySize.y;
	//push.translate[0] = -1.0f - draw_data->DisplayPos.x * push.scale[0];
	//push.translate[1] = -1.0f - draw_data->DisplayPos.y * push.scale[1];
	//push.texId = m_fontBind;
	//push.samplerId = static_cast<int>(vulkan::DefaultSampler::LinearWrap);
	pipelineBind.push_constants(push);
	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
	// Render command lists
	// (Because we merged all buffers into a single one, we maintain our own offset into them)
	int global_vtx_offset = 0;
	int global_idx_offset = 0;
	VkViewport viewport{
	.x = 0,
	.y = 0,
	.width = static_cast<float>(r_device.get_swapchain_width()),
	.height = static_cast<float>(r_device.get_swapchain_height()),
	.minDepth = 0,
	.maxDepth = 1,
	};
	pipelineBind.set_viewport(viewport);
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != NULL)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					assert(false);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec4 clip_rect;
				clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
				clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
				clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
				clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

				if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
				{
					// Negative offsets are illegal for vkCmdSetScissor
					if (clip_rect.x < 0.0f)
						clip_rect.x = 0.0f;
					if (clip_rect.y < 0.0f)
						clip_rect.y = 0.0f;

					// Apply scissor/clipping rectangle
					VkRect2D scissor {
						.offset {
							.x = (int32_t)(clip_rect.x),
							.y = (int32_t)(clip_rect.y),
						},
						.extent {
							.width = (uint32_t)(clip_rect.z - clip_rect.x),
							.height = (uint32_t)(clip_rect.w - clip_rect.y),
						}
					};
					pipelineBind.set_scissor(scissor);
					// Draw
					pipelineBind.draw_indexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
				}
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}
}

void nyan::ImguiRenderer::prep_buffer(ImDrawData* drawData)
{
	constexpr size_t alignment = 64;
	m_bufferOffsets[0] = 0;
	m_bufferOffsets[1] = (drawData->TotalVtxCount * sizeof(ImVec2) + alignment - 1) / alignment * alignment;
	m_bufferOffsets[2] = m_bufferOffsets[1] + (drawData->TotalVtxCount * sizeof(ImVec2) + alignment - 1) / alignment * alignment;
	m_bufferOffsets[3] = m_bufferOffsets[2] + (drawData->TotalVtxCount * sizeof(ImU32) + alignment - 1) / alignment * alignment;
	//auto totalBufferSize = drawData->TotalVtxCount * sizeof(ImVec2)
	//					+ (drawData->TotalVtxCount * sizeof(ImVec2) +63) / 64 * 64
	//					+ drawData->TotalVtxCount * sizeof(ImU32)
	//					+ drawData->TotalIdxCount * sizeof(ImDrawIdx);
	auto totalBufferSize = m_bufferOffsets[3] + drawData->TotalIdxCount * sizeof(ImDrawIdx);
	if (!m_dataBuffer || (*m_dataBuffer)->get_size() < totalBufferSize) {
		BufferInfo info{
			.size = totalBufferSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.offset = 0,
			.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU
		};
		m_dataBuffer = r_device.create_buffer(info, {});
	}

	auto bufferMap = (*m_dataBuffer)->map_data();
	int vtxOffset = 0;
	int idxOffset = 0;
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		for (int i = 0; i < cmd_list->VtxBuffer.Size; i++, vtxOffset++) {
			memcpy(reinterpret_cast<char*>(bufferMap) + m_bufferOffsets[0] + sizeof(ImVec2) * vtxOffset, &cmd_list->VtxBuffer[i].pos, sizeof(ImVec2));
			memcpy(reinterpret_cast<char*>(bufferMap) + m_bufferOffsets[1] + sizeof(ImVec2) * vtxOffset, &cmd_list->VtxBuffer[i].uv, sizeof(ImVec2));
			memcpy(reinterpret_cast<char*>(bufferMap) + m_bufferOffsets[2] + sizeof(ImU32) * vtxOffset, &cmd_list->VtxBuffer[i].col, sizeof(ImU32));
		}
		memcpy(reinterpret_cast<char*>(bufferMap) + m_bufferOffsets[3] + idxOffset * sizeof(ImDrawIdx), cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		idxOffset += cmd_list->IdxBuffer.Size;
	}
}

void nyan::ImguiRenderer::set_up_pipeline(vulkan::ShaderManager& shaderManager, nyan::Renderpass& pass)
{
	GraphicsPipelineConfig config{
		.dynamicState = defaultDynamicGraphicsPipelineState,
		.state = alphaBlendedGraphicsPipelineState,
		.vertexInputCount = 3,
		.shaderCount = 2,
		.vertexInputFormats {
			VK_FORMAT_R32G32_SFLOAT,
			VK_FORMAT_R32G32_SFLOAT,
			VK_FORMAT_R8G8B8A8_UNORM
		},
		.shaderInstances {
			shaderManager.get_shader_instance_id("imgui_vert"),
			shaderManager.get_shader_instance_id("imgui_frag")
		},
		.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	config.dynamicState.depth_write_enable = VK_FALSE;
	config.dynamicState.depth_test_enable = VK_FALSE;
	config.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	m_pipeline = pass.add_pipeline(config);
}

void nyan::ImguiRenderer::set_up_font()
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	auto info = ImageInfo::immutable_2d_image(width, height, VK_FORMAT_R8G8B8A8_UNORM, false);
	vulkan::InitialImageData data{
			.data = reinterpret_cast<char*>(pixels),
			//.rowLength = 0,
			//.height = 0,
	};
	m_font = r_device.create_image(info, &data);
	m_fontBind = r_device.get_bindless_set().set_sampled_image(VkDescriptorImageInfo{ 
		.imageView = (* m_font)->get_view()->get_image_view(),
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		});
}
