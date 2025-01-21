#pragma once

#include "ShaderDriver.h"

#include <stdio.h>
#include <stdlib.h>

namespace ShaderDriver {

    int Shader::count = 0;
    int Shader::maxCount = 0;

    // vertex, frag, etc.
    char* shaderNames[] {
        SHADER_FOLDER "terrainVertShader.GLSL",
        SHADER_FOLDER "terrainFragShader.GLSL",
        SHADER_FOLDER "terrainTescShader.GLSL",
        SHADER_FOLDER "terrainTeseShader.GLSL",
        SHADER_FOLDER "basicVertShader.GLSL",
        SHADER_FOLDER "basicFragShader.GLSL",
        NULL,
        NULL
    };

    Shader* shaders = NULL;

    int load(char* filename, GLenum type, GLuint* outShader) {

        FILE* file = fopen(filename,"rb");
        if (!file) {
            return 1;
        }

        fseek(file, 0, SEEK_END);
        long size = ftell(file) + 1;
        fseek(file, 0, SEEK_SET);

        char* src = (char*) malloc(size * sizeof(char));
        fread(src, size, 1, file);
        src[size - 1] = '\0';

        fclose(file);

        if (compile(&src, type, outShader)) return 1;

        return 0;

    }

    int compile(char** src, GLenum type, GLuint* outShader) {

        GLuint shader = glCreateShader(type);

        glShaderSource(shader, 1, src, NULL);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        *outShader = shader;
        return (status != GL_TRUE);

    }

    int loadAll() {

        const int count = sizeof(shaderNames) / sizeof(char*);

        free(shaders);
        shaders = (Shader*) malloc(count * sizeof(Shader) / 4);

        for (int i = 0; i < count; i += 4) {

            unsigned int vertexShader = -1;
            if (shaderNames[i])
            if (load(shaderNames[i], GL_VERTEX_SHADER, &vertexShader)) return 1;

            unsigned int fragmentShader = -1;
            if (shaderNames[i + 1])
            if (load(shaderNames[i + 1], GL_FRAGMENT_SHADER, &fragmentShader)) return 1;

            unsigned int tesselationControlShader = -1;
            if (shaderNames[i + 2])
            if (load(shaderNames[i + 2], GL_TESS_CONTROL_SHADER, &tesselationControlShader)) return 1;

            unsigned int tesselationEvaluationShader = -1;
            if (shaderNames[i + 3])
            if (load(shaderNames[i + 3], GL_TESS_EVALUATION_SHADER, &tesselationEvaluationShader)) return 1;

            (shaders + i / 4)->vertexShader = vertexShader;
            (shaders + i / 4)->fragmentShader = fragmentShader;
            (shaders + i / 4)->tessellationControlShader = tesselationControlShader;
            (shaders + i / 4)->tessellationEvaluationShader = tesselationEvaluationShader;

        }

        return 0;

    }

}

