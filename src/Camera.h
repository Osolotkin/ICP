#pragma once

#include "Entity.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

struct Camera {

    enum Type {
        CT_FIRST_PERSON,
        CT_THIRD_PERSON,
        CT_TOP_DOWN,
        CT_COUNT
    };

    Entity* target;
    Type type = CT_FIRST_PERSON;
    glm::vec3 offset = {0, 0, 0};

    float fov; // in rads
    float yPitch = 0;

    glm::mat4 getView() {

        glm::vec3 eye;
        glm::vec3 loc;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        switch (type) {
            case CT_THIRD_PERSON : {
                eye = target->pos - (target->moveVec * 10.0f);
                eye.y += 5;
                loc = target->pos;
                break;
            }
            case CT_TOP_DOWN : {
                eye = target->pos + glm::vec3(0, 70, 0);
                loc = target->pos;
                up = glm::vec3(0.0f, 0.0f, 1.0f);
                break;
            }
            case CT_FIRST_PERSON :
            default : {
                eye = target->pos + offset;
                loc = eye + *(target->lookVec);
            }
        }

        return glm::lookAt(eye, loc, up);

    }

};
