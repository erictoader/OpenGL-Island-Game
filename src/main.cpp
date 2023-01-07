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

gps::Model3D sunObject;
gps::Model3D moonObject;

gps::Model3D waterfallObject[15];
int currentWaterfall = 0;
bool reachedWaterfallEnd = false;
double lastGetTime = 0.0;

gps::Shader myCustomShader;
gps::Shader skyboxShader;
gps::SkyBox mySkyBox;
float dayNightOffset = 0.0f;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

glm::vec3 sunLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 sunLightDir = glm::vec3(1.0f, 1.0f, 0.0f);

glm::mat3 normalMatrix;

gps::Model3D screenQuad;
gps::Shader screenQuadShader;
GLuint shadowMapFBO;
GLuint depthMapTexture;
gps::Shader depthMapShader;
bool showDepthMap;
const int SHADOW_WIDTH = 1920;
const int SHADOW_HEIGHT = 1080;

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	glfwSetWindowSize(window, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;
	
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

void dayNightCycle() {
//	dayNightOffset += 0.001;
//	model = glm::mat4(1.0f);
//	model = glm::rotate(model, dayNightOffset, glm::vec3(1, 0, 0));
//	sunLightDir = glm::normalize(glm::vec3(model * glm::vec4(sunLightDir, 1.0f)));
}

void selectNextWaterfall() {
	if (!reachedWaterfallEnd) {
		currentWaterfall++;
		if (currentWaterfall == 14) reachedWaterfallEnd = true;
	} else {
		currentWaterfall--;
		if (currentWaterfall == 0) reachedWaterfallEnd = false;
	}
}

void frameEvent() {
	double currentGetTime = glfwGetTime();
	if (currentGetTime - lastGetTime > 1.0/60.0) {
		dayNightCycle();
		selectNextWaterfall();
		lastGetTime = currentGetTime;
	}
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(sunLightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	const GLfloat near_plane = 0.1f, far_plane = 5.0f;
	glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void drawDepthMap() {
	glViewport(0, 0, retina_width, retina_height);

	glClear(GL_COLOR_BUFFER_BIT);

	screenQuadShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

	glDisable(GL_DEPTH_TEST);
	screenQuad.Draw(screenQuadShader);
	glEnable(GL_DEPTH_TEST);
}

void drawObjects(gps::Shader shader, bool depthPass) {
	glViewport(0, 0, retina_width, retina_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	shader.useShaderProgram();
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	
	// World
	model = glm::mat4(1.0f);
	model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	view = myCamera.getViewMatrix();
	
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "sunLightDir"), 1, glm::value_ptr(sunLightDir));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "sunLightColor"), 1, glm::value_ptr(sunLightColor));
	glUniform1ui(glGetUniformLocation(myCustomShader.shaderProgram, "astralObject"), 0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	
	sceneObject.Draw(myCustomShader);
	
	// Pirate ship
	model = glm::mat4(1.0f);
	model = glm::translate(model, myShip.getPosition());
	model = glm::rotate(model, myShip.getYawAngle(), glm::vec3(0, 1, 0));
	view = myCamera.getViewMatrix();
	
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	shipObject.Draw(myCustomShader);
	
	// Waterfalls
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	waterfallObject[currentWaterfall].Draw(myCustomShader);
	
	// Sun
	model = glm::mat4(1.0f);
	model = glm::rotate(model, dayNightOffset, glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1ui(glGetUniformLocation(myCustomShader.shaderProgram, "astralObject"), 1);
	sunObject.Draw(myCustomShader);
	
	// Moon
	model = glm::mat4(1.0f);
	model = glm::rotate(model, dayNightOffset, glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	moonObject.Draw(myCustomShader);
}

void drawSkybox() {
	skyboxShader.useShaderProgram();
	model = glm::mat4(1.0f);
	model = glm::rotate(model, dayNightOffset, glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	mySkyBox.Draw(skyboxShader, view, projection);
}

void renderScene() {
	frameEvent();
	
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	if (showDepthMap) {
		drawDepthMap();
	} else {
		drawObjects(myCustomShader, false);
		drawSkybox();
	}
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
	sunObject.LoadModel("objects/sun.obj", "textures/");
	moonObject.LoadModel("objects/moon.obj", "textures/");
	screenQuad.LoadModel("objects/quad.obj");
	for(int i = 0; i < 15; i++) {
		char objLocation[40];
		sprintf(objLocation, "objects/waterfallObj/waterfall%d.obj", i+1);
		waterfallObject[i].LoadModel(objLocation, "textures/waterfallMtl/");
	}
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
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

void initShadowMap() {
	glGenFramebuffers(1, &shadowMapFBO);
	
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	initShadowMap();
			
	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();
		
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}
	
	cleanup();
	return 0;
}
