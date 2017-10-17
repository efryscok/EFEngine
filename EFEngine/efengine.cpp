#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include <efe\efengine.h>
#include <sstream>
#include <locale>
#include <random>
#include <bitmap\bitmap_image.hpp>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>

#ifdef _DEBUG
int main(int argc, char * argv[]) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	srand(static_cast<unsigned int>(time(NULL)));

	for (int i = 0; i < argc; ++i) {
		efe::cOpenGLApp::msArgs.push_back(argv[i]);
	}

	efe::cOpenGLApp::msInfo.flags.debug = true;

	return efe::cOpenGLApp::msApp->run();
}
#else
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	std::string parameter;
	std::istringstream cmdLine(lpCmdLine);

	while (cmdLine >> parameter) {
		efe::cOpenGLApp::msArgs.push_back(parameter);
	}

	efe::cOpenGLApp::msInfo.flags.debug = false;

	return efe::cOpenGLApp::msApp->run();
}
#endif

namespace efe {
	cOpenGLApp::sInfo cOpenGLApp::msInfo;
	GLFWwindow* cOpenGLApp::msWindow = nullptr;

	std::vector<std::string> cOpenGLApp::msArgs;
	std::map<std::string, GLuint> cOpenGLApp::msTextures;
	std::map<std::string, cOpenGLApp::sObject> cOpenGLApp::msObjects;

	cOpenGLApp* cOpenGLApp::msApp = nullptr;
	std::string cOpenGLApp::msAssetRoot("..\\common\\assets");
	std::vector<std::thread> cOpenGLApp::msThreadPool(std::thread::hardware_concurrency() - 1);
	std::map<std::string, cOpenGLApp::sImage> cOpenGLApp::msImages;
	std::map<std::string, cOpenGLApp::sMesh> cOpenGLApp::msMeshes;

