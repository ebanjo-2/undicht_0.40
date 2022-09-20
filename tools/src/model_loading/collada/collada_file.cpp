#include "collada_file.h"

#include "debug.h"
#include <file_tools.h>

namespace undicht {

	namespace tools {

		const std::vector<std::string> primitive_types{ "polylist", "triangles", "polygons", "lines", "trifans", "tristrips", "linestrips" };


		ColladaFile::ColladaFile() {
			//ctor
		}

		ColladaFile::ColladaFile(const std::string& file_name) {

			open(file_name);
		}

		ColladaFile::~ColladaFile() {
			//dtor
		}

		////////////////////////////////////////////////// ModelLoader api functions //////////////////////////////////////////////////
		////////////////////////////////////////// functions to load single meshes / textures /////////////////////////////////////////


		int ColladaFile::getMeshCount() {
			/** @return the number of meshes stored in the file */

			return getAllElements({ "COLLADA", "library_geometries", "geometry" }).size();
		}


		int ColladaFile::getTextureCount() {
			/** @return the number of unique textures used by the meshes
			* if a mesh should use for example a color + normal texture, thats 2 */

			return getAllElements({ "COLLADA", "library_materials", "material" }).size();
		}


		void ColladaFile::getMesh(MeshData& loadTo_mesh, unsigned int id) {
			/** loads the vertices of the mesh
			* @param id: to iterate through the meshes of the file */

			// all geometries stored in the file
			std::vector<XmlElement*> geometries = getAllElements({ "COLLADA", "library_geometries", "geometry" });

			if (geometries.size() <= id) return;
			XmlElement* geom_element = geometries.at(id);

			// loading the mesh data
			loadGeometry(*geom_element, loadTo_mesh.vertices, loadTo_mesh.vertex_layout);

			// finding the right textures for the model
			std::vector<XmlElement*> materials = getAllElements({ "COLLADA", "library_materials", "material" }); // all materials stored in the file

			XmlElement* mesh = geom_element->getElement({ "mesh" });
			std::string material_id;

			// getting the material name
			for (const std::string& primitive_type : primitive_types) {

				XmlElement* primitive = mesh->getElement({ primitive_type });

				if (primitive) {
					const XmlTagAttrib* mat_attrib = primitive->getAttribute("material");

					if (mat_attrib) {

						material_id = mat_attrib->m_value;
					}

					break;
				}
			}

			// getting the id of the material containing the texture
			// since there is one texture per material (no bump-mapping rn), that id is the same for material and texture
			for (unsigned int i = 0; i < materials.size(); i++) {
				if (!materials.at(i)->getAttribute("id")->m_value.compare(material_id)) {
					// found the right material
					loadTo_mesh.color_texture = i;
					break;
				}
			}

		}

		void ColladaFile::getTexture(ImageData& loadTo_texture, int id) {
			/** @param id: to iterate through the texture of the file */

			std::vector<XmlElement*> materials = getAllElements({ "COLLADA", "library_materials", "material" }); // all materials stored in the file

			loadMaterialTextures(materials.at(id), loadTo_texture);

		}

		////////////////////////////////////////// functions to load all meshes / textures ///////////////////////////////////////////

		void ColladaFile::loadAllMeshes(std::vector<MeshData>& loadTo_meshes) {

			// all materials stored in the file
			std::vector<XmlElement*> materials = getAllElements({ "COLLADA", "library_materials", "material" });

			// all geometries stored in the file
			std::vector<XmlElement*> geometries = getAllElements({ "COLLADA", "library_geometries", "geometry" });


			for (XmlElement* e : geometries) {

				loadTo_meshes.emplace_back(MeshData());

				loadGeometry(*e, loadTo_meshes.back().vertices, loadTo_meshes.back().vertex_layout);

				// finding the material to the mesh
				XmlElement* mesh = e->getElement({ "mesh" });
				std::string material_id;

				// getting the material name
				for (const std::string& primitive_type : primitive_types) {

					XmlElement* primitive = mesh->getElement({ primitive_type });

					if (primitive) {
						material_id = primitive->getAttribute("material")->m_value;
						break;
					}
				}

				// getting the id of the material containing the texture
				// since there is one texture per material (no bump-mapping rn), that id is the same for material and texture
				for (unsigned int i = 0; i < materials.size(); i++) {
					if (!materials.at(i)->getAttribute("id")->m_value.compare(material_id)) {
						// found the right material
						loadTo_meshes.back().color_texture = i;
						break;
					}
				}

			}
		}

		void ColladaFile::loadAllTextures(std::vector<ImageData>& loadTo_textures) {

			// all materials stored in the file
			std::vector<XmlElement*> materials = getAllElements({ "COLLADA", "library_materials", "material" });

			for (XmlElement* material : materials) {
				loadTo_textures.emplace_back(ImageData());
				loadMaterialTextures(material, loadTo_textures.back());
			}

		}


