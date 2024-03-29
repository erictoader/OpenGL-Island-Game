#include "main.hpp"

// Camera/player object
gps::Camera camera(glm::vec3(0.0f, 20.0f, 100.0f), glm::vec3(0.0f, 20.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f), 3.0f, 0.0f, 0.0f);

// Movable objects
etoader::PirateShip ship(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, 3.0f);
etoader::Dragon dragon(glm::vec3(-500.0f, 300.0f, -300.0f), glm::vec3(1.0f, 300.0f, 0.0f), 0.0f, 0.8f);

// Bounding spheres
etoader::BoundingSphere shipBounding(glm::vec3(0, 0, 0), 40.0f);
etoader::BoundingSphere pirateHouseBounding(glm::vec3(480, 0, 360), 140.0f);
etoader::BoundingSphere villageHouseBounding(glm::vec3(-410, 0, 290), 245.0f);
etoader::BoundingSphere pyramidBounding(glm::vec3(230, 0, -460), 220.0f);
etoader::BoundingSphere pyramidShoreBounding(glm::vec3(-120, 0, -580), 130.0f);

// Random generator
std::random_device dev;
std::mt19937 rng(dev());

// 3D Model Objects
gps::Model3D shipObject;
gps::Model3D shipWheel;
gps::Model3D shipWindows;
bool controllingShip = false;

gps::Model3D dragonBody;
gps::Model3D dragonWingLeft;
gps::Model3D dragonWingRight;
std::uniform_int_distribution<int> dragonAngles(-1, 1);
int lastDragonRotate;
float dragonWingsAngle = 15.f;
bool dragonWingsAscending = false;

gps::Model3D sceneObject;
gps::Model3D villageWindows;

gps::Model3D sunObject;
gps::Model3D moonObject;

gps::Model3D waterfallObject[15];
int currentWaterfall = 0;
bool reachedWaterfallEnd = false;
double lastGetTime = 0.0;

gps::Shader myCustomShader;

// Skybox
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

// Day - Night cycle
float dayNightOffset = 0.0f;
float dayNightAccelerator = 1.0f;
float dayNightDelta = 0.0f;
bool dayNightAccelerating = false;
float dayNightAngle;
float dayFactor;

// Uniform information
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

glm::vec3 lightColor = glm::vec3(0.986f, 1.0f, 0.709f);
glm::vec3 lightDir = glm::vec3(0.0f, 500.0f, 0.0f);
glm::vec3 moonLightColor = glm::vec3(0.454f, 0.925f, 0.940f);
glm::vec3 moonLightDir = glm::vec3(0.0f, -500.0f, 0.0f);

// Shadows
const unsigned int SHADOW_WIDTH = 16384;
const unsigned int SHADOW_HEIGHT = 16384;
gps::Model3D screenQuad;
gps::Shader screenQuadShader;
GLuint shadowMapFBO;
GLuint depthMapTexture;
gps::Shader depthMapShader;
bool showDepthMap;

// Audio player
SoundDevice* mysounddevice = SoundDevice::get();
SoundBuffer* soundBuffer = SoundBuffer::get();
bool shouldStop = false;
bool muteAudio = false;

ALuint seaSound;
std::thread seaPlayerThread;

// Mouse callback data
double prevX = 0;
double prevY = 0;
float sensitivity = 0.15f;

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	glfwSetWindowSize(window, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		camera.toggleMode();
	
	if (pressedKeys[GLFW_KEY_LEFT_CONTROL] && key == GLFW_KEY_M && action == GLFW_PRESS)
		muteAudio = !muteAudio;
	
	if (!pressedKeys[GLFW_KEY_LEFT_CONTROL] && key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;
	
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		controllingShip = !controllingShip;
	
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		dayNightAccelerator = 10.0f;
		dayNightAccelerating = true;
	}
	if (key == GLFW_KEY_T && action == GLFW_RELEASE) {
		dayNightAccelerator = 1.0f;
		dayNightAccelerating = false;
		dayNightOffset = dayNightAngle;
		dayNightDelta = 0.0f;
	}
	
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
		polygonMode = (polygonMode + 1) % 3;
		switch(polygonMode) {
			case POLYGON_FILL:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;
			case POLYGON_LINE:
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				break;
			case POLYGON_POINT:
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				break;
		}
	}
	
	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	double deltaX = xpos - prevX;
	double deltaY = prevY - ypos;
	prevY = ypos;
	prevX = xpos;
	
	float deltaYaw = deltaX * sensitivity;
	float deltaPitch = deltaY * sensitivity;
	
	camera.rotate(deltaPitch, deltaYaw);
}

