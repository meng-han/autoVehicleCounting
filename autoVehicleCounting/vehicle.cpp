#include "vehicle.h"
#include <qmath.h>
#include <QDebug>

#define RANGE 15
#define RANGE2 30
int vehicle::getManeuver()
{
    int sizeOftra = this->aerialTrajectory.size();

    int oneFourthIndex = floor((sizeOftra-1) * 0.25);
    int threeFourthIndex = floor((sizeOftra - 1)*0.75);
    int xDiff = this->aerialTrajectory.at(sizeOftra-1).x - this->aerialTrajectory.at(oneFourthIndex).x;
    int yDiff = this->aerialTrajectory.at(sizeOftra-1).y - this->aerialTrajectory.at(oneFourthIndex).y;
    //int xDiff = this->aerialTrajectory.at(sizeOftra - 1).x - this->aerialTrajectory.at(0).x;
    //int yDiff = this->aerialTrajectory.at(sizeOftra - 1).y - this->aerialTrajectory.at(0).y;
    yDiff *= -1;
    double angle = qAtan2(yDiff,xDiff) * 180 / 3.141592653;

    //qDebug() << this->id << " X Y DIFF: " << this->aerialTrajectory.at(0).x << " " << this->aerialTrajectory.at(0).y <<  xDiff << " " << yDiff << " " << angle;
    /*if(this->id == 34)
    {
        for(int i = 0; i < this->aerialTrajectory.size(); i++)
            qDebug() << "VEHICLE 34 " << this->aerialTrajectory.at(i).x << " " << this->aerialTrajectory.at(i).y;
    }*/
    if ((angle >= 180 - RANGE && angle <= 180) || (angle >= -180 && angle <= -180 + RANGE))
    {
        return 8;
    }

    if (angle >= 0 - RANGE && angle <= 0 + RANGE)
    {
        return 2;
    }

    if (angle >= 90 - RANGE && angle <= 90 + RANGE)
    {

        return 5;
    }

    if (angle >= -90 - RANGE && angle <= -90 + RANGE)
    {
        //qDebug() << this->id << " " << angle;
        return 11;
    }

    if (angle > 45 - RANGE2 && angle < 45 + RANGE2)
    {
        if(this->aerialTrajectory.at(oneFourthIndex).x < 211)
        {
            return 1;
        }else{
            return 6;
        }
    }
    if (angle > -45 - RANGE2 && angle < -45 + RANGE2)
    {
        if(this->aerialTrajectory.at(oneFourthIndex).y > 179)
        {
            return 3;
        }else{
            return 10;
        }
    }
    if (angle > 135 - RANGE2 && angle < 135 + RANGE2)
    {
        if(this->aerialTrajectory.at(oneFourthIndex).y < 179)
        {
            return 9;
        }else{
            return 4;
        }
    }

    if (angle > -135 - RANGE2 && angle < -135 + RANGE2)
    {
        if(this->aerialTrajectory.at(oneFourthIndex).x < 210)
        {
            return 12;
        }else{
            //qDebug() << xDiff << " yDiff " << yDiff << " " <<angle;
            return 7;
        }
    }
    //qDebug() << angle;
    return 1;

}
