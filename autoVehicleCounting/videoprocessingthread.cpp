#include "videoprocessingthread.h"
#include <iomanip>      // std::setprecision

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
            emit showAerial(this->aerial);
            if(cnt == 2)
            {
                this->initializeVehicles();
            } else {

                this->assignVehicles();

                this->displayResults();

            }
            this->wtr.write(this->frame);
            /*if(cnt == 15)
            {
                imwrite("result.png",this->background);
                imwrite("img_closing.png",this->img_closing);
                imwrite("track.png",this->frame);
            }*/
        }

    }
    this->wtr.release();
    destroyAllWindows();
}

void videoProcessingThread::init()
{
    colors << cv::Scalar(93,130,150) << cv::Scalar(150, 113, 93) << cv::Scalar(93, 150, 118) << cv::Scalar(150, 93, 125)
              << cv::Scalar(179, 146, 89) << cv::Scalar(89, 122, 179) << cv::Scalar(179, 89, 90) << cv::Scalar(89, 179, 178)
                 << cv::Scalar(155, 155, 155) << cv::Scalar(140, 97, 126) ;
    this->tMatrix[0][0] = 0.0017273867;
    this->tMatrix[0][1] = 0.000639764;
    this->tMatrix[0][2] = 0.721077348;
    this->tMatrix[1][0] = 0.0017145203;
    this->tMatrix[1][1] = 0.008397438347;
    this->tMatrix[1][2] = -0.69277943536;
    this->tMatrix[2][0] = 0.00000068245336377;
    this->tMatrix[2][1] = 0.00002001935931552;
    this->tMatrix[2][2] = 0.005221229;
    this->maneuvers << "West Left Turn" << "West Straight" << "West Right Turn" << "South Left Turn" << "South Straight"
                       << "South Right Turn" << "East Left Turn" << "East Straight" << "East Right Turn" <<
                          "North Left Turn" << "North Straight" << "North Right Turn";
    //>> t = [0.00172738676563552,0.000639764294494792,0.721077348549566;0.00171452030901329,0.00839743834713821,-0.692779435360021;6.82453363774232e-07,2.00193593155218e-05,0.00522122932491429]
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
    this->aerial = imread("C:\\Users\\Meng\\Documents\\Qt Projects\\AutoCarCounting\\aerial.png");
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


    this->rotatedRect.resize(contours.size());
    //Fit rect
    for( int n = (int)contours.size() - 1; n >= 0; n-- )
    {
        approxPolyDP( Mat(contours[n]), contours_poly[n], 3, true );
        boundRect[n] = cv::boundingRect( Mat(contours_poly[n]) );
        this->rotatedRect[n] = minAreaRect( Mat(contours_poly[n]) );
        if(boundRect[n].area() <= img_closing.cols * img_closing.rows * this->rect_delete_thresh
                && boundRect[n].area() < img_closing.cols * img_closing.rows * 0.4)
        {
            boundRect.erase(boundRect.begin() + n);
            this->rotatedRect.erase(this->rotatedRect.begin() + n);
        }
        //if(rotatedRect[n].area() <= img_closing.cols * img_closing.rows * this->rect_delete_thresh
        //        && rotatedRect[n].area() < img_closing.cols * img_closing.rows * 0.4)
        //{
        //    this->rotatedRect.erase(this->rotatedRect.begin() + n);
        //}
    }

    int rotatedRectSize = this->rotatedRect.size();
    for(int i = rotatedRectSize - 1; i >=0; i--)
    {
        Point2f vertices[4];
        this->rotatedRect[i].points(vertices);
        double width = pow(pow(vertices[0].x - vertices[1].x,2) + pow(vertices[0].y - vertices[1].y,2),0.5);
        double height = pow(pow(vertices[0].x - vertices[3].x,2) + pow(vertices[0].y - vertices[3].y,2),0.5);
        double longer = max(width,height);
        double shorter = min(width,height);
        if(longer / shorter > 4)
        {
            this->rotatedRect.erase(this->rotatedRect.begin() + i);
        }
    }


}

void videoProcessingThread::initializeVehicles(){
    for (int j = 0; j < this->rotatedRect.size(); j ++)
    //for (int j = 0; j < this->boundRect.size(); j ++)
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
    //v.rect = this->boundRect[j];
    v.rRect = this->rotatedRect[j];
    return v;
}

cv::Point videoProcessingThread::transformToAerial(int x1, int y1)
{
    double x2 = x1 * 1.0 * this->tMatrix[0][0] + y1 * 1.0 * this->tMatrix[0][1] + 1.0 * this->tMatrix[0][2];
    double y2 = x1 * 1.0 * this->tMatrix[1][0] + y1 * 1.0 * this->tMatrix[1][1] + 1.0 * this->tMatrix[1][2];
    double norm = x1 * 1.0 * this->tMatrix[2][0] + y1 * 1.0 * this->tMatrix[2][1] + 1.0 * this->tMatrix[2][2];
    x2 /= norm;
    y2 /= norm;
    return cv::Point(x2,y2);
}

