
#ifdef RENDER_SNIPPET

#include <stdio.h>
#include<ctime>
//
#include <myInclude\GL\glew.h>
//#include <myInclude\GL\glut.h>

#include <myInclude\GL\wglew.h>
#include <myInclude\glm\glm.hpp>
#include <myInclude/glm/gtc/matrix_transform.hpp>
#include <myInclude\GLFW\glfw3.h>
#include <myInclude\SOIL\SOIL.h>

#include <windows.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <random>
#include <stdlib.h>

#include <mat_helper.h>
#include "shader.h"
#include "camera.h"
#include "model.h"


#include <vector>

#include "PxPhysicsAPI.h"

#include "../SnippetRender/SnippetRender.h"
#include "../SnippetRender/SnippetCamera.h"

using namespace physx;

extern void initPhysics(float* verticesPlane, unsigned int* indicesBox, Model &chair1, bool interactive);
extern void push(PxVec3 position, PxQuat rotation, PxVec3 velocity);
extern void stepPhysics(bool interactive);
extern void cleanupPhysics(bool interactive);
extern void keyPress(unsigned char key, const PxTransform& camera);



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float PI = acos(-1.0);

// camera
//Camera camera(glm::vec3(-500.0f, 1000.0f, 1000.0f));
Camera camera(glm::vec3(100.0f, 400.0f, 500.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float cX = 0;
float cY = 0;
float cZ = -0.49239;

float EPS = 1e-6;
int sgn(float x) {
	return (x > EPS) - (x < -EPS);
}
int classify(float x, float y, float z) {
	if (sgn(x - cX) > 0) {
		if (sgn(y - cY) > 0) {
			if (sgn(z - cZ) > 0) {
				return 0;
			}
			if (sgn(z - cZ) < 0) {
				return 4;
			}
		}
		if (sgn(y - cY) < 0) {
			if (sgn(z - cZ) > 0) {
				return 3;
			}
			if (sgn(z - cZ) < 0) {
				return 7;
			}
		}
	}
	if (sgn(x - cX) < 0) {
		if (sgn(y - cY) > 0) {
			if (sgn(z - cZ) > 0) {
				return 1;
			}
			if (sgn(z - cZ) < 0) {
				return 5;
			}
		}
		if (sgn(y - cY) < 0) {
			if (sgn(z - cZ) > 0) {
				return 2;
			}
			if (sgn(z - cZ) < 0) {
				return 6;
			}
		}
	}
	return -1;
}
float xmx = -1e10;
float xmn = 1e10;
void getLeg(Model & model, float boxLeg[4][8][3]) {
	int meshId = 0;
	for (int index = 0; index < 4; ++index) {
		int id = index + 4;
		float mn[3] = { 1e10, 1e10, 1e10 };
		float mx[3] = { -1e10, -1e10, -1e10 };
		int sz = model.meshes[meshId].vertices.size();
		for (int i = 0; i < model.meshes[meshId].vertices.size(); ++i) {
			float x = model.meshes[meshId].vertices[i].Position.x;
			if (x > xmx) xmx = x;
			if (x < xmn) xmn = x;
			float y = model.meshes[meshId].vertices[i].Position.y;
			float z = model.meshes[meshId].vertices[i].Position.z;
			//		if(z < 0) printf("%10.5f %c", z, i % 10 == 0 ? '\n' : ' ');
			if (classify(x, y, z) == id) {
				if (x < mn[0]) mn[0] = x;
				if (x > mx[0]) mx[0] = x;
				if (y < mn[1]) mn[1] = y;
				if (y > mx[1]) mx[1] = y;
				if (z < mn[2]) mn[2] = z;
				if (z > mx[2]) mx[2] = z;
			}
		}
		if (index == 0 || index == 1) mn[1] += 1.2;
		if (index == 3) mn[0] = mx[0] - (boxLeg[2][1][0] - boxLeg[2][0][0]);
		boxLeg[index][0][0] = mn[0]; boxLeg[index][3][0] = mn[0]; boxLeg[index][4][0] = mn[0]; boxLeg[index][7][0] = mn[0];
		boxLeg[index][1][0] = mx[0]; boxLeg[index][2][0] = mx[0]; boxLeg[index][5][0] = mx[0]; boxLeg[index][6][0] = mx[0];
		boxLeg[index][0][1] = mn[1]; boxLeg[index][1][1] = mn[1]; boxLeg[index][4][1] = mn[1]; boxLeg[index][5][1] = mn[1];
		boxLeg[index][2][1] = mx[1]; boxLeg[index][3][1] = mx[1]; boxLeg[index][6][1] = mx[1]; boxLeg[index][7][1] = mx[1];
		boxLeg[index][0][2] = mn[2]; boxLeg[index][1][2] = mn[2]; boxLeg[index][2][2] = mn[2]; boxLeg[index][3][2] = mn[2];
		boxLeg[index][4][2] = mx[2]; boxLeg[index][5][2] = mx[2]; boxLeg[index][6][2] = mx[2]; boxLeg[index][7][2] = mx[2];
	}
	/*for (int i = 0; i < 8; ++i) {
		boxLeg[3][i][0] = -boxLeg[2][i][0];
		boxLeg[3][i][1] = boxLeg[2][i][1];
		boxLeg[3][i][2] = boxLeg[2][i][2];
	}*/
}
/*
void getSeat(Model & model, float boxSeat[8][3]) {
	float mn[3] = { 1e10, 1e10, 1e10 };
	float mx[3] = { -1e10, -1e10, -1e10 };
	for (int p = 0; p < model.meshes.size(); ++p) {
		int sz = model.meshes[p].vertices.size();
		for (int i = 0; i < model.meshes[p].vertices.size(); ++i) {
			float z = model.meshes[p].vertices[i].Position.z;
			if (sgn(z - cZ) > 0) {
				float x = model.meshes[p].vertices[i].Position.x;
				float y = model.meshes[p].vertices[i].Position.y;
				if (x < mn[0]) mn[0] = x;
				if (x > mx[0]) mx[0] = x;
				if (y < mn[1]) mn[1] = y;
				if (y > mx[1]) mx[1] = y;
				if (z < mn[2]) mn[2] = z;
				if (z > mx[2]) mx[2] = z;
			}
		}
	}
	boxSeat[0][0] = mn[0]; boxSeat[3][0] = mn[0]; boxSeat[4][0] = mn[0]; boxSeat[7][0] = mn[0];
	boxSeat[1][0] = mx[0]; boxSeat[2][0] = mx[0]; boxSeat[5][0] = mx[0]; boxSeat[6][0] = mx[0];
	boxSeat[0][1] = mn[1]; boxSeat[1][1] = mn[1]; boxSeat[4][1] = mn[1]; boxSeat[5][1] = mn[1];
	boxSeat[2][1] = mx[1]; boxSeat[3][1] = mx[1]; boxSeat[6][1] = mx[1]; boxSeat[7][1] = mx[1];
	boxSeat[0][2] = mn[2]; boxSeat[1][2] = mn[2]; boxSeat[2][2] = mn[2]; boxSeat[3][2] = mn[2];
	boxSeat[4][2] = mx[2]; boxSeat[5][2] = mx[2]; boxSeat[6][2] = mx[2]; boxSeat[7][2] = mx[2];
}
void boxMerge(float verticesBox[40 * 6], float boxSeat[8][3], float boxLeg[4][8][3]) {
	for (int i = 0; i < 40; ++i) {
		if (i < 8) {
			for (int j = 0; j < 3; ++j) verticesBox[i * 6 + j] = boxSeat[i][j];
			verticesBox[i * 6 + 3] = verticesBox[i * 6 + 4] = 0;
			verticesBox[i * 6 + 5] = 1.0;
		}
		else {
			for (int j = 0; j < 3; ++j) verticesBox[i * 6 + j] = boxLeg[i / 8 - 1][i % 8][j];
			verticesBox[i * 6 + 3] = verticesBox[i * 6 + 4] = 0;
			verticesBox[i * 6 + 5] = 1.0;
		}
	}
}*/
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
		//return;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		//return;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		camera.ProcessKeyboard(LEFT, deltaTime);
		//return;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		camera.ProcessKeyboard(RIGHT, deltaTime);
		//return;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		camera.ProcessKeyboard(RIGHT, deltaTime);
		//return;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
		camera.ProcessKeyboard(UP, deltaTime);
		//return;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
		camera.ProcessKeyboard(DOWN, deltaTime);
		//return;
	}
	/*keyPress(key, sCamera->getTransform());
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) pushObject(10, 2.0f);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) createStack(PxTransform(PxVec3(0, 0, 0)), 10, 2.0f);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) createDynamic(camera, PxSphereGeometry(3.0f), camera.rotate(PxVec3(0, 0, -1)) * 200);*/
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}


