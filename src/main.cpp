//
//  main.cpp
//  OpenGL_Shader_Example_step1
//
//  Created by CGIS on 02/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "Shader.hpp"
#include "PirateShip.hpp"
#include "SkyBox.hpp"
#include "Camera.hpp"
#include "utils/Model3D.hpp"

const char APPNAME[] = "Island Explorer by Eric Toader";
const int glWindowWidth = 1920;
const int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

gps::Camera myCamera(glm::vec3(0.0f, 20.0f, 100.0f), glm::vec3(0.0f, 20.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f), 5.0f, 0.0f, 0.0f);

pirate::PirateShip myShip(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.1f);

bool pressedKeys[1024];
float angle = 0.0f;

gps::Model3D shipObject;
bool controllingShip = false;

gps::Model3D sceneObject;

gps::Model3D waterfallObject[15];
int currentWaterfall = 0;
bool reachedWaterfallEnd = false;
double lastGetTime = 0.0;

gps::Shader myCustomShader;
gps::Shader skyboxShader;
gps::SkyBox mySkyBox;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 lightDir = glm::vec3(1.0f, 1.0f, 0.0f);

glm::mat3 normalMatrix;

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	glfwSetWindowSize(window, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

double prevX = 0;
double prevY = 0;
float sensitivity = 0.15f;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	double deltaX = xpos - prevX;
	double deltaY = prevY - ypos;
	prevY = ypos;
	prevX = xpos;

	float deltaYaw = deltaX * sensitivity;
	float deltaPitch = deltaY * sensitivity;
	
	myCamera.rotate(deltaPitch, deltaYaw);
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 0.1f;
	}
	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 0.1f;
	}
	if (pressedKeys[GLFW_KEY_F]) {
		controllingShip = !controllingShip;
	}
	if (pressedKeys[GLFW_KEY_W]) {
		if (controllingShip) myShip.move(pirate::MOVE_FORWARD);
		else myCamera.move(gps::MOVE_FORWARD);
	}
	if (pressedKeys[GLFW_KEY_S]) {
		if (controllingShip) myShip.move(pirate::MOVE_BACKWARD);
		else myCamera.move(gps::MOVE_BACKWARD);
	}
	if (pressedKeys[GLFW_KEY_A]) {
		if (controllingShip) myShip.rotate(0.01f);
		else myCamera.move(gps::MOVE_LEFT);
	}
	if (pressedKeys[GLFW_KEY_D]) {
		if (controllingShip) myShip.rotate(-0.01f);
		else myCamera.move(gps::MOVE_RIGHT);
	}
	if (pressedKeys[GLFW_KEY_SPACE]) {
		myCamera.move(gps::MOVE_UPWARD);
	}
	if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
		myCamera.move(gps::MOVE_DOWNWARD);
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, APPNAME, NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}
	
	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	glfwMakeContextCurrent(glWindow);
	
	glfwSwapInterval(1);
	
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
	
	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
	
	return true;
}

void drawWaterfallModel() {
	double currentGetTime = glfwGetTime();
	// 60 fps capping
	if (currentGetTime - lastGetTime > 0.016) {
		if (!reachedWaterfallEnd) {
			currentWaterfall++;
			if (currentWaterfall == 14) reachedWaterfallEnd = true;
		} else {
			currentWaterfall--;
			if (currentWaterfall == 0) reachedWaterfallEnd = false;
		}
		lastGetTime = currentGetTime;
	}
	model = glm::mat4(1.0f);
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	waterfallObject[currentWaterfall].Draw(myCustomShader);
}

void renderScene() {
	glClearColor(0.8, 0.8, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glViewport(0, 0, retina_width, retina_height);
	
	myCustomShader.useShaderProgram();
	
	// World
	model = glm::mat4(1.0f);
	model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	view = myCamera.getViewMatrix();
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
	glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	sceneObject.Draw(myCustomShader);
	
	// Pirate ship
	model = glm::mat4(1.0f);
	model = glm::translate(model, myShip.getPosition());
	model = glm::rotate(model, myShip.getYawAngle(), glm::vec3(0, 1, 0));
	view = myCamera.getViewMatrix();
	
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	shipObject.Draw(myCustomShader);
	
	// Waterfalls
	drawWaterfallModel();
	
	// Skybox
	skyboxShader.useShaderProgram();
	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"	// Added due to sprintf
void initObjects() {
	sceneObject.LoadModel("objects/scene.obj", "textures/");
	shipObject.LoadModel("objects/ship.obj", "textures/");
	for(int i = 0; i < 15; i++) {
		char objLocation[40];
		sprintf(objLocation, "objects/waterfallObj/waterfall%d.obj", i+1);
		waterfallObject[i].LoadModel(objLocation, "textures/waterfallMtl/");
	}
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	
	model = glm::mat4(1.0f);
	view = myCamera.getViewMatrix();
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1500.0f);
}

void initSkybox() {
	std::vector<const GLchar*> faces;
	faces.push_back("textures/skybox/right.tga");
	faces.push_back("textures/skybox/left.tga");
	faces.push_back("textures/skybox/top.tga");
	faces.push_back("textures/skybox/bottom.tga");
	faces.push_back("textures/skybox/back.tga");
	faces.push_back("textures/skybox/front.tga");
	
	mySkyBox.Load(faces);
}

int main(int argc, const char * argv[]) {
	
	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}
	
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
	
	initObjects();
	initShaders();
	initSkybox();
			
	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();
		
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}
	
	cleanup();
	return 0;
}
