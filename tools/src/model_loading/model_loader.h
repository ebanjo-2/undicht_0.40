#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <vector>
#include <string>
#include <buffer_layout.h>
#include "images/image_file.h"

namespace undicht {

	namespace tools {


		struct MeshData {

			std::vector<float> vertices;
			std::vector<int> indices;

			BufferLayout vertex_layout;

			// ids of the textures used by this mesh
			int color_texture = -1;

		};

		class ModelLoader {
			/** the base class to all classes that load Meshes, Textures ... from files */

		public:

			ModelLoader() = default;
			virtual ~ModelLoader() = default;

		public:
			/** the functions that should be implemented by derived model loading classes */

			////////////////////////////////////////// functions to load single meshes / textures ///////////////////////////////////////////

			/** @return the number of meshes stored in the file */
			virtual int getMeshCount() = 0;

			/** @return the number of unique textures used by the meshes
			* if a mesh should use for example a color + normal texture, thats 2 */
			virtual int getTextureCount() = 0;

			/** loads the vertices of the mesh
			* @param id: to iterate through the meshes of the file */
			virtual void getMesh(MeshData& loadTo_mesh, unsigned int id = 0) = 0;

			/** @param id: to iterate through the texture of the file */
			virtual void getTexture(ImageData& loadTo_texture, int id = 0) = 0;

			////////////////////////////////////////// functions to load all meshes / textures ///////////////////////////////////////////

			virtual void loadAllMeshes(std::vector<MeshData>& loadTo_meshes) = 0;

			virtual void loadAllTextures(std::vector<ImageData>& loadTo_textures) = 0;

		protected:
			// universal functions that may be useful for loading models

			/** some file formats may store the attributes in a different order (i.e. pos, uv, normal or pos, normal, uv)
			* and with them the attribute indices. since undicht uses always the same order (pos, uv, normal), the indices may have to be rearranged
			* @param attribute_indices: the indices as they come from the file, @param new_order: the way they have to be rearranged to form the default order */
			void rearrangeAttribIndices(const std::vector<int>& attrib_indices, std::vector<int> new_order, std::vector<int>& loadTo);

			/** takes an attribute (size determined by vertex_layout, can be UND_FLOAT, UND_VEC3F, ...) from each attribute_data list,
			* which attribute is determined by the attribute index, to build the vertices */
			virtual void buildMeshVertices(std::vector<float>& loadTo_vertices, const std::vector<std::vector<float>>& vertex_attributes,
				const BufferLayout& vertex_layout, const std::vector<int>& attribute_indices);


			/** removes double vertices by adding indices referencing the first version of that vertex to the loadTo_indices vector*/
			virtual void buildIndices(const std::vector<float>& vertices, const BufferLayout& vertex_layout, std::vector<float>& loadTo_vertices, std::vector<int>& loadTo_indices);

		};

	} // tools

} // undicht


#endif // MODEL_LOADER_H
