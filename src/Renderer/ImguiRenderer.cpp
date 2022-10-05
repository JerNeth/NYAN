#include "Renderer/ImguiRenderer.h"
#include "entt/entt.hpp"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Pipeline.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/Image.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/CameraController.h"
#include "Renderer/Light.h"
//#include "Renderer/DDGIManager.h"
using namespace vulkan;

static nyan::MaterialManager* manager;
static nyan::DDGIManager* ddgiManager;



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
	template <>
	void ComponentEditorWidget<nyan::PerspectiveCamera>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& t = reg.get<nyan::PerspectiveCamera>(e);
		ImGui::DragFloat("near", &t.nearPlane, 0.1f);
		ImGui::DragFloat("far", &t.farPlane, 0.1f);
		ImGui::DragFloat("fov", &t.fovX, 0.1f);
		ImGui::DragFloat("aspect", &t.aspect, 0.1f);
		ImGui::DragFloat("up_x", &t.up.x(), 0.1f);
		ImGui::DragFloat("up_y", &t.up.y(), 0.1f);
		ImGui::DragFloat("up_z", &t.up.z(), 0.1f);
		ImGui::DragFloat("right_x", &t.right.x(), 0.1f);
		ImGui::DragFloat("right_y", &t.right.y(), 0.1f);
		ImGui::DragFloat("right_z", &t.right.z(), 0.1f);
		ImGui::DragFloat("forward_x", &t.forward.x(), 0.1f);
		ImGui::DragFloat("forward_y", &t.forward.y(), 0.1f);
		ImGui::DragFloat("forward_z", &t.forward.z(), 0.1f);
	}
	template <>
	void ComponentEditorWidget<nyan::MaterialId>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& t = reg.get<nyan::MaterialId>(e);
		auto& mat = manager->get_material(t);
		if (mat.albedoTexId != nyan::shaders::INVALID_BINDING) {
			ImGui::Image(static_cast<ImTextureID>(mat.albedoTexId + 1), ImVec2(64, 64));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Image(static_cast<ImTextureID>(mat.albedoTexId + 1), ImVec2(512, 512));
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Text("Albedo Texture");
		}
		if (mat.emissiveTexId != nyan::shaders::INVALID_BINDING) {
			ImGui::Image(static_cast<ImTextureID>(mat.emissiveTexId + 1), ImVec2(64, 64));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Image(static_cast<ImTextureID>(mat.emissiveTexId + 1), ImVec2(512, 512));
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Text("Emissive Texture");
		}
		if (mat.normalTexId != nyan::shaders::INVALID_BINDING) {
			ImGui::Image(static_cast<ImTextureID>(mat.normalTexId + 1), ImVec2(64, 64));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Image(static_cast<ImTextureID>(mat.normalTexId + 1), ImVec2(512, 512));
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Text("Normal Texture");
		}
		if (mat.pbrTexId != nyan::shaders::INVALID_BINDING) {
			ImGui::Image(static_cast<ImTextureID>(mat.pbrTexId + 1), ImVec2(64, 64));
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Image(static_cast<ImTextureID>(mat.pbrTexId + 1), ImVec2(512, 512));
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			ImGui::Text("PBR Texture");
		}
		ImGui::DragFloat("roughness", &mat.roughness, 0.001f, 0, 1);
		ImGui::DragFloat("metalness", &mat.metalness, 0.001f, 0, 1);
		ImGui::DragFloat("alpha discard", &mat.alphaDiscard, 0.001f, 0, 1);
		//ImGui::ColorEdit3("F0", &mat.F0_R);
		ImGui::ColorEdit4("albedo", &mat.albedo_R);
		ImGui::ColorEdit3("emissive", &mat.emissive_R);
	}
	template <>
	void ComponentEditorWidget<nyan::Deferred>([[maybe_unused]] entt::registry& reg, [[maybe_unused]] entt::registry::entity_type e)
	{
	}
	template <>
	void ComponentEditorWidget<nyan::DeferredAlphaTest>([[maybe_unused]] entt::registry& reg, [[maybe_unused]] entt::registry::entity_type e)
	{
	}
	template <>
	void ComponentEditorWidget<nyan::Forward>([[maybe_unused]] entt::registry& reg, [[maybe_unused]] entt::registry::entity_type e)
	{
	}
	template <>
	void ComponentEditorWidget<nyan::ForwardTransparent>([[maybe_unused]] entt::registry& reg, [[maybe_unused]] entt::registry::entity_type e)
	{
	}
	template <>
	void ComponentEditorWidget<nyan::DDGIManager::DDGIVolumeParameters>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& volume = reg.get<nyan::DDGIManager::DDGIVolumeParameters>(e);
		//auto& mat = manager->get_material(t);
		//ImGui::Image(static_cast<ImTextureID>(mat.albedoTexId + 1), ImVec2(64, 64));
		ImGui::DragFloat3("Spacing", &volume.spacing.x());
		ImGui::DragFloat3("Origin", &volume.origin.x());
		ImGui::DragInt3("Probe Count", reinterpret_cast<int*>(&volume.probeCount.x()), 1, 1, 256, "%d", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragInt("Rays per Probe", reinterpret_cast<int*>(&volume.raysPerProbe), 1, 1, 256, "%d", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragInt("Irradiance Probe Size", reinterpret_cast<int*>(&volume.irradianceProbeSize), 1, 1, 32, "%d", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragInt("Depth Probe Size", reinterpret_cast<int*>(&volume.depthProbeSize), 1, 1, 32,"%d", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragInt("Fixed Ray Count", reinterpret_cast<int*>(&volume.fixedRayCount), 1, 0, volume.raysPerProbe, "%d", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Backface Threshold", &volume.relocationBackfaceThreshold, 0.001, 0, 1, "%.3f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Min Front Face Distance", &volume.minFrontFaceDistance, 0.1, 0, 100.f, "%.3f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Depth Bias", &volume.depthBias, 0.01f, 0.f, 10000.f, "%.3f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Max Ray Distance", &volume.maxRayDistance, 1.f, 0.00001f, 100000.0f);
		ImGui::DragFloat("Hysteresis", &volume.hysteresis, 0.01f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Irradiance Threshold", &volume.irradianceThreshold, 0.01f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Light To Dark Threshold", &volume.lightToDarkThreshold, 0.01f, 0.01f, 1.f, "%.3f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::DragFloat("Visualizer Radius", &volume.visualizerRadius, 0.1f, 0.01f, 100.f, "%.3f", ImGuiSliderFlags_::ImGuiSliderFlags_AlwaysClamp);
		ImGui::Checkbox("Use Moments", &volume.useMoments);
		ImGui::Checkbox("Enabled", &volume.enabled);

		{
			static constexpr const char* renderTargetFormats[] = { "R16G16B16A16F", "R32G32B32A32F" };
			static const char* currentRenderTargetFormat = renderTargetFormats[0];
			if (ImGui::BeginCombo("##Render Target Format", currentRenderTargetFormat))
			{
				for (const auto& format : renderTargetFormats) {
					bool selected = (format == currentRenderTargetFormat);
					if (ImGui::Selectable(format, selected))
						currentRenderTargetFormat = format;
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (currentRenderTargetFormat == renderTargetFormats[0]) {
				volume.renderTargetImageFormat = nyan::shaders::R16G16B16A16F;
			}
			else if (currentRenderTargetFormat == renderTargetFormats[1]) {
				volume.renderTargetImageFormat = nyan::shaders::R32G32B32A32F;
			}
			ImGui::SameLine();
			ImGui::Text("Render Target Format");
		}

		{
			static constexpr const char* irradianceFormats[] = {"R10G10B10A2F", "R16G16B16A16F", "R11G11B10F" };
			static const char* currentIrradianceFormat = irradianceFormats[0];
			if (ImGui::BeginCombo("##Irradiance Format", currentIrradianceFormat))
			{
				for (const auto& format : irradianceFormats) {
					bool selected = (format == currentIrradianceFormat);
					if (ImGui::Selectable(format, selected))
						currentIrradianceFormat = format;
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (currentIrradianceFormat == irradianceFormats[0]) {
				volume.irradianceImageFormat = nyan::shaders::R10G10B10A2F;
			}
			else if (currentIrradianceFormat == irradianceFormats[1]) {
				volume.irradianceImageFormat = nyan::shaders::R16G16B16A16F;
			}
			else if (currentIrradianceFormat == irradianceFormats[2]) {
				volume.irradianceImageFormat = nyan::shaders::R11G11B10F;
			}
			ImGui::SameLine();
			ImGui::Text("Irradiance Format");
		}
		{
			static constexpr const char* depthFormats[] = { "R16G16B16A16F", "R32G32B32A32F" };
			static const char* currentDepthFormat = depthFormats[1];
			if (ImGui::BeginCombo("##Depth Format", currentDepthFormat))
			{
				for (const auto& format : depthFormats) {
					bool selected = (format == currentDepthFormat);
					if (ImGui::Selectable(format, selected))
						currentDepthFormat = format;
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (currentDepthFormat == depthFormats[0]) {
				volume.depthImageFormat = nyan::shaders::R16G16B16A16F;
			}
			else if (currentDepthFormat == depthFormats[1]) {
				volume.depthImageFormat = nyan::shaders::R32G32B32A32F;
			}
			ImGui::SameLine();
			ImGui::Text("Depth Format");
		}
		ImGui::Checkbox("Visualization Enabled", &volume.visualization);
		ImGui::Checkbox("Visualizate Depth", &volume.visualizeDepth);
		ImGui::Checkbox("Visualizate Directions", &volume.visualizeDirections);
		ImGui::Checkbox("Relocation Enabled", &volume.relocationEnabled);
		ImGui::Checkbox("Classification Enabled", &volume.classificationEnabled);
		if (volume.ddgiVolume != ~0) {
			auto& devvolume = ddgiManager->get(volume.ddgiVolume);
			ImGui::Text("Irradiance Texture");
			ImGui::SameLine(0, 25);
			ImGui::Text("Depth Texture");
			if (devvolume.irradianceTextureBinding != ~0) {
				ImGui::Image(static_cast<ImTextureID>(devvolume.irradianceTextureBinding + 1), ImVec2(128, 256));
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Image(static_cast<ImTextureID>(devvolume.irradianceTextureBinding + 1), ImVec2(512, 1024));
					ImGui::EndTooltip();
				}
			}
			if (devvolume.depthTextureBinding != ~0) {
				ImGui::SameLine();
				ImGui::Image(static_cast<ImTextureID>(devvolume.depthTextureBinding + 1), ImVec2(128, 256));

				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Image(static_cast<ImTextureID>(devvolume.depthTextureBinding + 1), ImVec2(512, 1024));
					ImGui::EndTooltip();
				}
			}
		}
		//ImGui::ColorEdit3("F0", &mat.F0_R);
	}
	template <>
	void ComponentEditorWidget<nyan::CameraMovement>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& movement = reg.get<nyan::CameraMovement>(e);
		//auto& mat = manager->get_material(t);
		//ImGui::Image(static_cast<ImTextureID>(mat.albedoTexId + 1), ImVec2(64, 64));
		ImGui::DragFloat("Speed", &movement.speed);
		ImGui::DragFloat("Rotational Speed", &movement.rotationalSpeed);
		//ImGui::ColorEdit3("F0", &mat.F0_R);
	}
	template <>
	void ComponentEditorWidget<nyan::Directionallight>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& light = reg.get<nyan::Directionallight>(e);
		ImGui::Checkbox("Enabled", &light.enabled);
		ImGui::ColorEdit3("Color", &light.color.x());
		ImGui::DragFloat("Intensity", &light.intensity);
		ImGui::DragFloat3("Direction", &light.direction.x(), 0.05f, -1.f, 1.f);
	}
	template <>
	void ComponentEditorWidget<nyan::Pointlight>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& light = reg.get<nyan::Pointlight>(e);
		ImGui::ColorEdit3("Color", &light.color.x());
		ImGui::DragFloat("Intensity", &light.intensity);
		ImGui::DragFloat("Attenuation", &light.attenuation);
	}
}
nyan::ImguiRenderer::ImguiRenderer(LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, glfww::Window* window) :
	r_device(device),
	r_registry(registry),
	ptr_window(window)
{
	pass.add_swapchain_attachment();
	manager = &renderManager.get_material_manager();
	ddgiManager = &renderManager.get_ddgi_manager();

	start = std::chrono::high_resolution_clock::now();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = static_cast<float>(r_device.get_swapchain_width());
	io.DisplaySize.y = static_cast<float>(r_device.get_swapchain_height());
	io.BackendRendererName = "imgui_custom_vulkan";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	set_up_pipeline(renderManager.get_shader_manager(), pass);
	set_up_font();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass &)
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
	m_editor.registerComponent<PerspectiveCamera>("Camera");
	m_editor.registerComponent<MaterialId>("Material");
	m_editor.registerComponent<Deferred>("Deferred");
	m_editor.registerComponent<DeferredAlphaTest>("Alpha Test");
	m_editor.registerComponent<Forward>("Forward");
	m_editor.registerComponent<ForwardTransparent>("Forward Alpha Blended");
	m_editor.registerComponent<DDGIManager::DDGIVolumeParameters>("DDGI Volume");
	m_editor.registerComponent<CameraMovement>("Camera Controller");
	m_editor.registerComponent<Directionallight>("Directional Light");
	m_editor.registerComponent<Pointlight>("Point Light");
	//if (r_registry.data()) {
	//	m_entity = *r_registry.data();
	//}
}

nyan::ImguiRenderer::~ImguiRenderer()
{
	ImGui::DestroyContext();
}

void nyan::ImguiRenderer::update([[maybe_unused]] std::chrono::nanoseconds dt)
{

}

void nyan::ImguiRenderer::begin_frame()
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

void nyan::ImguiRenderer::create_cmds(ImDrawData* draw_data, CommandBuffer& cmd)
{
	auto pipelineBind = cmd.bind_graphics_pipeline(m_pipeline);
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
		int customTexId;
		int customSamplerId;
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
		.samplerId {static_cast<int>(vulkan::DefaultSampler::NearestClamp)},
		//.customTexId {-1},
		//.customSamplerId{static_cast<int>(vulkan::DefaultSampler::LinearWrap)}
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
	pipelineBind.set_viewport_with_count(1, &viewport);
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
					auto tex = static_cast<int>(pcmd->TextureId) -1;
					if (tex != -1) {
						push.texId = tex;
					}
					else {
						push.texId = static_cast<int>(m_fontBind);
					}
					pipelineBind.push_constants(push);
					pipelineBind.set_scissor_with_count(1, &scissor);
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
	pass.add_pipeline(config, &m_pipeline);
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
		.imageView = *(* m_font)->get_view(),
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		});
}
