#include <efe\efengine.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <generator\BezierMesh.hpp>
#include <generator\BoxMesh.hpp>
#include <generator\CappedConeMesh.hpp>
#include <generator\CappedCylinderMesh.hpp>
#include <generator\CappedTubeMesh.hpp>
#include <generator\CapsuleMesh.hpp>
#include <generator\ConeMesh.hpp>
#include <generator\ConvexPolygonMesh.hpp>
#include <generator\CylinderMesh.hpp>
#include <generator\DiskMesh.hpp>
#include <generator\DodecahedronMesh.hpp>
#include <generator\PlaneMesh.hpp>
#include <generator\IcosahedronMesh.hpp>
#include <generator\IcoSphereMesh.hpp>
#include <generator\ParametricMesh.hpp>
#include <generator\RoundedBoxMesh.hpp>
#include <generator\SphereMesh.hpp>
#include <generator\SphericalConeMesh.hpp>
#include <generator\SphericalTriangleMesh.hpp>
#include <generator\SpringMesh.hpp>
#include <generator\TeapotMesh.hpp>
#include <generator\TorusMesh.hpp>
#include <generator\TorusKnotMesh.hpp>
#include <generator\TriangleMesh.hpp>
#include <generator\TubeMesh.hpp>

using namespace efe;
using namespace efe::utils;
using namespace std;
using namespace glm;
using namespace generator;

