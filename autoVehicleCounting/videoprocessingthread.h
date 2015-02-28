#ifndef VIDEOPROCESSINGTHREAD_H
#define VIDEOPROCESSINGTHREAD_H

#include <QThread>
#include <QDebug>

//OpenCV Headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//C++ Headers
#include <iostream>
#include "WeightedMovingVarianceBGS.h"

using namespace std;
using namespace cv;

class videoProcessingThread : public QThread
{
    Q_OBJECT
public:
    explicit videoProcessingThread(QObject *parent = 0);
    void run();
    bool stop;
    string videoName;

signals:
    void showResults(Mat);
public slots:

private:
    int fps;
    WeightedMovingVarianceBGS* bgs;
    Mat background;
    Mat bgModel;
    VideoCapture capture;
    int width;
    int height;
    int totalFrameNumber;
    int morph_size = 10;
    Mat element = getStructuringElement( MORPH_RECT, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
    void init();
    void readNextFrame();

    int processingFPS = 5;

    Mat frame;

};

#endif // VIDEOPROCESSINGTHREAD_H
