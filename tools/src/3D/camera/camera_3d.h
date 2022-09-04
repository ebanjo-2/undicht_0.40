#ifndef CAMERA_3D_H
#define CAMERA_3D_H

#include <math/orientation_3d.h>

namespace undicht {

	namespace tools {

		class Camera3D : public tools::Orientation3D {

		public:

			glm::mat4 m_view_mat;
			glm::vec3 m_view_dir;
			glm::vec3 m_up_dir;
			glm::vec3 m_right_dir;

		protected:
			// function called when the transformation matrix needs to be updated, and with it the view matrix

			virtual void updateTransf();
			virtual void updateRotation();

		public:
			// functions returning camera matrices

			const glm::mat4& getViewMatrix();
			virtual const glm::mat4& getCameraProjectionMatrix() = 0;

			/** a unit vector pointing in the direction the camera is "facing" */
			const glm::vec3& getViewDirection();

			const glm::vec3& getUpDirection();

			const glm::vec3& getRightDirection();

			Camera3D();
			virtual ~Camera3D();

		};

	} // tools

} // undicht

#endif // CAMERA_3D_H
