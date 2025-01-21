#pragma once

#include "ControlledUnit.h"
#include "Mesh.h"
#include "ShaderDriver.h"
#include <set>
#include <limits>

#define POW2(x) ((x) * (x))

#define TORQUE_CURVE_SIZE 23
#define POWER_CURVE_SIZE 23

const float g = 9.8;

struct CurveNode {
    float x;
    float y;
};

struct Car : ControlledUnit {
    
    //static const float maxWheelOffset = 0.1f;

    ShaderDriver::BasicShader* shd;
    Mesh::Model* model;

    glm::mat4 transform;

    // indices to model->mesh
    // F as front, B as back, R as right, L as left
    int wheelFR;
    int wheelFL;
    int wheelBR;
    int wheelBL;

    int tyreFR;
    int tyreFL;
    int tyreBR;
    int tyreBL;

    float tyreRadius;
    float wheelCenter;

    glm::vec3 wheelFRCenter;
    glm::vec3 wheelFLCenter;
    glm::vec3 wheelBRCenter;
    glm::vec3 wheelBLCenter;

    float centerFromRearAxle;
    float frontRearAxleDist;
    float frontAxleSize;

    float wheelFROffset;
    float wheelFLOffset;
    float wheelBROffset;
    float wheelBLOffset;

    // meaning front wheels
    float wheelsMaxAngle = M_PI / (180 / 25.0);
    float wheelsAngle = 0;

    int toggleGearUp = 0;
    int toggleGearDown = 0;

    float turnRate = 0;
    float speed = 0; // current speed cat is moving

    float throttle = 0; // 0 to 1, how full we press gas pedal
    float brakes = 0;
    
    
    float maxVelocity = 74570;
    float velocityAmp = 0.25;

    float mass = 660; // kg
    float torque = 450; // N * m

    float maxRpm = 11000;
    CurveNode torqueCurve[TORQUE_CURVE_SIZE + 1] = {
        { 0, 0 },
        { 500, 60 },
        { 1000, 132 },
        { 1500, 148 },
        { 2000, 172 },
        { 2500, 215 },
        { 3000, 247 },
        { 3500, 271 },
        { 4000, 288 },
        { 4500, 297 },
        { 5000, 324 },
        { 5500, 374 },
        { 6000, 411 },
        { 6500, 442 },
        { 7000, 449 },
        { 7500, 452 },
        { 8000, 451 },
        { 8500, 450 },
        { 9000, 452 },
        { 9500, 454 },
        { 10000, 456 },
        { 10500, 438 },
        { 11000, 422 },
        { 11001, 422 }
    };

    CurveNode powerCurve[POWER_CURVE_SIZE + 1] = {
        { 0, 0 },
		{ 500, 4 },
		{ 1000, 19 },
		{ 1500, 31 },
		{ 2000, 48 },
		{ 2500, 75 },
		{ 3000, 104 },
		{ 3500, 133 },
		{ 4000, 162 },
		{ 4500, 188 },
		{ 5000, 227 },
		{ 5500, 289 },
		{ 6000, 346 },
		{ 6500, 403 },
		{ 7000, 441 },
		{ 7500, 476 },
		{ 8000, 507 },
		{ 8500, 537 },
		{ 9000, 571 },
		{ 9500, 606 },
		{ 10000, 640 },
		{ 10500, 646 },
		{ 11000, 652 },
        { 11000, 653 },
    };

    const int GEAR_REVERSE = 0;
    const int GEAR_NEUTRAL = 1;
    const int GEAR_FIRST = 2;
    const int GEAR_MAX = 6;

    int gear = 1;
    int prevGear = 1;
    float gearRatios[8] = {
        10/67.0,
        0,
        16/40.0,
        19/36.0,
        23/35.0,
        26/34.0,
        30/34.0,
        33/33.0
    };

    float differentialRatio = 3.4;
    float transmissionEfficiency = 0.9;

    glm::vec2 velocityVec = { 0, 0 };

