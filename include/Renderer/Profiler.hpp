#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <chrono>
#include <VulkanForwards.h>

namespace nyan {

	class Profiler {
	public:
		Profiler(vulkan::LogicalDevice& device);
		void begin_frame();
		void begin_profile(vulkan::CommandBuffer& cmd, const std::string& name);
		void end_profile(vulkan::CommandBuffer& cmd);
		void begin_profile(const std::string& name);
		void end_profile();
	private:
		struct Profile {
			std::chrono::steady_clock::time_point firstTimepoint{};
			std::chrono::steady_clock::time_point secondTimepoint{};
			uint32_t depth{ 0u };
			std::string name{};
		};
		struct GPUProfile {
			uint32_t firstQueryId{ 0u };
			uint32_t secondQueryId{ ~0u };
			uint32_t depth{ 0u };
			std::string name{};
		};
		struct ThreadData {
			std::vector<size_t> stack{};
			std::vector<Profile> timestamp{};

		};
		struct GPUThreadData {
			std::vector<size_t> stack{};
			std::vector<GPUProfile> timestamp{};

		};

		vulkan::LogicalDevice& r_device;
		float m_timestampPeriod;
		std::unordered_map<std::thread::id, ThreadData> m_profiles;
		std::unordered_map<std::thread::id, GPUThreadData> m_GPUprofiles;
	};
}