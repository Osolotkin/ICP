

#include <opencv2/opencv.hpp>
#include <numeric>

#include "error.h"
#include "log.h"

namespace FaceTracker {
    
    int camEnabled;
    int camRunning;

    cv::VideoCapture videoCapture;
    cv::Point2f faceCoords;

    /*
    namespace Cam {

        std::thread thread;

        std::atomic<cv::Mat*> recentFrame;

        int isFrameReady = 0;
        std::atomic<int> isFramePicked;

        int enabled = 1;

    };
    */

    int init() {

        videoCapture = cv::VideoCapture(0, cv::CAP_DSHOW);
        if (!videoCapture.isOpened()) {
            Log::logError(Err::NO_VIDEO_CAPTURE_SOURCE);
            return Err::NO_VIDEO_CAPTURE_SOURCE;
        }

        camEnabled = 0;
        camRunning = 1;

        return Err::OK;

    }

    cv::Point2f detectFace(cv::Mat* frame) {

        cv::Mat frameHSV;
        cv::cvtColor(*frame, frameHSV, cv::COLOR_BGR2HSV);

        cv::Scalar lthreshold = cv::Scalar(10, 0, 0);
        cv::Scalar uthreshold = cv::Scalar(20, 255, 255);

        cv::Mat frameThreshold;
        cv::inRange(frameHSV, lthreshold, uthreshold, frameThreshold);

        std::vector<cv::Point> whitePixels;
        cv::findNonZero(frameThreshold, whitePixels);

        cv::Point whiteAccum = std::accumulate(whitePixels.begin(), whitePixels.end(), cv::Point(0.0, 0.0));

        cv::Point2f centroid_absolute = whiteAccum / (int)whitePixels.size();

        return { centroid_absolute.x / frame->cols, centroid_absolute.y / frame->rows };

    }

    void run() {

        cv::Mat frame;

        while (camRunning) {

            if (!camEnabled) continue;
            
            videoCapture.read(frame);
            if (frame.empty()) {
                Log::logError(Err::VIDEO_CAPTURE_NO_FRAME);
            }

            faceCoords = detectFace(&frame);

        }

    }

    void close() {

        videoCapture.release();
    
    }

    /*
    void drawCrossRelative(cv::Mat* img, const cv::Point2f center_relative, const int size) {

        cv::Point p;
        p.x = center_relative.x * img->cols;
        p.y = center_relative.y * img->rows;

        cv::Point p1 = cv::Point(p.x - size / 2, p.y);
        cv::Point p2 = cv::Point(p.x + size / 2, p.y);
        cv::Point p3 = cv::Point(p.x, p.y - size / 2);
        cv::Point p4 = cv::Point(p.x, p.y + size / 2);

        cv::line(*img, p1, p2, CV_RGB(255, 0, 0), 3);
        cv::line(*img, p3, p4, CV_RGB(255, 0, 0), 3);

    }
    */

    /*
    int runCam() {

        Cam::isFramePicked = 0;

        cv::Mat frameBuff[2];
        cv::Mat* frame = frameBuff;

        int i = 0;

        while (Cam::enabled) {

            videoCapture.read(*frame);
            if (frame->empty()) {
                Log::logError(Err::VIDEO_CAPTURE_NO_FRAME);
                return Err::VIDEO_CAPTURE_NO_FRAME;
            }

            faceCoords = detectFace(frame);

            if (!Cam::isFramePicked) {

                Cam::recentFrame = frame;
                Cam::isFrameReady = 1;

                frame = frameBuff + (i % 2);
                i++;

            }

        }

    }


    void drawString(cv::Mat* frame, std::string str, cv::Point loc) {

        cv::putText(
            *frame,
            str,
            loc, //top-left position
            cv::FONT_HERSHEY_DUPLEX,
            1.0,
            CV_RGB(118, 185, 0), //font color
            2
        );

    }

    int run() {

        cv::Mat scene = cv::Mat(640, 480, CV_64FC1, 0.0);

        Cam::thread = std::thread(runCam);

        while (1) {

            if (Cam::isFrameReady) {

                Cam::isFramePicked = 1;

                scene = *Cam::recentFrame;

                Cam::isFrameReady = 0;
                Cam::isFramePicked = 0;

            }

            const int key = cv::waitKey(1);

            if (key == 27) break;

        }

        Cam::enabled = 0;
        Cam::thread.join();

        return Err::OK;

    }
    */

}