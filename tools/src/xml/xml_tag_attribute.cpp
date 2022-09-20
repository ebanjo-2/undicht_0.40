#include "xml_tag_attribute.h"
#include <iostream>


namespace undicht {

	namespace tools {

		XmlTagAttrib::XmlTagAttrib() {
			//ctor
		}

		XmlTagAttrib::XmlTagAttrib(const std::string& data) {
			setData(data);
		}

		XmlTagAttrib::~XmlTagAttrib() {
			//dtor
		}


		void XmlTagAttrib::setData(const std::string& data) {
			/** @param data would be something like "name="first_tag_attribute""
			* "name" would be the attrib name and "first_tag_attribute" its value */

			// splitting the data into name and value
			size_t split_pos = data.find("=");
			m_name = data.substr(0, split_pos);
			m_value = data.substr(split_pos + 1); // skipping the "="

		}


		bool XmlTagAttrib::operator== (const std::string& data) const {
			/** @example tag attributes can be negated using "!=" */

			bool negate_value = (data.find("!=") != std::string::npos);

			size_t split_pos = data.find("=");

			if (split_pos == std::string::npos) {
				// no attribute found
				return false;
			}

			std::string name = data.substr(0, split_pos - negate_value);
			std::string value = data.substr(split_pos + 1); // skipping the "="

			if (!m_name.compare(name)) {

				if (bool(m_value.compare(value)) == negate_value) {
					// attribute data equal
					return true;
				}
			}

			// attribute data not equal
			return false;
		}

	} // tools

} // undicht
