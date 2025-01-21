#pragma once

#include "Game.h"
#include "Player.h"
#include "Controls.h"
#include "KeyMap.h"
#include "error.h"
#include "Mesh.h"
#include "FaceTracker.h"

#include <string>
#include <fstream>
#include <direct.h>


#undef ERROR
#include "log.h"

namespace Game {

	const int mapCount = 1;
	char* mapNames[] = {
		"./resorces/maps/map_01/",
	};

	Map map;
	Terrain terrain;
	Player* player;
	Car car;

	ControlledUnit* controlledUnit;

	int playerInCar = 0;

	int tickRate = 64;
	
	// in ticks
	int interactKeyLockTime = 32;
	int interactKeyLocked = 0;

	int wasFaceTrackerDisabledLastTick = 1;
	float faceSens = 2;
	float lastFaceX;

	int currentMap = 0;

	int loadMap(int id) {

		char elevationImgTmp[256];
		char elevationImg[256];
		char terrainImg[256];
		char data[256];
		
		sprintf(elevationImg, "./resorces/maps/map_0%i/elevation8bit.BMP", id);
		sprintf(elevationImgTmp, "./resorces/maps/map_0%i/elevation.BMP", id);
		sprintf(terrainImg, "./resorces/maps/map_0%i/terrain.BMP", id);
		sprintf(data, "./resorces/maps/map_0%i/data.txt", id);

		char buffer[256];
		char* ptr = _getcwd(buffer, 256);

		std::ifstream dataFile(data);
		if (!dataFile.is_open()) {
			Log::log(Log::ERROR, "Unable to load levels data file!");
			return -1;
		}

		map.scale = -1;

		std::string line;
		while (std::getline(dataFile, line)) {
			if (line.find("meter_scale") != std::string::npos) {
				int idx = 11;
				idx = line.find('=', idx) + 1;
				map.scale = std::stof(line.c_str() + idx);
			}
		}

		if (map.scale < 0) {
			Log::log(Log::ERROR, "Invalid value for meter_scale param in levels data file!");
			return -1;
		}

		//map.depths = (uint8_t*) Render::loadTexture(elevationImg, Render::BIT_DEPTH_8);
		terrain.etex = Render::loadTexture(elevationImgTmp, Render::BIT_DEPTH_8);//Render::loadTexture(elevationImgTmp);
		terrain.ctex = Render::loadTexture(terrainImg);

		map.width = terrain.etex->width;
		map.height = terrain.etex->height;
		map.depths = (uint8_t*) terrain.etex->pixels;

		return Err::OK;
	
	}

	int loadCar(int id) {

		car.model = new Mesh::Model();
		Mesh::load("./resorces/models/car/", car.model);
		
		car.wheelFR = 21;
		car.wheelFL = 18;
		car.wheelBR = 20;
		car.wheelBL = 19;

		car.tyreFR = 4;
		car.tyreFL = 7;
		car.tyreBR = 3;
		car.tyreBL = 6;

		Mesh::MeshNode tmp = car.model->meshes[car.tyreFR];
		car.tyreRadius = (tmp.maxX - tmp.minX) / 2;
		
		car.wheelFRCenter = Mesh::getMeshCenter(&car.model->meshes[car.tyreFR]);
		car.wheelFLCenter = Mesh::getMeshCenter(&car.model->meshes[car.tyreFL]);
		car.wheelBRCenter = Mesh::getMeshCenter(&car.model->meshes[car.tyreBR]);
		car.wheelBLCenter = Mesh::getMeshCenter(&car.model->meshes[car.tyreBL]);

		car.camXOffset = 10;
		car.camYOffset = 10;

		car.angle = 0;
		car.rotationAngle = 1 / (double)Game::tickRate;
		car.velocity = 0;
		car.moveVec = {0, 0, 1};

		car.frontRearAxleDist = glm::distance(car.wheelFRCenter, car.wheelBRCenter);
		car.frontAxleSize = glm::distance(car.wheelFRCenter, car.wheelFLCenter);
		car.centerFromRearAxle = -car.wheelBLCenter.z;

		car.reorderMeshes();

		return Err::OK;		

	}

	int init() {

		// player
		player = new Player();// (Player*) malloc(sizeof(Player));
		player->pos = glm::vec3(640.0f, 2.0f, 900.0f);// glm::vec3(1.0f, 0.5f, 1.0f);
		// player->moveVec = glm::vec3(-1.0f, 0.0f, 0.0f);//glm::vec3(1.0f, 0.0f, 0.0f);
		player->angle = 3.4;
		player->rotationAngle = 3.2 / (double)Game::tickRate;
		player->velocity =1 ;//20 * 2 * 3.2 / (double)Game::tickRate;
		player->radius = 0.5;
		player->rotate(0);

		controlledUnit = player;

		if (loadMap(0) < 0) {
			return -1;
		}

		if (loadCar(0) < 0) {
			return -1;
		}

		car.pos = glm::vec3(650.0f, 1.0f, 900.0f);
		car.angle = 3.4;

		return Err::OK;
		
	}

	void getIntoCar(Car* car) {
		
		playerInCar = 1;

		controlledUnit = car;
		Render::cam.target = car;
		Render::cam.type = Camera::CT_THIRD_PERSON;
		Render::cam.offset = {+0.25, 2, 0};

	}

	void getOutOfCar(Car* car) {
		
		playerInCar = 0;

		controlledUnit = player;
		Render::cam.target = player;
		Render::cam.type = Camera::CT_FIRST_PERSON;
		Render::cam.offset = {0, 0, 0};

	}

