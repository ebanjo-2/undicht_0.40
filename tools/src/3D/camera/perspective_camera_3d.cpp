#include "perspective_camera_3d.h"

namespace undicht {

	namespace tools {

		PerspectiveCamera3D::PerspectiveCamera3D() {
			//ctor
		}

		PerspectiveCamera3D::~PerspectiveCamera3D() {
			//dtor
		}

		const glm::mat4& PerspectiveCamera3D::getCameraProjectionMatrix() {

			// PerspectiveProjection::getProjectionMatrix()
			return getProjectionMatrix();
		}

	} // tools

} // undicht
