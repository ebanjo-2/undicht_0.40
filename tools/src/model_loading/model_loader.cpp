#include "model_loader.h"
#include "debug.h"

namespace undicht {

	namespace tools {


		//////////////////////////////////////////// universal functions that may be useful for loading models ////////////////////////////////////////////

		void ModelLoader::rearrangeAttribIndices(const std::vector<int>& attrib_indices, std::vector<int> new_order, std::vector<int>& loadTo) {
			/** some file formats may store the attributes in a different order (i.e. pos, uv, normal or pos, normal, uv)
			* and with them the attribute indices. since undicht uses always the same order (pos, uv, normal), the indices may have to be rearranged
			* @param attribute_indices: the indices as they come from the file, @param new_order: the way they have to be rearranged to form the default order */

			int attributes_per_vertex = new_order.size();
			int vertex_count = attrib_indices.size() / attributes_per_vertex;

			for (int vertex_id = 0; vertex_id < vertex_count; vertex_id += 1) {
				// going through each vertex

				for (int attrib = 0; attrib < attributes_per_vertex; attrib++) {
					// going through each attribute index of the new indices list

					loadTo.push_back(attrib_indices.at(vertex_id * attributes_per_vertex + new_order.at(attrib)));
				}

			}

		}


		void ModelLoader::buildMeshVertices(std::vector<float>& loadTo_vertices, const std::vector<std::vector<float>>& vertex_attributes,
			const BufferLayout& vertex_layout, const std::vector<int>& attribute_indices) {
			/** takes an attribute (size determined by vertex_layout, can be UND_FLOAT, UND_VEC3F, ...) from each attribute_data list,
			* which attribute is determined by the attribute index, to build the vertices */

			int attributes_per_vertex = vertex_layout.m_types.size();
			int vertex_count = attribute_indices.size() / attributes_per_vertex;

			//std::cout << "attributes: " << attributes_per_vertex << " vertex_count: " << vertex_count << "\n";

			std::vector<int> attribute_sizes; // the number of floats of each attribute used in a vertex

			for (const FixedType& attribute_type : vertex_layout.m_types) {

				attribute_sizes.push_back(attribute_type.m_num_components);
				//std::cout << "attribute size: " << core::getNumberOfComponents(attribute_type) << "\n";
			}

			for (int vertex_id = 0; vertex_id < vertex_count; vertex_id++) {
				// going through each vertex

				for (int attrib = 0; attrib < attributes_per_vertex; attrib++) {
					// going through each attribute

					const std::vector<float>& current_attrib_list = vertex_attributes.at(attrib);
					int index = attribute_indices.at(vertex_id * attributes_per_vertex + attrib);

					std::vector<float>::const_iterator data_begin = current_attrib_list.begin() + index * attribute_sizes.at(attrib);
					std::vector<float>::const_iterator data_end = data_begin + attribute_sizes.at(attrib);

					loadTo_vertices.insert(loadTo_vertices.end(), data_begin, data_end);
				}

			}

		}

		void ModelLoader::buildIndices(const std::vector<float>& vertices, const BufferLayout& vertex_layout, std::vector<float>& loadTo_vertices, std::vector<int>& loadTo_indices) {
			/** removes double vertices by adding indices referencing the first version of that vertex to the loadTo_indices vector*/

			int vertex_size = vertex_layout.getTotalSize() / sizeof(float);
			int vertex_count = vertices.size() / vertex_size;

			for (int vertex = 0; vertex < vertex_count; vertex++) {

				int indexed_vertex; // the index of the vertex in loadTo_vertices equal to the current vertex
				bool vertices_are_equal = false; // (only if this is true)

				// comparing the vertex to the ones that where already confirmed unique
				for (indexed_vertex = 0; indexed_vertex < loadTo_vertices.size() / vertex_size; indexed_vertex++) {

					vertices_are_equal = true;

					// testing if one float is different
					for (int f = 0; f < vertex_size; f++) {

						if (vertices.at(vertex * vertex_size + f) != loadTo_vertices.at(indexed_vertex * vertex_size + f)) {
							// vertices are not equal
							vertices_are_equal = false;
							break;
						}

					}

					if (vertices_are_equal) {
						// found one, adding the index to the indices list
						//std::cout << "found an redundant vertex " << "\n";
						loadTo_indices.push_back(indexed_vertex);
						break;
					}

				}

				if (!vertices_are_equal) {
					// no equal vertices were found, so adding the vertex to loadTo_vertices
					// std::cout << "vertex is unique" << "\n";
					loadTo_indices.push_back(loadTo_vertices.size() / vertex_size);
					loadTo_vertices.insert(loadTo_vertices.end(), vertices.begin() + vertex * vertex_size, vertices.begin() + (vertex + 1) * vertex_size);
				}

			}

		}



	} // tools

} // tools
