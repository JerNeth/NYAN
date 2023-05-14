#pragma once
#ifndef RDCAMERCONTROLLER_H
#define RDCAMERCONTROLLER_H
#include <chrono>
#include "entt/entt.hpp"

namespace nyan {
	struct CameraMovement {
		float speed{ 10.f };
		float rotationalSpeed{90.f};
	};
	class RenderManager;
	class Input;
	class CameraController {
	public:
		CameraController(RenderManager& renderManager, Input& input);
		void update(std::chrono::nanoseconds dt);
		bool changed() const;
	private:
		RenderManager& r_renderManager;
		Input& r_input;
		bool m_changed;
	};
}

#endif !RDCAMERCONTROLLER_H