bool isOutOfBounds() {
	if (ship.getPosition().x > 720.f || ship.getPosition().x < -720.f) return true;
	if (ship.getPosition().z > 720.f || ship.getPosition().z < -720.f) return true;
	return false;
}

bool isLegalMove(etoader::MOVE_DIRECTION direction) {
	
	// ship move for calculation
	ship.move(direction);
	shipBounding.updateObjectPosition(ship.getPosition());
	bool legal = true;
	
	if (shipBounding.isColliding(pirateHouseBounding)) {
		legal = false;
		goto doneEvaluating;
	} else if (shipBounding.isColliding(villageHouseBounding)) {
		legal = false;
		goto doneEvaluating;
	} else if (shipBounding.isColliding(pyramidBounding)) {
		legal = false;
		goto doneEvaluating;
	} else if (shipBounding.isColliding(pyramidShoreBounding)) {
		legal = false;
		goto doneEvaluating;
	}
	
	if (isOutOfBounds()) legal = false;
	goto doneEvaluating;
	
	// undo ship move
	doneEvaluating:
	if (direction == etoader::MOVE_BACKWARD) ship.move(etoader::MOVE_FORWARD);
	else ship.move(etoader::MOVE_BACKWARD);
	shipBounding.updateObjectPosition(ship.getPosition());
	
	return legal;
}

void processMovement() {
	
	if (pressedKeys[GLFW_KEY_W]) {
		if (controllingShip) {
			if (isLegalMove(etoader::MOVE_FORWARD)) ship.move(etoader::MOVE_FORWARD);
		} else camera.move(gps::MOVE_FORWARD);
	}
	if (pressedKeys[GLFW_KEY_S]) {
		if (controllingShip) {
			if (isLegalMove(etoader::MOVE_BACKWARD)) ship.move(etoader::MOVE_BACKWARD);
		} else camera.move(gps::MOVE_BACKWARD);
	}
	if (pressedKeys[GLFW_KEY_A]) {
		if (controllingShip) ship.rotate(0.01f);
		else camera.move(gps::MOVE_LEFT);
	}
	if (pressedKeys[GLFW_KEY_D]) {
		if (controllingShip) ship.rotate(-0.01f);
		else camera.move(gps::MOVE_RIGHT);
	}
	
	if (pressedKeys[GLFW_KEY_SPACE]) {
		camera.move(gps::MOVE_UPWARD);
	}
	if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
		camera.move(gps::MOVE_DOWNWARD);
	}
}

bool initOpenGLWindow() {
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
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

void initOpenGLState() {
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glViewport(0, 0, retina_width, retina_height);
	
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
	
	glEnable(GL_FRAMEBUFFER_SRGB);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void dayNightCycle() {
	
	if (dayNightAccelerating) dayNightAccelerator += 0.1f;
	
	if (dayNightAccelerating) {
		dayNightDelta += 0.1;
	} else {
		dayNightOffset += 0.1;
	}
	
	dayNightAngle = dayNightOffset + dayNightAccelerator * dayNightDelta;
	lightDir = glm::vec3(0.0f, 500.0f * glm::cos(glm::radians(dayNightAngle)), 500.0f * glm::sin(glm::radians(dayNightAngle)));
	moonLightDir = glm::vec3(0.0f, -500.0f * glm::cos(glm::radians(dayNightAngle)), -500.0f * glm::sin(glm::radians(dayNightAngle)));
	
	int days = dayNightAngle / 360;
	float dayNightAngleN = dayNightAngle - days * 360.0f;
	
	if (dayNightAngleN > 90.0f && dayNightAngleN <= 100.0f) {
		// sunset
		dayFactor = (100.0f - dayNightAngleN) / 10.0f;
		
	} else if (dayNightAngleN > 100.0f && dayNightAngleN <= 270.0f) {
		// night
		dayFactor = 0.0f;
		
	} else if (dayNightAngleN > 270.0f && dayNightAngleN <= 280.0f) {
		// sunrise
		dayFactor = (dayNightAngleN - 270.0f) / 10.0f;
		
	} else {
		// noon
		dayFactor = 1.0f;
	}
}

void moveDragon() {
	int currentRotate;
	
	// turn around
	if (dragon.getPosition().x > 730) {
		currentRotate = -2;
		goto commit;

	} else if (dragon.getPosition().x < -730) {
		currentRotate = -2;
		goto commit;
		
	} else if (dragon.getPosition().z > 730) {
		currentRotate = -2;
		goto commit;
		
	} else if (dragon.getPosition().z < -730) {
		currentRotate = -2;
		goto commit;
	}
	
	currentRotate = dragonAngles(rng);
	goto commit;
	
	commit:
	dragon.move(etoader::MOVE_FORWARD);
	dragon.rotate(0.005f * currentRotate);
	lastDragonRotate = currentRotate;
}

void moveDragonWings() {
	if (dragonWingsAscending) {
		dragonWingsAngle += 0.5f;
	} else {
		dragonWingsAngle -= 0.5f;
	}
	
	if (dragonWingsAngle > 15.f) {
		dragonWingsAscending = false;
	}
	
	if (dragonWingsAngle < -15.f) {
		dragonWingsAscending = true;
	}
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
		moveDragon();
		moveDragonWings();
		
		selectNextWaterfall();
		lastGetTime = currentGetTime;
	}
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	const GLfloat near_plane = 0.1f, far_plane = 2000.0f;
	glm::mat4 lightProjection = glm::ortho(-750.0f, 750.0f, -750.0f, 750.0f, near_plane, far_plane);
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

void drawWaterfalls() {
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	waterfallObject[currentWaterfall].Draw(myCustomShader);
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -750.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-750.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	waterfallObject[currentWaterfall].Draw(myCustomShader);
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-750.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-750.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	waterfallObject[currentWaterfall].Draw(myCustomShader);
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 750.0f));
	model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-750.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	waterfallObject[currentWaterfall].Draw(myCustomShader);
}

