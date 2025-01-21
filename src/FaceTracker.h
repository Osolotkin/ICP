#pragma once
#include <opencv2/opencv.hpp>

namespace FaceTracker {

    extern int camEnabled;
    extern int camRunning;
    extern cv::Point2f faceCoords;

    int init();
    void run();
    void close();

}