void videoProcessingThread::assignVehicles(){
    Mat_<float> measurement(2,1);
    bool newDetections[100];
    bool unassignedTracks[100];
    vector<int> vehiclesPresent;

    for(int i = 0; i < this->rotatedRect.size(); i++) newDetections[i] = false;
    for(int i = 0; i < this->vehicles.size(); i++)
    {
        unassignedTracks[i] = false;
        if (vehicles[i].present)
        {
            vehiclesPresent.push_back(i);
        }
    }

    vector<int> newDetectionsIndex;
    for(int i = 0; i < this->rotatedRect.size(); i++)
    {
        newDetectionsIndex.push_back(i);
    }
    //Calculate distance matrix: Row: detections, Column: Present tracks
    vector<vector<double> > distanceMatrix;
    for (int i = 0; i < this->rotatedRect.size(); i++)
    {
        vector<double> distanceRow;
        for (int j = 0; j < vehiclesPresent.size(); j++)
        {
            double per = predictKalman(this->rotatedRect[i], vehiclesPresent[j]);
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
        vehicles[trackClosestIndex].rRect = this->rotatedRect[detectClosestIndex];
        vehicles[trackClosestIndex].present = true;
        vehicles[trackClosestIndex].totalVisibleCount ++;
        vehicles[trackClosestIndex].invisibleCount = 0;
        Point2f vertices[4];
        this->rotatedRect[detectClosestIndex].points(vertices);

        measurement(0) = vertices[0].x * 0.5 + vertices[2].x * 0.5; //cols
        measurement(1) = vertices[0].y * 0.5 + vertices[2].y * 0.5; //rows
        //if(vehicles[trackClosestIndex].trajectory.size() == 10)
        //{
        //    vehicles[trackClosestIndex].trajectory.pop_front();
        //}
        vehicles[trackClosestIndex].trajectory.push_back(cv::Point(measurement(0),measurement(1)));
        vehicles[trackClosestIndex].aerialTrajectory.push_back(transformToAerial(measurement(0),measurement(1)));
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
                    //qDebug() << vehicles[i];

                    if(vehicles[i].aerialTrajectory.size()>5)
                    {
                        emit maneuverCount(vehicles[i].getManeuver());
                    }
                    vehicles[i].totalVisibleCount = 0;
                } else {
                    vehicles[i].present = true;
                    vehicles[i].totalVisibleCount ++;
                    unassignedTracks[i] = true;
                }

            } else {
                //If his predicted point is out of the scene, regard him as gone.
                vehicles[i].present = false;
                if(vehicles[i].aerialTrajectory.size()>5)
                {
                    emit maneuverCount(vehicles[i].getManeuver());
                }
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
    for(int i = 0; i < this->rotatedRect.size() ; i++)
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

double videoProcessingThread::predictKalman(RotatedRect rect, int j)
{
    Mat_<float> measurement(2,1);
    Mat prediction = vehicles[j].KF.predict();
    Point predictPt(prediction.at<float>(0), prediction.at<float>(1));
    Point2f vertices[4];
    rect.points(vertices);
    measurement(0) = vertices[0].x * 0.5 +  vertices[2].x * 0.5;
    measurement(1) = vertices[0].y * 0.5 +  vertices[2].y * 0.5;
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
            int colorIndex = vehicles[i].id % 10;
            qDebug() << colorIndex;
            //cout << "operator" << i << endl;
            Point2f rect_points[4]; vehicles[i].rRect.points( rect_points );
            for( int j = 0; j < 4; j++ )
                 line( this->frame, rect_points[j], rect_points[(j+1)%4], this->colors.at(colorIndex), 2, 8 );
            //rectangle( this->frame, vehicles[i].rRect.tl(), vehicles[i].rRect.br(), cv::Scalar(204,102,0), 2, 8, 0 );
            //rectangle( this->frame, cv::Point(vehicles[i].rect.x, vehicles[i].rect.height - 30 + vehicles[i].rect.y), vehicles[i].rect.br(), cv::Scalar(204,102,0), -1, 8, 0);
            std::ostringstream s;
            std::ostringstream t;
            s << "Vehicle " << vehicles[i].id;
            //s << vehicles[i].id;
            if(vehicles[i].trajectory.size() > 5)
            {

                s << "  M: "<< this->maneuvers.at(vehicles[i].getManeuver()-1).toStdString();
                double speed = 0;
                int sizeOfTrajectory = vehicles[i].trajectory.size();
                for(int k = sizeOfTrajectory - 1; k >= sizeOfTrajectory - 4; k--)
                {
                    int xDiff = vehicles[i].aerialTrajectory.at(k).x - vehicles[i].aerialTrajectory.at(k-1).x;
                    int yDiff = vehicles[i].aerialTrajectory.at(k).y - vehicles[i].aerialTrajectory.at(k-1).y;
                    double pixelDistance = pow(xDiff*xDiff + yDiff*yDiff,0.5);
                    double actualDistance = pixelDistance * 0.403;
                    speed += actualDistance * this->processingFPS * 0.681818; //Calculate speed in MPH
                }
                speed/=4;

                t << setprecision(3)<<  speed << " MPH";
            }
            string speedText = t.str();
            string text = s.str();
            putText(this->frame, speedText, Point(rect_points[1].x, rect_points[1].y), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, 8, false);
            putText(this->frame, text, Point(rect_points[0].x, rect_points[0].y),  FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0), 1, 8, false);
            for(int j = 0; j < vehicles[i].trajectory.size(); j++)
                circle(this->frame, vehicles[i].trajectory.at(j), 5, this->colors.at(colorIndex), -1, 8, 0);

            for(int j = 0; j < vehicles[i].trajectory.size(); j++)
                circle(this->aerial, vehicles[i].aerialTrajectory.at(j), 5, this->colors.at(colorIndex), -1, 8, 0);





        }
    }

}
