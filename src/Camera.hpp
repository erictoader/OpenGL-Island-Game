#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <string>

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UPWARD, MOVE_DOWNWARD};
    
    class Camera
    {
    public:
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp, float cameraSpeed, float yaw, float pitch);

        glm::mat4 getViewMatrix();
		
        void move(MOVE_DIRECTION direction);

        void rotate(float deltaPitch, float deltaYaw);
        
    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
		float cameraSpeed;
		float yaw;
		float pitch;
    };
    
}

#endif /* Camera_hpp */
