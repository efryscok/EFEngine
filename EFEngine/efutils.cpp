#include <efe\efutils.h>

namespace efe {
	namespace xml {
		template <typename T>
		void setElement(pugi::xml_node& node, const T& value) {
			node.first_child().set_value(utils::toString(value).c_str());
		}

		template <typename T>
		void setElement(pugi::xml_node& node, const std::string& element, const T& value) {
			setElement(node.child(element.c_str()), value);
		}

		template <typename T>
		void setElement(pugi::xml_node_iterator& nodeIT, const T& value) {
			nodeIT->first_child().set_value(utils::toString(value).c_str());
		}

		template <typename T>
		void setElement(pugi::xml_node_iterator& nodeIT, const std::string& element, const T& value) {
			setElement(nodeIT->child(element.c_str()), value);
		}

		template <typename T>
		void setAttribute(pugi::xml_attribute& attribute, const T& value) {
			attribute.set_value(utils::toString(value).c_str());
		}

		template <typename T>
		void setAttribute(pugi::xml_node& node, const std::string& attribute, const T& value) {
			setAttribute(node.attribute(attribute.c_str()), value);
		}

		template <typename T>
		void setAttribute(pugi::xml_attribute_iterator& attributeIT, const T& value) {
			attributeIT->set_value(utils::toString(value).c_str());
		}

		template <typename T>
		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& attribute, const T& value) {
			setAttribute(nodeIT->attribute(attribute.c_str()), value);
		}

		void setAttribute(pugi::xml_node& node, const std::string& format, const glm::vec3& vector) {
			setAttribute(node.attribute(format.substr(0, 1).c_str()), vector[0]);
			setAttribute(node.attribute(format.substr(1, 1).c_str()), vector[1]);
			setAttribute(node.attribute(format.substr(2, 1).c_str()), vector[2]);
		}

		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& format, const glm::vec3& vector) {
			setAttribute(nodeIT->attribute(format.substr(0, 1).c_str()), vector[0]);
			setAttribute(nodeIT->attribute(format.substr(1, 1).c_str()), vector[1]);
			setAttribute(nodeIT->attribute(format.substr(2, 1).c_str()), vector[2]);
		}

		void setAttribute(pugi::xml_node& node, const std::string& format, const glm::vec4& vector) {
			setAttribute(node.attribute(format.substr(0, 1).c_str()), vector[0]);
			setAttribute(node.attribute(format.substr(1, 1).c_str()), vector[1]);
			setAttribute(node.attribute(format.substr(2, 1).c_str()), vector[2]);
			setAttribute(node.attribute(format.substr(3, 1).c_str()), vector[3]);
		}

		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& format, const glm::vec4& vector) {
			setAttribute(nodeIT->attribute(format.substr(0, 1).c_str()), vector[0]);
			setAttribute(nodeIT->attribute(format.substr(1, 1).c_str()), vector[1]);
			setAttribute(nodeIT->attribute(format.substr(2, 1).c_str()), vector[2]);
			setAttribute(nodeIT->attribute(format.substr(3, 1).c_str()), vector[3]);
		}

		void setAttribute(pugi::xml_node& node, const std::string& element, const std::string& format, const glm::vec3& vector) {
			setAttribute(node.child(element.c_str()), format, vector);
		}

		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& element, const std::string& format, const glm::vec3& vector) {
			setAttribute(nodeIT->child(element.c_str()), format, vector);
		}

		void setAttribute(pugi::xml_node& node, const std::string& element, const std::string& format, const glm::vec4& vector) {
			setAttribute(node.child(element.c_str()), format, vector);
		}

		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& element, const std::string& format, const glm::vec4& vector) {
			setAttribute(nodeIT->child(element.c_str()), format, vector);
		}

		bool load(std::string filename, document& doc) {
			pugi::xml_parse_result result = doc.load_file(filename.c_str(), pugi::parse_default | pugi::parse_declaration);

			if (!result) {
				std::cout << "[ERROR] Cannot open xml file: Parse error: " << result.description() << " , character position: " << result.offset << std::endl;
				return false;
			}

			return true;
		}
	}

	namespace utils {
		btVector3 btVec3(const glm::vec3& vec) {
			return btVector3(vec.x, vec.y, vec.z);
		}

		glm::vec3 glmVec3(const btVector3& vec) {
			return glm::vec3(vec.x(), vec.y(), vec.z());
		}

		glm::vec3 toVec3(const pugi::xml_node& node, const std::string& format) {
			return glm::vec3(node.attribute(format.substr(0, 1).c_str()).as_float(), node.attribute(format.substr(1, 1).c_str()).as_float(), node.attribute(format.substr(2, 1).c_str()).as_float());
		}

		glm::vec3 toVec3(const pugi::xml_node_iterator& nodeIT, const std::string& format) {
			return glm::vec3(nodeIT->attribute(format.substr(0, 1).c_str()).as_float(), nodeIT->attribute(format.substr(1, 1).c_str()).as_float(), nodeIT->attribute(format.substr(2, 1).c_str()).as_float());
		}

		glm::vec4 toVec4(const pugi::xml_node& node, const std::string& format) {
			return glm::vec4(toVec3(node, format), node.attribute(format.substr(3, 1).c_str()).as_float());
		}

		glm::vec4 toVec4(const pugi::xml_node_iterator& nodeIT, const std::string& format) {
			return glm::vec4(toVec3(nodeIT, format), nodeIT->attribute(format.substr(3, 1).c_str()).as_float());
		}

		bool fileExists(const std::string& filename) {
			std::ifstream ifs(filename);
			return ifs.good();
		}
	}
}