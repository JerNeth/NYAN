#pragma once
#ifndef GLTFREADER_H
#define GLTFREADER_H
#include "Renderer/RenderManager.h"
namespace nyan {
	class GLTFReader {
	public:
		GLTFReader(nyan::RenderManager& renderManager);
		void load_file(const std::filesystem::path& path);
	private:
		nyan::RenderManager& r_renderManager;
	};
}
#endif //!GLTFREADER_H