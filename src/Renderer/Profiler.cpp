#include "Renderer/Profiler.hpp"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/Instance.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/QueryPool.hpp"
#include "VulkanWrapper/PhysicalDevice.hpp"
#include "imgui.h"
#include "implot.h"

nyan::Profiler::Profiler(vulkan::LogicalDevice& device) :
	r_device(device),
	m_timestampPeriod(device.get_physical_device().get_properties().limits.timestampPeriod)
{
}

void nyan::Profiler::begin_frame()
{
	ImGui::Begin("Profiler");
	if(ImGui::CollapsingHeader("GPU"))
	{
		auto& timestamps = r_device.previous_frame().get_timestamps().get_results();
		for (auto& [threadId, threadData] : m_GPUprofiles) {
			for (const auto& profile : threadData.timestamp) {
				std::string tabs(profile.depth , '\t' );
				if (profile.firstQueryId < timestamps.size() &&
					profile.secondQueryId < timestamps.size()) {
					std::chrono::duration<float, std::nano> fp_ns{ (timestamps[profile.secondQueryId] - timestamps[profile.firstQueryId]) * m_timestampPeriod };
					std::chrono::duration<float, std::milli> fp_ms = fp_ns;
					ImGui::Text("%s%s : %f ms", tabs.c_str(), profile.name.c_str(), fp_ms.count());
				}
			}
			threadData.timestamp.clear();
			threadData.stack.clear();
		}
	}
	if (ImGui::CollapsingHeader("CPU"))
	{
		for (auto& [threadId, threadData] : m_profiles) {
			for (const auto& profile : threadData.timestamp) {
				std::string tabs(profile.depth, '\t');
				std::chrono::duration<float, std::milli> fp_ms = profile.secondTimepoint - profile.firstTimepoint;
				ImGui::Text("%s%s : %f ms", tabs.c_str(), profile.name.c_str(), fp_ms.count());
			}
			threadData.timestamp.clear();
			threadData.stack.clear();
		}
	}

	ImGui::End();

}

void nyan::Profiler::begin_profile(vulkan::CommandBuffer& cmd, const std::string& name)
{
	begin_profile(name);
	auto& threadData = m_GPUprofiles[std::this_thread::get_id()];
	assert(m_GPUprofiles.size() == 1 && "TODO: will not work if multiple threads access simultaniously");
	auto& queryPool = r_device.frame().get_timestamps();
	GPUProfile timestamp{
		.firstQueryId {queryPool++},
		.depth {static_cast<uint32_t>(threadData.stack.size())},
		.name {name}
	};
	VkPipelineStageFlagBits2 pipelineStage{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };
	auto type = cmd.get_type();
	if (type == vulkan::CommandBufferType::Generic)
		pipelineStage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
	else if (type == vulkan::CommandBufferType::Compute)
		pipelineStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	else if (type == vulkan::CommandBufferType::Transfer)
		pipelineStage = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
	cmd.write_timestamp(pipelineStage, queryPool, timestamp.firstQueryId);
	threadData.stack.push_back(threadData.timestamp.size());
	threadData.timestamp.push_back(timestamp);
}

void nyan::Profiler::end_profile(vulkan::CommandBuffer& cmd)
{
	end_profile();
	auto& threadData = m_GPUprofiles[std::this_thread::get_id()];
	auto& queryPool = r_device.frame().get_timestamps();
	VkPipelineStageFlagBits2 pipelineStage{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };
	auto type = cmd.get_type();
	if (type == vulkan::CommandBufferType::Generic)
		pipelineStage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
	else if (type == vulkan::CommandBufferType::Compute)
		pipelineStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
	else if (type == vulkan::CommandBufferType::Transfer)
		pipelineStage = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
	auto queryId = queryPool++;
	cmd.write_timestamp(pipelineStage, queryPool, queryId);
	auto profileId = threadData.stack.back();
	threadData.timestamp[profileId].secondQueryId = queryId;
	threadData.stack.pop_back();
}

void nyan::Profiler::begin_profile(const std::string& name)
{
	auto& threadData = m_profiles[std::this_thread::get_id()];
	assert(m_profiles.size() == 1 && "TODO: will not work if multiple threads access simultaniously");
	Profile timestamp{
		.firstTimepoint {std::chrono::steady_clock::now()},
		.depth {static_cast<uint32_t>(threadData.stack.size())},
		.name {name}
	};
	threadData.stack.push_back(threadData.timestamp.size());
	threadData.timestamp.push_back(timestamp);
}

void nyan::Profiler::end_profile()
{
	auto& threadData = m_profiles[std::this_thread::get_id()];
	auto profileId = threadData.stack.back();
	threadData.timestamp[profileId].secondTimepoint = std::chrono::steady_clock::now();
	threadData.stack.pop_back();
}
