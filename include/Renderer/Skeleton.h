#pragma once
#ifndef RDSKELETON_H
#define RDSKELETON_H
#include "VkWrapper.h"
#include "LinAlg.h"
namespace nyan {
	class Skeleton {
		const uint32_t maxBones = 64;
	public:
		void bind(vulkan::CommandBufferHandle& cmd);
		void update();
	private:
		std::vector<Math::mat43> bones;
		//std::vector<uint32_t> boneParents;
		//std::vector<std::string> boneNames;
		vulkan::Buffer* buffer= nullptr;
		uint32_t offset = 0;
		uint32_t size = 0;
	};
}
#endif