namespace
{
	Snippets::Camera*	sCamera;

	void motionCallback(int x, int y)
	{
		sCamera->handleMotion(x, y);
	}

	void keyboardCallback(unsigned char key, int x, int y)
	{
		if (key == 27)
			exit(0);

		if (!sCamera->handleKey(key, x, y))
			keyPress(key, sCamera->getTransform());
	}

	void mouseCallback(int button, int state, int x, int y)
	{
		sCamera->handleMouse(button, state, x, y);
	}

	void idleCallback()
	{
		glutPostRedisplay();
	}

	void renderCallback(float *verticesBox, unsigned int* indicesBox, Model &chair, Shader &ourShader, Camera &camera, Shader &sampleShader, unsigned int VAO, unsigned int VBO, unsigned int VAOBOX, unsigned int VBOBOX, unsigned int EBOBOX)
	{
		stepPhysics(true);

		//Snippets::startRender(sCamera->getEye(), sCamera->getDir());

		PxScene* scene;
		PxGetPhysics().getScenes(&scene, 1);
		PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
		if (nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
			Snippets::renderActors(verticesBox, indicesBox, chair, ourShader, camera, sampleShader, VAO, VBO, VAOBOX, VBOBOX, EBOBOX, &actors[0], static_cast<PxU32>(actors.size()), true);
		}

		//Snippets::finishRender();
	}

