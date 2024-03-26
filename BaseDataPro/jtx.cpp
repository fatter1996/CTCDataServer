#include "jtx.h"

CJTX::CJTX()
{
    p1.setX(1);
    p1.setY(1);
    p2.setX(1);
    p2.setY(1);
    p3.setX(1);
    p3.setY(1);
    p4.setX(1);
    p4.setY(1);
}

//成员功能函数实现
void CJTX::JTXInit()
{
    p1Const.setX(p1.x());
    p1Const.setY(p1.y());
    p2Const.setX(p2.x());
    p2Const.setY(p2.y());
    p3Const.setX(p3.x());
    p3Const.setY(p3.y());
    p4Const.setX(p4.x());
    p4Const.setY(p4.y());
    pCenterConst.setX(pCenter.x());
    pCenterConst.setY(pCenter.y());
}
void CJTX::Draw(QPainter *painter,long nElapsed,float nDiploid,QPoint offset,int type)
{
    //坐标变换 2021.1.11 BJT
    p1.setX(p1Const.x() * nDiploid+offset.x()*nDiploid);
    p1.setY(p1Const.y() * nDiploid+offset.y()*nDiploid);
    p2.setX(p2Const.x() * nDiploid+offset.x()*nDiploid);
    p2.setY(p2Const.y() * nDiploid+offset.y()*nDiploid);
    p3.setX(p3Const.x() * nDiploid+offset.x()*nDiploid);
    p3.setY(p3Const.y() * nDiploid+offset.y()*nDiploid);
    p4.setX(p4Const.x() * nDiploid+offset.x()*nDiploid);
    p4.setY(p4Const.y() * nDiploid+offset.y()*nDiploid);
    pCenter.setX(pCenterConst.x() * nDiploid+offset.x()*nDiploid);
    pCenter.setY(pCenterConst.y() * nDiploid+offset.y()*nDiploid);

    //反走样,防止出现锯齿状线条
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::white,1));

    if (getType() == 41)
    {
        if(getJTXType() == 1)
        {
            painter->drawLine(p1, QPointF(p1.x()-5*nDiploid,p1.y()));
            painter->drawLine(p2, QPointF(p2.x()-5*nDiploid,p2.y()));
            painter->drawLine(p1, p2);
        }
        painter->drawLine(pCenter, QPointF(p1.x(),pCenter.y()));
        painter->drawLine(p4, QPointF(pCenter.x(),p4.y()));
    }
    else
    {
        if(getJTXType() == 1)
        {
            painter->drawLine(p1, QPointF(p1.x()+5*nDiploid,p1.y()));
            painter->drawLine(p2, QPointF(p2.x()+5*nDiploid,p2.y()));
            painter->drawLine(p1, p2);
        }
        painter->drawLine(pCenter, QPointF(p1.x(),pCenter.y()));
        painter->drawLine(p4, QPointF(pCenter.x(),p4.y()));
    }

}
void CJTX::setVollover(QPoint pt_Base)
{
    double x1=0;
    double x2=0;
    double y1=0;
    double y2=0;
    double y3=0;
    if(getType() == 41)
    {
        setType(42);
    }
    else if(getType() == 42)
    {
        setType(41);
    }
    setTextRect(QRectF(pt_Base.x() - (getTextRect().right()-pt_Base.x()),pt_Base.y() - (getTextRect().bottom()-pt_Base.y()),getTextRect().width(),getTextRect().height()));
    x1=getp1().x();
    x2=getp2().x();
    y1=getp1().y();
    y2=getp2().y();
    setp1(QPointF(pt_Base.x() - (x1-pt_Base.x()),pt_Base.y() - (y2 - pt_Base.y())));
    setp2(QPointF(pt_Base.x() - (x2-pt_Base.x()),pt_Base.y() - (y1 - pt_Base.y())));
    setp4(QPointF(pt_Base.x() - (getp4().x()-pt_Base.x()),pt_Base.y() - (y1 - pt_Base.y())));
    setp3(QPointF(pt_Base.x() - (getp3().x()-pt_Base.x()),pt_Base.y() - (getp3().y() - pt_Base.y())));
    setCenterPt(QPointF(pt_Base.x() - (getCenterPt().x()-pt_Base.x()),pt_Base.y() - (getCenterPt().y() - pt_Base.y())));
}
unsigned int CJTX::getDevType()
{
    return Dev_JTX;
}
//成员变量封装函数实现
void CJTX::setJTXType(unsigned int nType)
{
    m_nJTXType = nType;
}
unsigned int CJTX::getJTXType()
{
    return m_nJTXType;
}
void CJTX::setp1(QPointF pt)
{
    p1=pt;
    p1Const=pt;
}
QPointF CJTX::getp1()
{
    return p1Const;
}
void CJTX::setp2(QPointF pt)
{
    p2=pt;
    p2Const=pt;
}
QPointF CJTX::getp2()
{
    return p2Const;
}
void CJTX::setp3(QPointF pt)
{
    p3=pt;
    p3Const=pt;
}
QPointF CJTX::getp3()
{
    return p3Const;
}
void CJTX::setp4(QPointF pt)
{
    p4=pt;
    p4Const=pt;
}
QPointF CJTX::getp4()
{
    return p4Const;
}
