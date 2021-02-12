#pragma once
#ifndef RDRENDERER_H
#define RDRENDERER_H

namespace nyan {
	class Renderer {
	public:
		virtual void next_frame() = 0;
		virtual void end_frame() = 0;
	};
	class VulkanRenderer : public Renderer {
	public:
		void next_frame();
		void end_frame();
	private:

	};
}

#endif !RDRENDERER_H