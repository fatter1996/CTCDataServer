#include "xhbtn.h"

XhBtn::XhBtn(QObject *parent) : QObject(parent)
{
    m_nANTYPE = 0;//信号按钮类型（调车按钮/列车按钮）
    m_pCenter = QPoint(0,0);//按钮中心点
    m_rectBtn = QRect(-1,-1,-1,-1);//按钮轮廓坐标
    m_textRect = QRect(-1,-1,-1,-1);
    m_RangeVisible = false;
    m_nBtnIsDown = false;
    m_nBtnFlash = false;
    m_nFuncLockState = false;
    m_nCode = -1;
    m_bNameUp = false;
}
