//
//  BoundingSphere.hpp
//  Laborator 6
//
//  Created by Eric Toader on 13.01.2023.
//

#ifndef BoundingSphere_hpp
#define BoundingSphere_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <stdio.h>

#endif /* BoundingSphere_hpp */

namespace etoader {
	
	class BoundingSphere {
	public:
		
		BoundingSphere(glm::vec3 objectOrigin, float radius);
		
		bool isColliding(etoader::BoundingSphere other);
		
		void updateObjectPosition(glm::vec3 objectPosition);
		
	private:
		float radius;
		glm::vec3 objectPosition;
	};
	
}
