#include "Core/Input.h"

nyan::Input::Input(glfww::Window& window) :
	r_window(window),
	m_input(),
	m_axis(),
	m_axisMapping()
{
}

void nyan::Input::set_axis_mapping(Inputs input, Axis axis, float scale)
{
	m_axisMapping[input] = Mapping{ axis, scale };
}

void nyan::Input::set_input(Inputs input, float value)
{
	m_input[static_cast<size_t>(input)] = value;
}

const float& nyan::Input::get_axis(Axis axis)
{
	return m_axis[static_cast<size_t>(axis)];
}

void nyan::Input::update()
{

	for (auto& axis : m_axis) {
		axis = 0.f;
	}
	if (r_window.is_focused()) {
		for (size_t i{ 0 }; i < inputMapping.size(); ++i) {
			m_input[i] = r_window.get_key(inputMapping[i]) == GLFW_PRESS ? 1.f : 0.f;
		}
		for (auto& [input, axisMapping] : m_axisMapping) {
			auto [axis, mapping] = axisMapping;
			m_axis[static_cast<size_t>(axis)] += m_input[static_cast<size_t>(input)] * mapping;
		}
	}
}