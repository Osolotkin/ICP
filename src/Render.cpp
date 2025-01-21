#pragma once

#include "Camera.h"
#include "Game.h"
#include "ShaderDriver.h"

#define _USE_MATH_DEFINES
#include <corecrt_math_defines.h>
#include <iostream>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define GLM_FORCE_RADIANS

#include <opencv2/opencv.hpp>

#define DEG_2_RAD(deg) (((deg) / 180) * M_PI)

namespace Render {

    struct Vertex {
        float x;
        float y;
        float z;
        float tx;
        float ty;
    };

    struct VertexEx {
        float x;
        float y;
        float z;
        float tx;
        float ty;
        float nx;
        float ny;
        float nz;
    };

    int width = 800;
    int height = 640;
    Camera cam;
    int maxTesselationLevel;

    GLuint frameBuffer;
    GLuint frameBufferTex;
    int frameBufferWidth = 200;
    int frameBufferHeight = 100;

    glm::vec4 fogColor = { 0.1, 0.1, 0.1, 1 };
    glm::vec3 ambientColor = { 0.1, 0.1, 0.1 };

    glm::vec4 moonColor = { 0.6, 0.6, 0.9, 1.0 };
    glm::vec4 moonDiffuseColor = { 0.3, 0.3, 0.4, 1.0 };
    glm::vec3 moonPos = { 100.0, 50.0, 50.0 };

    glm::vec4 frontLightColor = { 0.2, 0.2, 0.3, 1.0 };
    glm::vec4 frontLightDiffuseColor = { 1.0, 1.0, 0.9, 1.0 };

    void* shaderPrograms[ShaderDriver::SHDR_COUNT];



    int loadBMP(char* const path, uint32_t** bmpData, int* width, int* height, BitDepth depth = BIT_DEPTH_8) {

        FILE* file = fopen(path, "rb");
        if (file <= 0) return 1;

        // skip header till the data offset
        fseek(file, 10, SEEK_CUR);

        int offset;
        fread(&offset, sizeof(int32_t), 1, file);

        // skip to the width and height
        fseek(file, 4, SEEK_CUR);

        fread(width, sizeof(int32_t), 1, file);
        fread(height, sizeof(int32_t), 1, file);

        int pixelCount = (*width * *height);

        *bmpData = (uint32_t*) malloc(sizeof(uint32_t) * pixelCount);
        if (*bmpData == NULL) {
            fclose(file);
            return 2;
        }

        // skip till data
        fseek(file, offset, SEEK_SET);

        fread(*bmpData, sizeof(uint32_t), pixelCount, file);

        if (depth == BIT_DEPTH_8) {

            uint8_t* tmp = (uint8_t*)malloc(pixelCount);
            for (int i = 0; i < pixelCount; i++) {
                tmp[i] = (uint8_t)((*bmpData)[i]);
            }
            free(*bmpData);
            *bmpData = (uint32_t*)tmp;
        }

        fclose(file);

        return 0;

    };

    Texture* loadTexture(char* const path, BitDepth depth) {

        int width = 0;
        int height = 0;
        uint32_t* bmpData;
        if (loadBMP(path, &bmpData, &width, &height, depth)) return NULL;

        Texture* texture = (Texture*) malloc(sizeof(Texture));
        if (!texture) return NULL;

        texture->pixels = (uint32_t*) malloc(depth * width * height);
        if (!(texture->pixels)) return NULL;

        switch (depth) {
            case Render::BIT_DEPTH_8: {
                uint8_t* dest = (uint8_t*) texture->pixels;
                uint8_t* src = (uint8_t*) bmpData;
                for (int k = 0; k < height; k++) {
                    memcpy(dest + k * width, src + (height - k - 1) * width, width);
                }
                break;
            }
            case Render::BIT_DEPTH_24:
            default:
                for (int k = 0; k < height; k++) {
                    memcpy(texture->pixels + k * width, bmpData + (height - k - 1) * width, width * depth);
                }
                break;
        }

        free(bmpData);

        texture->width = width;
        texture->height = height;

        return texture;

    }

