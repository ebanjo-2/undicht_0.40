#ifndef ORIENTATION_H
#define ORIENTATION_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>


namespace undicht {

	namespace tools {

		enum Axis {
			UND_X_AXIS, // default orientation: right is positive x direction
			UND_Y_AXIS, // default orientation: up is positive y direction
			UND_Z_AXIS  // default orientation: back is positive z direction
		};


		class Orientation3D {
			/** a class that has functions to rotate and translate an Object */

		protected:
			// members to correctly copy the relative orientation reference

			// this pointer will point to the object this orientations data was copied to
			// so that the orientations relative to the object copied to can access that object when being copied
			Orientation3D* m_last_copy = 0;
			int m_childs_left_to_copy = 0; // the number of childs left to be copied after this orientation was copied

			// number of orientations who use this one as their relative orientation
			int m_total_childs = 0;

		public:
			// members to store the orientation

			Orientation3D* m_relative_orientation = 0;

			glm::vec3 m_position;
			glm::quat m_rotation;
			glm::vec3 m_scale;

			// whether the matrices representing the transformations need to be updated
			bool m_update_pos = true;
			bool m_update_rot = true;
			bool m_update_transf = true;

			glm::mat4 m_transl_mat;
			glm::mat4 m_rot_mat;
			glm::mat4 m_transf_mat; // transformation relative to relative_orientation


		protected:
			// functions to update matrices
			// can be reimplemented by other classes to also update their matrices (i.e. view matrix)

			/** called whenever the translation matrix needs to be updated */
			virtual void updatePosition();

			virtual void updateRotation();

			virtual void updateTransf();

		public:
			// translation

			void setPosition(const glm::vec3& position);

			// the position in relation to the relative orientation
			const glm::vec3& getPosition() const;
			/** @return a matrix, that multiplied with a 4D vector (x,y,z,1) will result in a 3D vector (x + pos.x, y + pos.y, z + pos.z, 1) */
			const glm::mat4& getTranslMat();

			// the absolute position in the world
			glm::vec3 getWorldPosition() const;
			glm::mat4 getWorldTranslMat();

			/** moves the object */
			void addTranslation(const glm::vec3& v);


		public:
			// rotation

			// rotation around the orientations world position
			void setRotation(const glm::quat& rot);

			/** Rotation around multiple axis
			* @param angles: in degrees
			* @param axes: the axis around which to rotate (Order of rotation matters!!)
			* @param axes: default is euler rotation: z (roll), x (pitch), y (yaw)*/
			void setAxesRotation(std::vector<float> angles, std::vector<Axis> axes = std::vector<Axis>({ UND_Z_AXIS, UND_X_AXIS, UND_Y_AXIS }));

			const glm::quat& getRotation() const;
			const glm::mat4& getRotMat();

			// the absolute rotation relative to the world
			glm::quat getWorldRot() const;
			glm::mat4 getWorldRotMat();

		public:
			// scaling

			/** scaling the translation of child elements + can be used to scale the model */
			void setScale(const glm::vec3& scale);

			const glm::vec3& getScale() const;

			// scale multiplied by the scale of the orientations parents
			glm::vec3 getWorldScale() const;

		public:
			// transformation (translation + rotation combined)

			const glm::mat4& getTransfMat();

			// the absolute transformation relative to the world
			glm::mat4 getWorldTransfMat();

			/** sets this orientations transformation (position + rotation) to be relative to the orientation passed
			* @param : if 0 is passed, the transformation will be relative to the world */
			void setTransfRelTo(Orientation3D* rel_transf);

		public:

			void operator= (const Orientation3D& o);

			Orientation3D();
			Orientation3D(const Orientation3D& o);
			virtual ~Orientation3D();

		};

	} // tools

} // undicht

#endif // ORIENTATION_H
