#include "axlecounter.h"

AxleCounter::AxleCounter(QObject *parent) : QObject(parent)
{
    strJuncXHD = "";
    codejuncXHD = 0;

    //按钮状态-复零（0弹起,1按下）
    btnStateFL = 0;
    //按钮状态-使用（0弹起,1按下）
    btnStateSY = 0;
    //按钮状态-停止（0弹起,1按下）
    btnStateTZ = 0;

    //指示灯的状态-复零灯
    lightStateFL = 0;
    //指示灯的状态-使用灯
    lightStateSY = 0;
    //指示灯的状态-停止灯
    lightStateTZ = 0;
    //指示灯的状态-区轨灯
    lightStateQG = 0;

    lightStateQJ = 0;
    lightStateJZBJ = 0;
}