    const double defaultYPitchDelta = 0.1;
    void updateYPitch(const double delta) {
        Render::cam.yPitch += delta;
		if (Render::cam.yPitch > M_PI / 2) {
			Render::cam.yPitch = M_PI / 2;
		} else if (Render::cam.yPitch < -M_PI / 2) {
			Render::cam.yPitch = -M_PI / 2;
		}
    }

    int getTerrainHeight(const float x, const float y) {

        // x direction
        // f(x, y1) = (x2 - x)Q11 + (x - x1)Q21
        //          = (x1 + 1 - x)Q11 + (x - x1)Q21
        //          = (x - x1)(Q21 - Q11) + Q11
        // f(x, y2) = (x2 - x)Q12 + (x - x1)Q22


        // y direction
        // f(x, y) = (y2 - y)f(x, y1) + (y - y1)f(x, y2)
        
        const int wd = map.width;

        const int x1 = floor(x);
        const int y1 = floor(y);

		if (x1 + 1 >= wd || y1 + 1 >= map.height) return 0;

        uint8_t Q22 = map.depths[(x1 + 1) + (y1 + 1) * wd];
        uint8_t Q12 = map.depths[x1 + (y1 + 1) * wd];
        uint8_t Q21 = map.depths[(x1 + 1) + y1 * wd];
        uint8_t Q11 = map.depths[x1 + y1 * wd];

        const float xd1 = (x1 + 1 - x) * Q11 + (x - x1) * Q21;
        const float xd2 = (x1 + 1 - x) * Q12 + (x - x1) * Q22;

        return ((y1 + 1 - y) * xd1 + (y - y1) * xd2) / 2;

    }

	glm::vec3 computeEquation(glm::mat3 mat, glm::vec3 vec) {

		float a11 = mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1];
		float a12 = -(mat[0][1] * mat[2][2] - mat[0][2] * mat[2][1]);
		float a13 = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];

		float a21 = -(mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0]);
		float a22 = mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0];
		float a23 = -(mat[0][0] * mat[1][2] - mat[0][2] * mat[1][0]);

		float a31 = mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0];
		float a32 = -(mat[0][0] * mat[2][1] - mat[0][1] * mat[2][0]);
		float a33 = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

		float det = glm::determinant(mat);

		mat[0] = { a11, a12, a13 };
		mat[1] = { a21, a22, a23 };
		mat[2] = { a31, a32, a33 };

		mat = mat / det;
		return vec * mat; // fuck glm

	}

	float getAngle(glm::vec3& a, glm::vec3& b) {
		return glm::acos(glm::dot(a, b));
	}

	int update() {

		const double dt = 1.0 / tickRate;

		if (Controls::mouseEnabled) {

			const double angleDeltaX = Controls::mouseX * Controls::mouseXSens * (tickRate / (double) 1000);
			const double angleDeltaY = -Controls::mouseY * Controls::mouseYSens * (tickRate / (double) 1000);

			controlledUnit->rotate(angleDeltaX);
            updateYPitch(angleDeltaY);

		} else {
			Render::cam.yPitch = 0;
		}

		if (Controls::keyPressed[KeyMap::MOVE_FORWARD]) {
			controlledUnit->moveForward(dt);
		} else {
			controlledUnit->dontMoveForward(dt);
		}

		if (Controls::keyPressed[KeyMap::MOVE_BACK]) {
			controlledUnit->moveBack(dt);
		} else {
			controlledUnit->dontMoveBack(dt);
		}

		if (Controls::keyPressed[KeyMap::MOVE_LEFT]) {
			controlledUnit->moveLeft(dt);
		} else {
			controlledUnit->dontMoveLeft(dt);
		}

		if (Controls::keyPressed[KeyMap::MOVE_RIGHT]) {
			controlledUnit->moveRight(dt);
		} else {
			controlledUnit->dontMoveRight(dt);
		}

        if (Controls::keyPressed[KeyMap::ROTATE_LEFT]) {
            controlledUnit->rotateLeft();
        }

        if (Controls::keyPressed[KeyMap::ROTATE_RIGHT]) {
            controlledUnit->rotateRight();
        }

        if (Controls::keyPressed[KeyMap::ROTATE_UP]) {
            updateYPitch(defaultYPitchDelta);
        }

        if (Controls::keyPressed[KeyMap::ROTATE_DOWN]) {
            updateYPitch(-defaultYPitchDelta);
        }

		if (Controls::keyPressed[KeyMap::INTERACT]) {
			if (playerInCar) getOutOfCar(&car);
			else getIntoCar(&car);
			Controls::keyPressed[KeyMap::INTERACT] = 0;
		}

		if (Controls::keyPressed[KeyMap::CAM_1]) {
			Render::cam.type = Camera::CT_FIRST_PERSON;
		}

		if (Controls::keyPressed[KeyMap::CAM_2]) {
			Render::cam.type = Camera::CT_THIRD_PERSON;
		}

		if (Controls::keyPressed[KeyMap::CAM_3]) {
			Render::cam.type = Camera::CT_TOP_DOWN;
		}

		if (FaceTracker::camEnabled) {
			float x = FaceTracker::faceCoords.x;
			if (!wasFaceTrackerDisabledLastTick) {
				float dx = (x - lastFaceX) * faceSens;
				controlledUnit->rotate(dx);
			}
			lastFaceX = x;
			wasFaceTrackerDisabledLastTick = 0;
		} else {
			wasFaceTrackerDisabledLastTick = 1;
		}

		car.transform = glm::mat4(1);

		player->pos.y = getTerrainHeight(player->pos.x, player->pos.z) + 2;
		car.pos.y = getTerrainHeight(car.pos.x, car.pos.z) + 1;

		car.update(tickRate);

		return 0;
	}

}