	GLuint cOpenGLApp::loadTexture(std::string filename) {
		sImage data = msLoadImage(filename);

		GLuint textureID = 0;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, data.width, data.height);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.width, data.height, GL_RGB, GL_UNSIGNED_BYTE, data.pixels.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		return textureID;
	}

	GLuint cOpenGLApp::loadCubemap(std::string posXFile, std::string negXFile, std::string posYFile, std::string negYFile, std::string posZFile, std::string negZFile) {
		std::vector<sImage> data(6);
		if (std::thread::hardware_concurrency() < 6) {
			data[0] = msLoadImage(posXFile);
			data[1] = msLoadImage(negXFile);
			data[2] = msLoadImage(negYFile);
			data[3] = msLoadImage(posYFile);
			data[4] = msLoadImage(posZFile);
			data[5] = msLoadImage(negZFile);
		}
		else {
			msThreadPool[0].swap(std::thread(&msLoadImageSingle, posXFile, std::ref(data[0])));
			msThreadPool[1].swap(std::thread(&msLoadImageSingle, negXFile, std::ref(data[1])));
			msThreadPool[2].swap(std::thread(&msLoadImageSingle, negYFile, std::ref(data[2])));
			msThreadPool[3].swap(std::thread(&msLoadImageSingle, posYFile, std::ref(data[3])));
			msThreadPool[4].swap(std::thread(&msLoadImageSingle, posZFile, std::ref(data[4])));
			msThreadPool[5].swap(std::thread(&msLoadImageSingle, negZFile, std::ref(data[5])));
		}

		for (std::vector<std::thread>::iterator threadIT = msThreadPool.begin(); threadIT != msThreadPool.end(); ++threadIT) {
			if (threadIT->joinable()) {
				threadIT->join();
			}
		}

		GLuint textureID = 0;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 10, GL_RGBA8, data[0].width, data[0].height);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		for (size_t i = 0; i < data.size(); ++i) {
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i), 0, 0, 0, data[i].width, data[i].height, GL_RGB, GL_UNSIGNED_BYTE, data[i].pixels.data());
		}

		return textureID;
	}

	void cOpenGLApp::loadTextures() {
		std::map<std::string, GLuint>::iterator beginIT = msTextures.begin();
		size_t diff = msTextures.size() / std::thread::hardware_concurrency();
		size_t mods = msTextures.size() % std::thread::hardware_concurrency();
		std::vector<int> filesPerThread;

		for (size_t threadIdx = 1, fileIdx = 0; threadIdx < std::thread::hardware_concurrency(); ++threadIdx) {
			if (diff > 0) {
				if (mods >= threadIdx) {
					auto endIT = beginIT;
					std::advance(endIT, diff + 1);
					msThreadPool[threadIdx - 1].swap(std::thread(&msLoadImageMultiple, beginIT, endIT));
					beginIT = endIT;
					fileIdx += diff + 1;
				}
				else {
					auto endIT = beginIT;
					std::advance(endIT, diff);
					msThreadPool[threadIdx - 1].swap(std::thread(&msLoadImageMultiple, beginIT, endIT));
					beginIT = endIT;
					fileIdx += diff;
				}
			}
			else if (fileIdx + 1 >= threadIdx && fileIdx < msTextures.size()) {
				auto endIT = beginIT;
				std::advance(endIT, 1);
				msThreadPool[threadIdx - 1].swap(std::thread(&msLoadImageMultiple, beginIT, endIT));
				beginIT = endIT;
				if (fileIdx < msTextures.size()) {
					++fileIdx;
				}
			}
		}

		msLoadImageMultiple(beginIT, msTextures.end());

		for (std::vector<std::thread>::iterator threadIT = msThreadPool.begin(); threadIT != msThreadPool.end(); ++threadIT) {
			if (threadIT->joinable()) {
				threadIT->join();
			}
		}

		for (std::map<std::string, sImage>::iterator it = msImages.begin(); it != msImages.end(); ++it) {
			glGenTextures(1, &msTextures[it->first]);
			glBindTexture(GL_TEXTURE_2D, msTextures[it->first]);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, it->second.width, it->second.height);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, it->second.width, it->second.height, GL_RGB, GL_UNSIGNED_BYTE, it->second.pixels.data());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
	}

	GLuint cOpenGLApp::loadObject(const sMesh& mesh) {
		// Create the buffers
		GLuint vao = -1;
		GLuint vertexBuffer = -1;
		GLuint indexBuffer = -1;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(sVertex), &mesh.vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), &mesh.indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		size_t positionOffset = 0;
		size_t normalOffset = positionOffset + sizeof(((sVertex*)0)->position);
		size_t texCoordOffset = normalOffset + sizeof(((sVertex*)0)->normal);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)positionOffset);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)normalOffset);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)texCoordOffset);

		// The "scene" pointer will be deleted automatically by "importer"
		return vao;
	}

	bool cOpenGLApp::loadObject(std::string filename, GLuint& vao, size_t& indices) {
		sMesh data = msLoadMesh(filename);
		indices = data.indices.size();

		// Create the buffers
		vao = -1;
		GLuint vertexBuffer = -1;
		GLuint indexBuffer = -1;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(sVertex), &data.vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(GLuint), &data.indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		size_t positionOffset = 0;
		size_t normalOffset = positionOffset + sizeof(((sVertex*)0)->position);
		size_t texCoordOffset = normalOffset + sizeof(((sVertex*)0)->normal);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)positionOffset);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)normalOffset);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)texCoordOffset);

		return true;
	}

	void cOpenGLApp::loadObjects() {
		std::map<std::string, sObject>::iterator beginIT = msObjects.begin();
		size_t diff = msObjects.size() / std::thread::hardware_concurrency();
		size_t mods = msObjects.size() % std::thread::hardware_concurrency();

		for (size_t threadIdx = 1, fileIdx = 0; threadIdx < std::thread::hardware_concurrency() - 1; ++threadIdx) {
			if (diff > 0) {
				if (mods >= threadIdx) {
					auto endIT = beginIT;
					std::advance(endIT, diff + 1);
					msApp->msThreadPool[threadIdx - 1].swap(std::thread(&msLoadMeshMultiple, beginIT, endIT));
					beginIT = endIT;
					fileIdx += diff + 1;
				}
				else {
					auto endIT = beginIT;
					std::advance(endIT, diff);
					msApp->msThreadPool[threadIdx - 1].swap(std::thread(&msLoadMeshMultiple, beginIT, endIT));
					beginIT = endIT;
					fileIdx += diff;
				}
			}
			else if (fileIdx + 1 >= threadIdx && fileIdx < msObjects.size()) {
				auto endIT = beginIT;
				std::advance(endIT, 1);
				msApp->msThreadPool[threadIdx - 1].swap(std::thread(&msLoadMeshMultiple, beginIT, endIT));
				beginIT = endIT;
				if (fileIdx < msObjects.size()) {
					++fileIdx;
				}
			}
		}

		msLoadMeshMultiple(beginIT, msObjects.end());

		for (std::vector<std::thread>::iterator threadIT = msApp->msThreadPool.begin(); threadIT != msApp->msThreadPool.end(); ++threadIT) {
			if (threadIT->joinable()) {
				threadIT->join();
			}
		}

		for (std::map<std::string, sMesh>::iterator it = msApp->msMeshes.begin(); it != msApp->msMeshes.end(); ++it) {
			msObjects[it->first].vao = -1;
			msObjects[it->first].indices = static_cast<GLsizei>(it->second.indices.size());
			GLuint vertexBuffer = -1;
			GLuint indexBuffer = -1;

			glGenVertexArrays(1, &msObjects[it->first].vao);
			glBindVertexArray(msObjects[it->first].vao);

			glGenBuffers(1, &vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, it->second.vertices.size() * sizeof(sVertex), &it->second.vertices[0], GL_STATIC_DRAW);

			glGenBuffers(1, &indexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, it->second.indices.size() * sizeof(GLuint), &it->second.indices[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);

			size_t positionOffset = 0;
			size_t normalOffset = positionOffset + sizeof(((sVertex*)0)->position);
			size_t texCoordOffset = normalOffset + sizeof(((sVertex*)0)->normal);

			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)positionOffset);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)normalOffset);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(sVertex), (GLvoid*)texCoordOffset);
		}
	}

	GLuint cOpenGLApp::loadShader(std::string filename, GLenum shaderType) {
		GLuint result = 0;
		FILE* fp;
		size_t filesize;
		char* data;

		fp = fopen((msAssetRoot + "\\shaders\\" + filename).c_str(), "rb");

		if (!fp) {
			return result;
		}

		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		data = new char[filesize + 1];

		if (!data) {
			return result;
		}

		fread(data, 1, filesize, fp);
		data[filesize] = 0;

		result = glCreateShader(shaderType);

		if (!result) {
			return result;
		}

		glShaderSource(result, 1, &data, NULL);

		delete[] data;

		glCompileShader(result);

#ifdef _DEBUG
		GLint status = 0;
		glGetShaderiv(result, GL_COMPILE_STATUS, &status);

		if (!status) {
			char buffer[4096];
			glGetShaderInfoLog(result, 4096, NULL, buffer);
			std::cout << filename << ":" << buffer << std::endl;
			glDeleteShader(result);
			return result;
		}
#endif
		return result;
	}

	GLuint cOpenGLApp::linkShaders(const GLuint* shaders, int shaderCount) {
		GLuint program;
		program = glCreateProgram();

		for (int i = 0; i < shaderCount; ++i) {
			glAttachShader(program, shaders[i]);
		}

		glLinkProgram(program);

#ifdef _DEBUG
		GLint status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);

		if (!status) {
			char buffer[4096];
			glGetProgramInfoLog(program, 4096, NULL, buffer);
			std::cout << buffer << std::endl;
			glDeleteProgram(program);
			return 0;
		}
