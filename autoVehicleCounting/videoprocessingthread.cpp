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
        if(boundRect[n].area() <= img_closing.cols * img_closing.rows * this->rect_delete_thresh)
        {
            boundRect.erase(boundRect.begin() + n);
        }
    }
    for(int i = 0; i < boundRect.size(); i++)
    {
        rectangle(this->frame,boundRect[i],cv::Scalar(0,0,255),2,8,0);
    }
}
