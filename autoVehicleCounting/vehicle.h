#ifndef VEHICLE_H
#define VEHICLE_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"
using namespace std;
using namespace cv;

class vehicle{
public:
    int id;
    Mat detection;
    Rect rect;
    bool present;
    KalmanFilter KF;
    int totalVisibleCount;
    int invisibleCount;
    int start;
    int stop;
};





#endif // VEHICLE_H
