#pragma once

#include <iostream>
#include <chrono>

#include "FaceTracker.h"
#include "error.h"
#include "log.h"

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h> 
// WGLEW = Windows GL Extension Wrangler (change for different platform) 
// platform specific functions (in this case Windows)
#include <GL/wglew.h> 

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
#include <GLFW/glfw3.h>

// OpenGL math (and other additional GL libraries, at the end)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#pragma comment(lib, "opengl32.lib")



#include "Config.h"
#include "KeyMap.h"
#include "FPSCounter.h"
#include "Controls.h"
#include "Render.h"
#include "Game.h"

int infoUpdateTicks = 32;
int ticksAfterUpdate = 0;

int glbVsync = 1;
int glbFullScreen = 0;

GLFWwindow * window = NULL;


#define MICROSECONDS 1000000

#define SET_KEY_STATE(key){\
    if (action == GLFW_PRESS) {\
        Controls::keyPressed[key] = 1;\
    } else if (action == GLFW_RELEASE) {\
        Controls::keyPressed[key] = 0;\
    };\
}

#define STATUS_STRING(b) ((b) ? "enabled" : "disabled")



void reshapeCallback(GLFWwindow* window, int width, int height) {

    Render::width = width;
    Render::height = height;
    glViewport(0, 0, width, height);

}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    switch (key) {

    case KeyMap::FACE_TRACKING: {
        
        if (action == GLFW_PRESS) {
            FaceTracker::camEnabled = !FaceTracker::camEnabled;
            printf("Face tracking: %s;\n", STATUS_STRING(FaceTracker::camEnabled));
        }

        break;

    }

    case KeyMap::V_SYNC: {

        if (action == GLFW_PRESS) {
            glfwSwapInterval(glbVsync = !glbVsync);
            printf("V-Sync: %s;\n", STATUS_STRING(glbVsync));
        }

        break;

    }

    case KeyMap::ENABLE_MOUSE: {

        if (action == GLFW_PRESS) {
            Controls::mouseEnabled = !Controls::mouseEnabled;
            if (Controls::mouseEnabled) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            printf("Mouse x-axis input: %s;\n", STATUS_STRING(Controls::mouseEnabled));
        }

        break;

    }

    case KeyMap::FULL_SCREEN: {

        if (action == GLFW_PRESS) {

            glbFullScreen = !glbFullScreen;

            if (glbFullScreen) {

                GLFWmonitor* const monitor = glfwGetPrimaryMonitor();

                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                if (!mode) {
                    glbFullScreen = 0;
                    break;
                }

                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

            } else {
                glfwSetWindowMonitor(window, nullptr, 100, 100, Config::DF_SCREEN_WIDTH, Config::DF_SCREEN_HEIGHT, 0);
            }

            printf("Fullscreen: %s;\n", STATUS_STRING(glbFullScreen));

        }

        break;

    }

    case KeyMap::MOVE_FORWARD: {

        SET_KEY_STATE(KeyMap::MOVE_FORWARD);
        break;

    }

    case KeyMap::MOVE_BACK: {

        SET_KEY_STATE(KeyMap::MOVE_BACK);
        break;

    }

    case KeyMap::MOVE_LEFT: {

        SET_KEY_STATE(KeyMap::MOVE_LEFT);
        break;

    }

    case KeyMap::MOVE_RIGHT: {

        SET_KEY_STATE(KeyMap::MOVE_RIGHT);
        break;

    }

    case KeyMap::ROTATE_LEFT: {

        SET_KEY_STATE(KeyMap::ROTATE_LEFT);
        break;

    }

    case KeyMap::ROTATE_RIGHT: {

        SET_KEY_STATE(KeyMap::ROTATE_RIGHT);
        break;

    }

    case KeyMap::FIRE: {

        SET_KEY_STATE(KeyMap::FIRE);
        break;

    }

    case KeyMap::INTERACT: {

        SET_KEY_STATE(KeyMap::INTERACT);
        break;

    }

    case KeyMap::CAM_1: {

        SET_KEY_STATE(KeyMap::CAM_1);
        break;

    }

    case KeyMap::CAM_2: {

        SET_KEY_STATE(KeyMap::CAM_2);
        break;

    }

    case KeyMap::CAM_3: {

        SET_KEY_STATE(KeyMap::CAM_3);
        break;

    }

    }
}

void printGLInfo() {
    std::cout << "" << std::endl;
    std::cout << "" << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "" << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "" << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "" << "OpenGL Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "" << std::endl;
}

int main() {
    
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(800, 600, "OpenGL context", NULL, NULL);
    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK) return -1;
    wglewInit();

    // v-sync
    glfwSwapInterval(glbVsync);

    // mouse
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // init callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, reshapeCallback);

    // printGLInfo();

    if (Game::init()) exit(EXIT_FAILURE);
    if (Render::init()) exit(EXIT_FAILURE);

    Render::cam.target = Game::player;

    auto timeStart = std::chrono::high_resolution_clock::now();
    auto tickLength = (1 / (double) Game::tickRate) * MICROSECONDS;

    if (Controls::mouseEnabled) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    double lastMouseX = 0;
    double lastMouseY = 0;

    if (FaceTracker::init() < 0) return 1;
    std::thread faceTrackerThread (FaceTracker::run);

    printf("Everything seems to be successfully initialized...");
    printGLInfo();

    while (!glfwWindowShouldClose(window)) {

        FPSCounter::tick();

        auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - timeStart
        ).count();
        if (elapsedTime >= tickLength) {

            timeStart = std::chrono::high_resolution_clock::now();

            if (Controls::mouseEnabled) glfwGetCursorPos(window, &Controls::mouseX, &Controls::mouseY);

            if (ticksAfterUpdate >= infoUpdateTicks) {
                std::stringstream ss;
                ss << "FPS: " << FPSCounter::fps << "; Gear: " << Game::car.gear - 1;
                glfwSetWindowTitle(window, ss.str().c_str());
                ticksAfterUpdate = 0;
            }
            ticksAfterUpdate++;

            double tmpX = Controls::mouseX;
            double tmpY = Controls::mouseY;
            Controls::mouseX -= lastMouseX;
            Controls::mouseY -= lastMouseY;
            lastMouseX = tmpX;
            lastMouseY = tmpY;

            Game::update();
            //if (Controls::mouseEnabled) glfwSetCursorPos(window, Render::width / 2, Render::height / 2);

        }
        
        Render::render();

        glfwSwapBuffers(window);
		glfwPollEvents();

	}

    FaceTracker::camRunning = 0;
    faceTrackerThread.join();

    if (window) glfwDestroyWindow(window);
	glfwTerminate();

    return Err::OK;

}