#pragma once

#include "GL/glew.h"
#include "GL/wglew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include <limits>
#include "Render.h"

namespace Mesh {

    const int SHDLOC_POSITION   = 0;
    const int SHDLOC_TEXCOORD   = 1;
    const int SHDLOC_NORMALS    = 2;

    enum BufferType {
        BT_INDEX    = 0,
        BT_POSITION = 1,
        BT_TEXCOORD = 2,
        BT_NORMALS  = 3,
        BT_COUNT    = 4
    };

    struct MeshNode {

        unsigned int materialIdx;

        unsigned int verticesCnt;
        unsigned int indicesCnt;
        
        unsigned int verticesOff;
        unsigned int indicesOff;

        float minX = 1000000;//std::numeric_limits<float>::max();
        float maxX = -1000000;//std::numeric_limits<float>::min();

        float minY = 1000000;//std::numeric_limits<float>::max();
        float maxY = -1000000;///std::numeric_limits<float>::min();

        float minZ = 1000000;//std::numeric_limits<float>::max();
        float maxZ = -1000000;//std::numeric_limits<float>::min();

    };
    
    struct Model {

        GLuint vao;
        GLuint buffers[BT_COUNT];

        std::vector<MeshNode> meshes;
        std::vector<Render::Texture*> texs;

    };

    int load(char* fname, Model* model);
    void render(Model* model);
    void render(Model* model, MeshNode* mesh);

    glm::vec3 getMeshCenter(MeshNode* node);

}