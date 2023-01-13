//
//  main.hpp
//  Laborator 6
//
//  Created by Eric Toader on 13.01.2023.
//

#ifndef main_h
#define main_h

#define POLYGON_FILL 0
#define POLYGON_LINE 1
#define POLYGON_POINT 2

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <OpenAL/OpenAL.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include "utils/Shader.hpp"
#include "utils/Model3D.hpp"
#include "camera/Camera.hpp"
#include "skybox/SkyBox.hpp"
#include "ship/PirateShip.hpp"
#include "collision/BoundingSphere.hpp"
#include "sound/SoundBuffer.h"
#include "sound/SoundDevice.h"
#include "sound/SoundSource.h"

#endif /* main_h */
