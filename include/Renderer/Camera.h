#pragma once

namespace nyan {
	struct PerspectiveCamera {
		float nearPlane { 0.1f};
		float farPlane {1000.f };
		float fovX {75.f };
		float aspect { 16.f / 9.f };
		Math::vec3 forward {1.f, 0.f, 0.f};
		Math::vec3 up{ 0.f, 1.f, 0.f };
		Math::vec3 right{ 0.f, 0.f, 1.f };
	};
}