#include "videoprocessingthread.h"

videoProcessingThread::videoProcessingThread(QObject *parent) :
    QThread(parent)
{
}

void videoProcessingThread::run()
{
    this->init();
    int cnt = 0;
    while(true)
    {
        ++cnt;
        if(this->stop) {destroyAllWindows();break;}
        this->readNextFrame();
        if(this->frame.empty()) break;
        //pMOG2->operator()(frame, background);
        //this->bgs->process(this->frame,this->background,this->bgModel);
        this->fbgs->process(this->frame,this->background,this->bgModel);
        if(!this->background.empty())
        {
            this->detectObjects();
            emit showResults(this->frame);
            if(cnt == 2)
            {
                this->initializeVehicles();
            } else {
                this->assignVehicles();
                this->displayResults();
            }
            this->wtr.write(this->frame);
        }

    }
    this->wtr.release();
    destroyAllWindows();
}

void videoProcessingThread::init()
{
    this->bgs = new WeightedMovingVarianceBGS;
    this->pMOG2 = new BackgroundSubtractorMOG2(5, 16, false); //MOG2 approach
    this->fbgs = new FrameDifferenceBGS;
    this->capture.open(this->videoName);
    this->width = this->capture.get(CV_CAP_PROP_FRAME_WIDTH);
    this->height = this->capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    this->fps = this->capture.get(CV_CAP_PROP_FPS);
    if (this->fps == 29) this->fps = 30;
    this->totalFrameNumber = this->capture.get(CV_CAP_PROP_FRAME_COUNT);
    this->wtr.open("C:/Users/Meng/Documents/Qt Projects/AutoCarCounting/output.avi",CV_FOURCC('X','V','I','D'),15, cv::Size(this->width, this->height));
}

void videoProcessingThread::readNextFrame()
{
    int framesToSkip = (this->fps - this->processingFPS) / this->processingFPS;
    for(int i = 0; i <= framesToSkip; i++)
    {
        this->capture.read(this->frame);
        if(this->frame.empty())break;
    }
}

void videoProcessingThread::detectObjects()
{
    cv::morphologyEx(this->background, this->img_closing, MORPH_CLOSE, this->element);
    Mat threshold_output;	//contours map
    cv::blur( img_closing, img_closing, cv::Size(3,3) );
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    cv::threshold( img_closing, threshold_output, this->contour_thresh, 255, THRESH_BINARY );
    cv::findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    vector<vector<Point> > contours_poly( contours.size() );
    this->boundRect.resize(contours.size());
    //Fit rect
    for( int n = (int)contours.size() - 1; n >= 0; n-- )
    {
        approxPolyDP( Mat(contours[n]), contours_poly[n], 3, true );
        boundRect[n] = cv::boundingRect( Mat(contours_poly[n]) );
        if(boundRect[n].area() <= img_closing.cols * img_closing.rows * this->rect_delete_thresh
                && boundRect[n].area() < img_closing.cols * img_closing.rows * 0.4)
        {
            boundRect.erase(boundRect.begin() + n);
        }
    }

}

void videoProcessingThread::initializeVehicles(){
    for (int j = 0; j < this->boundRect.size(); j ++)
    {
        vehicle v = setUpNewVehicle(j);
        v.start = 0;
        v.stop = -1;
        this->vehicles.push_back(v);
    }

}

