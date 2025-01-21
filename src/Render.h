#pragma once

#include "stdint.h"

#include "Camera.h"

namespace Render {

    enum BitDepth {
        BIT_DEPTH_8 = 1,
        BIT_DEPTH_24 = 4
    };

    struct Vec2f {
        float x;
        float y;
    };

    struct Vec3f {
        float x;
        float y;
        float z;
    };

    struct Vec4c {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
	
    struct Texture {

        uint32_t* pixels;
        int width;
        int height;
        unsigned int glId;

    };

    extern Camera cam;

    extern int width;
    extern int height;

    extern glm::vec4 fogColor;
    extern glm::vec3 ambientColor;

    extern glm::vec4 moonColor;
    extern glm::vec4 moonDiffuseColor;
    extern glm::vec3 moonPos;

    extern glm::vec4 frontLightColor;
    extern glm::vec4 frontLightDiffuseColor;

    Texture* loadTexture(char* const path, BitDepth depth = BIT_DEPTH_24);

	int init();
    int render();

    void genTexture(Texture* tex, unsigned int iformat, unsigned int eformat, unsigned int type);

}