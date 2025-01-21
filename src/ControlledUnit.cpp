#pragma once

#include "ControlledUnit.h"

void ControlledUnit::moveForward(const double dt) {
    pos += moveVec * velocity;
}

void ControlledUnit::dontMoveForward(const double dt) {
}

void ControlledUnit::moveBack(const double dt) {
    pos += moveVec * -velocity;
}

void ControlledUnit::dontMoveBack(const double dt) {
}

void ControlledUnit::moveLeft(const double dt) {
    pos.x += moveVec.z * velocity;
    // pos.y += moveVec.y * -velocity;
    pos.z += moveVec.x * -velocity;
}

void ControlledUnit::dontMoveLeft(const double dt) {
}

void ControlledUnit::moveRight(const double dt) {
    pos.x += moveVec.z * -velocity;
    // pos.y += moveVec.y * -velocity;
    pos.z += moveVec.x * velocity;
}

void ControlledUnit::dontMoveRight(const double dt) {
}

void ControlledUnit::rotate(const double deltaAngle) {

    angle += deltaAngle;
    angle = keepAngleInRange(angle);
    //printf("%f\n", angle);

    moveVec.z = cos(angle);
    moveVec.x = sin(angle);

}

void ControlledUnit::rotateLeft() {
    rotate(rotationAngle);
}

void ControlledUnit::rotateLeft(const double delta) {
    rotate(-delta);
}

void ControlledUnit::rotateRight() {
    rotate(-rotationAngle);
}

void ControlledUnit::rotateRight(const double delta) {
    rotate(delta);
}