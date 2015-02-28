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

        this->bgs->process(this->frame,this->background,this->bgModel);
        if(cnt > 2) emit showResults(this->background);
    }
}

void videoProcessingThread::init()
{
    this->bgs = new WeightedMovingVarianceBGS;
    this->capture.open(this->videoName);
    this->width = this->capture.get(CV_CAP_PROP_FRAME_WIDTH);
    this->height = this->capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    this->fps = this->capture.get(CV_CAP_PROP_FPS);
    if (this->fps == 29) this->fps = 30;
    this->totalFrameNumber = this->capture.get(CV_CAP_PROP_FRAME_COUNT);
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
