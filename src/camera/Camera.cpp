#include "Camera.hpp"

namespace gps {

Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp, float cameraSpeed, float yaw, float pitch) {
	this->cameraPosition = cameraPosition;
	this->cameraTarget = cameraTarget;
	this->cameraUpDirection = cameraUp;
	this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
	this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
	this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
	this->cameraSpeed = cameraSpeed;
	this->yaw = yaw;
	this->pitch = pitch;
}

glm::mat4 Camera::getViewMatrix() {
	return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
}

void Camera::move(MOVE_DIRECTION direction) {
	if (mode != FREE_LOOK) return;
	switch (direction) {
		case MOVE_LEFT:
			cameraPosition -= cameraRightDirection * cameraSpeed;
			break;
		case MOVE_RIGHT:
			cameraPosition += cameraRightDirection * cameraSpeed;
			break;
		case MOVE_FORWARD:
			cameraPosition += cameraFrontDirection * cameraSpeed;
			break;
		case MOVE_BACKWARD:
			cameraPosition -= cameraFrontDirection * cameraSpeed;
			break;
		case MOVE_UPWARD:
			cameraPosition += cameraUpDirection * cameraSpeed;
			break;
		case MOVE_DOWNWARD:
			cameraPosition -= cameraUpDirection * cameraSpeed;
			break;
		default:
			break;
	}
}

glm::vec3 Camera::getPosition() {
	return cameraPosition;
}

glm::vec3 Camera::getTarget() {
	return cameraFrontDirection;
}

void startPresenting(CAMERA_MODE &mode, glm::vec3 &cameraPosition, glm::vec3 &cameraFrontDirection, double lastGetTime) {
	
	float t = 0.0f;
	int pos = 0;
	
	while (mode == PRESENTING) {
	
		if (t >= 1.0f) {
			pos += 2;
			t = 0.0f;
		}
		
		if (!isValidIndex(pos)) break;
		
		double currentGetTime = glfwGetTime();
		if (currentGetTime - lastGetTime > 1.0/60.0) {
			
			glm::vec3 nextPosition = getNextPosition(t, pos);
			cameraFrontDirection = nextPosition - cameraPosition;
			cameraPosition = nextPosition;
			
			t += 0.001;
			
			lastGetTime = currentGetTime;
		}
	}
	
	mode = FREE_LOOK;
}

void Camera::toggleMode() {
	
	if (mode == FREE_LOOK) {
		
		if (presenterThread.joinable()) {
			presenterThread.join();
		}
		
		mode = PRESENTING;
		presenterThread = std::thread(startPresenting, std::ref(mode), std::ref(cameraPosition), std::ref(cameraFrontDirection), 0.0);
	} else {
		mode = FREE_LOOK;
		presenterThread.join();
	}
}

//yaw - camera rotation around the y axis
//pitch - camera rotation around the x axis
void Camera::rotate(float deltaPitch, float deltaYaw) {
	if (mode != FREE_LOOK) return;
	
	pitch += deltaPitch;
	yaw += deltaYaw;
	
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;
	
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	
	cameraFrontDirection = glm::normalize(front);
	cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
	
}

void Camera::dispose() {
	
	mode = DISPOSED;
	
	if (presenterThread.joinable()) {
		
		presenterThread.join();

	}
}
}