void drawObjects(gps::Shader shader, bool depthPass) {
	
	shader.useShaderProgram();

	glUniform1f(glGetUniformLocation(shader.shaderProgram, "dayFactor"), dayFactor);
	
	// World
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1ui(glGetUniformLocation(shader.shaderProgram, "astralObject"), 0);
	
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	sceneObject.Draw(shader);
	
	// Pirate ship
	model = glm::mat4(1.0f);
	model = glm::translate(model, ship.getPosition());
	model = glm::rotate(model, ship.getYawAngle(), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	shipBounding.updateObjectPosition(ship.getPosition());
	shipObject.Draw(shader);
	
	// Pirate ship wheel
	model = glm::mat4(1.0f);
	model = glm::translate(model, ship.getPosition());
	model = glm::rotate(model, ship.getYawAngle(), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-15.2f, +9.8f, +0.2f));
	model = glm::rotate(model, 10 * -ship.getYawAngle(), glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(15.2f, -9.8f, -0.2f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	shipWheel.Draw(shader);
	
	// Dragon
	model = glm::mat4(1.0f);
	model = glm::translate(model, dragon.getPosition());
	model = glm::rotate(model, dragon.getYawAngle(), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(90.f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	dragonBody.Draw(shader);
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, dragon.getPosition());
	model = glm::rotate(model, dragon.getYawAngle(), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(90.f), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-7.f, 10.f, 40.f));
	model = glm::rotate(model, glm::radians(-dragonWingsAngle), glm::vec3(0, 0, 1));
	model = glm::translate(model, glm::vec3(7.f, -10.f, -40.f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	dragonWingLeft.Draw(shader);
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, dragon.getPosition());
	model = glm::rotate(model, dragon.getYawAngle(), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(90.f), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(7.f, 10.f, 40.f));
	model = glm::rotate(model, glm::radians(dragonWingsAngle), glm::vec3(0, 0, 1));
	model = glm::translate(model, glm::vec3(-7.f, -10.f, -40.f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	dragonWingRight.Draw(shader);
	
	// Windows
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	villageWindows.Draw(shader);
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, ship.getPosition());
	model = glm::rotate(model, ship.getYawAngle(), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	shipWindows.Draw(shader);
	
	// Waterfalls
	drawWaterfalls();
	
	// Sun
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(dayNightAngle), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1ui(glGetUniformLocation(myCustomShader.shaderProgram, "astralObject"), 1);
	sunObject.Draw(myCustomShader);
	
	// Moon
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(dayNightAngle), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	moonObject.Draw(myCustomShader);
}

void drawSkybox() {
	skyboxShader.useShaderProgram();
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(dayNightAngle), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	mySkyBox.Draw(skyboxShader, view, projection);
}

void renderScene() {
	frameEvent();
	
	depthMapShader.useShaderProgram();
	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
					   1,
					   GL_FALSE,
					   glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// render depth map on screen - toggled with the M key
	
	if (showDepthMap) {
		drawDepthMap();
	} else {
		glViewport(0, 0, retina_width, retina_height);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		myCustomShader.useShaderProgram();
		
		view = camera.getViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		
		glm::mat4 lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightDir"), 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
		glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "moonLightDir"), 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * moonLightDir));
		
		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
		
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
						   1,
						   GL_FALSE,
						   glm::value_ptr(computeLightSpaceTrMatrix()));
		
		drawObjects(myCustomShader, false);
		drawSkybox();
	}
}

void cleanup() {
	shouldStop = true;
	seaPlayerThread.join();
	camera.dispose();
	
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"	// Added due to sprintf
void initObjects() {	
	sceneObject.LoadModel("res/objects/scene.obj", "res/mtl/");
	villageWindows.LoadModel("res/objects/villagewindows.obj", "res/mtl/");
	
	shipObject.LoadModel("res/objects/shipObj/ship.obj", "res/mtl/shipMtl/");
	shipWheel.LoadModel("res/objects/shipObj/shipwheel.obj", "res/mtl/shipMtl/");
	shipWindows.LoadModel("res/objects/shipObj/shipwindows.obj", "res/mtl/shipMtl/");
	
	dragonBody.LoadModel("res/objects/dragonObj/dragon_body.obj", "res/mtl/dragonMtl/");
	dragonWingLeft.LoadModel("res/objects/dragonObj/dragon_wing_left.obj", "res/mtl/dragonMtl/");
	dragonWingRight.LoadModel("res/objects/dragonObj/dragon_wing_right.obj", "res/mtl/dragonMtl/");
	
	sunObject.LoadModel("res/objects/sun.obj", "res/mtl/");
	moonObject.LoadModel("res/objects/moon.obj", "res/mtl/");
	
	screenQuad.LoadModel("res/objects/quad.obj");
	
	for(int i = 0; i < 15; i++) {
		char objLocation[60];
		sprintf(objLocation, "res/objects/waterfallObj/waterfall%d.obj", i+1);
		waterfallObject[i].LoadModel(objLocation, "res/mtl/waterfallMtl/");
	}
}

void initShaders() {
	myCustomShader.loadShader("src/shaders/shaderStart.vert", "src/shaders/shaderStart.frag");
	skyboxShader.loadShader("src/shaders/skyboxShader.vert", "src/shaders/skyboxShader.frag");
	screenQuadShader.loadShader("src/shaders/screenQuad.vert", "src/shaders/screenQuad.frag");
	depthMapShader.loadShader("src/shaders/depthMapShader.vert", "src/shaders/depthMapShader.frag");
}

void initUniforms() {
	myCustomShader.useShaderProgram();
	
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	view = camera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix3fv(glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 2000.0f);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	//set the light direction (direction towards the light)
	glm::mat4 lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightDir"), 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "moonLightDir"), 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * moonLightDir));
	
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos1"), 1, glm::value_ptr(glm::vec3(-400.f, 45.f, 252.f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos2"), 1, glm::value_ptr(glm::vec3(-360.f, 45.f, 304.5f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos3"), 1, glm::value_ptr(glm::vec3(-391.f, 47.f, 340.f)));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos4"), 1, glm::value_ptr(glm::vec3(-350.f, 48.f, 409.f)));
	
	//set light color
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "moonLightColor"), 1, glm::value_ptr(moonLightColor));
}

