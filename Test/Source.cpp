#include <efe\efengine.h>
#include <efe\efutils.h>
#include <efe\efcolour.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <generator\DiskMesh.hpp>
#include <generator\PlaneMesh.hpp>
#include <generator\SphereMesh.hpp>
#include <generator\BoxMesh.hpp>
#include <bullet\btBulletCollisionCommon.h>
#include <bullet\btBulletDynamicsCommon.h>

using namespace efe;
using namespace efe::utils;
using namespace std;
using namespace glm;
using namespace generator;

void coutVec3(const vec3& v) {
	cout << v.x << "," << v.y << "," << v.z << endl;
}

class myApp : public cOpenGLApp {

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

	struct sModel {
		int id;
		bool wireframe;
		string mesh;
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
		float shininess;
		float alpha;
		mat4 matrix;
		vec3 position;
		float scale;
		quat orientation;
		bool useLight;
		vec3 velocity;
		vec3 acceleration;
		vec3 lastPosition;
		bool hasPhysics;
		bool hasCollision;
		float mass;
		bool isColliding;
		bool collisionResponded;
	};

	vector<sLight> lights;
	vector<sModel*> models;
	vector<sModel*> spheres;

	GLuint program;
	sShader shaders;
	sCamera camera;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	const float GRAVITY = -98.f;
	const float SIZE = 30.f;
	
	void init() {
		cOpenGLApp::init();

		shaders.vertex = "vertex.glsl";
		shaders.fragment = "fragment.glsl";

		//camera.position = vec3(0.f, 90.f, -1.f);
		camera.position = vec3(0.f, 15.f, -70.f);
		//camera.lookAt = vec3(0.f, 50.f, 0.f);
		camera.lookAt = vec3(0.f);
		camera.upAxis = vec3(0.f, 1.f, 0.f);
		camera.fov = 1.f;
		camera.zNear = .01f;
		camera.zFar = 1000.f;
		camera.speed = 1.f;

		{
			sLight light;
			light.position = vec3(-10.f, 11.f, -5.f);
			light.ambient = vec3(.2f, .2f, .2f);
			light.diffuse = vec3(1.f, 1.f, 1.f);
			light.specular = vec3(1.f, 1.f, 1.f);
			light.attenConst = 0.f;
			light.attenLinear = .1f;
			light.attenQuad = .01f;
			lights.push_back(light);
		}

		{
			sLight light;
			light.position = vec3(10.f, 11.f, -5.f);
			light.ambient = vec3(.2f, .2f, .2f);
			light.diffuse = vec3(1.f, 1.f, 1.f);
			light.specular = vec3(1.f, 1.f, 1.f);
			light.attenConst = 0.f;
			light.attenLinear = .1f;
			light.attenQuad = .01f;
			lights.push_back(light);
		}

		{
			sLight light;
			light.position = vec3(-10.f, 11.f, 5.f);
			light.ambient = vec3(.2f, .2f, .2f);
			light.diffuse = vec3(1.f, 1.f, 1.f);
			light.specular = vec3(1.f, 1.f, 1.f);
			light.attenConst = 0.f;
			light.attenLinear = .1f;
			light.attenQuad = .01f;
			lights.push_back(light);
		}

		{
			sLight light;
			light.position = vec3(10.f, 11.f, 5.f);
			light.ambient = vec3(.2f, .2f, .2f);
			light.diffuse = vec3(1.f, 1.f, 1.f);
			light.specular = vec3(1.f, 1.f, 1.f);
			light.attenConst = 0.f;
			light.attenLinear = .1f;
			light.attenQuad = .01f;
			lights.push_back(light);
		}
	}

	bool isCollision(sModel* m1, sModel* m2) {
		return distance(m1->position, m2->position) < m1->scale + m2->scale;
	}

	void checkCollisionLoad(sModel* m1) {
		bool collided = false;
		for (sModel* m2 : spheres) {
			while (m1 != m2 && isCollision(m1, m2)) {
				collided = true;
				m1->position = vec3(random(-SIZE, SIZE), random(1.f, SIZE), random(-SIZE, SIZE));
			}
		}
		if (collided) {
			checkCollisionLoad(m1);
		}
	}