    // c like const/coef
    float cDrag = 0.85;
    float cLift = 3;
    float cRollingResistance = 0.002;
    float cFrontCorneringStiffness = 150000;
    float cFriction = 1.4;
    float cBrake = cFriction;

    // f like force
    float fEngine;
    float fBrakes;


    virtual void moveForward(const double dt) {

        throttle += dt;
        if (throttle > 1) throttle = 1;
        
    }

    virtual void dontMoveForward(const double dt) {

        throttle -= dt;
        if (throttle < 0) throttle = 0;

    }

    virtual void moveBack(const double dt) {
        
        brakes += maxVelocity * 2;
        if (brakes > 1) brakes = 1;

    }

    virtual void dontMoveBack(const double dt) {
        
        brakes -= maxVelocity * 2;
        if (brakes < 0) brakes = 0;
    
    }

    virtual void moveLeft(const double dt) {

        if (toggleGearDown) return;
        toggleGearDown = 1;

        if (gear == GEAR_NEUTRAL) gear = GEAR_REVERSE;
        else if (gear == GEAR_FIRST) gear = GEAR_NEUTRAL;
        else if (gear > GEAR_FIRST) gear--;
    
        // printf("%i\n", gear);

    }

    virtual void dontMoveLeft(const double dt) {

        toggleGearDown = 0;
        
    }

    virtual void moveRight(const double dt) {

        if (toggleGearUp) return;
        toggleGearUp = !toggleGearUp;
        
        if (gear == GEAR_REVERSE) gear = GEAR_NEUTRAL;
        else if (gear == GEAR_NEUTRAL) gear = GEAR_FIRST;
        else if (gear < GEAR_MAX) gear++;

        // printf("%i\n", gear);
    
    }

    virtual void dontMoveRight(const double dt) {
        
        toggleGearUp = 0;

    }

    virtual void rotate(const double deltaAngle) {

        wheelsAngle += deltaAngle;
        wheelsAngle = keepAngleInRange(wheelsAngle, wheelsMaxAngle);
        // printf("wheels angle: %f\n", wheelsAngle);
        //moveVec.x = cos(angle);
        //moveVec.z = sin(angle);

    }

    virtual void rotateLeft() {
        rotate(-rotationAngle);
    }

    virtual void rotateLeft(const double delta) {
        rotate(-delta);
    }

    virtual void rotateRight() {
        rotate(rotationAngle);
    }

    virtual void rotateRight(const double delta) {
        rotate(delta);
    }

    void reorderMeshes() {
        
        //std::set<int> indices = {wheelFR, wheelFL, wheelBR, wheelBL};

        Mesh::MeshNode mesh = model->meshes[0];
        model->meshes[0] = model->meshes[wheelFL];
        model->meshes[wheelFL] = mesh;

        mesh = model->meshes[1];
        model->meshes[1] = model->meshes[tyreFL];
        model->meshes[tyreFL] = mesh;

        mesh = model->meshes[2];
        model->meshes[2] = model->meshes[wheelFR];
        model->meshes[wheelFR] = mesh;

        mesh = model->meshes[3];
        model->meshes[3] = model->meshes[tyreFR];
        model->meshes[tyreFR] = mesh;

    }

    float getCurveYTorque(CurveNode* curve, const int size, const float refX) {
        
        for (int i = 0; i < size + 1; i++) {
            
            if ((curve + i)->x > refX) {
                const CurveNode n1 = curve[i - 1];
                const CurveNode n2 = curve[i];
                return n1.y - (n2.y - n1.y) * ((n1.x - refX) / (n2.x - n1.x));
            }

        }

    }

    float getCurveXTorque(CurveNode* curve, const int size, const float refY) {
        
        for (int i = 0; i < size + 1; i++) {
            
            if ((curve + i)->y > refY) {
                const CurveNode n1 = curve[i - 1];
                const CurveNode n2 = curve[i];
                return n1.x - (n2.x - n1.x) * ((n1.y - refY) / (n2.y - n1.y));
            }

        }

    }

