#pragma once

#include "Render.h"
#include "Player.h"
#include "Car.h"
#include "ShaderDriver.h"

namespace Game {

    extern int tickRate;
	
    struct Map {

        uint8_t* depths;
        uint32_t* colors;

        int width;
        int height;

        double scale;

    };


    struct Terrain {

        Render::Texture* etex;
        Render::Texture* ctex;
        
        ShaderDriver::TerrainShader* shd;
        unsigned int vao;

        int patchRez;
        int patchPoints;

    };

    extern Map map;
    extern Terrain terrain;
    extern Player* player;
    extern Car car;

    int init();
    int update();

}