	void collisionResponse(sModel* a, sModel* b, float deltaTime) {
		vec3 U1x, U1y, U2x, U2y, V1x, V1y, V2x, V2y;


		float m1, m2, x1, x2;
		vec3 v1temp, v1, v2, v1x, v2x, v1y, v2y, x(a->position - b->position);

		normalize(x);
		v1 = a->velocity;
		x1 = dot(x, v1);
		v1x = x * x1;
		v1y = v1 - v1x;
		m1 = a->mass;

		x = x*-1.f;
		v2 = b->velocity;
		x2 = dot(x, v2);
		v2x = x * x2;
		v2y = v2 - v2x;
		m2 = b->mass;

		a->velocity = vec3(v1x* deltaTime*(m1 - m2) / (m1 + m2) + v2x* deltaTime*(2 * m2)  / (m1 + m2) + v1y);
		b->velocity = vec3(v1x* deltaTime*(2 * m1) / (m1 + m2) + v2x* deltaTime*(m2 - m1)  / (m1 + m2) + v2y);
		updateColour(a, colour::Red);
		updateColour(b, colour::Red);
		a->isColliding = b->isColliding = a->collisionResponded = b->collisionResponded = true;
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
		uniforms.useLight = glGetUniformLocation(program, "useLight");

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

		DiskMesh disk;
		PlaneMesh plane;
		SphereMesh sphere;
		BoxMesh box;

		generateMesh("disk", disk.vertices(), disk.triangles());
		generateMesh("plane", plane.vertices(), plane.triangles());
		generateMesh("sphere", sphere.vertices(), sphere.triangles());
		generateMesh("box", box.vertices(), box.triangles());

		loadObjects();

		int modelID = 0;

		for (int i = 0; i < 100; ++i) {
			sModel* model = new sModel();
			model->mesh = "sphere";
			model->wireframe = false;
			model->diffuse = colour::Green;
			model->ambient = model->diffuse / 2.f;
			model->specular = vec3(.5f);
			model->shininess = 100.f;
			model->alpha = 1.f;
			model->position = vec3(random(-SIZE, SIZE), random(1.f, SIZE), random(-SIZE, SIZE));

			checkCollisionLoad(model);

			model->scale = 1.f;
			model->orientation *= quat(radians(vec3(0.f, 0.f, 0.f)));
			model->useLight = true;
			model->hasCollision = true;
			model->hasPhysics = true;
			model->mass = 1.f;
			model->id = modelID++;
			models.push_back(model);
			spheres.push_back(model);
		}
		//{
		//	sModel* model = new sModel();
		//	model->mesh = "sphere";
		//	model->wireframe = false;
		//	model->diffuse = colour::Red;
		//	model->ambient = model->diffuse / 2.f;
		//	model->specular = vec3(.5f);
		//	model->shininess = 100.f;
		//	model->alpha = 1.f;
		//	model->position = vec3(0.f, 50.f, 0.f);
		//	model->scale = 1.f;
		//	model->orientation *= quat(radians(vec3(0.f, 0.f, 0.f)));
		//	model->useLight = true;
		//	model->hasCollision = false;
		//	model->hasPhysics = false;

		//	sModel* model2 = new sModel(*model);
		//	model2->position.y += 2.f;

		//	models.push_back(model);
		//	spheres.push_back(model);
		//	models.push_back(model2);
		//	spheres.push_back(model2);
		//}

		{
			sModel* model = new sModel();
			model->mesh = "box";
			model->wireframe = false;
			model->diffuse = colour::Blue;
			model->ambient = model->diffuse / 2.f;
			model->specular = vec3(.5f);
			model->shininess = 100.f;
			model->alpha = 1.f;
			model->position = vec3(0.f, -SIZE, 0.f);
			model->scale = SIZE;
			model->orientation *= quat(radians(vec3(-90.f, 0.f, 0.f)));
			model->useLight = true;
			model->id = modelID++;
			models.push_back(model);
		}
	}

	void updateColour(sModel* m, const vec4& c) {
		m->diffuse = c;
		m->ambient = c / 2.f;
	}

