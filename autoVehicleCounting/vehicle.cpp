#include "vehicle.h"
#include <qmath.h>
#include <QDebug>

#define RANGE 10
#define RANGE2 35
int vehicle::getManeuver()
{
    int sizeOftra = this->aerialTrajectory.size();
    int xDiff = this->aerialTrajectory.at(sizeOftra - 1).x - this->aerialTrajectory.at(0).x;
    int yDiff = this->aerialTrajectory.at(sizeOftra - 1).y - this->aerialTrajectory.at(0).y;
    yDiff *= -1;
    double angle = qAtan2(yDiff,xDiff) * 180 / 3.141592653;

    //qDebug() << "X Y DIFF: " <<  xDiff << " " << yDiff << " " << angle;

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
        return 11;
    }

    if (angle > 45 - RANGE2 && angle < 45 + RANGE2)
    {
        if(this->aerialTrajectory.at(0).x < 220)
        {
            return 1;
        }else{
            return 6;
        }
    }
    if (angle > -45 - RANGE2 && angle < -45 + RANGE2)
    {
        if(this->aerialTrajectory.at(0).x < 220)
        {
            return 3;
        }else{
            return 10;
        }
    }
    if (angle > 135 - RANGE2 && angle < 135 + RANGE2)
    {
        if(this->aerialTrajectory.at(0).x < 170)
        {
            return 9;
        }else{
            return 4;
        }
    }

    if (angle > -135 - RANGE2 && angle < -135 + RANGE2)
    {
        if(this->aerialTrajectory.at(0).y < 170)
        {
            return 12;
        }else{
            //qDebug() << xDiff << " yDiff " << yDiff << " " <<angle;
            return 7;
        }
    }
    qDebug() << angle;
    return 1;

}
