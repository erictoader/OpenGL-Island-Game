//
//  BoundingSphere.cpp
//  Laborator 6
//
//  Created by Eric Toader on 13.01.2023.
//

#include "BoundingSphere.hpp"

namespace etoader {

BoundingSphere::BoundingSphere(glm::vec3 objectOrigin, float radius) {
	this->objectPosition = objectOrigin;
	this->radius = radius;
}

bool BoundingSphere::isColliding(BoundingSphere other) {
	if (glm::length(objectPosition - other.objectPosition) < radius + other.radius) return true;
	else return false;
}

void BoundingSphere::updateObjectPosition(glm::vec3 objectPosition) {
	this->objectPosition = objectPosition;
}

}