	void update(double deltaTime) {
		for (sModel* model1 : models) {
			if (model1->hasPhysics) {
				model1->velocity.y += GRAVITY * (float)deltaTime;
				model1->lastPosition = model1->position;
				model1->position += model1->velocity * (float)deltaTime;

				if (model1->position.y < 1.f) {
					model1->velocity.y *= -.75f;
					model1->position = model1->lastPosition;
				}
			}
			if (model1->hasCollision) {
				for (sModel* model2 : spheres) {
					if (model2->hasCollision && !model1->collisionResponded && !model2->collisionResponded && model1 != model2 && isCollision(model1, model2)) {
						collisionResponse(model1, model2, (float)deltaTime);
					}
					else if (!model1->isColliding) {
						updateColour(model1, colour::Green);
					}
				}
			}
		}
		for (sModel* model : models) {
			model->isColliding = false;
			model->collisionResponded = false;
		}
	}

	virtual void render(double deltaTime) {
		glViewport(0, 0, msInfo.windowWidth, msInfo.windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		projectionMatrix = perspective(camera.fov, (float)msInfo.windowWidth / (float)msInfo.windowHeight, camera.zNear, camera.zFar);
		glUniformMatrix4fv(uniforms.projection, 1, GL_FALSE, value_ptr(projectionMatrix));

		viewMatrix = lookAt(camera.position, camera.lookAt, camera.upAxis);
		glUniformMatrix4fv(uniforms.view, 1, GL_FALSE, value_ptr(viewMatrix));

		glUniform3fv(uniforms.cameraPosition, 1, (const GLfloat*)value_ptr(camera.position));


		for (const sLight& light : lights) {
			glUniform3fv(light.uniforms.position, 1, (const GLfloat*)value_ptr(light.position));
			glUniform3fv(light.uniforms.ambient, 1, (const GLfloat*)value_ptr(light.ambient));
			glUniform3fv(light.uniforms.diffuse, 1, (const GLfloat*)value_ptr(light.diffuse));
			glUniform3fv(light.uniforms.specular, 1, (const GLfloat*)value_ptr(light.specular));
			glUniform1f(light.uniforms.attenConst, light.attenConst);
			glUniform1f(light.uniforms.attenLinear, light.attenLinear);
			glUniform1f(light.uniforms.attenQuad, light.attenQuad);
		}

		
		update(deltaTime);

		for (sModel* model : models) {
			model->matrix = translate(mat4(1.f), model->position);
			model->matrix *= mat4(model->orientation);
			model->matrix = scale(model->matrix, vec3(model->scale));
			glUniformMatrix4fv(uniforms.model, 1, GL_FALSE, value_ptr(model->matrix));
			
			glUniform3fv(uniforms.materialAmbient, 1, (const GLfloat*)value_ptr(model->ambient));
			glUniform3fv(uniforms.materialDiffuse, 1, (const GLfloat*)value_ptr(model->diffuse));
			glUniform3fv(uniforms.materialSpecular, 1, (const GLfloat*)value_ptr(model->specular));
			glUniform1f(uniforms.materialShininess, model->shininess);
			glUniform1f(uniforms.alpha, model->alpha);

			glUniform1i(uniforms.useLight, model->useLight ? GL_TRUE : GL_FALSE);
			if (model->wireframe) {
				glDisable(GL_CULL_FACE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else {
				glEnable(GL_CULL_FACE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			glBindVertexArray(msObjects[model->mesh].vao);
			glDrawElements(GL_TRIANGLES, msObjects[model->mesh].indices, GL_UNSIGNED_INT, (GLvoid*)0);
		}
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
				if (action == GLFW_PRESS) {
					coutVec3(camera.position);
				}
				break;
			default:
				break;
			}
		}
	}

	virtual void onMouseButton(int button, int action) {
		switch (button)
		{
		case 1:
			coutVec3(camera.position);
			break;
		default:
			break;
		}
	}

	virtual void onMouseMove(int x, int y) {
		
	}

	virtual void onMouseWheel(int pos) {
		
	}

	virtual void shutdown() {
		for (sModel* m : models) {
			delete m;
		}
	}
} app;