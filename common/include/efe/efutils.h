#pragma once

#include <sstream>
#include <iomanip>
#include <xlocale>
#include <random>
#include <string>
#include <array>
#include <pugixml\pugixml.hpp>
#include <glad\glad.h>
#include <glm\glm.hpp>
#include <glm\gtc\quaternion.hpp>
#include <iostream>
#include <fstream>
#include <bullet\LinearMath\btVector3.h>

namespace efe {
	namespace xml {
		using attribute = pugi::xml_attribute;
		using attribute_iterator = pugi::xml_attribute_iterator;
		using document = pugi::xml_document;
		using node = pugi::xml_node;
		using node_iterator = pugi::xml_node_iterator;

		template <typename T>
		void setElement(pugi::xml_node& node, const T& value);

		template <typename T>
		void setElement(pugi::xml_node& node, const std::string& element, const T& value);

		template <typename T>
		void setElement(pugi::xml_node_iterator& nodeIT, const T& value);

		template <typename T>
		void setElement(pugi::xml_node_iterator& nodeIT, const std::string& element, const T& value);

		template <typename T>
		void setAttribute(pugi::xml_attribute& attribute, const T& value);

		template <typename T>
		void setAttribute(pugi::xml_node& node, const std::string& attribute, const T& value);

		template <typename T>
		void setAttribute(pugi::xml_attribute_iterator& attributeIT, const T& value);

		template <typename T>
		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& attribute, const T& value);

		void setAttribute(pugi::xml_node& node, const std::string& format, const glm::vec3& vector);

		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& format, const glm::vec3& vector);

		void setAttribute(pugi::xml_node& node, const std::string& format, const glm::vec4& vector);

		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& format, const glm::vec4& vector);

		void setAttribute(pugi::xml_node& node, const std::string& element, const std::string& format, const glm::vec3& vector);

		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& element, const std::string& format, const glm::vec3& vector);

		void setAttribute(pugi::xml_node& node, const std::string& element, const std::string& format, const glm::vec4& vector);

		void setAttribute(pugi::xml_node_iterator& nodeIT, const std::string& element, const std::string& format, const glm::vec4& vector);

		bool load(std::string filename, document& doc);
	}

	namespace utils {
		struct sReflection {
			float reflect;
			float refract;
			float diffuse;
			float index;
		};

		class cCamera {
		public:
			float speed;
			float fov;
			float zNear;
			float zFar;
			glm::vec3 position;
			glm::vec3 lookAt;
			glm::vec3 upAxis;

		public:
			cCamera() : speed(1.f), fov(1.f), zNear(.1f), zFar(1000.f), position(0.f, 2.f, -2.f), lookAt(0.f), upAxis(0.f, 1.f, 0.f) {}
			cCamera(const glm::vec3& position) : speed(1.f), fov(1.f), zNear(.1f), zFar(1000.f), position(position), lookAt(0.f), upAxis(0.f, 1.f, 0.f) {}
			cCamera(const glm::vec3& position, const glm::vec3& lookAt) : speed(1.f), fov(1.f), zNear(.1f), zFar(1000.f), position(position), lookAt(lookAt), upAxis(0.f, 1.f, 0.f) {}
			cCamera(const glm::vec3& position, const glm::vec3& lookAt, const float& zNear, const float& zFar) : speed(1.f), fov(1.f), zNear(zNear), zFar(zFar), position(position), lookAt(lookAt), upAxis(0.f, 1.f, 0.f) {}
		};

		struct sShader {
			std::string vertex;
			std::string tessellationControl;
			std::string tessellationEvaluation;
			std::string geometry;
			std::string fragment;
		};

		struct sScissor {
			bool enabled;
			int id;
			GLint x;
			GLint y;
			GLsizei width;
			GLsizei height;
		};

		struct cLight {
		public:
			float attenConst;
			float attenLinear;
			float attenQuad;
			glm::vec3 position;
			glm::vec3 ambient;
			glm::vec3 diffuse;
			glm::vec3 specular;

			struct sLightUniforms {
				GLint position;
				GLint ambient;
				GLint diffuse;
				GLint specular;
				GLint attenConst;
				GLint attenLinear;
				GLint attenQuad;
			} uniforms;

		public:
			cLight() : attenConst(.1f), attenLinear(.01f), attenQuad(.001f), position(0.f), ambient(.5f), diffuse(1.f), specular(1.f) {}
		};

		struct sModel {
			bool wireframe;
			bool useLight;
			bool usePhysics;
			bool useCollision;
			bool isSkybox;
			int id;
			float shininess;
			float alpha;
			float scale;
			float mass;
			glm::vec3 ambient;
			glm::vec3 diffuse;
			glm::vec3 specular;
			glm::vec3 position;
			glm::vec3 velocity;
			glm::vec3 acceleration;
			glm::vec3 lastPosition;
			glm::quat orientation;
			glm::mat4 matrix;
			std::string mesh;
			std::string texture;
		};

		struct sUniforms {
			GLint model;
			GLint view;
			GLint projection;
			GLint cameraPosition;
			GLint materialAmbient;
			GLint materialDiffuse;
			GLint materialSpecular;
			GLint materialShininess;
			GLint alpha;
			GLint useLight;
			GLint skybox;
			GLint isSkybox;
		};

		template <typename T>
		T random(const T& low, const T& high) {
			return low + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (high - low)));
		}

		template <typename T>
		T fromString(const std::string& value) {
			T type;
			std::stringstream ss;
			ss << value;
			ss >> type;
			return type;
		}

		template <typename T>
		std::string toString(const T& value) {
			std::stringstream ss;
			ss.imbue(std::locale(""));
			ss << std::fixed << value;
			return ss.str();
		}

		btVector3 btVec3(const glm::vec3& vec);

		glm::vec3 glmVec3(const btVector3& vec);
		
		glm::vec3 toVec3(const pugi::xml_node& node, const std::string& format);

		glm::vec3 toVec3(const pugi::xml_node_iterator& nodeIT, const std::string& format);

		glm::vec4 toVec4(const pugi::xml_node& node, const std::string& format);

		glm::vec4 toVec4(const pugi::xml_node_iterator& nodeIT, const std::string& format);

		bool fileExists(const std::string& filename);
	}
}