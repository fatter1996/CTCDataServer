#include "limitspeed.h"

LimitSpeed::LimitSpeed(QObject *parent) : QObject(parent)
{
    cmdNum = 0;
    speed = 0;
    bSet = false;
    bSetStatus = false;
    bStartNow = false;
}
