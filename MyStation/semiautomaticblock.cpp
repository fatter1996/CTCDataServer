#include "semiautomaticblock.h"

SemiAutomaticBlock::SemiAutomaticBlock(QObject *parent) : QObject(parent)
{
    strJuncXHD = "";
    codejuncXHD = 0;
    code = 0;

    //接车箭头状态
    arrowStateReach = 0;
    //发车箭头状态
    arrowStateDepart = 0;
}
