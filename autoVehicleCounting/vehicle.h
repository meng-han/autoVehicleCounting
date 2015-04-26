#ifndef VEHICLE_H
#define VEHICLE_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/tracking.hpp"
#include <deque>

using namespace std;
using namespace cv;

class vehicle{
public:
    int id;
    Mat detection;
    Rect rect;
    RotatedRect rRect;
    bool present;
    KalmanFilter KF;
    int totalVisibleCount;
    int invisibleCount;
    int start;
    int stop;
    deque<cv::Point> trajectory;
    deque<cv::Point> aerialTrajectory;
    int maneuver;
    int getManeuver();

};





#endif // VEHICLE_H
