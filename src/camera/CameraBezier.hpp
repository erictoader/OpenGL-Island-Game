//
//  CameraBezier.hpp
//  Laborator 6
//
//  Created by Eric Toader on 15.01.2023.
//

#ifndef CameraBezier_hpp
#define CameraBezier_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#endif /* CameraBezier_hpp */

glm::vec3 Bezier3Points(float t, glm::vec3 P0, glm::vec3 P1, glm::vec3 P2);

glm::vec3 getNextPosition(float t, int pos);

bool isValidIndex(int pos);
