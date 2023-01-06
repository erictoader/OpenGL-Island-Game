//
//  PirateShip.cpp
//  Laborator 6
//
//  Created by Eric Toader on 17.12.2022.
//

#include "PirateShip.hpp"

namespace pirate {

	PirateShip::PirateShip(glm::vec3 shipPosition, glm::vec3 shipTarget, float shipYaw, float shipSpeed) {
		this->shipPosition = shipPosition;
		this->shipTarget = shipTarget;
		this->shipFrontDirection = glm::normalize(shipTarget - shipPosition);
		this->shipYaw = shipYaw;
		this->shipSpeed = shipSpeed;
	}

	float PirateShip::getYawAngle() {
		return shipYaw;
	}

	glm::vec3 PirateShip::getPosition() {
		return shipPosition;
	}

	void PirateShip::move(MOVE_DIRECTION direction) {
		switch (direction) {
			case MOVE_FORWARD:
				shipPosition += shipFrontDirection * shipSpeed;
				break;
			case MOVE_BACKWARD:
				shipPosition -= shipFrontDirection * shipSpeed;
				break;
			default:
				break;
		}
	}

	void PirateShip::rotate(float deltaYaw) {
		shipYaw += deltaYaw;
		glm::vec3 front;
		front.x = glm::cos(glm::radians(shipYaw));
		front.z = glm::sin(glm::radians(shipYaw));
		shipFrontDirection = glm::normalize(front);
	}
}
