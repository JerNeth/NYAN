#pragma once

namespace nyan {
	struct PerspectiveCamera {
		float nearPlane { 0.1};
		float farPlane {1000};
		float fovX {75};
		float aspect { 16. / 9};
		Math::vec3 forward {1, 0, 0};
		Math::vec3 up {0, 1, 0};
	};
}