#endif

		for (int i = 0; i < shaderCount; ++i) {
			glDetachShader(program, shaders[i]);
			glDeleteShader(shaders[i]);
		}

		return program;
	}

	GLuint cOpenGLApp::loadAndLinkShaders(const utils::sShader& shaderFiles) {
		GLuint shaderIDs[5];
		GLuint currID = 0;

		GLuint vertexID = loadShader(shaderFiles.vertex, GL_VERTEX_SHADER);
		GLuint tessellationControlID = loadShader(shaderFiles.tessellationControl, GL_TESS_CONTROL_SHADER);
		GLuint tessellationEvaluationID = loadShader(shaderFiles.tessellationEvaluation, GL_TESS_EVALUATION_SHADER);
		GLuint geometryID = loadShader(shaderFiles.geometry, GL_GEOMETRY_SHADER);
		GLuint fragmentID = loadShader(shaderFiles.fragment, GL_FRAGMENT_SHADER);

		if (vertexID != 0) {
			shaderIDs[currID++] = vertexID;
		}
		if (tessellationControlID != 0) {
			shaderIDs[currID++] = tessellationControlID;
		}
		if (tessellationEvaluationID != 0) {
			shaderIDs[currID++] = tessellationEvaluationID;
		}
		if (geometryID != 0) {
			shaderIDs[currID++] = geometryID;
		}
		if (fragmentID != 0) {
			shaderIDs[currID++] = fragmentID;
		}

		return linkShaders(shaderIDs, ++currID);

	}

	bool cOpenGLApp::bindTexture(GLint uniform, GLenum type, std::string texturename) {
		if (msTextures.find(texturename) != msTextures.end()) {
			glActiveTexture(msTextures[texturename] + GL_TEXTURE0);
			glBindTexture(type, msTextures[texturename]);
			glUniform1i(uniform, msTextures[texturename]);
			return true;
		}
		return false;
	}

	bool cOpenGLApp::bindTexture(GLint uniform, GLenum type, GLuint textureid) {
		if (std::find_if(msTextures.begin(), msTextures.end(), [textureid](const std::pair<std::string, GLuint>& item) {return item.second == textureid; }) != msTextures.end()) {
			glActiveTexture(textureid + GL_TEXTURE0);
			glBindTexture(type, textureid);
			glUniform1i(uniform, textureid);
			return true;
		}
		return false;
	}

	void cOpenGLApp::generateMesh(std::string name, generator::AnyGenerator<generator::MeshVertex> meshVertices, generator::AnyGenerator<generator::Triangle> meshTriangles) {
		std::vector<sVertex> vertices;
		std::vector<unsigned int> indices;

		while (!meshVertices.done()) {
			generator::MeshVertex meshVertex = meshVertices.generate();

			sVertex vertex;
			vertex.normal = glm::vec4(meshVertex.normal, 1.f);
			vertex.position = glm::vec4(meshVertex.position, 1.f);
			vertex.texCoord = glm::vec4(meshVertex.texCoord, 0.f, 1.f);
			vertices.push_back(vertex);

			meshVertices.next();
		}

		while (!meshTriangles.done()) {
			generator::Triangle triangle = meshTriangles.generate();

			indices.push_back(triangle.vertices[0]);
			indices.push_back(triangle.vertices[1]);
			indices.push_back(triangle.vertices[2]);

			meshTriangles.next();
		}

		sMesh mesh;

		mesh.vertices = vertices;
		mesh.indices = indices;

		msMeshes[name] = mesh;
	}

	void cOpenGLApp::init() {
		srand(static_cast<unsigned int>(time(NULL)));
		strcpy(msInfo.title, "EFEngine by Erik Fryscok");
		msInfo.windowWidth = 800;
		msInfo.windowHeight = 600;
		msInfo.majorVersion = 4;
		msInfo.minorVersion = 3;
		msInfo.samples = 0;

		msInfo.flags.cursor = true;
		msInfo.flags.fullscreen = false;
		msInfo.flags.robust = false;
		msInfo.flags.stereo = false;
	}

	int cOpenGLApp::run() {
		if (!glfwInit()) {
			std::cerr << "Failed to initialize GLFW" << std::endl;
			return EXIT_FAILURE;
		}

		bool running = true;

		init();

		if (msInfo.flags.debug) {
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		}

		if (msInfo.flags.robust) {
			glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, msInfo.majorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, msInfo.minorVersion);

		msWindow = glfwCreateWindow(msInfo.windowWidth, msInfo.windowHeight, msInfo.title, msInfo.flags.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);

		if (!msWindow) {
			std::cerr << "Failed to open window" << std::endl;
			return EXIT_FAILURE;
		}

		glfwSetKeyCallback(msWindow, msOnKey);
		glfwSetCursorPosCallback(msWindow, msOnMouseMove);
		glfwSetMouseButtonCallback(msWindow, msOnMouseButton);
		glfwSetScrollCallback(msWindow, msOnMouseWheel);

		if (!msInfo.flags.cursor) {
			glfwSetInputMode(msWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		glfwMakeContextCurrent(msWindow);
		glfwSwapInterval(1);

		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		std::cout << "VENDOR: " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "VERSION: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "RENDERER: " << glGetString(GL_RENDERER) << std::endl;

		glClearColor(0.f, 0.f, 0.f, 0.f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH);

		startup();

		double lastTime = glfwGetTime();

		do {
			glfwGetFramebufferSize(msWindow, &msInfo.windowWidth, &msInfo.windowHeight);

			render(glfwGetTime() - lastTime);
			lastTime = glfwGetTime();

			glfwSwapBuffers(msWindow);
			glfwPollEvents();

			running &= (glfwGetKey(msWindow, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
			running &= (glfwWindowShouldClose(msWindow) != GL_TRUE);

		}
		while (running);

		shutdown();

		glfwDestroyWindow(msWindow);
		glfwTerminate();

		return EXIT_SUCCESS;
	}

	cOpenGLApp::sImage cOpenGLApp::msLoadImage(std::string filename) {
		bitmap_image bitmap(msAssetRoot + "\\textures\\" + filename);

		sImage img;
		img.pixels.resize(bitmap.pixel_count());
		img.width = bitmap.width();
		img.height = bitmap.height();

		size_t pixelCount = bitmap.pixel_count() - 1;
		for (unsigned int row = 0; row < bitmap.height(); ++row) {
			for (unsigned int col = bitmap.width() - 1; col >= 0 && col < bitmap.width(); --col) {
				if (pixelCount < bitmap.pixel_count() && pixelCount != 0) {
					bitmap.get_pixel(col, row, img.pixels[pixelCount].r, img.pixels[pixelCount].g, img.pixels[pixelCount--].b);
				}
			}
		}

		std::cout << "Loaded image: " << filename << std::endl;

		return img;
	}

	void cOpenGLApp::msLoadImageSingle(std::string filename, sImage& image) {
		image = msLoadImage(filename);
	}

	void cOpenGLApp::msLoadImageMultiple(std::map<std::string, GLuint>::iterator begin, std::map<std::string, GLuint>::iterator end) {
		while (begin != end) {
			msImages[begin->first] = msLoadImage(begin->first);
			++begin;
		}
	}

	cOpenGLApp::sMesh cOpenGLApp::msLoadMesh(std::string filename) {
		sMesh data;
		Assimp::Importer importer;
		unsigned int flags = 0;
		const aiScene* scene = importer.ReadFile(msAssetRoot + "\\objects\\" + filename, flags);

		if (!scene) {
			std::cout << "[ERROR] " << importer.GetErrorString() << std::endl;
			return data;
		}

		const aiMesh* mesh = scene->mMeshes[0];

		size_t vertexOffset = data.vertices.size();
		size_t indexOffset = data.indices.size();
		size_t numVertices = mesh->mNumVertices;
		size_t indices = 3 * mesh->mNumFaces;

		// Fill vertices positions
		data.vertices.resize(vertexOffset + numVertices);

		float minX, minY, minZ, maxX, maxY, maxZ, extentX, extentY, extentZ, maxExtent;

		// Set the initial values of the 1st vertex
		minX = maxX = mesh->mVertices[0].x;
		minY = maxY = mesh->mVertices[0].y;
		minZ = maxZ = mesh->mVertices[0].z;

		for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
			if (mesh->mVertices[index].x < minX) {
				minX = mesh->mVertices[index].x;
			}
			if (mesh->mVertices[index].y < minY) {
				minY = mesh->mVertices[index].y;
			}
			if (mesh->mVertices[index].z < minZ) {
				minZ = mesh->mVertices[index].z;
			}

			if (mesh->mVertices[index].x > maxX) {
				maxX = mesh->mVertices[index].x;
			}
			if (mesh->mVertices[index].y > maxY) {
				maxY = mesh->mVertices[index].y;
			}
			if (mesh->mVertices[index].z > maxZ) {
				maxZ = mesh->mVertices[index].z;
			}
		}

		// What is the max extent
		extentX = maxX - minX;
		extentY = maxY - minY;
		extentZ = maxZ - minZ;

		maxExtent = extentX;
		if (extentY > maxExtent) {
			maxExtent = extentY;
		}
		if (extentZ > maxExtent) {
			maxExtent = extentZ;
		}

		float scaleValue = 1.f / maxExtent;

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			aiVector3D pos = mesh->mVertices[i];
			aiVector3D n = mesh->mNormals[i];
			aiVector3D tc = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0.f);

			sVertex& vert = data.vertices[vertexOffset + i];
			vert.position = glm::vec4(pos.x * scaleValue, pos.y * scaleValue, pos.z * scaleValue, 1.f);
			vert.normal = glm::vec4(n.x, n.y, n.z, 1.f);
			vert.texCoord = glm::vec4(tc.x, tc.y, tc.z, 1.f);
		}

		// Fill face indices
		data.indices.resize(indexOffset + indices);

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			unsigned int baseInd = 3 * i;

			// Assume the model has only triangles
			data.indices[indexOffset + baseInd] = mesh->mFaces[i].mIndices[0];
			data.indices[indexOffset + baseInd + 1] = mesh->mFaces[i].mIndices[1];
			data.indices[indexOffset + baseInd + 2] = mesh->mFaces[i].mIndices[2];
		}

		std::cout << "Loaded mesh: " << filename << std::endl;

		return data;
	}

	void cOpenGLApp::msLoadMeshSingle(std::string filename, sMesh& mesh) {
		mesh = msLoadMesh(filename);
	}

	void cOpenGLApp::msLoadMeshMultiple(std::map<std::string, sObject>::iterator begin, std::map<std::string, sObject>::iterator end) {
		while (begin != end) {
			msApp->msMeshes[begin->first] = msLoadMesh(begin->first);
			++begin;
		}
	}

	void cOpenGLApp::getMousePosition(int & x, int & y) {
		double dx, dy;
		glfwGetCursorPos(msWindow, &dx, &dy);

		x = static_cast<int>(floor(dx));
		y = static_cast<int>(floor(dy));
	}

	void cOpenGLApp::setWindowTitle(const char* title) {
		glfwSetWindowTitle(msWindow, title);
	}

	void cOpenGLApp::onResize(int width, int height) {
		msInfo.windowWidth = width; msInfo.windowHeight = height;
	}

	void cOpenGLApp::onDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message) {
		std::cout << "[INFO] " << message << std::endl;
	}

	void APIENTRY cOpenGLApp::msDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam) {
		reinterpret_cast<cOpenGLApp*>(userParam)->onDebugMessage(source, type, id, severity, length, message);
	}

	static glm::vec4 colourFromHex(const unsigned int hex) {
		return glm::vec4(float((hex >> 16) & 0xFF) / 255.0f, float((hex >> 8) & 0xFF) / 255.0f, float((hex >> 0) & 0xFF) / 255.0f, 1.0f);
	}
}