#ifndef PERSPECTIVE_CAMERA_3D_H
#define PERSPECTIVE_CAMERA_3D_H

#include "camera_3d.h"
#include <math/perspective_projection.h>

namespace undicht {

	namespace tools {

		class PerspectiveCamera3D : public Camera3D, public tools::PerspectiveProjection {

		public:

			virtual const glm::mat4& getCameraProjectionMatrix();

			PerspectiveCamera3D();
			virtual ~PerspectiveCamera3D();

		};

	} // tools

} // undicht

#endif // PERSPECTIVE_CAMERA_3D_H
