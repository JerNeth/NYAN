#include "Renderer/Renderer.h"
using namespace vulkan;
using namespace nyan;

void nyan::RenderQueue::clear()
{
	m_staticMeshes.clear();
	m_skinnedMeshes.clear();
}

nyan::VulkanRenderer::VulkanRenderer(vulkan::LogicalDevice& device,vulkan::ShaderManager* shaderManager) :
	r_device( device),
	m_shaderManager(shaderManager)
{
	RendererCamera init{};
	BufferInfo buffInfo;
	buffInfo.size = sizeof(RendererCamera);
	buffInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	m_cameraBuffer = r_device.create_buffer(buffInfo, &init);
}

void nyan::VulkanRenderer::queue_mesh(StaticMesh* mesh)
{
	if (mesh == nullptr)
		return;
	RenderId id = mesh->get_material()->get_id();
	id <<= 32;
	if (mesh->uses_tangent_space())
		id |= tangentSpaceBit;
	m_renderQueue.m_staticMeshes.emplace(id, mesh);
}

void nyan::VulkanRenderer::queue_mesh(SkinnedMesh* mesh)
{
	if (mesh == nullptr)
		return;
	RenderId id = mesh->get_material()->get_id();
	id <<= 32;
	if (mesh->uses_tangent_space())
		id |= tangentSpaceBit;
	if (mesh->has_blendshape())
		id |= blendShapeBit;
	m_renderQueue.m_skinnedMeshes.emplace(id, mesh);
}

void nyan::VulkanRenderer::update_camera(const RendererCamera& camera)
{
	auto* map = m_cameraBuffer->map_data();
	memcpy(map, &camera, sizeof(RendererCamera));
}

void nyan::VulkanRenderer::render(vulkan::CommandBufferHandle& cmd)
{
	//TODO good place to parallelize

	cmd->bind_uniform_buffer(0, 0, *m_cameraBuffer, 0, sizeof(RendererCamera));

	RenderId current = invalidId;
	for (auto& it : m_renderQueue.m_staticMeshes) {
		if (it.first != current) {
			std::string vertexShaderName = "static";
			std::string fragmentShaderName = it.second->get_material()->get_shader_name();
			if (it.first & tangentSpaceBit) {
				//assert(false);
				vertexShaderName = "staticTangent_vert";
				fragmentShaderName += "Tangent_frag";
			}
			else {
				vertexShaderName = "static_vert";
				fragmentShaderName += "_frag";
			}
			auto* program = m_shaderManager->request_program(vertexShaderName, fragmentShaderName);
			cmd->bind_program(program);
			it.second->get_material()->bind(cmd);
			current = it.first;
		}
		it.second->render(cmd);
	}

	current = invalidId;
	for (auto& it : m_renderQueue.m_skinnedMeshes) {
		if (it.first != current) {
			std::string vertexShaderName;
			if (it.first & (tangentSpaceBit| blendShapeBit) && !(current & (tangentSpaceBit | blendShapeBit))) {
				assert(false);
				//Todo
			} 
			else if (it.first & blendShapeBit && !(current & blendShapeBit)) {
				assert(false);
				//Todo
			}
			else if (it.first & tangentSpaceBit && !(current & tangentSpaceBit)) {
				assert(false);
			}
			else {
				vertexShaderName = "skinned_vert";
			}
			//Todo replace with ShaderIDs
			auto* program = m_shaderManager->request_program(vertexShaderName, it.second->get_material()->get_shader_name());
			cmd->bind_program(program);
			it.second->get_material()->bind(cmd);
			current = it.first;
		}
		it.second->render(cmd);
	}
}

void nyan::VulkanRenderer::next_frame()
{
	m_renderQueue.clear();
}

void nyan::VulkanRenderer::end_frame()
{
}
