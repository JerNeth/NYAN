#pragma once
#include "glfwWrapper.h"
#include <unordered_map>
#include <array>
namespace nyan {
	class Input {

	public:
		enum class Inputs {
			Up,
			Left,
			Down,
			Right,
			W,
			A,
			S,
			D,
			MouseX,
			MouseY,
			Size
		};
		enum class Axis {
			MoveForward,
			MoveRight,
			MoveUp,
			LookUp,
			LookRight,
			Size
		};
	private:
		struct Mapping {
			Axis axis;
			float scale;
		};
		constexpr static std::array inputMapping {
			GLFW_KEY_UP,
			GLFW_KEY_LEFT,
			GLFW_KEY_DOWN,
			GLFW_KEY_RIGHT,
			GLFW_KEY_W,
			GLFW_KEY_A,
			GLFW_KEY_S,
			GLFW_KEY_D
		};
	public:
		Input(glfww::Window& window);
		void set_axis_mapping(Inputs input, Axis axis, float scale);
		void set_input(Inputs input, float value);
		const float& get_axis(Axis axis);
		void update();
	private:
		glfww::Window& r_window;
		std::array<float, static_cast<size_t>(Inputs::Size)> m_input;
		std::array<float, static_cast<size_t>(Axis::Size)> m_axis;
		std::unordered_map<Inputs, Mapping> m_axisMapping;
	};
}