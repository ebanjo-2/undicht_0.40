#include "camera_3d.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

namespace undicht {

	namespace tools {

		Camera3D::Camera3D() {
			//ctor
		}

		Camera3D::~Camera3D() {
			//dtor
		}

		void Camera3D::updateTransf() {

			//std::cout << "updating transf" << "\n";

			// rotation has to be reversed too, but i dont know if -1 * getRotation()) * glm::translate(glm::mat4(1.0f) is right
			m_view_mat = glm::toMat4(glm::inverse(getRotation())) * glm::translate(glm::mat4(1.0f), -1.0f * getPosition());
			m_transf_mat = glm::toMat4(getRotation()) * glm::translate(glm::mat4(1.0f), getPosition());
		}

		void Camera3D::updateRotation() {

			// updating the view direction as well as the rotation matrix
			m_rot_mat = glm::toMat4(getRotation());

			m_view_dir = glm::rotate(getRotation(), glm::vec3(0, 0, 1));
			m_up_dir = glm::rotate(getRotation(), glm::vec3(0, 1, 0));
			m_right_dir = glm::rotate(getRotation(), glm::vec3(1, 0, 0));
		}



		const glm::mat4& Camera3D::getViewMatrix() {

			if (m_update_transf) {

				updateTransf();
				m_update_transf = false;
			}

			return m_view_mat;
		}

		const glm::vec3& Camera3D::getViewDirection() {
			/** a unit vector pointing in the direction the camera is "facing" */

			if (m_update_rot) {

				updateRotation();
				m_update_rot = false;
			}

			return m_view_dir;
		}


		const glm::vec3& Camera3D::getUpDirection() {

			if (m_update_rot) {

				updateRotation();
				m_update_rot = false;
			}

			return m_up_dir;
		}

		const glm::vec3& Camera3D::getRightDirection() {

			if (m_update_rot) {

				updateRotation();
				m_update_rot = false;
			}

			return m_right_dir;
		}

	} // tools

} // undicht
