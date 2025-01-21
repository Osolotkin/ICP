#pragma once

#include <chrono>
#include "FPSCounter.h"

#define NANOSECONDS 1000000000

namespace FPSCounter {

    uint32_t fps;

    using namespace std::chrono;
    typedef time_point<steady_clock> Time;

    Time lastTime{};

    void tick() {

        Time now = steady_clock::now();
        fps = (double) NANOSECONDS / (double) duration_cast<nanoseconds>(now - lastTime).count();
        lastTime = steady_clock::now();

    }

}