    void update(const int tickRate) {

        const float dt = (1.f / tickRate);



        //
        // rotation simulation
        // sin(a) = b / c;
        // 
        // sin(b) = (b + x) / ((((b + x) ^ 2 + a ^ 2) / c) * c

        float frontWheelsRR = 0; // RR as rotation radius
        float wheelsTurnRate = 0;
        float sinWheelsAngle = sin(wheelsAngle);
        if (fabs(sinWheelsAngle) > 0.001) {
            frontWheelsRR = (frontRearAxleDist / sinWheelsAngle);
            wheelsTurnRate = (velocity / frontWheelsRR) * dt;
        };

        wheelsAngle = keepAngleInRange(wheelsAngle - wheelsTurnRate, wheelsMaxAngle);
        angle += wheelsTurnRate;

        moveVec.x = -sin(angle);
        moveVec.z = cos(angle);



        //
        // movement simulation

        // 1) compute torque that motor outputs in relation to the throttle position
        //      # throttle -> motor power -> wrpm -> gears -> torque

        //throttle = 1.0;
        float rpm = getCurveXTorque(powerCurve, POWER_CURVE_SIZE, powerCurve[POWER_CURVE_SIZE - 1].y * throttle);
        
        if (prevGear != gear) {
            if (gear == GEAR_NEUTRAL) rpm = 0;
            else rpm *= gearRatios[prevGear] / gearRatios[gear];
        } else {
            rpm *= gearRatios[gear];
        }
        prevGear = gear;

        const float torque = getCurveYTorque(torqueCurve, POWER_CURVE_SIZE, rpm);

        
        // 2) compute longitudinal force, so we need to compute all forces applied to the car
        //      # tractive force (force of wheels to the ground)
        //          # for rare driven car its combined form : 
        //              # tractive force of the rare wheels with direction of the car
        //              # lateral force of the rotated front wheels

        //      # aerodynamic force (increasing with the square of velocity)
        //          # drag
        //          # downforce
        //      # rolling resistance (friction between tyres and road)
        //      # grade resistance (may omit for now)

        // velocityVec is normalized
        // moveVec is normalized in rare wheels direction

        
        const float fDrag = cDrag * velocity * velocity;
        const float fDownforce = cLift * velocity * velocity;
        const float fNormal = mass * g + fDownforce;
        const float fTractionMax = cFriction * fNormal;
        
        glm::vec2 fTractive;
        {
            float rare = torque / tyreRadius;

            glm::vec2 perpMoveVec = { -moveVec.z, moveVec.x };
            const float vx = glm::dot(velocityVec, { moveVec.x, moveVec.z });
            const float vy = glm::dot(velocityVec, perpMoveVec);
            
            float lateral = 0;
            if (abs(vx) > 0.0001) {
                lateral = cFrontCorneringStiffness * (wheelsAngle - atan2(vy, vx));
            }

            float totalForce = sqrt(rare * rare + lateral * lateral);
            if (totalForce > fTractionMax) {
                const float scale = fTractionMax / totalForce;
                rare *= scale;
                lateral *= scale;
            }

            // printf("r: %f, l: %f, a: %f\n", rare, lateral, wheelsAngle);

            fTractive.x = rare * moveVec.x + lateral * perpMoveVec.x;
            fTractive.y = rare * moveVec.z + lateral * perpMoveVec.y;
        }

        glm::vec2 fBraking; {
            fBraking = - (cBrake * fNormal * brakes) * velocityVec;
            if (glm::dot(velocityVec, fBraking) > 0) {
                fBraking = glm::vec2(0.0f, 0.0f);
            }
        }

        glm::vec2 fAerodynamic = -fDrag * velocityVec;
        glm::vec2 fRollingResistance = - (cRollingResistance * fNormal) * velocityVec;

        glm::vec2 fLongitudinal = fTractive + fAerodynamic + fRollingResistance + fBraking;

        glm::mat2 toGlobal = glm::mat2(moveVec.x, -moveVec.z, moveVec.z, moveVec.x);
        glm::vec2 globalForce = toGlobal * fLongitudinal;

        glm::vec2 acceleration = fLongitudinal / mass;
        

        //printf("torque: %f, throttle: %f\n", torque, throttle);
        //printf("%f, %f\n", acceleration.x, acceleration.y);
        //float oldVelocity = velocity;
        velocityVec = velocityVec * velocity + acceleration * dt;
        velocity = glm::length(velocityVec);
        
        pos.x += dt * velocityVec.x;
        pos.z += dt * velocityVec.y;

        if (velocity > 0.0001) {
            velocityVec /= velocity;
        }
        
        // normalize back
        /*
        if (acceleration.x * acceleration.x + acceleration.y * acceleration.y) {
            velocityVec += dt * glm::normalize(acceleration);
        }
        if (velocityVec.x * velocityVec.x + velocityVec.y * velocityVec.y > 0.0001) {
            velocityVec = glm::normalize(velocityVec);
        }
        */

        //printf("torque: %f\n", torque);
        // printf("velocity: %f\n", velocity);
        //printf("drag: %f\n", fDrag);


    }

