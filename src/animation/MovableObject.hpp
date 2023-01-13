//
//  MovableObject.hpp
//  Laborator 6
//
//  Created by Eric Toader on 13.01.2023.
//
#pragma once
#ifndef MovableObject_hpp
#define MovableObject_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <stdio.h>

#endif /* MovableObject_hpp */

namespace etoader {
	
	enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD};
	
	class MovableObject {
	public:
		
		explicit MovableObject(glm::vec3 position, glm::vec3 target, float yaw, float speed);
		
		float getYawAngle();
		
		glm::vec3 getPosition();
		
		glm::vec3 getFrontDirection();
			
		void move(MOVE_DIRECTION direction);

		void rotate(float deltaYaw);
		
	protected:
		float yaw;
		float speed;
		glm::vec3 position;
		glm::vec3 target;
		glm::vec3 frontDirection;
		glm::vec3 upDirection;
	};
	
}
