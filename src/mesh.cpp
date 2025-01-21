#pragma once

#include "mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#undef ERROR
#include "log.h"
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void applyNodeTransformations(const aiNode* node, const aiMatrix4x4& parentTransform, const aiScene* scene);



glm::vec3 Mesh::getMeshCenter(MeshNode* node) {
    
    return { node->minX + (node->maxX - node->minX) / 2, node->minY + (node->maxY - node->minY) / 2, node->minZ + (node->maxZ - node->minZ) / 2 };

}

void applyNodeTransformations(const aiNode* node, const aiMatrix4x4& parentTransform, const aiScene* scene) {
    
    aiMatrix4x4 globalTransform = parentTransform * node->mTransformation;

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[meshIndex];

        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            // Transform position
            mesh->mVertices[j] = globalTransform * mesh->mVertices[j];

            // Transform normals (ignore translation, use 3x3 upper-left matrix)
            if (mesh->mNormals) {
                aiMatrix3x3 normalTransform = aiMatrix3x3(globalTransform);
                mesh->mNormals[j] = normalTransform * mesh->mNormals[j];
                mesh->mNormals[j].Normalize();
            }

            // Similarly, transform tangents and bitangents if present
            if (mesh->mTangents && mesh->mBitangents) {
                mesh->mTangents[j] = aiMatrix3x3(globalTransform) * mesh->mTangents[j];
                mesh->mTangents[j].Normalize();

                mesh->mBitangents[j] = aiMatrix3x3(globalTransform) * mesh->mBitangents[j];
                mesh->mBitangents[j].Normalize();
            }
        }
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        applyNodeTransformations(node->mChildren[i], globalTransform, scene);
    }
}

