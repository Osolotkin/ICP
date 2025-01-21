#pragma once

#include <corecrt_math_defines.h>
#include <limits>
#include "Entity.h"

/*
inline double keepAngleInRange(double angle, const double range = std::numeric_limits<double>::max()) {

    const double pi2 = 2 * M_PI;

    if (angle > pi2) {
        angle = angle - pi2;
    } else if (angle < 0) {
        angle = pi2 + angle;
    }

    if (M_PI - angle > 0) return angle > range ? range : angle;
    else return angle < pi2 - range ? pi2 - range : angle;

}

*/
inline double keepAngleInRange(double angle, const double range = std::numeric_limits<double>::max()) {

    const double pi2 = 2 * M_PI;
    
    if (angle > pi2) {
        angle = angle - pi2;
    } else if (angle < -pi2) {
        angle = pi2 + pi2 + angle;
    }

    if (angle > range) return range;
    if (angle < -range) return -range;
    return angle;

}

struct ControlledUnit : Entity {

    float velocity;
    float angle = 0;
    float rotationAngle;
    float radius = 0.5;

    virtual void moveForward(const double dt);
    virtual void dontMoveForward(const double dt);
    virtual void moveBack(const double dt);
    virtual void dontMoveBack(const double dt);
    virtual void moveLeft(const double dt);
    virtual void dontMoveLeft(const double dt);
    virtual void moveRight(const double dt);
    virtual void dontMoveRight(const double dt);
    virtual void rotate(const double deltaAngle);
    virtual void rotateLeft();
    virtual void rotateLeft(const double delta);
    virtual void rotateRight();
    virtual void rotateRight(const double delta);

};