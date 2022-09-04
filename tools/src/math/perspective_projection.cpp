#include "perspective_projection.h"
#include <glm/gtc/matrix_transform.hpp>



namespace undicht {

	namespace tools {

		PerspectiveProjection::PerspectiveProjection() {
			//ctor
		}

		PerspectiveProjection::~PerspectiveProjection() {
			//dtor
		}


		//////////////////////////////////////// setting and getting params ////////////////////////////////////////

		void PerspectiveProjection::setViewRange(float near_plane, float far_plane) {

			m_near_plane = near_plane;
			m_far_plane = far_plane;
			m_update_proj = true;
		}

		float PerspectiveProjection::getNearPlane() {

			return m_near_plane;
		}

		float PerspectiveProjection::getFarPlane() {

			return m_far_plane;
		}


		float PerspectiveProjection::getViewRange() {
			/** @return far_plane - near_plane */

			return m_far_plane - m_near_plane;
		}


		void PerspectiveProjection::setFoV(float fov) {
			/** @param fov: all angels are radian */

			m_field_of_view = fov;
			m_update_proj = true;
		}

		float PerspectiveProjection::getFoV() {

			return m_field_of_view;
		}


		void PerspectiveProjection::setAspectRatio(float aspect_ratio) {
			/** captured height / width
			* i have a resolution of 1050 / 1680, so default aspect ratio is 0.625 */

			m_aspect_ratio = aspect_ratio;
			m_update_proj = true;
		}

		float PerspectiveProjection::getAspectRatio() {

			return m_aspect_ratio;
		}

		//////////////////////////////////////// returning the projection matrix ////////////////////////////////////////


		glm::mat4& PerspectiveProjection::getProjectionMatrix() {
			/** @return the projection matrix that can transform 3D positions into screen space */

			if (m_update_proj) {
				// fov for some reason has to be in degrees
				m_proj_mat = glm::perspective(glm::degrees(m_field_of_view), m_aspect_ratio, m_near_plane, m_far_plane);
				m_update_proj = false;
			}

			return m_proj_mat;
		}

	} // tools

} // undicht
