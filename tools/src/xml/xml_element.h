#ifndef XML_ELEMENT_H
#define XML_ELEMENT_H

#include <string>
#include <vector>
#include "xml_tag_attribute.h"



namespace undicht {

	namespace tools {

		class XmlElement {

		public:
			// the data a xml element can store
			std::string m_tag_name;
			std::vector<XmlTagAttrib> m_tag_attributes;
			std::string m_content;

			// each element can contain multiple child elements
			std::vector<XmlElement> m_child_elements;

			// the parent element
			XmlElement* m_parent_element = 0;


		public:
			// functions to access the data stored in the element

			/** @return whether the element has all the attributes (might have more)
			* @example tag attributes can be negated using a "!=" */
			bool hasAttributes(const std::vector<std::string>& tag_attributes) const;

			/** @return whether the element has child elements with all the names */
			bool hasChildElements(const std::vector<std::string>& elem_names) const;

			const std::string& getName() const;

			/// @return the content stored between the start and end tag of the element (excluding child elements)
			const std::string& getContent() const;

			const XmlTagAttrib* getAttribute(const std::string& attrib_name);


			/** searches the elements children for the first one which has the attributes stored in the attribute string at attrib_num
			* if multiple attribute strings are provided, its children in return will be checked
			* @param attrib_num: needed so that the function can be used recursivly (what attribute string to use)
			* @return 0 if the element could not be found */
			XmlElement* getElement(const std::vector<std::string>& attribute_strings, int attrib_num = 0);

			/** @return all xml elements that have all the requested tag attributes */
			std::vector<XmlElement*> getAllElements(const std::vector<std::string>& attribute_strings, int attrib_num = 0);

			/// the attribute string should look like this "name attr0=val0 attr1=val1 ..."
			std::vector<std::string> splitAttributeString(std::string attribute_string, std::string& loadTo_name) const;

		public:
			// functions to print the content of the element

			void printShortInfo(int indent = 0) const;

			void printRecursive(int indent = 0) const;

		public:
			// functions to set the elements data

			XmlElement* addChildElement();
			XmlElement* getParentElement();

			/** extracts data from the line which can be read from a xml file
			* @param a line containing the start tag and possibly the content of a xml element
			* start_tag: the start tag as it can be found in an xml file
			* @example <texture width="256" height="256"> */
			void setData(const std::string& line);


		public:

			XmlElement();
			XmlElement(XmlElement* parent);
			virtual ~XmlElement();


		};

	} // tools

} // undicht

#endif // XML_ELEMENT_H