	void exitCallback(void)
	{
		delete sCamera;
		cleanupPhysics(true);
	}
}
void renderLoop(){
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 
	GLFWwindow* window = glfwCreateWindow(800, 600, "chair", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return;
	}
	glfwSetWindowPos(window, 600, 400);
	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("1.model_loading.vs", "1.model_loading.fs");
	Shader sampleShader("sample.vs", "sample.fs");
	// load models
	// -----------
	Model chair1("resources/sChair/sChair.obj");
	Model chair2("resources/chair/chair.obj");

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	/*float tmp = 0.47710f;
	float* vertices;
	vertices = new float[18];
	vertices[0] = 50.0f;
	vertices[1] = -50.0f;
	vertices[2] = tmp;
	vertices[3] = 1.0f;
	vertices[4] = 0.0f;
	vertices[5] = 0.0f;

	vertices[6] = -50.0f;
	vertices[7] = -50.0f;
	vertices[8] = tmp;
	vertices[9] = 0.0f;
	vertices[10] = 1.0f;
	vertices[11] = 0.0f;

	vertices[12] = 0.0f;
	vertices[13] = 50.0f;
	vertices[14] = tmp;
	vertices[15] = 0.0f;
	vertices[16] = 0.0f;
	vertices[17] = 1.0f;*/
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Aray Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	/*glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);*/

	/*float vertices2[] = {
		// positions         // colors
		 50.0f, -50.0f, 39.1f,  1.0f, 0.0f, 0.0f,  // bottom right
		-50.0f, -50.0f, 39.1f,  0.0f, 1.0f, 0.0f,  // bottom left
		 0.0f,  50.0f, 39.1f,  0.0f, 0.0f, 1.0f   // top 
	};
	unsigned int VBO2, VAO2;
	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);*/

	//float boxSeat[8][3], boxLeg[4][8][3], verticesBox[40 * 6];
	//getSeat(chair1, boxSeat);
	//getLeg(chair1, boxLeg);
	//boxMerge(verticesBox, boxSeat, boxLeg);
	/*unsigned int indices[24 * 5] = { // 注意索引从0开始! 
	0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7
	};
	for (int i = 1; i < 5; ++i) {
		for (int j = 0; j < 24; ++j) {
			indices[i * 24 + j] = indices[j] + i * 8;
		}
	}*/
	/*for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 3; ++j) printf("%.5f%c", boxSeat[i][j], j == 2 ? '\n' : ' ');
	}

	for (int i = 0; i < 40; ++i) {
		for (int j = 0; j < 6; ++j) printf("%.5f%c", verticesBox[i * 6 + j], j == 5 ? '\n' : ' ');
	*/



	//draw concentric square
	float verticesPlane[20 * 24];
	float step = 0;
	for (int i = 0; i < 20; ++i) {
		step += 150;
		verticesPlane[i * 24 + 0] = step; verticesPlane[i * 24 + 1] = 0; verticesPlane[i * 24 + 2] = step;
		verticesPlane[i * 24 + 3] = 0; verticesPlane[i * 24 + 4] = 0; verticesPlane[i * 24 + 5] = 0;
		verticesPlane[i * 24 + 6] = step; verticesPlane[i * 24 + 7] = 0; verticesPlane[i * 24 + 8] = -step;
		verticesPlane[i * 24 + 9] = 0; verticesPlane[i * 24 + 10] = 0; verticesPlane[i * 24 + 11] = 0;
		verticesPlane[i * 24 + 12] = -step; verticesPlane[i * 24 + 13] = 0; verticesPlane[i * 24 + 14] = -step;
		verticesPlane[i * 24 + 15] = 0; verticesPlane[i * 24 + 16] = 0; verticesPlane[i * 24 + 17] = 0;
		verticesPlane[i * 24 + 18] = -step; verticesPlane[i * 24 + 19] = 0; verticesPlane[i * 24 + 20] = step;
		verticesPlane[i * 24 + 21] = 0; verticesPlane[i * 24 + 22] = 0; verticesPlane[i * 24 + 23] = 0;
	}
	unsigned int indicesPlane[20 * 8] = {
		0, 1, 1, 2, 2, 3, 3, 0
	};
	for (int i = 1; i < 20; ++i) {
		for (int j = 0; j < 8; ++j)
			indicesPlane[i * 8 + j] = indicesPlane[j] + i * 4;
	}
	unsigned int VBO3, VAO3, EBO3;
	glGenVertexArrays(1, &VAO3);
	glGenBuffers(1, &VBO3);
	glGenBuffers(1, &EBO3);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 20 * 24, verticesPlane, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 20 * 8, indicesPlane, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	unsigned int VAOBOX, VBOBOX, EBOBOX;
	glGenVertexArrays(1, &VAOBOX);
	glGenBuffers(1, &VBOBOX);
	glGenBuffers(1, &EBOBOX);
	srand(time(0));
	float verticesBox[40 * 6];
	unsigned int indicesBox[24 * 5] = { // 注意索引从0开始! 
		0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7
	};
	initPhysics(verticesBox, indicesBox, chair2, true);
	glBindVertexArray(VAOBOX);

	glBindBuffer(GL_ARRAY_BUFFER, VBOBOX);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 40 * 6, verticesBox, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOBOX);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 24 * 5, indicesBox, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);


	int pushX = 0, pushZ = 0;
	float push_currentFrame = 0, push_lastFrame = 0, push_deltaTime = 0;



	//pile four chairs up and throw a chair to hit them 
	push(PxVec3(-200, 39.1, 0.0), PxQuat(-PI / 2, PxVec3(1, 0, 0)), PxVec3(0, 0, 0));
	push(PxVec3(-200, 39.1 * 3, 0.0), PxQuat(-PI / 2, PxVec3(1, 0, 0)), PxVec3(0, 0, 0));
	push(PxVec3(-200, 39.1, 50.0), PxQuat(-PI / 2, PxVec3(1, 0, 0)), PxVec3(0, 0, 0));
	push(PxVec3(-200, 39.1 * 3, 50.0), PxQuat(-PI / 2, PxVec3(1, 0, 0)), PxVec3(0, 0, 0));
	push(PxVec3(0, 300, 42.0), PxQuat(-PI / 3, PxVec3(1 / sqrt(3.0), -1 / sqrt(3.0), 1 / sqrt(3.0))), PxVec3(-50*2, -37*2, 0));
	bool have = 0;
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = push_currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		push_deltaTime = push_currentFrame - push_lastFrame;
		lastFrame = currentFrame;
		if (push_deltaTime > 10 && have == 0) {
			have = 1;
			push(PxVec3(0, 300, 50.0), PxQuat(-PI / 3 * 1.2, PxVec3(1 / sqrt(3.0), -1 / sqrt(3.0), 1 / sqrt(3.0))), PxVec3(-50 * 2, -37 * 2, 0));
		}
		/*if (push_deltaTime > 0.1) {
			push_lastFrame = push_currentFrame;
			float rad = rand() * 1.0 / RAND_MAX * 3.14159;
			long long x = rand(), y = rand(), z = rand();
			long long q = x * x + y * y + z * z;
			float d = sqrt(q * 1.0);
			pushX += 120; 
			if (pushX == 1200) {
				pushX = 0;
				pushZ -= 120;
				if (pushZ == -1200) {
					pushZ = 0;
				}
			}
			PxVec3 position(pushX, 900 + rand() % 100, pushZ);
			float s = position.x * position.x + position.y * position.y + position.z * position.z;
			s /= 50000;
			s = sqrt(s);
		//	push(position, PxQuat(rad, PxVec3(x * 1.0 / d, y * 1.0 / d, z * 1.0 / d)), PxVec3(-position.x / s, -position.y / s, -position.z / s));
		}*/
		
		processInput(window);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		ourShader.setMat4("projection", projection);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("view", view);
		glm::mat4 model;
		ourShader.setMat4("model", model);
		//chair1.Draw(ourShader);
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(100.0f, 0.0f, 0.0f));
		model = glm::rotate(model, (float)(PI / 2.0), glm::vec3(0.0f, 0.0f, -1.0f));
		ourShader.setMat4("model", model);
		//chair2.Draw(ourShader);
		sampleShader.use();
		sampleShader.setMat4("projection", projection);
		sampleShader.setMat4("view", view);
		glm::mat4 model0;
		sampleShader.setMat4("model", model0);
		glBindVertexArray(VAO3);
		glDrawElements(GL_LINES, 20 * 8, GL_UNSIGNED_INT, 0);
		//sampleShader.setMat4("model", model);
		//glDrawElements(GL_LINES, 24 * 5, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		renderCallback(verticesBox, indicesBox, chair2, ourShader, camera, sampleShader, VAO, VBO, VAOBOX, VBOBOX, EBOBOX);
		/*glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);*/
		/*glBindVertexArray(VAO2);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);*/
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	cleanupPhysics(true);
	glfwTerminate();




	
	/*
	sCamera = new Snippets::Camera(PxVec3(50.0f, 50.0f, 50.0f), PxVec3(-0.6f, -0.2f, -0.7f));

	Snippets::setupDefaultWindow("PhysX Snippet HelloWorld");
	Snippets::setupDefaultRenderState();

	glutIdleFunc(idleCallback);
	glutDisplayFunc(renderCallback);
	glutKeyboardFunc(keyboardCallback);
	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
	motionCallback(0, 0);

	atexit(exitCallback);

	initPhysics(true);
	glutMainLoop();
	*/
}
#endif
