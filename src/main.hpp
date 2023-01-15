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
#include <random>
#include <thread>

#include "utils/Shader.hpp"
#include "utils/Model3D.hpp"
#include "camera/Camera.hpp"
#include "skybox/SkyBox.hpp"
#include "animation/ship/PirateShip.hpp"
#include "animation/dragon/Dragon.hpp"
#include "collision/BoundingSphere.hpp"
#include "sound/SoundBuffer.h"
#include "sound/SoundDevice.h"
#include "sound/SoundSource.h"

#endif /* main_h */

const char APPNAME[] = "Island Explorer by Eric Toader";
const int glWindowWidth = 1920;
const int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

bool pressedKeys[1024];

int polygonMode = 0;
