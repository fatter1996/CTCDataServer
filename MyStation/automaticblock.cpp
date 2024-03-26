#include "automaticblock.h"

AutomaticBlock::AutomaticBlock(QObject *parent) : QObject(parent)
{
    strJuncXHD = "";
    codejuncXHD = 0;

    //接发车箭头状态
    arrowState = 0;

    //按钮状态-总辅助（0弹起,1按下）
    btnStateZFZ = 0;
    //按钮状态-发辅助（0弹起,1按下）
    btnStateFFZ = 0;
    //按钮状态-接辅助（0弹起,1按下）
    btnStateJFZ = 0;
    //按钮状态-改方（0弹起,1按下）
    btnStateGF = 0;

    //指示灯的状态-监督区间
    lightStateJDQJ = 0;
    //指示灯的状态-区间灯
    lightStateQJD = 0;
    //指示灯的状态-辅助灯
    lightStateFZD = 0;
    //指示灯的状态-区轨灯
    lightStateQGD = 0;
    //指示灯的状态-允许发车灯
    lightStateYXFCD = 0;

    sectionLogicCheckState = 0;
}
