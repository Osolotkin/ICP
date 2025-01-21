#pragma once

#include "glm/glm.hpp"

struct Entity {

    glm::vec3 pos;
    glm::vec3 moveVec;
    glm::vec3* lookVec = &moveVec;

    float xScale;
    float yScale;

    int camLock = 0;
    float camXOffset = 0;
    float camYOffset = 0;

};