int Mesh::load(char* path, Model* model) {

    std::string dotgltf = std::string(path) + "scene.gltf";
    std::string dotbin = std::string(path) + "scene.bin";

    glGenVertexArrays(1, &model->vao);
    glBindVertexArray(model->vao);

    glGenBuffers(BT_COUNT, model->buffers);

    Assimp::Importer Importer;

    const uint64_t flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GlobalScale | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph ;//aiProcess_PreTransformVertices;//aiProcess_PreTransformVertices | aiProcess_GenSmoothNormals | aiProcess_GlobalScale | aiProcess_FindDegenerates; //|// |aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs;
    const aiScene* scene = Importer.ReadFile(dotgltf.c_str(), flags);
    if (!scene) {
        Log::log(Log::ERROR, "Failed to load model.");
        return -1;
    }

    aiMatrix4x4 identityMatrix;
    applyNodeTransformations(scene->mRootNode, identityMatrix, scene);

    model->meshes.resize(scene->mNumMeshes);
    model->texs.resize(scene->mNumMaterials);

    unsigned int verticesCount = 0;
    unsigned int indicesCount = 0;

    for (int i = 0; i < model->meshes.size(); i++) {
        model->meshes[i].materialIdx = scene->mMeshes[i]->mMaterialIndex;
        model->meshes[i].indicesCnt = scene->mMeshes[i]->mNumFaces * 3;
        model->meshes[i].verticesOff = verticesCount;
        model->meshes[i].indicesOff = indicesCount;

        verticesCount += scene->mMeshes[i]->mNumVertices;
        indicesCount += model->meshes[i].indicesCnt;
    }

    std::vector<Render::Vec3f> positions;
    positions.reserve(verticesCount);
    std::vector<Render::Vec3f> normals;
    normals.reserve(verticesCount);
    std::vector<Render::Vec2f> texCoords;
    texCoords.reserve(verticesCount);
    std::vector<unsigned int> indices;
    indices.reserve(indicesCount);

    for (int i = 0; i < model->meshes.size(); i++) {
        MeshNode* node = model->meshes.data() + i;
        const aiMesh* mesh = scene->mMeshes[i];

        aiVector3D zeroVec(0, 0, 0);
        for (int i = 0; i < mesh->mNumVertices; i++) {
            const aiVector3D pos = mesh->mVertices[i];
            const aiVector3D normal = mesh->mNormals[i];
            const aiVector3D texCoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : zeroVec;

            if (pos.x < node->minX) node->minX = pos.x;
            if (pos.y < node->minY) node->minY = pos.y;
            if (pos.z < node->minZ) node->minZ = pos.z;
            
            if (pos.x > node->maxX) node->maxX = pos.x;
            if (pos.y > node->maxY) node->maxY = pos.y;
            if (pos.z > node->maxZ) node->maxZ = pos.z;
            
            positions.push_back(Render::Vec3f { pos.x, pos.y, pos.z });
            normals.push_back(Render::Vec3f { normal.x, normal.y, normal.z });
            texCoords.push_back(Render::Vec2f { texCoord.x, texCoord.y });
        }

        
        for (int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace face = mesh->mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
    }

    for (int i = 0; i < scene->mNumMaterials; i++) {
        
        const aiMaterial* material = scene->mMaterials[i];

        aiString assPath;
        
        material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), assPath);

        if (assPath.data[0] != '*') {
            // maybe just a color
            aiColor3D assColor(0.f, 0.f, 0.f);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, assColor);

            Render::Vec4c* color = new Render::Vec4c;
            color->r = assColor.r * 255;
            color->g = assColor.g * 255;
            color->b = assColor.b * 255;
            color->a = 255;

            Render::Texture* tmp = new Render::Texture();
            tmp->height = 1;
            tmp->width = 1;
            tmp->pixels = (uint32_t*) color;

            Render::genTexture(tmp, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);

            model->texs[i] = tmp;

            // Log::log(Log::ERROR, "Embedded texture for material not found!");
            continue;
        }

        unsigned int idx = atoi(assPath.C_Str() + 1);
        if (idx >= scene->mNumTextures) {
            continue;
        }

        aiTexture* texture = scene->mTextures[idx];
        
        uint32_t* img;
        int width, height, nChannels;
        if (texture->mHeight == 0) {
            // compressed texture

            img = (uint32_t*) stbi_load_from_memory(
                (stbi_uc*) (texture->pcData), 
                texture->mWidth, 
                &width, 
                &height, 
                &nChannels, 
                0
            );

        } else {
            // raw pixel RGBA data
            
            img = (uint32_t*) texture->pcData;
            width = texture->mWidth;
            height = texture->mHeight;
            nChannels = 4;    
        }

        Render::Texture* tmp = new Render::Texture();
        tmp->height = height;
        tmp->width = width;
        tmp->pixels = img;

        Render::genTexture(tmp, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);

        model->texs[i] = tmp;

    }

    glBindBuffer(GL_ARRAY_BUFFER, model->buffers[BT_POSITION]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(SHDLOC_POSITION);
    glVertexAttribPointer(SHDLOC_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, model->buffers[BT_TEXCOORD]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(SHDLOC_TEXCOORD);
    glVertexAttribPointer(SHDLOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, model->buffers[BT_NORMALS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(SHDLOC_NORMALS);
    glVertexAttribPointer(SHDLOC_NORMALS, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->buffers[BT_INDEX]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
    
    //glBindVertexArray(0);

    return Err::OK;

}

void Mesh::render(Model* model) {

    glBindVertexArray(model->vao);

    for (int i = 0; i < model->meshes.size(); i++) {
        
        Render::Texture* tex = model->texs[model->meshes[i].materialIdx];
        if (tex) {
            glBindTextureUnit(0, tex->glId);
        }

        glDrawElementsBaseVertex(
            GL_TRIANGLES, 
            model->meshes[i].indicesCnt,
            GL_UNSIGNED_INT, 
            (void*) (sizeof(unsigned int) * model->meshes[i].indicesOff), 
            model->meshes[i].verticesOff
        );
    
    }

    glBindVertexArray(0);

}

void Mesh::render(Model* model, MeshNode* mesh) {

    Render::Texture* tex = model->texs[mesh->materialIdx];
    if (tex) {
        glBindTextureUnit(0, tex->glId);
    }

    glDrawElementsBaseVertex(
        GL_TRIANGLES, 
        mesh->indicesCnt,
        GL_UNSIGNED_INT, 
        (void*) (sizeof(unsigned int) * mesh->indicesOff), 
        mesh->verticesOff
    );

}