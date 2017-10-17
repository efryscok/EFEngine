#include <efe\efengine.h>
#include <efe\efutils.h>
#include <efe\efcolour.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <generator\DiskMesh.hpp>
#include <generator\PlaneMesh.hpp>
#include <generator\SphereMesh.hpp>
#include <generator\BoxMesh.hpp>
#include <generator\ConeMesh.hpp>
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
	sUniforms uniforms;
	vector<cLight> lights;
	vector<sModel*> models;
	vector<sModel*> spheres;

	GLuint program;
	sShader shaders;
	cCamera camera;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	const float GRAVITY = -98.f;
	const float SIZE = 30.f;
	
	btDiscreteDynamicsWorld* dynamicsWorld;
	vector<btRigidBody*> fallRigidBodies;
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btCollisionShape* groundShape;
	btCollisionShape* fallShape;
	btDefaultMotionState* groundMotionState;
	btRigidBody* groundRigidBody;

	sModel* sphere;

	vec3 camPosStart, camPosTop, camPosBottom;

	int numberOfBalls;
	int maxNumberOfBalls;

	void init() {
		cOpenGLApp::init();

		broadphase = new btDbvtBroadphase();

		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfiguration);

		solver = new btSequentialImpulseConstraintSolver;

		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

		dynamicsWorld->setGravity(btVector3(0, -9.8, 0));

		//groundShape = new btBoxShape(btVector3(1, 1, 1));
		//groundShape = new btConeShape(.5, 1);
		//groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
		groundShape = new btSphereShape(1);

		fallShape = new btSphereShape(1);
		//fallShape = new btBoxShape(btVector3(1, 1, 1));


		groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
		groundRigidBody = new btRigidBody(groundRigidBodyCI);
		dynamicsWorld->addRigidBody(groundRigidBody);

		numberOfBalls = 100;
		maxNumberOfBalls = 100;

		for (int i = 0; i < maxNumberOfBalls; ++i) {
			btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(random(-.25, .25), 25 + i * 3, random(-.25, .25))));
			btScalar mass = 1;
			btVector3 fallInertia(0, 0, 0);
			fallShape->calculateLocalInertia(mass, fallInertia);
			btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
			fallRigidBodies.push_back(new btRigidBody(fallRigidBodyCI));
			dynamicsWorld->addRigidBody(fallRigidBodies.back());
		}

		shaders.vertex = "vertex.glsl";
		shaders.fragment = "fragment.glsl";

		//camera.position = vec3(0.f, 90.f, -1.f);
		camera.position = vec3(0.f, 15.f, -70.f);
		camPosStart = camera.position;
		camPosTop = vec3(0.f, 70.f, -5.f);
		camPosBottom = vec3(0.f, -70.f, -5.f);

		cLight light;
		//light.position = vec3(-12.5f, 12.5f, -12.5f);
		lights.push_back(light);

		light.position = vec3(12.5f, 12.5f, -12.5f);
		//lights.push_back(light);
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
		uniforms.skybox = glGetUniformLocation(program, "skybox");
		uniforms.isSkybox = glGetUniformLocation(program, "isSkybox");

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

		DiskMesh diskMesh;
		PlaneMesh planeMesh;
		SphereMesh sphereMesh;
		BoxMesh box;
		ConeMesh cone;

		generateMesh("disk", diskMesh.vertices(), diskMesh.triangles());
		generateMesh("plane", planeMesh.vertices(), planeMesh.triangles());
		generateMesh("sphere", sphereMesh.vertices(), sphereMesh.triangles());
		generateMesh("box", box.vertices(), box.triangles());
		generateMesh("cone", cone.vertices(), cone.triangles());

		msObjects["icosphereinverted.obj"];

		loadObjects();

		msTextures["earth.bmp"];

		loadTextures();

		//msTextures["skybox"] = loadCubemap("debug_right.bmp", "debug_left.bmp", "debug_top.bmp", "debug_bottom.bmp", "debug_front.bmp", "debug_back.bmp");

		int modelID = 0;

		sphere = new sModel();
		sphere->mesh = "sphere";
		sphere->wireframe = false;
		sphere->diffuse = colour::Green;
		sphere->ambient = sphere->diffuse / 2.f;
		sphere->specular = vec3(.5f);
		sphere->shininess = 100.f;
		sphere->alpha = 1.f;
		sphere->position = vec3(0.f);
		sphere->scale = 1.f;
		sphere->orientation *= quat(radians(vec3(0.f, 0.f, 0.f)));
		sphere->useLight = true;
		sphere->useCollision = true;
		sphere->usePhysics = true;
		sphere->mass = 1.f;
		sphere->id = modelID++;

		sModel* model = new sModel();
		model->mesh = "sphere";
		model->wireframe = false;
		model->diffuse = colour::Gray;
		model->ambient = model->diffuse / 2.f;
		model->specular = vec3(.5f);
		model->shininess = 100.f;
		model->alpha = 1.f;
		model->position = vec3(0.f, 0.f, 0.f);
		model->scale = 1.f;
		model->orientation *= quat(radians(vec3(-90.f, 0.f, 0.f)));
		model->useLight = true;
		model->id = modelID++;
		models.push_back(model);

		sModel* skybox = new sModel();
		skybox->mesh = "icosphereinverted.obj";
		skybox->wireframe = false;
		skybox->texture = "earth.bmp";
		skybox->isSkybox = true;
		skybox->position = vec3(0.f);
		skybox->scale = 500.f;
		
	}

	void draw(sModel* model) {
		model->matrix = translate(mat4(1.f), model->position);
		model->matrix *= mat4(model->orientation);
		model->matrix = scale(model->matrix, vec3(model->scale));
		glUniformMatrix4fv(uniforms.model, 1, GL_FALSE, value_ptr(model->matrix));


		glUniform1i(uniforms.useLight, model->useLight ? GL_TRUE : GL_FALSE);
		if (model->wireframe) {
			glDisable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glEnable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (model->isSkybox) {
			//glDisable(GL_CULL_FACE);
			glUniform1i(uniforms.isSkybox, GL_TRUE);
			bindTexture(uniforms.skybox, GL_TEXTURE_2D, model->texture);
		}
		else {
			glUniform1i(uniforms.isSkybox, GL_FALSE);
			glUniform3fv(uniforms.materialAmbient, 1, (const GLfloat*)value_ptr(model->ambient));
			glUniform3fv(uniforms.materialDiffuse, 1, (const GLfloat*)value_ptr(model->diffuse));
			glUniform3fv(uniforms.materialSpecular, 1, (const GLfloat*)value_ptr(model->specular));
			glUniform1f(uniforms.materialShininess, model->shininess);
			glUniform1f(uniforms.alpha, model->alpha);
		}

		glBindVertexArray(msObjects[model->mesh].vao);
		glDrawElements(GL_TRIANGLES, msObjects[model->mesh].indices, GL_UNSIGNED_INT, (GLvoid*)0);
	}

	virtual void render(double deltaTime) {
		glViewport(0, 0, msInfo.windowWidth, msInfo.windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		projectionMatrix = perspective(camera.fov, (float)msInfo.windowWidth / (float)msInfo.windowHeight, camera.zNear, camera.zFar);
		glUniformMatrix4fv(uniforms.projection, 1, GL_FALSE, value_ptr(projectionMatrix));

		viewMatrix = lookAt(camera.position, camera.lookAt, camera.upAxis);
		glUniformMatrix4fv(uniforms.view, 1, GL_FALSE, value_ptr(viewMatrix));

		glUniform3fv(uniforms.cameraPosition, 1, (const GLfloat*)value_ptr(camera.position));


		for (const cLight& light : lights) {
			glUniform3fv(light.uniforms.position, 1, (const GLfloat*)value_ptr(light.position));
			glUniform3fv(light.uniforms.ambient, 1, (const GLfloat*)value_ptr(light.ambient));
			glUniform3fv(light.uniforms.diffuse, 1, (const GLfloat*)value_ptr(light.diffuse));
			glUniform3fv(light.uniforms.specular, 1, (const GLfloat*)value_ptr(light.specular));
			glUniform1f(light.uniforms.attenConst, light.attenConst);
			glUniform1f(light.uniforms.attenLinear, light.attenLinear);
			glUniform1f(light.uniforms.attenQuad, light.attenQuad);
		}

		dynamicsWorld->stepSimulation(1 / 60.f, 10);

		for (vector<btRigidBody*>::iterator thing = fallRigidBodies.begin(); thing != fallRigidBodies.end();) {

			btTransform trans;
			thing.operator*()->getMotionState()->getWorldTransform(trans);

			sphere->position = glmVec3(trans.getOrigin());

			if (sphere->position.y < -100.f) {
				dynamicsWorld->removeRigidBody(thing.operator*());
				delete thing.operator*()->getMotionState();
				delete thing.operator*();
				thing = fallRigidBodies.erase(thing);
			}
			else {
				++thing;
			}

			draw(sphere);
		}

		for (sModel* model : models) {
			draw(model);
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
			case GLFW_KEY_1:
				camera.position = camPosStart;
				break;
			case GLFW_KEY_2:
				camera.position = camPosTop;
				break;
			case GLFW_KEY_3:
				camera.position = camPosBottom;
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
		for (auto thing : fallRigidBodies) {
			dynamicsWorld->removeRigidBody(thing);
			delete thing->getMotionState();
			delete thing;
		}
		dynamicsWorld->removeRigidBody(groundRigidBody);
		delete groundRigidBody->getMotionState();
		delete groundRigidBody;
		delete fallShape;
		delete groundShape;
		delete dynamicsWorld;
		delete solver;
		delete collisionConfiguration;
		delete dispatcher;
		delete broadphase;
	}
} app;