    void render(glm::mat4& projection, glm::mat4& view, glm::mat4& model) {

        glm::mat4& modelGlobal = transform * glm::rotate(model, -angle, {0, 1, 0});
        
        glm::vec3 tmp = wheelFLCenter;
        glm::mat4& wheelFLModel = modelGlobal * glm::translate(model, tmp) * glm::rotate(model, -wheelsAngle, {0, 1, 0}) * glm::translate(model, -tmp);
        
        tmp = wheelFRCenter;
        glm::mat4& wheelFRModel = modelGlobal * glm::translate(model, tmp) * glm::rotate(model, -wheelsAngle, {0, 1, 0}) * glm::translate(model, -tmp);
        


        glBindVertexArray(this->model->vao);
        glUseProgram(shd->shaderProgram);
        
        glUniformMatrix4fv(shd->projection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(shd->view, 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(shd->worldPos, 1, glm::value_ptr(this->pos));

        glUniform4fv(shd->fogColor, 1, glm::value_ptr(Render::fogColor));
        glUniform3fv(shd->plPos, 1, glm::value_ptr(Render::cam.target->pos));
        glUniform3fv(shd->ambientColor, 1, glm::value_ptr(Render::ambientColor));
        glUniform3fv(shd->worldPos, 1, glm::value_ptr(pos));
        
        glUniform4fv(shd->moonColor, 1, glm::value_ptr(Render::moonColor));
        glUniform3fv(shd->moonPos, 1, glm::value_ptr(Render::moonPos));
        glUniform4fv(shd->moonDiffuseColor, 1, glm::value_ptr(Render::moonDiffuseColor));
        
        glm::vec3 frontLightPos = pos + moveVec * 2.0f;
        glUniform3fv(shd->frontLightPos, 1, glm::value_ptr(frontLightPos));
        glUniform3fv(shd->frontLightDir, 1, glm::value_ptr(moveVec));
        glUniform3fv(shd->frontLightColor, 1, glm::value_ptr(Render::moonColor));
        glUniform3fv(shd->frontLightDiffuseColor, 1, glm::value_ptr(Render::moonDiffuseColor));

        glUniformMatrix4fv(shd->model, 1, GL_FALSE, glm::value_ptr(wheelFLModel));
        for (int i = 0; i < 2; i++) {
            Mesh::render(this->model, &this->model->meshes[i]);
        }

        glUniformMatrix4fv(shd->model, 1, GL_FALSE, glm::value_ptr(wheelFRModel));
        for (int i = 2; i < 4; i++) {
            Mesh::render(this->model, &this->model->meshes[i]);
        }

        // glUniform3fv(shd->worldPos, 1, glm::value_ptr(this->pos));
        glUniformMatrix4fv(shd->model, 1, GL_FALSE, glm::value_ptr(modelGlobal));
        for (int i = 4; i < this->model->meshes.size(); i++) {
            Mesh::render(this->model, &this->model->meshes[i]);
        }

        glBindVertexArray(0);

    }

};