#ifndef Camera_hpp
#define Camera_hpp

#include <GLFW/glfw3.h>

#include <string>
#include <thread>

#include "CameraBezier.hpp"

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UPWARD, MOVE_DOWNWARD};

	enum CAMERA_MODE {FREE_LOOK, PRESENTING, DISPOSED};
    
    class Camera
    {
    public:
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp, float cameraSpeed, float yaw, float pitch);

        glm::mat4 getViewMatrix();
		
        void move(MOVE_DIRECTION direction);
		
		glm::vec3 getPosition();
		
		glm::vec3 getTarget();
		
		void toggleMode();

        void rotate(float deltaPitch, float deltaYaw);
        
		void dispose();
		
    private:
		
		CAMERA_MODE mode = FREE_LOOK;
		
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
		
		float cameraSpeed;
		float yaw;
		float pitch;
		
		std::thread presenterThread;
    };
    
}

#endif /* Camera_hpp */
