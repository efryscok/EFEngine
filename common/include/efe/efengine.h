#pragma once

#include <Windows.h>
#include <glad\glad.h>
#include <glfw\glfw3.h>
#ifdef _DEBUG
int main(int argc, char* argv[]);
#else
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
#endif
#include <glm\glm.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <queue>
#include <map>
#include <efe\efutils.h>
#include <generator\AnyGenerator.hpp>
#include <generator\MeshVertex.hpp>
#include <generator\Triangle.hpp>

namespace efe {
	class cOpenGLApp {
	protected:
		struct sInfo {
			char title[128];
			int windowWidth;
			int windowHeight;
			int majorVersion;
			int minorVersion;
			int samples;

			struct {
				bool cursor;
				bool debug;
				bool fullscreen;
				bool robust;
				bool stereo;
			} flags;
		};

		struct sPixel {
			unsigned char r;
			unsigned char g;
			unsigned char b;
		};

		struct sImage {
			std::vector<sPixel> pixels;
			GLsizei width;
			GLsizei height;
		};

		struct sVertex {
			glm::vec4 position;
			glm::vec4 normal;
			glm::vec4 texCoord;
		};

		struct sMesh {
			std::vector<sVertex> vertices;
			std::vector<unsigned int> indices;
		};

		struct sObject {
			GLuint vao;
			GLsizei indices;
		};

	protected:
		static sInfo msInfo;
		static GLFWwindow* msWindow;

		static std::string msAssetRoot;
		static std::vector<std::string> msArgs;
		static std::map<std::string, GLuint> msTextures;
		static std::map<std::string, efe::cOpenGLApp::sObject> msObjects;

	private:
		static cOpenGLApp* msApp;
		static std::vector<std::thread> msThreadPool;
		static std::map<std::string, sImage> msImages;
		static std::map<std::string, sMesh> msMeshes;

	public:
		cOpenGLApp() {
			msApp = this;
		}
		virtual ~cOpenGLApp() {}

		GLuint loadTexture(std::string filename);
		GLuint loadCubemap(std::string posXFile, std::string negXFile, std::string posYFile, std::string negYFile, std::string posZFile, std::string negZFile);
		void loadTextures();

		GLuint loadObject(const sMesh& mesh);
		bool loadObject(std::string filename, GLuint& vao, size_t& indices);
		void loadObjects();

		GLuint loadShader(std::string filename, GLenum shaderType);
		GLuint linkShaders(const GLuint* shaders, int shaderCount);
		GLuint loadAndLinkShaders(const utils::sShader& shaderFiles);

		bool bindTexture(GLint uniform, GLenum type, std::string texturename);
		bool bindTexture(GLint uniform, GLenum type, GLuint textureid);

		void generateMesh(std::string name, generator::AnyGenerator<generator::MeshVertex> meshVertices, generator::AnyGenerator<generator::Triangle> meshTriangles);

		virtual void init();
		virtual void render(double deltaTime) {}
		virtual int run();
		virtual void shutdown() {}
		virtual void startup() {}

	private:
		static sImage msLoadImage(std::string filename);
		static void msLoadImageSingle(std::string filename, sImage& image);
		static void msLoadImageMultiple(std::map<std::string, GLuint>::iterator begin, std::map<std::string, GLuint>::iterator end);

		static sMesh msLoadMesh(std::string filename);
		static void msLoadMeshSingle(std::string filename, sMesh& mesh);
		static void msLoadMeshMultiple(std::map<std::string, sObject>::iterator begin, std::map<std::string, sObject>::iterator end);

	public:
		void getMousePosition(int& x, int& y);
		void setWindowTitle(const char* title);
		virtual void onResize(int width, int height);
		virtual void onKey(int key, int action) {}
		virtual void onMouseButton(int button, int action) {}
		virtual void onMouseMove(int x, int y) {}
		virtual void onMouseWheel(int pos) {}
		virtual void onDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message);

	protected:
		static void APIENTRY msDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);

		static void msOnResize(GLFWwindow* mWindow, int w, int h) {
			msApp->onResize(w, h);
		}
		static void msOnKey(GLFWwindow* mWindow, int key, int scancode, int action, int mods) {
			msApp->onKey(key, action);
		}
		static void msOnMouseButton(GLFWwindow* mWindow, int button, int action, int mods) {
			msApp->onMouseButton(button, action);
		}
		static void msOnMouseMove(GLFWwindow* mWindow, double x, double y) {
			msApp->onMouseMove(static_cast<int>(x), static_cast<int>(y));
		}
		static void msOnMouseWheel(GLFWwindow* mWindow, double xoffset, double yoffset) {
			msApp->onMouseWheel(static_cast<int>(yoffset));
		}

#ifdef _DEBUG
		friend int ::main(int argc, char* argv[]);
#else
		friend int CALLBACK::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
#endif
	};
};