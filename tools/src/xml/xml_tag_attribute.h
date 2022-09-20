#ifndef XML_TAG_ATTRIBUTE_H
#define XML_TAG_ATTRIBUTE_H

#include <string>

namespace undicht {

	namespace tools {

		class XmlTagAttrib {
			/// a class containing the content of a single xml tag attribute
		public:

			std::string m_name;
			std::string m_value;

			/** @param data would be something like "name="first_tag_attribute""
			* "name" would be the attrib name and "first_tag_attribute" its value */
			void setData(const std::string& data);

			/** @example tag attributes can be negated using "!=" */
			bool operator== (const std::string& data) const;

			XmlTagAttrib();
			XmlTagAttrib(const std::string& data);
			virtual ~XmlTagAttrib();

		};

	} // tools

} // undicht

#endif // XML_TAG_ATTRIBUTE_H
