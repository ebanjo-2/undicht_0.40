#ifndef XML_FILE_H
#define XML_FILE_H


#include "xml_element.h"
#include "fstream"

namespace undicht {

	namespace tools {

		class XmlFile : public XmlElement {
			/** a class that can be used to read xml style files
			* after being opened, the XmlFile object resembles the xml information element of the file
			* the other elements are stored in m_child_elements */
        protected:

            std::string m_file_name;

        private:

			XmlElement* m_last_element = 0;

			/**  reads and processes the next line of the file
			*  @return whether the line was valid */
			virtual bool readNextLine(std::ifstream& file);

			/** @return the type of element tag stored in the line
			* one of XML_START_TAG, XML_END_TAG, XML_ONE_LINE_ELEMENT
			* @return 0 if invalid line */
			int getLineTagType(const std::string& tag);

			/** removes all free space characters in front of the '<'
			* @return false if the string does not contain a '<' */
			bool removeIndentation(std::string& str);



		public:

			// loads the root element and all its sub elements
			virtual bool open(const std::string& file_name);

			XmlFile();
			XmlFile(const std::string& file_name);
			virtual ~XmlFile();


		};

	} // tools

} // undicht

#endif // XML_FILE_H
