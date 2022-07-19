#pragma once
#ifndef RDCAMERCONTROLLER_H
#define RDCAMERCONTROLLER_H
#include <chrono>
#include "entt/entt.hpp"

namespace nyan {
	class RenderManager;
	class Input;
	class CameraController {
	public:
		CameraController(RenderManager& renderManager, Input& input);
		void update(std::chrono::nanoseconds dt);
	private:
		RenderManager& r_renderManager;
		Input& r_input;
	};
}

#endif !RDCAMERCONTROLLER_H