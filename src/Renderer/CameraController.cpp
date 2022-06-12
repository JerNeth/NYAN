#include "Renderer/CameraController.h"
#include "Renderer/RenderManager.h"
#include "Core/Input.h"

nyan::CameraController::CameraController(RenderManager& renderManager, Input& input) :
	r_renderManager(renderManager),
	r_input(input)
{
}

void nyan::CameraController::update(std::chrono::nanoseconds dt)
{
	auto camera = r_renderManager.get_primary_camera();
	auto dtf = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
	auto& transform = r_renderManager.get_registry().get<Transform>(camera);
	auto& perspectiveCamera = r_renderManager.get_registry().get<PerspectiveCamera>(camera);

	transform.orientation.x() += dtf * (45.f) * r_input.get_axis(Input::Axis::LookUp);
	transform.orientation.y() += dtf * (45.f) * r_input.get_axis(Input::Axis::LookRight);

	auto mat = Math::mat33::rotation_matrix(transform.orientation);

	transform.position += mat * perspectiveCamera.right * dtf * 100 * r_input.get_axis(Input::Axis::MoveRight);
	transform.position += mat * perspectiveCamera.forward * dtf * 100 * r_input.get_axis(Input::Axis::MoveForward);
}