void initSkybox() {
	std::vector<const GLchar*> faces;
	faces.push_back("res/textures/skybox/right.tga");
	faces.push_back("res/textures/skybox/left.tga");
	faces.push_back("res/textures/skybox/top.tga");
	faces.push_back("res/textures/skybox/bottom.tga");
	faces.push_back("res/textures/skybox/back.tga");
	faces.push_back("res/textures/skybox/front.tga");
	
	mySkyBox.Load(faces);
}

void initShadowMap() {
	glGenFramebuffers(1, &shadowMapFBO);
	
	
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
						   0);
	
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void playSea(SoundSource &speaker) {
	seaSound = soundBuffer->addSoundEffect("res/sounds/sea.mp3");
	speaker.Play(seaSound, shouldStop);
	
	soundBuffer->removeSoundEffect(seaSound);
}

void adjustSeaVolume(glm::vec3 position, SoundSource &seaSpeaker) {
	float volume = 0.f;
	if (position.y < 250.f)
		volume = muteAudio ? 0.f : (250.f - position.y) / 250.f;
	
	seaSpeaker.setVolume(volume);
}


int main(int argc, const char * argv[]) {
	
	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}
	
	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initSkybox();
	initShadowMap();
	
	SoundSource seaSpeaker;
	seaPlayerThread = std::thread(playSea, std::ref(seaSpeaker));
	
	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();
		adjustSeaVolume(camera.getPosition(), seaSpeaker);
		
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}
	
	cleanup();
	return 0;
}