		///////////////////////////// functions to bring more structure to the loading process /////////////////////////////////////


		void ColladaFile::loadGeometry(XmlElement& geometry, std::vector<float>& vertices, BufferLayout& vertex_layout) {
			/** loading the vertices from a geometry element */

			// getting vertex data
			XmlElement* mesh = geometry.getElement({ "mesh" });
			if (!mesh)
				return;

			XmlElement* position_source = getMeshSource(mesh, "POSITION");
			XmlElement* uv_source = getMeshSource(mesh, "TEXCOORD");
			XmlElement* normal_source = getMeshSource(mesh, "NORMAL");

			std::vector<std::vector<float>> attribute_data;

			if (position_source) {
				attribute_data.emplace_back(std::vector<float>());
				extractFloatArray(attribute_data.back(), position_source->getContent(), -1);
				vertex_layout.m_types.push_back(UND_VEC3F);
			}

			if (uv_source) {
				attribute_data.emplace_back(std::vector<float>());
				extractFloatArray(attribute_data.back(), uv_source->getContent(), -1);
				vertex_layout.m_types.push_back(UND_VEC2F);
			}

			if (normal_source) {
				attribute_data.emplace_back(std::vector<float>());
				extractFloatArray(attribute_data.back(), normal_source->getContent(), -1);
				vertex_layout.m_types.push_back(UND_VEC3F);
			}

			// getting attribute index data
			XmlElement* index_source = 0;
			for (const std::string& primitive_type : primitive_types) {
				index_source = mesh->getElement({ primitive_type, "p" });
				if (index_source)
					break;
			}

			if (!index_source)
				return;

			std::vector<int> attrib_indices, rearranged_attr_indices;
			extractIntArray(attrib_indices, index_source->getContent(), -1);

			rearrangeAttribIndices(attrib_indices, { 0,2,1 }, rearranged_attr_indices);

			// building the vertices
			buildMeshVertices(vertices, attribute_data, vertex_layout, rearranged_attr_indices);

		}


		XmlElement* ColladaFile::getMeshSource(XmlElement* mesh, const std::string& source_name) {
			/** @param source_name: POSITION, NORMAL or TEXCOORD
			* @return the element containing the actual data (float array) */

			if (!mesh)
				return 0;

			bool position_source = !source_name.compare("POSITION");

			XmlElement* input_semantic = 0;

			if (position_source) {

				input_semantic = mesh->getElement({ "vertices", "input semantic=" + ('"' + source_name) + '"' });
			}
			else {

				for (const std::string& primitive_type : primitive_types) {

					input_semantic = mesh->getElement({ primitive_type, "input semantic=" + ('"' + source_name) + '"' });

					if (input_semantic)
						break;
				}
			}

			if (!input_semantic)
				return 0;

			const XmlTagAttrib* source_attribute = input_semantic->getAttribute("source");
			if (!source_attribute)
				return 0;

			std::string source_id = source_attribute->m_value;
			source_id.erase(1, 1); // they put a stupid # here

			return mesh->getElement({ "source id=" + source_id, "float_array" });
		}

		//////////////////////////////// functions to load textures for a material ////////////////////////////////

		void ColladaFile::loadMaterialTextures(XmlElement* material, ImageData& loadTo_texture) {
			// its a long and tedious process to get the name of the Texture-File ...

			XmlElement* instance_effect = material->getElement({ "instance_effect" });
			if (!instance_effect)
				return;

			const XmlTagAttrib* effect_url = instance_effect->getAttribute("url");
			if (!effect_url)
				return;

			std::string effect_id = effect_url->m_value;
			effect_id.erase(1, 1); // removing stupid #
			XmlElement* effect = getElement({ "COLLADA", "library_effects", "effect id=" + effect_id });
			if (!effect)
				return;

			XmlElement* diffuse_texture = effect->getElement({ "profile_COMMON", "technique", "phong", "diffuse", "texture" });
			if (!diffuse_texture)
				return;

			const XmlTagAttrib* sampler_name = diffuse_texture->getAttribute("texture");
			if (!sampler_name)
				return;

			XmlElement* sampler_2D_source = effect->getElement({ "profile_COMMON", "newparam sid=" + sampler_name->m_value, "sampler2D", "source" });
			if (!sampler_2D_source)
				return;

			XmlElement* image_name = effect->getElement({ "profile_COMMON", "newparam sid=" + ('"' + sampler_2D_source->getContent()) + '"', "surface", "init_from" });
			if (!image_name)
				return;

			XmlElement* image_file_name = getElement({ "COLLADA", "library_images", "image id=" + ('"' + image_name->getContent()) + '"', "init_from" });
			if (!image_file_name)
				return;

            ImageFile image_file(getFilePath(m_file_name) + image_file_name->getContent(), loadTo_texture);

		}

	} // tools

} // undicht
