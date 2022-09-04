#ifndef PERSPECTIVE_PROJECTION_H
#define PERSPECTIVE_PROJECTION_H

#include <glm/glm.hpp>

namespace undicht {

	namespace tools {

		class PerspectiveProjection {

		protected:
			// members describing the projection

			float m_near_plane = 0.1f; // which z coordinate should be mapped to a depth of 0
			float m_far_plane = 1000.0f; // which z coordinate should be mapped to a depth of 1

			float m_field_of_view = 1.2;//1.75; // about 100 degrees of vision

			// captured width / height
			// i have a resolution of 1680 / 1050, so default aspect ratio is 1.6
			float m_aspect_ratio = 1.6;

			// whether to update the projection matrix
			bool m_update_proj = true;
			glm::mat4 m_proj_mat;

		public:
			// setting and getting params

			void setViewRange(float near_plane, float far_plane);
			float getNearPlane();
			float getFarPlane();
			/** @return far_plane - near_plane */
			float getViewRange();

			/** @param fov: all angels are radian */ // angels?
			void setFoV(float fov);
			float getFoV();

			/** captured width / height
			* i have a resolution of 1680 / 1050, so default aspect ratio is 1.6 */
			void setAspectRatio(float aspect_ratio);
			float getAspectRatio();

		public:
			// returning the projection matrix

			/** @return the projection matrix that can transform 3D positions into screen space */
			glm::mat4& getProjectionMatrix();


		public:

			PerspectiveProjection();
			virtual ~PerspectiveProjection();

		};

	} // tools

} // undicht

#endif // PERSPECTIVE_PROJECTION_H
