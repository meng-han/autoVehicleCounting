#ifndef VIDEOPROCESSINGTHREAD_H
#define VIDEOPROCESSINGTHREAD_H

#include <QThread>
#include <QDebug>
#include <QList>
//OpenCV Headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>


//C++ Headers
#include <iostream>
#include "WeightedMovingVarianceBGS.h"
#include "framedifferencebgs.h"
#include "vehicle.h"

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
    QList<vehicle> vehicles;
    int fps;
    WeightedMovingVarianceBGS* bgs;
    Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
    FrameDifferenceBGS* fbgs;
    VideoCapture capture;
    VideoWriter wtr;
    int width;
    int height;
    int totalFrameNumber;
    int morph_size = 5;
    Mat element = getStructuringElement( MORPH_RECT, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
    void init();
    void readNextFrame();
    void detectObjects();
    int processingFPS = 15;
    int totalNumberOfVehicles = 0;

    Mat frame;
    Mat background;
    Mat bgModel;
    Mat img_closing;
    vector<Rect> boundRect;

    int contour_thresh = 100;
    double rect_delete_thresh = 0.02;

    void initializeVehicles();
    vehicle setUpNewVehicle(int j);
    void assignVehicles();
    bool withinFrame(vehicle t);
    double predictKalman(Rect rect, int j);
    int min_row;
    int min_column;
    double findMinElement(vector<vector<double> > distanceMatrix);
    void displayResults();






};

#endif // VIDEOPROCESSINGTHREAD_H
