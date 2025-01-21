#pragma once

#include "GL/glew.h"
#include "GL/wglew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#define SHADER_FOLDER "C:/Users/fsfds/OneDrive/Plocha/uni II/ICP/app/resorces/shaders/"

namespace ShaderDriver {

    struct Shader {

        static int count;
        static int maxCount;

        unsigned int fragmentShader;
        unsigned int vertexShader;
        unsigned int tessellationControlShader;
        unsigned int tessellationEvaluationShader;

    };

    struct BasicShader {

        unsigned int shaderProgram;

        unsigned int projection;
        unsigned int view;
        unsigned int model;
        unsigned int worldPos;
        unsigned int plPos;
        unsigned int fogColor;
        unsigned int ambientColor;
        unsigned int moonColor;
        unsigned int moonDiffuseColor;
        unsigned int moonPos;
        unsigned int frontLightPos;
        unsigned int frontLightDir;
        unsigned int frontLightColor;
        unsigned int frontLightDiffuseColor;

    };

    struct TerrainShader {

        unsigned int shaderProgram;

        unsigned int projection;
        unsigned int view;
        unsigned int model;
        unsigned int etex;
        unsigned int ctex;
        unsigned int plPos;
        unsigned int fogColor;
        unsigned int ambientColor;
        unsigned int moonColor;
        unsigned int moonDiffuseColor;
        unsigned int moonPos;
        unsigned int frontLightPos;
        unsigned int frontLightDir;
        unsigned int frontLightColor;
        unsigned int frontLightDiffuseColor;
        
    };

    struct SpriteShader {

        unsigned int shaderProgram;

        unsigned int projection;
        unsigned int view;
        unsigned int model;
        unsigned int center;
        unsigned int texShiftX;
        unsigned int texShiftY;

    };

    enum ShaderType {

        SHDR_TERRAIN    = 0,
        SHDR_BASIC  = 1,
        SHDR_SPRITE = 2,
        SHDR_COUNT  = 5,

    };

    extern Shader* shaders;

    int load(char* filename, GLenum type, GLuint* outShader);
    int compile(char** src, GLenum type, GLuint* outShader);

    int loadAll();

}
