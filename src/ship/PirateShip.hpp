//
//  PirateShip.hpp
//  Laborator 6
//
//  Created by Eric Toader on 17.12.2022.
//

#ifndef PirateShip_hpp
#define PirateShip_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <string>

#endif /* PirateShip_hpp */

namespace etoader {
	
	enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD};
	
	class PirateShip {
	public:
		
		PirateShip(glm::vec3 shipPosition, glm::vec3 shipTarget, float shipYaw, float shipSpeed);
		
		float getYawAngle();
		
		glm::vec3 getPosition();
		
		glm::vec3 getFrontDirection();
			
		void move(MOVE_DIRECTION direction);

		void rotate(float yaw);
		
	private:
		float shipYaw;
		float shipSpeed;
		glm::vec3 shipPosition;
		glm::vec3 shipTarget;
		glm::vec3 shipFrontDirection;
		glm::vec3 shipUpDirection;
	};
	
}
