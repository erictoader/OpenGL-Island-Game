//
//  CameraBezier.cpp
//  Laborator 6
//
//  Created by Eric Toader on 15.01.2023.
//

#include "CameraBezier.hpp"

#include <vector>

std::vector<glm::vec3> points = {
	glm::vec3(-750.f, 20.f, 750.f),
	glm::vec3(-250.f, 20.f, 630.f),
	glm::vec3(-190.f, 30.f, 510.f),
	glm::vec3(-480.f, 50.f, 370.f),
	glm::vec3(-410.f, 50.f, 135.f),
	glm::vec3(-350.f, 50.f, -487.f),
	glm::vec3(360.f, 250.f, -300.f),
	glm::vec3(600.f, 100.f, 120.f),
	glm::vec3(430.f, 50.f, 340.f)
};

glm::vec3 Bezier3Points(float t, glm::vec3 P0, glm::vec3 P1, glm::vec3 P2) {
	return (1 - t) * (P0 * (1 - t) + P1 * t) + (P1 * (1 - t) + P2 * t) * t;
}

glm::vec3 getNextPosition(float t, int pos) {
	return Bezier3Points(t, points[pos], points[pos + 1], points[pos + 2]);
}

bool isValidIndex(int pos) {
	if (pos > points.size() - 3) {
		return false;
	}
	return true;
}