    void genTexture(Texture* tex, unsigned int iformat, unsigned int eformat, unsigned int type) {

        glCreateTextures(GL_TEXTURE_2D, 1, &tex->glId);
        glTextureStorage2D(tex->glId, 1, iformat, tex->width, tex->height);
        GLenum error = glGetError();

        if (eformat == GL_RED) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
        glTextureSubImage2D(tex->glId, 0, 0, 0, tex->width, tex->height, eformat, type, tex->pixels);

        glTextureParameteri(tex->glId, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(tex->glId, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureParameteri(tex->glId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(tex->glId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (eformat == GL_RED) {
            glTextureParameteri(tex->glId, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTextureParameteri(tex->glId, GL_TEXTURE_SWIZZLE_B, GL_RED);
        }
        
    }

    int init() {

        // load and compile all shaders
        if (ShaderDriver::loadAll()) return 1;

        const ShaderDriver::Shader terrainShader = ShaderDriver::shaders[ShaderDriver::SHDR_TERRAIN];
        const ShaderDriver::Shader basicShader = ShaderDriver::shaders[ShaderDriver::SHDR_BASIC];
        


        glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTesselationLevel);

        glEnable(GL_DEPTH_TEST);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);



        // init terrain
        Texture* etex = Game::terrain.etex;
        Texture* ctex = Game::terrain.ctex;

        genTexture(etex, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
        genTexture(ctex, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        
        // generate vertices for tesselation
        const int patchPoints = 4;
        const int patchRez = 20;

        unsigned int terrainVAO;
        glCreateVertexArrays(1, &terrainVAO);

        unsigned int terrainVBO;
        {

            const int verticesSize = 20 * sizeof(float) * patchRez * patchRez;
            float* vertices = (float*)malloc(verticesSize);

            int idx = 0;
            const int width = etex->width;
            const int height = etex->height;
            for (unsigned i = 0; i < patchRez; i++) {

                for (unsigned j = 0; j < patchRez; j++) {

                    // v.x, v.y, v.z, u, v

                    vertices[idx] = width * i / (float) patchRez;
                    vertices[idx + 1] = 0.0f;
                    vertices[idx + 2] = height * j / (float) patchRez;
                    vertices[idx + 3] = i / (float) patchRez;
                    vertices[idx + 4] = j / (float) patchRez;

                    vertices[idx + 5] = width * (i + 1) / (float) patchRez;
                    vertices[idx + 6] = 0.0f;
                    vertices[idx + 7] = height * j / (float) patchRez;
                    vertices[idx + 8] = (i + 1) / (float) patchRez;
                    vertices[idx + 9] = j / (float) patchRez;

                    vertices[idx + 10] = width * i / (float) patchRez;
                    vertices[idx + 11] = 0.0f;
                    vertices[idx + 12] = height * (j + 1) / (float) patchRez;
                    vertices[idx + 13] = i / (float) patchRez;
                    vertices[idx + 14] = (j + 1) / (float) patchRez;

                    vertices[idx + 15] = width * (i + 1) / (float) patchRez;
                    vertices[idx + 16] = 0.0f;
                    vertices[idx + 17] = height * (j + 1) / (float)patchRez;
                    vertices[idx + 18] = (i + 1) / (float) patchRez;
                    vertices[idx + 19] = (j + 1) / (float) patchRez;

                    idx += 20;

                }

            }

            // vertices
            glCreateBuffers(1, &terrainVBO);
            glNamedBufferStorage(terrainVBO, verticesSize, vertices, GL_MAP_READ_BIT);
            
        }

        glVertexArrayAttribFormat(terrainVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(terrainVAO, 0, 0);
        glEnableVertexArrayAttrib(terrainVAO, 0);
        glVertexArrayVertexBuffer(terrainVAO, 0, terrainVBO, 0, sizeof(float) * 5);
        
        glVertexArrayAttribFormat(terrainVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
        glVertexArrayAttribBinding(terrainVAO, 1, 0);
        glEnableVertexArrayAttrib(terrainVAO, 1);
        glVertexArrayVertexBuffer(terrainVAO, 0, terrainVBO, 0, sizeof(float) * 5);


        glPatchParameteri(GL_PATCH_VERTICES, patchPoints);



        // create terrains shader program
        unsigned int shdProgTerrain = glCreateProgram();
        glAttachShader(shdProgTerrain, terrainShader.vertexShader);
        glAttachShader(shdProgTerrain, terrainShader.fragmentShader);
        glAttachShader(shdProgTerrain, terrainShader.tessellationControlShader);
        glAttachShader(shdProgTerrain, terrainShader.tessellationEvaluationShader);
        glLinkProgram(shdProgTerrain);

        Game::terrain.shd = (ShaderDriver::TerrainShader*) malloc((sizeof(ShaderDriver::TerrainShader)));
        Game::terrain.shd->model = glGetUniformLocation(shdProgTerrain, "model");
        Game::terrain.shd->projection = glGetUniformLocation(shdProgTerrain, "projection");
        Game::terrain.shd->view = glGetUniformLocation(shdProgTerrain, "view");
        Game::terrain.shd->etex = glGetUniformLocation(shdProgTerrain, "etex");
        Game::terrain.shd->ctex = glGetUniformLocation(shdProgTerrain, "ctex");
        Game::terrain.shd->fogColor = glGetUniformLocation(shdProgTerrain, "fragFogColor");
        Game::terrain.shd->plPos = glGetUniformLocation(shdProgTerrain, "plPos");
        Game::terrain.shd->ambientColor = glGetUniformLocation(shdProgTerrain, "ambientColor");
        Game::terrain.shd->moonColor = glGetUniformLocation(shdProgTerrain, "moonColor");
        Game::terrain.shd->moonPos = glGetUniformLocation(shdProgTerrain, "moonPos");
        Game::terrain.shd->moonDiffuseColor = glGetUniformLocation(shdProgTerrain, "moonDiffuseColor");
        Game::terrain.shd->frontLightPos = glGetUniformLocation(shdProgTerrain, "frontLightPos");
        Game::terrain.shd->frontLightDir = glGetUniformLocation(shdProgTerrain, "frontLightDir");
        Game::terrain.shd->frontLightColor = glGetUniformLocation(shdProgTerrain, "frontLightColor");
        Game::terrain.shd->frontLightDiffuseColor = glGetUniformLocation(shdProgTerrain, "frontLightDiffuseColor");
        Game::terrain.shd->shaderProgram = shdProgTerrain;



        // cars
        unsigned int shdProgCar = glCreateProgram();
        glAttachShader(shdProgCar, basicShader.vertexShader);
        glAttachShader(shdProgCar, basicShader.fragmentShader);
        glLinkProgram(shdProgCar);

        Game::car.shd = (ShaderDriver::BasicShader*) malloc((sizeof(ShaderDriver::BasicShader)));
        Game::car.shd->model = glGetUniformLocation(shdProgCar, "model");
        Game::car.shd->projection = glGetUniformLocation(shdProgCar, "projection");
        Game::car.shd->view = glGetUniformLocation(shdProgCar, "view");
        Game::car.shd->worldPos = glGetUniformLocation(shdProgCar, "worldPos");
        Game::car.shd->fogColor = glGetUniformLocation(shdProgCar, "fragFogColor");
        Game::car.shd->plPos = glGetUniformLocation(shdProgCar, "plPos");
        Game::car.shd->ambientColor = glGetUniformLocation(shdProgCar, "ambientColor");
        Game::car.shd->moonColor = glGetUniformLocation(shdProgCar, "moonColor");
        Game::car.shd->moonPos = glGetUniformLocation(shdProgCar, "moonPos");
        Game::car.shd->moonDiffuseColor = glGetUniformLocation(shdProgCar, "moonDiffuseColor");
        Game::car.shd->frontLightPos = glGetUniformLocation(shdProgCar, "frontLightPos");
        Game::car.shd->frontLightDir = glGetUniformLocation(shdProgCar, "frontLightDir");
        Game::car.shd->frontLightColor = glGetUniformLocation(shdProgCar, "frontLightColor");
        Game::car.shd->frontLightDiffuseColor = glGetUniformLocation(shdProgCar, "frontLightDiffuseColor");
        Game::car.shd->shaderProgram = shdProgCar;



        // cache data
        Game::terrain.vao = terrainVAO;
        Game::terrain.patchRez = patchRez;
        Game::terrain.patchPoints = patchPoints;

        // set cam
        cam.target = NULL;
        cam.fov = DEG_2_RAD(85.0f);



        return 0;
    
    }

    void applyShaderTerrain(ShaderDriver::TerrainShader* shd, glm::mat4& projection, glm::mat4& view, glm::mat4& model) {

        glUseProgram(shd->shaderProgram);

        glUniformMatrix4fv(shd->projection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(shd->view, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(shd->model, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(shd->etex, 0);
        glUniform1i(shd->ctex, 1);

        glUniform4fv(shd->fogColor, 1, glm::value_ptr(fogColor));
        glUniform3fv(shd->plPos, 1, glm::value_ptr(cam.target->pos));
        glUniform3fv(shd->ambientColor, 1, glm::value_ptr(ambientColor));

        glUniform3fv(shd->moonPos, 1, glm::value_ptr(moonPos));
        glUniform4fv(shd->moonColor, 1, glm::value_ptr(moonColor));
        glUniform4fv(shd->moonDiffuseColor, 1, glm::value_ptr(moonDiffuseColor));

        glm::vec3 frontLightPos = Game::car.pos + Game::car.moveVec * 2.0f;
        glUniform3fv(shd->frontLightPos, 1, glm::value_ptr(frontLightPos));
        glUniform3fv(shd->frontLightDir, 1, glm::value_ptr(Game::car.moveVec));
        glUniform4fv(shd->frontLightColor, 1, glm::value_ptr(frontLightColor));
        glUniform4fv(shd->frontLightDiffuseColor, 1, glm::value_ptr(frontLightDiffuseColor));

    }

    int render() {

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        


        // preset matrices, as the remain the same for all objects
        glm::mat4 projection = glm::perspective(cam.fov, ((float) Render::width / (float) Render::height), 0.1f, 10000.0f);
        glm::mat4 view = cam.getView();
        glm::mat4 model = glm::mat4(1.0f);// glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));



        // terrain
        applyShaderTerrain(Game::terrain.shd, projection, view, model);
        
        glBindTextureUnit(0, Game::terrain.etex->glId);
        glBindTextureUnit(1, Game::terrain.ctex->glId);

        glBindVertexArray(Game::terrain.vao);

        glDrawArrays(GL_PATCHES, 0, Game::terrain.patchPoints * Game::terrain.patchRez * Game::terrain.patchRez);

        // cars
        Game::car.render(projection, view, model);

        return 0;

    }





}