vehicle videoProcessingThread::setUpNewVehicle(int j){
    vehicle v;
    v.id = ++totalNumberOfVehicles;
    v.present = true;
    v.totalVisibleCount = 1;
    v.invisibleCount = 0;
    v.KF.init(6, 2, 0);
    v.KF.transitionMatrix = *(Mat_<float>(6, 6) << 1,0,1,0,0.5,0,
                                   0,1,0,1,0,0.5,
                                   0,0,1,0,1,0,
                                   0,0,0,1,0,1,
                                   0,0,0,0,1,0,
                                   0,0,0,0,0,1
                                   );
    v.KF.measurementMatrix = *(Mat_<float>(2, 6) << 1,0,1,0,0.5,0,
                                    0,1,0,1,0,0.5
                                    );
    v.KF.statePre.at<float>(0) = this->boundRect[j].x + 0.5 * this->boundRect[j].width;
    v.KF.statePre.at<float>(1) = this->boundRect[j].y + 0.5 * this->boundRect[j].height;
    v.KF.statePre.at<float>(2) = 0;
    v.KF.statePre.at<float>(3) = 0;
    v.KF.statePre.at<float>(4) = 0;
    v.KF.statePre.at<float>(5) = 0;
    setIdentity(v.KF.measurementMatrix);
    setIdentity(v.KF.processNoiseCov, Scalar::all(1e-4));
    setIdentity(v.KF.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(v.KF.errorCovPost, Scalar::all(.1));
    v.rect = this->boundRect[j];
    return v;
}

void videoProcessingThread::assignVehicles(){
    Mat_<float> measurement(2,1);
    bool newDetections[100];
    bool unassignedTracks[100];
    vector<int> vehiclesPresent;

    for(int i = 0; i < this->boundRect.size(); i++) newDetections[i] = false;
    for(int i = 0; i < this->vehicles.size(); i++)
    {
        unassignedTracks[i] = false;
        if (vehicles[i].present)
        {
            vehiclesPresent.push_back(i);
        }
    }

    vector<int> newDetectionsIndex;
    for(int i = 0; i < this->boundRect.size(); i++)
    {
        newDetectionsIndex.push_back(i);
    }
    //Calculate distance matrix: Row: detections, Column: Present tracks
    vector<vector<double> > distanceMatrix;
    for (int i = 0; i < this->boundRect.size(); i++)
    {
        vector<double> distanceRow;
        for (int j = 0; j < vehiclesPresent.size(); j++)
        {
            double per = predictKalman(this->boundRect[i], vehiclesPresent[j]);
            distanceRow.push_back(per);
        }
        distanceMatrix.push_back(distanceRow);
    }

    while(!distanceMatrix.empty() && !vehiclesPresent.empty())
    {
        double min_element = findMinElement(distanceMatrix);
        int trackClosestIndex = vehiclesPresent[min_column];
        //cout << "THIS IS MIN_ELEMENT = " << min_element << " closest index" << trackClosestIndex << endl;
        int detectClosestIndex = newDetectionsIndex[min_row];
        //vehicles[trackClosestIndex].detection = this->subDetections[detectClosestIndex];
        vehicles[trackClosestIndex].rect = this->boundRect[detectClosestIndex];
        vehicles[trackClosestIndex].present = true;
        vehicles[trackClosestIndex].totalVisibleCount ++;
        vehicles[trackClosestIndex].invisibleCount = 0;
        measurement(0) = this->boundRect[detectClosestIndex].x + 0.5 * this->boundRect[detectClosestIndex].width;
        measurement(1) = this->boundRect[detectClosestIndex].y + 0.5 * this->boundRect[detectClosestIndex].height;
        Mat estimated = vehicles[trackClosestIndex].KF.correct(measurement);
        Point statePt(estimated.at<float>(0), estimated.at<float>(1));
        circle(this->frame, statePt, 10, cv::Scalar(0, 255, 0), -1, 8, 0);
        circle(this->frame, cv::Point(measurement(0), measurement(1)), 10, cv::Scalar(255, 0, 0), -1, 8, 0);
        newDetections[detectClosestIndex] = true;
        unassignedTracks[trackClosestIndex] = true;
        //Delete row
        distanceMatrix.erase(distanceMatrix.begin() + min_row);
        //Delete column
        newDetectionsIndex.erase(newDetectionsIndex.begin() + min_row);
        vehiclesPresent.erase(vehiclesPresent.begin() + min_column);
        if (!distanceMatrix.empty())
        {
            for(int i = 0; i < distanceMatrix.size(); i++)
            {
                distanceMatrix[i].erase(distanceMatrix[i].begin() + min_column);
            }
        }

    }

    //If we have more visible tracks than detections, either he left the scene, or he was static (nearly)
    vector<int> vehiclesToDelete;
    for(int i = 0; i < vehicles.size(); i++)
    {
        if(unassignedTracks[i] == false && vehicles[i].present)
        {
            //If his predicted point is within the scene, regard him as still in the scene;
            //Give him 100 frames of time to move a little bit.
            //If he didn't seize this opportunity, sorry, but he has to be regarded as out of the scene.
            if (withinFrame(vehicles[i])){
                vehicles[i].invisibleCount ++;
                if (vehicles[i].invisibleCount > 1000)
                {
                    vehicles[i].present = false;
                    //vehicles[i].stop = this->cnt_iter;

                    vehicles[i].totalVisibleCount = 0;
                } else {
                    vehicles[i].present = true;
                    vehicles[i].totalVisibleCount ++;
                    unassignedTracks[i] = true;
                }

            } else {
                //If his predicted point is out of the scene, regard him as gone.
                vehicles[i].present = false;
                vehiclesToDelete.push_back(i);
                //vehicles[i].stop = this->cnt_iter;
                //this->logToBuffer(i);
                vehicles[i].totalVisibleCount = 0;
                unassignedTracks[i] = false;
            }
        }
    }
    if(vehiclesToDelete.size()>0){
        for(int i = vehiclesToDelete.size() - 1; i >=0; i--)
        {
            vehicles.removeAt(vehiclesToDelete[i]);
        }
    }

    //Check if we have more detections than tracks, the new guy can be someone who showed up before,
    //or someone who never showed up
    for(int i = 0; i < this->boundRect.size() ; i++)
    {
        //If a guy was never here before, but now he showed up: create a new track for this guy
        if (newDetections[i] == false)
        {
            vehicle v = setUpNewVehicle(i);
            //v.start = this->cnt_iter;
            vehicles.push_back(v);
        }
    }
}

bool videoProcessingThread::withinFrame(vehicle t)
{
    int margin = 100;
    if (t.KF.statePost.at<float>(0) > margin && t.KF.statePost.at<float>(0) < this->width - margin
            && t.KF.statePost.at<float>(1) > margin && t.KF.statePost.at<float>(1) < this->height - margin)
    {
        return true;
    } else {
        return false;
    }
}

double videoProcessingThread::predictKalman(Rect rect, int j)
{
    Mat_<float> measurement(2,1);
    Mat prediction = vehicles[j].KF.predict();
    Point predictPt(prediction.at<float>(0), prediction.at<float>(1));
    measurement(0) = rect.x + 0.5 * rect.width;
    measurement(1) = rect.y + 0.5 * rect.height;
    Point measPt(measurement(0), measurement(1));
    //Calculate the distance between predicted point and measurement
    double distance =  pow(pow(predictPt.x - measurement(0), 2) + pow(predictPt.y - measurement(1), 2), 0.5);
    //Calculate the percentage of distance to frame diagonal.
    double per = distance / pow(pow(this->frame.rows, 2) + pow(this->frame.cols, 2), 0.5);
    return per;
}

double videoProcessingThread::findMinElement(vector<vector<double> > distanceMatrix)
{
    double min_element = 9999;
    for(int i = 0; i < distanceMatrix.size(); i++)
    {
        for(int j = 0; j < distanceMatrix[i].size(); j++)
        {
            if (distanceMatrix[i][j] < min_element)
            {
                min_element = distanceMatrix[i][j];
                min_row = i;
                min_column = j;
            }
        }
    }
    return min_element;
}

void videoProcessingThread::displayResults()
{

    for(int i = 0; i < vehicles.size(); i++)
    {
        if(vehicles[i].present == true)
        {
            //cout << "operator" << i << endl;
            rectangle( this->frame, vehicles[i].rect.tl(), vehicles[i].rect.br(), cv::Scalar(204,102,0), 2, 8, 0 );
            rectangle( this->frame, cv::Point(vehicles[i].rect.x, vehicles[i].rect.height - 30 + vehicles[i].rect.y), vehicles[i].rect.br(), cv::Scalar(204,102,0), -1, 8, 0);
            std::ostringstream s;
            s << "Vehicle " << vehicles[i].id;
            //s << vehicles[i].id;
            string text = s.str();
            putText(this->frame, text, Point(vehicles[i].rect.x, vehicles[i].rect.y + vehicles[i].rect.height - 10),  FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1, 8, false);
        }
    }

}
