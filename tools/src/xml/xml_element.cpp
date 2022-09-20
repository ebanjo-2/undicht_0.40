#include "xml_element.h"
#include <iostream>
#include <algorithm>


namespace undicht {

	namespace tools {

		XmlElement::XmlElement() {
			//ctor
		}

		XmlElement::XmlElement(XmlElement* parent) {

			m_parent_element = parent;
		}


		XmlElement::~XmlElement() {
			//dtor
		}


		///////////////////////////////////////// functions to access the data stored in the element //////////////////////////

		bool XmlElement::hasAttributes(const std::vector<std::string>& tag_attributes) const {
			/// @return whether the element has all the attributes (might have more)

			int attributes_found = 0; // has to equal the length of tag_attributes at the end

			for (const std::string& param_attr : tag_attributes) {

				for (const XmlTagAttrib& elem_attr : m_tag_attributes) {

					if (elem_attr == param_attr) {

						attributes_found += 1;

						break; // searching for the next attribute requested
					}

				}

			}

			return (attributes_found == tag_attributes.size());
		}

		bool XmlElement::hasChildElements(const std::vector<std::string>& elem_names) const {

			int elements_found = 0; // has to equal the length of elem_names at the end

			for (const std::string& name : elem_names) {

				for (const XmlElement& child_elem : m_child_elements) {

					if (!name.compare(child_elem.getName())) {

						elements_found += 1;

						break; // searching for the next attribute requested
					}

				}

			}

			return (elements_found == elem_names.size());
		}

		const std::string& XmlElement::getName() const {

			return m_tag_name;
		}

		const std::string& XmlElement::getContent() const {
			/// @return the content stored between the start and end tag of the element (excluding child elements)

			return m_content; // this one is easy
		}

		const XmlTagAttrib* XmlElement::getAttribute(const std::string& attr_name) {

			for (XmlTagAttrib& attr : m_tag_attributes) {

				if (!attr.m_name.compare(attr_name)) {

					return &attr;
				}

			}

			return 0;
		}

		XmlElement* XmlElement::getElement(const std::vector<std::string>& attribute_strings, int attrib_num) {
			/** searches the elements children for the first one which has the attributes stored in the attribute string at attrib_num
			* if multiple attribute strings are provided, its children in return will be checked
			* @param attrib_num: needed so that the function can be used recursivly (what attribute string to use)
			* @return 0 if the element could not be found */

			// splitting the attributes
			std::string elem_name;
			std::vector<std::string> attributes = splitAttributeString(attribute_strings.at(attrib_num), elem_name);

			// searching for a child element
			for (XmlElement& elem : m_child_elements) {

				if (elem.hasAttributes(attributes) && (!elem.getName().compare(elem_name))) {
					// found the element matching the current attributes

					if (attrib_num + 1 >= attribute_strings.size()) {
						// last element of the search queue
						return &elem;
					}
					else {
						// the search continues
						return elem.getElement(attribute_strings, attrib_num + 1);
					}

				}

			}

			return 0;
		}

		std::vector<XmlElement*> XmlElement::getAllElements(const std::vector<std::string>& attribute_strings, int attrib_num) {
			/** @return all xml elements that have all the requested tag attributes */

			// splitting the attributes
			std::string elem_name;
			std::vector<std::string> attributes = splitAttributeString(attribute_strings.at(attrib_num), elem_name);

			std::vector<XmlElement*> elements;

			// searching for child elements
			for (XmlElement& elem : m_child_elements) {

				if (elem.hasAttributes(attributes) && (!elem.getName().compare(elem_name))) {
					// found an element matching the current attributes

					if (attrib_num + 1 >= attribute_strings.size()) {
						// at end of search queue
						elements.push_back(&elem);
					}
					else {
						// the search continues

						std::vector<XmlElement*> new_elements = elem.getAllElements(attribute_strings, attrib_num + 1);
						elements.insert(elements.end(), new_elements.begin(), new_elements.end());
					}

				}
			}


			return elements;
		}

		std::vector<std::string> XmlElement::splitAttributeString(std::string attribute_string, std::string& loadTo_name) const {
			/// the attribute string should look like this "name attr0=val0 attr1=val1 ..."

			std::vector<std::string> attributes;

			// loading the name
			size_t attr_start = 0;
			size_t attr_end = attribute_string.find(' ');

			if (attr_end == std::string::npos) {
				// there probably are no attributes following the name
				attr_end = attribute_string.size();
			}

			loadTo_name = attribute_string.substr(attr_start, attr_end - attr_start);

			// loading the attributes
			while (attr_end != attribute_string.size()) {

				attr_start = attr_end + 1;
				attr_end = attribute_string.find(' ', attr_start);

				if (attr_end == std::string::npos) {
					// there probably are no further attributes
					attr_end = attribute_string.size();
				}

				attributes.push_back(attribute_string.substr(attr_start, attr_end - attr_start));

			}

			return attributes;
		}


		////////////////////////////////////////// functions to print the content of the element ////////////////////////////////////////

		void XmlElement::printShortInfo(int indent) const {

			for (int i = 0; i < indent; i++) {
				// there has to be a more efficient way
				std::cout << " ";
			}

			std::cout << "<" << m_tag_name;
			for (const XmlTagAttrib& attr : m_tag_attributes) {

				std::cout << " " << attr.m_name << "=" << attr.m_value;

			}
			std::cout << ">";

			// content
			if (m_content.size() < 40) {
				std::cout << " " << m_content;
			}
			else {
				std::cout << "content with more then 40 characters";
			}


			std::cout << " <>\n";

		}

		void XmlElement::printRecursive(int indent) const {

			printShortInfo(indent);

			for (const XmlElement& child : m_child_elements) {

				child.printRecursive(indent + 2);
			}

		}

		/////////////////////////////////////////////// functions to set the elements data ///////////////////////////////////////////////

		XmlElement* XmlElement::addChildElement() {

			m_child_elements.emplace_back(XmlElement(this));

			return &m_child_elements.back();
		}

		XmlElement* XmlElement::getParentElement() {

			return m_parent_element;
		}


		void XmlElement::setData(const std::string& line) {
			/** extracts data from the line which can be read from a xml file
			* @param a line containing the start tag and possibly the content of a xml element
			* start_tag: the start tag as it can be found in an xml file
			* @example <texture width="256" height="256"> */

			// tag name
			size_t tag_start = line.find('<');
			size_t tag_end = line.find('>');
			size_t name_length = std::min(tag_end - tag_start, line.find(' ') - tag_start) - 1;

			m_tag_name = line.substr(tag_start + 1, name_length);

			// tag attributes
			size_t attr_start = tag_start + name_length + 1; // before the ' ', the +1 is for the '<'
			size_t attr_end = 0;
			size_t attr_length = 0;
			while (attr_start < tag_end) {

				attr_start = line.find(' ', attr_start) + 1;
				attr_end = std::min(std::min(line.find(' ', attr_start), line.find('>', attr_start)), line.find('/', attr_start));
				attr_length = attr_end - attr_start;

				if (attr_start - 1 == std::string::npos) {
					break;
				}

				m_tag_attributes.emplace_back(XmlTagAttrib(line.substr(attr_start, attr_length)));

			}

			// content
			m_content = line.substr(tag_end + 1, line.find('<', tag_end + 1) - tag_end - 1);

		}

	} // tools

} // undicht
