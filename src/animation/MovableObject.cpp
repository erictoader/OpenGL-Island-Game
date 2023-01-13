//
//  MovableObject.cpp
//  Laborator 6
//
//  Created by Eric Toader on 13.01.2023.
//

#include "MovableObject.hpp"

namespace etoader {

MovableObject::MovableObject(glm::vec3 position, glm::vec3 target, float yaw, float speed) {
	this->position = position;
	this->target = target;
	this->frontDirection = glm::normalize(target - position);
	this->yaw = yaw;
	this->speed = speed;
}

float MovableObject::getYawAngle() {
	return yaw;
}

glm::vec3 MovableObject::getPosition() {
	return position;
}

glm::vec3 MovableObject::getFrontDirection() {
	return frontDirection;
}

void MovableObject::move(MOVE_DIRECTION direction) {
	switch (direction) {
		case MOVE_FORWARD:
			position += frontDirection * speed;
			break;
		case MOVE_BACKWARD:
			position -= frontDirection * speed;
			break;
		default:
			break;
	}
}

void MovableObject::rotate(float deltaYaw) {
	yaw += deltaYaw;
	frontDirection.x = glm::cos(yaw);
	frontDirection.z = -glm::sin(yaw);
}

}