class myApp : public cOpenGLApp {
	bool wireframe;
	int meshGroup;

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
	} uniforms;

	struct sLight {
		vec3 position;
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
		float attenConst;
		float attenLinear;
		float attenQuad;

		struct sLightUniforms{
			GLint position;
			GLint ambient;
			GLint diffuse;
			GLint specular;
			GLint attenConst;
			GLint attenLinear;
			GLint attenQuad;
		} uniforms;
	};

	vector<sLight> lights;

	GLuint program;
	sShader shaders;
	sCamera camera;
	string mesh;
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	void init() {
		cOpenGLApp::init();
		wireframe = false;

		shaders.vertex = "vertex.glsl";
		shaders.fragment = "fragment.glsl";
		mesh = "box";

		camera.position = vec3(0.f, 0.f, -3.f);
		camera.lookAt = vec3(0.f);
		camera.upAxis = vec3(0.f, 1.f, 0.f);
		camera.fov = 1.f;
		camera.zNear = .01f;
		camera.zFar = 1000.f;
		camera.speed = 1.f;

		meshGroup = 1;

		sLight l1;
		l1.position = vec3(2.5f);
		l1.ambient = vec3(.2f, .2f, .2f);
		l1.diffuse = vec3(1.f, 1.f, 1.f);
		l1.specular = vec3(1.f, 1.f, 1.f);
		l1.attenConst = .1f;
		l1.attenLinear = .1f;
		l1.attenQuad = .1f;

		sLight l2;
		l2.position = vec3(-2.5f);
		l2.ambient = vec3(.2f, .2f, .2f);
		l2.diffuse = vec3(1.f, 1.f, 1.f);
		l2.specular = vec3(1.f, 1.f, 1.f);
		l2.attenConst = .1f;
		l2.attenLinear = .1f;
		l2.attenQuad = .1f;

		lights.push_back(l1);
		lights.push_back(l2);
	}

	virtual void startup() {
		program = loadAndLinkShaders(shaders);
		glUseProgram(program);

		uniforms.model = glGetUniformLocation(program, "modelMatrix");
		uniforms.view = glGetUniformLocation(program, "viewMatrix");
		uniforms.projection = glGetUniformLocation(program, "projectionMatrix");
		uniforms.cameraPosition = glGetUniformLocation(program, "cameraPosition");
		uniforms.materialAmbient = glGetUniformLocation(program, "materialAmbient");
		uniforms.materialDiffuse = glGetUniformLocation(program, "materialDiffuse");
		uniforms.materialShininess = glGetUniformLocation(program, "materialShininess");
		uniforms.materialSpecular = glGetUniformLocation(program, "materialSpecular");
		uniforms.alpha = glGetUniformLocation(program, "alpha");

		for (int i = 0; i < lights.size(); ++i) {
			stringstream ss;
			ss << "lights[" << i << "].";

			lights[i].uniforms.position = glGetUniformLocation(program, (ss.str() + "position").c_str());
			lights[i].uniforms.ambient = glGetUniformLocation(program, (ss.str() + "ambient").c_str());
			lights[i].uniforms.diffuse = glGetUniformLocation(program, (ss.str() + "diffuse").c_str());
			lights[i].uniforms.specular = glGetUniformLocation(program, (ss.str() + "specular").c_str());
			lights[i].uniforms.attenConst = glGetUniformLocation(program, (ss.str() + "attenConst").c_str());
			lights[i].uniforms.attenLinear = glGetUniformLocation(program, (ss.str() + "attenLinear").c_str());
			lights[i].uniforms.attenQuad = glGetUniformLocation(program, (ss.str() + "attenQuad").c_str());
		}

		BoxMesh box;
		CappedConeMesh cappedCone;
		CappedCylinderMesh cappedCylinder;
		CappedTubeMesh cappedTube;
		CapsuleMesh capsule;
		ConeMesh cone;
		ConvexPolygonMesh convexPolygon;
		CylinderMesh cylinder;
		DiskMesh disk;
		DodecahedronMesh dodecahedron;
		PlaneMesh plane;
		IcosahedronMesh icosahedron;
		IcoSphereMesh icoSphere(1.0, 1);
		RoundedBoxMesh roundedBox;
		SphereMesh sphere;
		SphericalConeMesh sphericalCone(.5);
		SphericalTriangleMesh sphereicalTriangle;
		SpringMesh spring;
		TeapotMesh teapot;
		TorusMesh torus;
		TorusKnotMesh torusKnot;
		TriangleMesh triangle;
		TubeMesh tube;

		generateMesh("box", box.vertices(), box.triangles());
		generateMesh("cappedCone", cappedCone.vertices(), cappedCone.triangles());
		generateMesh("cappedCylinder", cappedCylinder.vertices(), cappedCylinder.triangles());
		generateMesh("cappedTube", cappedTube.vertices(), cappedTube.triangles());
		generateMesh("capsule", capsule.vertices(), capsule.triangles());
		generateMesh("cone", cone.vertices(), cone.triangles());
		generateMesh("convexPolygon", convexPolygon.vertices(), convexPolygon.triangles());
		generateMesh("cylinder", cylinder.vertices(), cylinder.triangles());
		generateMesh("disk", disk.vertices(), disk.triangles());
		generateMesh("dodecahedron", dodecahedron.vertices(), dodecahedron.triangles());
		generateMesh("plane", plane.vertices(), plane.triangles());
		generateMesh("icosahedron", icosahedron.vertices(), icosahedron.triangles());
		generateMesh("icoSphere", icoSphere.vertices(), icoSphere.triangles());
		generateMesh("roundedBox", roundedBox.vertices(), roundedBox.triangles());
		generateMesh("sphere", sphere.vertices(), sphere.triangles());
		generateMesh("sphericalCone", sphericalCone.vertices(), sphericalCone.triangles());
		generateMesh("sphereicalTriangle", sphereicalTriangle.vertices(), sphereicalTriangle.triangles());
		generateMesh("spring", spring.vertices(), spring.triangles());
		generateMesh("teapot", teapot.vertices(), teapot.triangles());
		generateMesh("torus", torus.vertices(), torus.triangles());
		generateMesh("torusKnot", torusKnot.vertices(), torusKnot.triangles());
		generateMesh("triangle", triangle.vertices(), triangle.triangles());
		generateMesh("tube", tube.vertices(), tube.triangles());

		loadObjects();
	}

	virtual void render(double detlaTime) {
		glViewport(0, 0, msInfo.windowWidth, msInfo.windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		projectionMatrix = perspective(camera.fov, (float)msInfo.windowWidth / (float)msInfo.windowHeight, camera.zNear, camera.zFar);
		glUniformMatrix4fv(uniforms.projection, 1, GL_FALSE, value_ptr(projectionMatrix));

		viewMatrix = lookAt(camera.position, camera.lookAt, camera.upAxis);
		glUniformMatrix4fv(uniforms.view, 1, GL_FALSE, value_ptr(viewMatrix));

		modelMatrix = mat4(1.f);
		glUniformMatrix4fv(uniforms.model, 1, GL_FALSE, value_ptr(modelMatrix));

		glUniform3fv(uniforms.cameraPosition, 1, (const GLfloat*)value_ptr(camera.position));

		glUniform3fv(uniforms.materialAmbient, 1, (const GLfloat*)value_ptr(vec3(.2f, .1f, 0.f)));
		glUniform3fv(uniforms.materialDiffuse, 1, (const GLfloat*)value_ptr(vec3(1.f, .5f, 0.f)));
		glUniform3fv(uniforms.materialSpecular, 1, (const GLfloat*)value_ptr(vec3(.6f, .6f, .6f)));
		glUniform1f(uniforms.materialShininess, 80.f);
		glUniform1f(uniforms.alpha, 1.f);

		for (const sLight& light : lights) {
			glUniform3fv(light.uniforms.position, 1, (const GLfloat*)value_ptr(light.position));
			glUniform3fv(light.uniforms.ambient, 1, (const GLfloat*)value_ptr(light.ambient));
			glUniform3fv(light.uniforms.diffuse, 1, (const GLfloat*)value_ptr(light.diffuse));
			glUniform3fv(light.uniforms.specular, 1, (const GLfloat*)value_ptr(light.specular));
			glUniform1f(light.uniforms.attenConst, light.attenConst);
			glUniform1f(light.uniforms.attenLinear, light.attenLinear);
			glUniform1f(light.uniforms.attenQuad, light.attenQuad);
		}

		if (wireframe) {
			glDisable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glEnable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		glBindVertexArray(msObjects[mesh].vao);
		glDrawElements(GL_TRIANGLES, msObjects[mesh].indices, GL_UNSIGNED_INT, (GLvoid*)0);
	}

	virtual void onKey(int key, int action) {
		if (action) {
			switch (key) {
			case GLFW_KEY_W:
				camera.position.z += camera.speed;
				break;
			case GLFW_KEY_S:
				camera.position.z -= camera.speed;
				break;
			case GLFW_KEY_A:
				camera.position.x -= camera.speed;
				break;
			case GLFW_KEY_D:
				camera.position.x += camera.speed;
				break;
			case GLFW_KEY_Q:
				camera.position.y += camera.speed;
				break;
			case GLFW_KEY_E:
				camera.position.y -= camera.speed;
				break;
			case GLFW_KEY_ENTER:
				wireframe = !wireframe;
				break;
			case GLFW_KEY_1:
				switch (meshGroup) {
				case 1:
					mesh = "box";
					break;
				case 2:
					mesh = "dodecahedron";
					break;
				case 3:
					mesh = "teapot";
					break;
				}
				break;
			case GLFW_KEY_2:
				switch (meshGroup) {
				case 1:
					mesh = "cappedCone";
					break;
				case 2:
					mesh = "plane";
					break;
				case 3:
					mesh = "torus";
					break;
				}
				break;
			case GLFW_KEY_3:
				switch (meshGroup) {
				case 1:
					mesh = "cappedCylinder";
					break;
				case 2:
					mesh = "icosahedron";
					break;
				case 3:
					mesh = "torusKnot";
					break;
				}
				break;
			case GLFW_KEY_4:
				switch (meshGroup) {
				case 1:
					mesh = "cappedTube";
					break;
				case 2:
					mesh = "icoSphere";
					break;
				case 3:
					mesh = "triangle";
					break;
				}
				break;
			case GLFW_KEY_5:
				switch (meshGroup) {
				case 1:
					mesh = "capsule";
					break;
				case 2:
					mesh = "roundedBox";
					break;
				case 3:
					mesh = "tube";
					break;
				}
				break;
			case GLFW_KEY_6:
				switch (meshGroup) {
				case 1:
					mesh = "cone";
					break;
				case 2:
					mesh = "sphere";
					break;
				case 3:
					break;
				}
				break;
			case GLFW_KEY_7:
				switch (meshGroup) {
				case 1:
					mesh = "convexPolygon";
					break;
				case 2:
					mesh = "sphericalCone";
					break;
				case 3:
					break;
				}
				break;
			case GLFW_KEY_8:
				switch (meshGroup) {
				case 1:
					mesh = "cylinder";
					break;
				case 2:
					mesh = "sphereicalTriangle";
					break;
				case 3:
					break;
				}
				break;
			case GLFW_KEY_9:
				switch (meshGroup) {
				case 1:
					mesh = "disk";
					break;
				case 2:
					mesh = "spring";
					break;
				case 3:
					break;
				}
				break;
			case GLFW_KEY_0:
				if (++meshGroup > 3) {
					meshGroup = 1;
				}
				break;
			default:
				break;
			}
		}

		if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9 && action == GLFW_RELEASE) {
			cout << mesh << endl;
		}
	}
} app;