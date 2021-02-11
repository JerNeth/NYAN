#include "ImguiRenderer.h"
#include "ShaderManager.h"

Vulkan::Imgui::Imgui(LogicalDevice& device) : r_device(device) {
	start = std::chrono::high_resolution_clock::now();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = r_device.get_swapchain_width();
	io.DisplaySize.y = r_device.get_swapchain_height();
	io.BackendRendererName = "imgui_custom_vulkan";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	set_up_program();
	set_up_font();
}

Vulkan::Imgui::~Imgui()
{
	ImGui::DestroyContext();
}

void Vulkan::Imgui::next_frame()
{
	std::chrono::duration<float> delta = std::chrono::high_resolution_clock::now() - start;
	values[values_offset] = delta.count();
	start = std::chrono::high_resolution_clock::now();
	ImGui::GetIO().DeltaTime = delta.count();
	ImGui::NewFrame();
	ImGui::Begin("Metrics");                          // Create a window called "Hello, world!" and append into it.

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
		char overlay[32];
		sprintf_s(overlay, "avg %f", average);
		ImGui::PlotLines("Frame Times", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, max * 1.2f, ImVec2(0, 80.0f));
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::ShowDemoWindow();
}

void Vulkan::Imgui::end_frame(CommandBufferHandle& cmd)
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	if (drawData->TotalVtxCount > 0)
	{
		prep_buffer(drawData);
		create_cmds(drawData, cmd);
	}
}

void Vulkan::Imgui::create_cmds(ImDrawData* draw_data, CommandBufferHandle& cmd)
{
	cmd->bind_program(m_program);
	cmd->set_cull_mode(VK_CULL_MODE_NONE);
	cmd->disable_depth();
	cmd->enable_alpha();

	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	cmd->set_vertex_attribute(0, 0, VK_FORMAT_R32G32_SFLOAT);
	cmd->set_vertex_attribute(1, 0, VK_FORMAT_R32G32_SFLOAT);
	cmd->set_vertex_attribute(2, 0, VK_FORMAT_R8G8B8A8_UNORM);
	cmd->bind_texture(0, 0, *m_font->get_view(), DefaultSampler::LinearWrap);
	cmd->bind_index_buffer(IndexState{.buffer = m_ibo->get_handle(),.offset = 0,.indexType = sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32 });
	cmd->bind_vertex_buffer(0, *m_vbo, 0, VK_VERTEX_INPUT_RATE_VERTEX);
	float scale[2];
	scale[0] = 2.0f / draw_data->DisplaySize.x;
	scale[1] = 2.0f / draw_data->DisplaySize.y;
	float translate[2];
	translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
	translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
	cmd->push_constants(scale, 0, sizeof(float) * 2);
	cmd->push_constants(translate, sizeof(float) * 2, sizeof(float) * 2);
	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
	auto oldScissor = cmd->get_scissor();
	// Render command lists
	// (Because we merged all buffers into a single one, we maintain our own offset into them)
	int global_vtx_offset = 0;
	int global_idx_offset = 0;
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
					VkRect2D scissor;
					scissor.offset.x = (int32_t)(clip_rect.x);
					scissor.offset.y = (int32_t)(clip_rect.y);
					scissor.extent.width = (uint32_t)(clip_rect.z - clip_rect.x);
					scissor.extent.height = (uint32_t)(clip_rect.w - clip_rect.y);
					cmd->set_scissor(scissor);
					// Draw
					cmd->draw_indexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
				}
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}
	cmd->set_scissor(oldScissor);
}

void Vulkan::Imgui::prep_buffer(ImDrawData* drawData)
{
	auto vertSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
	auto idxSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
	if (!m_vbo.is_valid() || m_vbo->get_size() < vertSize) {
		BufferInfo info{
			.size = vertSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.offset = 0,
			.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU
		};
		m_vbo = r_device.create_buffer(info);
	}
	if (!m_ibo.is_valid() || m_ibo->get_size() < idxSize) {
		BufferInfo info{
			.size = idxSize,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.offset = 0,
			.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU
		};
		m_ibo = r_device.create_buffer(info);
	}
	
	auto vertMap = m_vbo->map_data();
	auto idxMap = m_ibo->map_data();
	size_t vertOffset = 0;
	size_t idxOffset = 0;
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		memcpy(vertMap.get() + vertOffset, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxMap.get() + idxOffset, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vertOffset += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
		idxOffset += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
	}
}

void Vulkan::Imgui::set_up_program()
{
	ShaderManager s(r_device);
	m_program = s.request_program("imgui_vert.spv", "imgui_frag.spv");
}

void Vulkan::Imgui::set_up_font()
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	auto info = ImageInfo::immutable_2d_image(width, height, VK_FORMAT_R8G8B8A8_UNORM, false);

	m_font = r_device.create_image(info, pixels);
}
