#include "..\..\include\Renderer\RenderGraph.h"
#include "Rendergraph.h"

using namespace nyan;
//using namespace vulkan;

nyan::Renderpass::Renderpass(Rendergraph& graph, Type type, uint32_t id) : r_graph(graph), m_type(type), m_id(id)
{
}

void nyan::Renderpass::add_input(const std::string& name)
{
	auto& resource = r_graph.get_ressource(name);
	resource.m_readIn.push_back(m_id);
	m_reads.push_back(resource.m_id);
}

void nyan::Renderpass::add_output(const std::string& name, ImageAttachment attachment)
{
	auto& resource = r_graph.get_ressource(name);
	resource.m_writeToIn.push_back(m_id);
	resource.attachment = attachment;
	m_writes.push_back(resource.m_id);
}

void nyan::Renderpass::add_read_dependency(const std::string& name)
{
	auto& resource = r_graph.get_ressource(name);
	resource.m_readIn.push_back(m_id);
	m_reads.push_back(resource.m_id);
}

void nyan::Renderpass::add_write_dependency(const std::string& name)
{
	auto& resource = r_graph.get_ressource(name);
	resource.m_writeToIn.push_back(m_id);
	m_writes.push_back(resource.m_id);
}

nyan::Rendergraph::Rendergraph(vulkan::LogicalDevice& device)
	: r_device(device)
{
	
}

Renderpass& nyan::Rendergraph::add_pass(const std::string& name, Renderpass::Type type)
{
	return m_renderpasses.emplace(name, *this, type, m_renderpassCount++);
}

Renderpass& nyan::Rendergraph::get_pass(const std::string& name)
{
	return m_renderpasses.get(name);
}

void nyan::Rendergraph::build()
{
	int i = 0;

	m_renderpasses.for_each([&](Renderpass& pass) {
		i++;
		std::cout << pass.get_id() << '\n';
	});
	std::cout << i << "\n";
}

void nyan::Rendergraph::execute()
{
	//check if attachments are still valid?

	m_renderpasses.for_each([&](Renderpass& pass) {
		vulkan::CommandBuffer::Type commandBufferType;
		switch (pass.get_type()) {
		case Renderpass::Type::Compute:
			commandBufferType = vulkan::CommandBuffer::Type::Compute;
			break;
		case Renderpass::Type::Graphics:
			commandBufferType = vulkan::CommandBuffer::Type::Generic;
			break;
		}
		auto cmd = r_device.request_command_buffer(commandBufferType);
		if (pass.get_type() == Renderpass::Type::Graphics)
			cmd->begin_render_pass(pass.get_info());
		pass.execute(cmd);
		if (pass.get_type() == Renderpass::Type::Graphics)
			cmd->end_render_pass();
		r_device.submit(cmd);
	});
}

void nyan::Rendergraph::add_ressource(const std::string& name, Attachment attachment)
{
}

RenderRessource& nyan::Rendergraph::get_ressource(const std::string& name)
{
	return m_renderressources.get(name);
}

void nyan::Rendergraph::set_swapchain(const std::string& name)
{
	m_swapchainRessource = name;
}

