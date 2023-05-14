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
		template<size_t N>
		struct Average {
		private:
			std::array<float, N> samples{};
			size_t index;
			float sum;
		public:
			void add_sample(float sample) {
				auto localIdx = index++ % N;
				sum -= samples[localIdx];
				samples[localIdx] = sample;
				sum += sample;
			}
			float average() const {
				return sum / std::max(std::min(index, N), 1ull);
			}
			void clear() {
				index = 0;
				sum = 0.f;
			}
		};
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
		std::unordered_map<std::string, Average<1000> > m_averages;
	};
}