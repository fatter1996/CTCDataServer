#include <QtDebug>
#include "gddc.h"
#include "GlobalHeaders/Global.h"

CGDDC::CGDDC()
{
    m_nQDCode = 0xffff;    //关联区段设备编号
    m_nCxjy = 0;      //超限绝缘节
    m_nZ = 32;         //折点
    m_nJyj = 56;       //绝缘节
    p1.setX(1);
    p1.setY(1);
    p2.setX(1);
    p2.setY(1);
    p3.setX(1);
    p3.setY(1);
    p4.setX(1);
    p4.setY(1);
    p5.setX(1);
    p5.setY(1);
    p6.setX(1);
    p6.setY(1);
    p12.setX(1);
    p12.setY(1);
    p34.setX(1);
    p34.setY(1);
    p56.setX(1);
    p56.setY(1);
    pz12.setX(1);
    pz12.setY(1);
    pz34.setX(1);
    pz34.setY(1);
    pz56.setX(1);
    pz56.setY(1);
    p1Const.setX(1);
    p1Const.setY(1);
    p2Const.setX(1);
    p2Const.setY(1);
    p3Const.setX(1);
    p3Const.setY(1);
    p4Const.setX(1);
    p4Const.setY(1);
    p5Const.setX(1);
    p5Const.setY(1);
    p6Const.setX(1);
    p6Const.setY(1);
    p12Const.setX(10);
    p12Const.setY(1);
    p34Const.setX(1);
    p34Const.setY(1);
    p56Const.setX(1);
    p56Const.setY(1);
    pz12Const.setX(1);
    pz12Const.setY(1);
    pz34Const.setX(1);
    pz34Const.setY(1);
    pz56Const.setX(1);
    pz56Const.setY(1);
    isMainGD = false;
    m_nQ = -1;        //岔前设备
    m_nD = -1;        //定位设备
    m_nF = -1;        //反位设备
//    m_nCQ_Code=65535;  //岔前关联设备
//    m_nDW_Code=65535;  //定位关联设备
//    m_nFW_Code=65535;  //反位关联设备
    m_nDCWZ = DCDW;
//    m_nDCWZ_Old = DCDW;
    p34C.setX(1);
    p34C.setY(1);
    p56C.setX(1);
    p56C.setY(1);  //用于存放岔心关键坐标，不从数据读取，而是再init中计算得到
    p34CConst.setX(1);
    p34CConst.setY(1);
    p56CConst.setX(1);
    p56CConst.setY(1);
//    m_nSDDCCode = 0xffff;
//    m_TrainNum = "";
//    m_TrainDown = false;
    m_bDrawGD = true;
//    isSDDC=false;
//    m_nOneToMore=false;
//    m_nSameQDDCCode=0xffff;
//    isWGDW=false;
//    GLB_DCCQ=0;    //公里标
//    GLB_DCCX=0;    //公里标
//    GLB_DCDW=0;    //公里标
//    GLB_DCFW=0;    //公里标
//    isTSDC=false;
//    CarrierFrequency="2300-2";
//    Dir_DMH=LEFT;
//    isDrawDPZPFlag=true;
    isFLBL_CQ=false;
    isFLBL_DW=false;
    isFLBL_FW=false;
    m_bToolTip=false;
    m_strToolTip="";
    isDisDCWZ=false;
    isDisLight=false;
    isDS=false;  //单锁
    isFS=false;  //封锁
    isPowerCutDW = false;
    isPowerCutFW = false;
    isPowerCutCQ = false;

    m_strCheCiNum = "";//车次
    m_nCheciLost = false;//车次是否丢失
    m_bElectric = true;//电力牵引
    m_nSXCheCi = 0;//1右行 0左行
    m_bLCTW = false;//列车挺稳标志
    m_nKHType = LCTYPE_KC;//客货类型：客车/货车
    m_nSpeed = 0;

    m_nQDState = 0;
    m_nOldQDState = 0;
    m_nOldQDState2 = 0;

    isJGGZ = false;
    isXGGZ = false;
    flblStatusDW = 0;
    flblStatusFW = 0;
    flblStatusCQ = 0;
//    isPowerDW = true;
//    isPowerFW = true;
//    isPowerCQ = true;
    speedLimitStatus = 0;

    isSpeedLimit = false;
    LimitSpeed = 0;
    m_nDCSXAllLock = 0;
}

//bool CGDDC::getDWFLBL()
//{
//    return isFLBL[0];
//}

//bool CGDDC::getFWFLBL()
//{
//    return isFLBL[1];
//}

//bool CGDDC::getCQFLBL()
//{
//    return isFLBL[2];
//}

//void CGDDC::setDWFLBL(bool flag)
//{
//    isFLBL[0]=flag;
//}

//void CGDDC::setFWFLBL(bool flag)
//{
//    isFLBL[1]=flag;
//}

//void CGDDC::setCQFLBL(bool flag)
//{
//    isFLBL[2]=flag;
//}
/***************设置道岔状态******************
*功能：设置道岔的状态
*参数：btState： 道岔的状态数据
**********************************************/
void CGDDC::setDCState(unsigned char btState)
{
    m_nState &= CLEANDC;  //清除该道岔状态
    unsigned char bt=btState & 0x03; //低两位
    if (bt == 1)
        m_nState |= DCDW;  //道岔定位
    else if (bt == 2)
        m_nState |= DCFW;	 //道岔反位
    else
        m_nState |= DCSK;   //道岔四开
    bt=(btState>>2)&0x01;  //第二位,道岔锁闭标志
    if(bt)
        setIsDS(true);   //m_DsState=TRUE;
    else
        setIsDS(false);  //m_DsState=FALSE;

    bt=(btState>>3)&0x01; //第三位,道岔封锁标志
    if(bt)
        setIsFS(true);//m_DsState=TRUE;
    else
        setIsFS(false);//m_DsState=FALSE;
}
//成员功能函数
void CGDDC::GDDCInit(int type)
{
    p1Const.setX(p1.x());
    p1Const.setY(p1.y());
    p2Const.setX(p2.x());
    p2Const.setY(p2.y());
    p3Const.setX(p3.x());
    p3Const.setY(p3.y());
    p4Const.setX(p4.x());
    p4Const.setY(p4.y());
    p5Const.setX(p5.x());
    p5Const.setY(p5.y());
    p6Const.setX(p6.x());
    p6Const.setY(p6.y());
    p12Const.setX(p12.x());
    p12Const.setY(p12.y());
    p34Const.setX(p34.x());
    p34Const.setY(p34.y());
    p56Const.setX(p56.x());
    p56Const.setY(p56.y());
    pz12Const.setX(pz12.x());
    pz12Const.setY(pz12.y());
    pz34Const.setX(pz34.x());
    pz34Const.setY(pz34.y());
    pz56Const.setX(pz56.x());
    pz56Const.setY(pz56.y());
    pCenterConst.setX(pCenter.x());
    pCenterConst.setY(pCenter.y());
    m_textRectConst.setLeft(m_textRect.left());
    m_textRectConst.setTop(m_textRect.top());
    m_textRectConst.setRight(m_textRect.right());
    m_textRectConst.setBottom(m_textRect.bottom());

    double a, b, c;
    a = pCenter.x() - pz34.x();
    b = pCenter.y() - pz34.y();
    c = sqrt(a*a + b*b);
    p34C.setX(pCenter.x() - 10 * a / c);
    p34C.setY(pCenter.y() - 10 * b / c);

    a = pCenter.x() - pz56.x();
    b = pCenter.y() - pz56.y();
    c = sqrt(a*a + b*b);
    p56C.setX(pCenter.x() - 10 * a / c);
    p56C.setY(pCenter.y() - 10 * b / c);

    p34CConst.setX(p34C.x());
    p34CConst.setY(p34C.y());
    p56CConst.setX(p56C.x());
    p56CConst.setY(p56C.y());
}
void CGDDC::Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type)
{
    if(type==0x55)
    {
        Draw_Th_kb(painter,nElapsed, nDiploid,offset);
    }
}
void CGDDC::Draw_Th_kb(QPainter *painter, long nElapsed, double nDiploid,QPoint offset)
{
    QPointF p_CQCenter;
    QPointF p_DWCenter;
    QPointF p_FWCenter;
//    QColor CQcolor = Qt::red;
//    QColor DWcolor = Qt::red;
//    QColor FWcolor = Qt::red;
    QFont font;
    //坐标变换 2021.1.11 BJT
    p1.setX(p1Const.x() * nDiploid+offset.x()*nDiploid);
    p1.setY(p1Const.y() * nDiploid+offset.y()*nDiploid);
    p2.setX(p2Const.x() * nDiploid+offset.x()*nDiploid);
    p2.setY(p2Const.y() * nDiploid+offset.y()*nDiploid);
    p3.setX(p3Const.x() * nDiploid+offset.x()*nDiploid);
    p3.setY(p3Const.y() * nDiploid+offset.y()*nDiploid);
    p4.setX(p4Const.x() * nDiploid+offset.x()*nDiploid);
    p4.setY(p4Const.y() * nDiploid+offset.y()*nDiploid);
    p5.setX(p5Const.x() * nDiploid+offset.x()*nDiploid);
    p5.setY(p5Const.y() * nDiploid+offset.y()*nDiploid);
    p6.setX(p6Const.x() * nDiploid+offset.x()*nDiploid);
    p6.setY(p6Const.y() * nDiploid+offset.y()*nDiploid);
    p12.setX(p12Const.x() * nDiploid+offset.x()*nDiploid);
    p12.setY(p12Const.y() * nDiploid+offset.y()*nDiploid);
    p34.setX(p34Const.x() * nDiploid+offset.x()*nDiploid);
    p34.setY(p34Const.y() * nDiploid+offset.y()*nDiploid);
    p56.setX(p56Const.x() * nDiploid+offset.x()*nDiploid);
    p56.setY(p56Const.y() * nDiploid+offset.y()*nDiploid);
    pz12.setX(pz12Const.x() * nDiploid+offset.x()*nDiploid);
    pz12.setY(pz12Const.y() * nDiploid+offset.y()*nDiploid);
    pz34.setX(pz34Const.x() * nDiploid+offset.x()*nDiploid);
    pz34.setY(pz34Const.y() * nDiploid+offset.y()*nDiploid);
    pz56.setX(pz56Const.x() * nDiploid+offset.x()*nDiploid);
    pz56.setY(pz56Const.y() * nDiploid+offset.y()*nDiploid);
    pCenter.setX(pCenterConst.x() * nDiploid+offset.x()*nDiploid);
    pCenter.setY(pCenterConst.y() * nDiploid+offset.y()*nDiploid);
    m_textRect.setLeft(m_textRectConst.left() * nDiploid+offset.x()*nDiploid);
    m_textRect.setTop(m_textRectConst.top() * nDiploid+offset.y()*nDiploid);
    m_textRect.setRight(m_textRectConst.right() * nDiploid+offset.x()*nDiploid);
    m_textRect.setBottom(m_textRectConst.bottom() * nDiploid+offset.y()*nDiploid);
    p34C.setX(p34CConst.x() * nDiploid+offset.x()*nDiploid);
    p34C.setY(p34CConst.y() * nDiploid+offset.y()*nDiploid);
    p56C.setX(p56CConst.x() * nDiploid+offset.x()*nDiploid);
    p56C.setY(p56CConst.y() * nDiploid+offset.y()*nDiploid);

    //反走样,防止出现锯齿状线条
    painter->setRenderHint(QPainter::Antialiasing, true);

    setDCColor(&CQcolor, &DWcolor, &FWcolor); //设置岔前、定位、反位绘制颜色
    //绘制岔前 2021.1.15 BJT
//    if(true==getIsFLBL_CQ())//分路不良画虚线
//    {
//        painter->setPen(QPen(CQcolor,GDWIDTH * nDiploid,Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
//    }
//    else
//    {
//        painter->setPen(QPen(CQcolor,GDWIDTH * nDiploid,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//    }
    painter->setPen(QPen(CQcolor,GDWIDTH * nDiploid,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (true == getZ(JYJ12))  //岔前有折点
    {
        painter->drawLine(p12, pz12);
        painter->drawLine(pz12, pCenter);
    }
    else
    {
        painter->drawLine(p12, pCenter);
    }
    if(true==getIsFLBL_CQ())//分路不良画粉红色外包线
    {
        if (true == getZ(JYJ12))  //岔前有折点
        {
            Draw_FLBL_WBX(painter,p12,pz12,nDiploid);
            Draw_FLBL_WBX(painter,pz12,pCenter,nDiploid);
        }
        else
        {
            Draw_FLBL_WBX(painter,p12,pCenter,nDiploid);
        }
    }
    //绘制定位 2021.1.15 BJT
//    if(true==getIsFLBL_DW())  //分路不良画虚线
//    {
//        painter->setPen(QPen(DWcolor,GDWIDTH * nDiploid,Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
//    }
//    else
//    {
//        painter->setPen(QPen(DWcolor,GDWIDTH * nDiploid,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//    }
    painter->setPen(QPen(DWcolor,GDWIDTH * nDiploid,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (true == getZ(JYJ34))  //定位有折点
    {
        painter->drawLine(p34, pz34);
        painter->drawLine(pz34, pCenter);
    }
    else
    {
        painter->drawLine(p34, pCenter);
    }
    if(true==getIsFLBL_CQ())//分路不良画粉红色外包线
    {
        if (true == getZ(JYJ34))  //定位有折点
        {
            Draw_FLBL_WBX(painter,p34,pz34,nDiploid);
            Draw_FLBL_WBX(painter,pz34,pCenter,nDiploid);
        }
        else
        {
            Draw_FLBL_WBX(painter,p34,pCenter,nDiploid);
        }
    }
    //绘制反位 2021.1.15 BJT
//    if(true==getIsFLBL_FW())//分路不良画虚线
//    {
//        painter->setPen(QPen(FWcolor,GDWIDTH * nDiploid,Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
//    }
//    else
//    {
//        painter->setPen(QPen(FWcolor,GDWIDTH * nDiploid,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//    }
    painter->setPen(QPen(FWcolor,GDWIDTH * nDiploid,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (true == getZ(JYJ56))  //反位有折点
    {
        painter->drawLine(p56, pz56);
        painter->drawLine(pz56, pCenter);
    }
    else
    {
        painter->drawLine(p56, pCenter);
    }
    if(true==getIsFLBL_CQ())//分路不良画粉红色外包线
    {
        if (true == getZ(JYJ56))  //反位有折点
        {
            Draw_FLBL_WBX(painter,p56,pz56,nDiploid);
            Draw_FLBL_WBX(painter,pz56,pCenter,nDiploid);
        }
        else
        {
            Draw_FLBL_WBX(painter,p56,pCenter,nDiploid);
        }
    }
    //绘制岔心 2021.1.15 BJT
    if (DCDW == getDCWZ())
    {
        painter->setPen(QPen(Qt::black,GDWIDTH * nDiploid));
        painter->drawLine(pCenter, p56C);
        if(getDisDCWZ()==true)
        {
            painter->setPen(QPen(Qt::green,GDWIDTH * nDiploid));
            painter->drawLine(pCenter, p34C);
        }
        else
        {
            painter->setPen(QPen(CQcolor,GDWIDTH * nDiploid));
            painter->drawLine(pCenter, p34C);
        }
//        if(getIsDS() == true)
//        {
//            painter->setPen(QPen(Qt::red, 1.5 * nDiploid));
//            painter->setBrush(Qt::NoBrush);
//            painter->drawEllipse((QPointF((pCenter.x()+p34C.x())/2,(pCenter.y()+p34C.y())/2)), 6 * nDiploid, 6 * nDiploid);
//        }
    }
    else if (DCFW == getDCWZ())
    {
        painter->setPen(QPen(Qt::black,GDWIDTH * nDiploid));
        painter->drawLine(pCenter, p34C);
        if(getDisDCWZ()==true)
        {
            painter->setPen(QPen(Qt::yellow,GDWIDTH * nDiploid));
            painter->drawLine(pCenter, p56C);
        }
        else
        {
            painter->setPen(QPen(CQcolor,GDWIDTH * nDiploid));
            painter->drawLine(pCenter, p56C);
        }
//        //单锁
//        if(getIsDS())
//        {
//            painter->setPen(QPen(Qt::red, 1.5 * nDiploid));
//            painter->setBrush(Qt::NoBrush);
//            painter->drawEllipse((QPointF((pCenter.x()+p56C.x())/2,(pCenter.y()+p56C.y())/2)), 6 * nDiploid, 6 * nDiploid);
//        }
    }
    else
    {
        if (0 == nElapsed % 2)
        {
            painter->setPen(QPen(Qt::red,GDWIDTH * nDiploid));
            painter->drawLine(pCenter, p34C);
            painter->setPen(QPen(Qt::red,GDWIDTH * nDiploid));
            painter->drawLine(pCenter, p56C);
        }
        else
        {
            painter->setPen(QPen(Qt::black,GDWIDTH * nDiploid));
            painter->drawLine(pCenter, p34C);
            painter->setPen(QPen(Qt::black,GDWIDTH * nDiploid));
            painter->drawLine(pCenter, p56C);
        }
//        if(getIsDS() == true)
//        {
//            painter->setPen(QPen(Qt::red, 1.5 * nDiploid));
//            painter->setBrush(Qt::NoBrush);
//            painter->drawEllipse((QPointF((pCenter.x()+p34C.x())/2,(pCenter.y()+p34C.y())/2)), 6 * nDiploid, 6 * nDiploid);
//        }
    }
    //单锁
    if(getIsDS() == true
        || ((m_nSX == 0) && (m_nDCSXAllLock & GDDC_X_LOCK))
        || ((m_nSX == 1) && (m_nDCSXAllLock & GDDC_S_LOCK))
        )
    {
        painter->setPen(QPen(Qt::red, 1.5 * nDiploid));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse((QPointF((pCenter.x()+p34C.x())/2,(pCenter.y()+p34C.y())/2)), 6 * nDiploid, 6 * nDiploid);
    }


    //绘制绝缘节 2021.1.14 BJT
    if (true == getJyj(JYJ12))   //绘制岔前绝缘节
    {
        if (true == getCxjy(JYJ12))
        {
            painter->setPen(QPen(Qt::red,1));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(p12, 5, 5);
        }
        painter->setPen(QPen(SkyBlue,1));
        painter->setBrush(Qt::black);
        painter->drawLine(p1, p2);
    }
    if (true == getJyj(JYJ34))   //绘制定位绝缘节
    {
        if (true == getCxjy(JYJ34))
        {
            painter->setPen(QPen(Qt::red,1));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(p34, 5, 5);
        }
        painter->setPen(QPen(SkyBlue,1));
        painter->setBrush(Qt::black);
        painter->drawLine(p3, p4);
    }
    if (true == getJyj(JYJ56))   //绘制反位绝缘节
    {
        if (true == getCxjy(JYJ56))
        {
            painter->setPen(QPen(Qt::red,1));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(p56, 5, 5);
        }
        painter->setPen(QPen(SkyBlue,1));
        painter->setBrush(Qt::black);
        painter->drawLine(p5, p6);
    }
    //绘制道岔名称 2021.1.14 BJT
    if (true == getDisplayName()) //单站界面名称显示
    {
        font.setFamily("宋体");
        font.setPointSize(10 * nDiploid);//字号
        font.setItalic(false);//斜体
        painter->setFont(font);//设置字体
        if (DCDW == getDCWZ())
        {
            painter->setPen(Qt::green);//设置画笔颜色
            painter->drawText(m_textRect.left(), m_textRect.bottom(), getName()); //绘制文本
        }
        else if (DCFW == getDCWZ())
        {
            painter->setPen(Qt::yellow);//设置画笔颜色
            painter->drawText(m_textRect.left(), m_textRect.bottom(), getName()); //绘制文本
        }
        else
        {
            painter->setPen(Qt::red);//设置画笔颜色
            painter->drawText(m_textRect.left(), m_textRect.bottom(), getName()); //绘制文本
        }

        if(getIsFS() == true)
        {
            QFontMetrics fm(font);
            QRect rec = fm.boundingRect(getName());
            painter->setPen(QPen(Qt::red, 1.5 * nDiploid));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(m_textRect.left()-1, m_textRect.top()+4, rec.width()+4, rec.height()+2);
        }
    }
}
void CGDDC::Draw_FLBL_WBX(QPainter *painter,QPointF pt1,QPointF pt2, double nDiploid)
{
    painter->setPen(QPen(Qt::magenta,1*nDiploid,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if(pt1.y()==pt2.y())
    {
        painter->drawLine(QPoint(pt1.x(),pt1.y()-2*nDiploid), QPoint(pt2.x(),pt2.y()-2*nDiploid));//绘制分路不良外包线
        painter->drawLine(QPoint(pt1.x(),pt1.y()+2*nDiploid), QPoint(pt2.x(),pt2.y()+2*nDiploid));//绘制分路不良外包线
    }
    else
    {
        painter->drawLine(QPoint(pt1.x(),pt1.y()-4*nDiploid), QPoint(pt2.x(),pt2.y()-4*nDiploid));//绘制分路不良外包线
        painter->drawLine(QPoint(pt1.x(),pt1.y()+4*nDiploid), QPoint(pt2.x(),pt2.y()+4*nDiploid));//绘制分路不良外包线
    }
}
void CGDDC::Draw_ToolTip(QPainter *painter, double nDiploid)
{
    QFont font;
    QPoint pt;
    //反走样,防止出现锯齿状线条
    painter->setRenderHint(QPainter::Antialiasing, true);
    //绘制鼠标进入文字提示信息
    if((true == getToolTipFlag()) && (""!=getToolTipStr()))
    {
        pt.setX(m_textRect.left()+5*nDiploid);
        pt.setY(m_textRect.top()-10*nDiploid);
        font.setFamily("宋体");
        font.setPointSize(9 * nDiploid);//字号
        font.setItalic(false);//斜体

        QFontMetrics fm(font);
        QRect rec = fm.boundingRect(getToolTipStr());

        painter->setPen(QPen(QColor(38, 38, 38), 1));
        painter->setBrush(QColor(252, 245, 221));
        painter->drawRect(pt.x(), pt.y(), rec.width()+20*nDiploid, rec.height()+4*nDiploid);

        painter->setFont(font);//设置字体
        painter->setPen(QColor(38, 38, 38));//设置画笔颜色
        //绘制文本
        painter->drawText(pt.x()+3*nDiploid, pt.y()+12* nDiploid, getToolTipStr()+"#");
    }
}
void CGDDC::setVollover(QPoint pt_Base)
{
    double x1=0;
    double x2=0;
    double y1=0;
    double y2=0;
    double y3=0;
    if(getType() == 21)
    {
        setType(23);
    }
    else if(getType() == 22)
    {
        setType(24);
    }
    else if(getType() == 23)
    {
        setType(21);
    }
    else if(getType() == 24)
    {
        setType(22);
    }
    setTextRect(QRectF(pt_Base.x() - (getTextRect().right()-pt_Base.x()),pt_Base.y() - (getTextRect().bottom()-pt_Base.y()),getTextRect().width(),getTextRect().height()));
    x1=getp1().x();
    x2=getp2().x();
    y1=getp1().y();
    y2=getp2().y();
    y3=getp12().y();
    setp1(QPointF(pt_Base.x() - (x2-pt_Base.x()),pt_Base.y() - (y3 - pt_Base.y())-(((y2-y1)+1)/2)));
    setp2(QPointF(pt_Base.x() - (x1-pt_Base.x()),pt_Base.y() - (y3 - pt_Base.y())+(((y2-y1)+1)/2)));
    x1=getp3().x();
    x2=getp4().x();
    y1=getp3().y();
    y2=getp4().y();
    y3=getp34().y();
    setp3(QPointF(pt_Base.x() - (x2-pt_Base.x()),pt_Base.y() - (y3 - pt_Base.y())-(((y2-y1)+1)/2)));
    setp4(QPointF(pt_Base.x() - (x1-pt_Base.x()),pt_Base.y() - (y3 - pt_Base.y())+(((y2-y1)+1)/2)));
    x1=getp5().x();
    x2=getp6().x();
    y1=getp5().y();
    y2=getp6().y();
    y3=getp56().y();
    setp5(QPointF(pt_Base.x() - (x2-pt_Base.x()),pt_Base.y() - (y3 - pt_Base.y())-(((y2-y1)+1)/2)));
    setp6(QPointF(pt_Base.x() - (x1-pt_Base.x()),pt_Base.y() - (y3 - pt_Base.y())+(((y2-y1)+1)/2)));
    setp12(QPointF(pt_Base.x() - (getp12().x()-pt_Base.x()),pt_Base.y() - (getp12().y() - pt_Base.y())));
    setp34(QPointF(pt_Base.x() - (getp34().x()-pt_Base.x()),pt_Base.y() - (getp34().y() - pt_Base.y())));
    setp56(QPointF(pt_Base.x() - (getp56().x()-pt_Base.x()),pt_Base.y() - (getp56().y() - pt_Base.y())));
    setpz12(QPointF(pt_Base.x() - (getpz12().x()-pt_Base.x()),pt_Base.y() - (getpz12().y() - pt_Base.y())));
    setpz34(QPointF(pt_Base.x() - (getpz34().x()-pt_Base.x()),pt_Base.y() - (getpz34().y() - pt_Base.y())));
    setpz56(QPointF(pt_Base.x() - (getpz56().x()-pt_Base.x()),pt_Base.y() - (getpz56().y() - pt_Base.y())));
    setCenterPt(QPointF(pt_Base.x() - (getCenterPt().x()-pt_Base.x()),pt_Base.y() - (getCenterPt().y() - pt_Base.y())));

    double a, b, c;
    a = pCenter.x() - pz34.x();
    b = pCenter.y() - pz34.y();
    c = sqrt(a*a + b*b);
    p34C.setX(pCenter.x() - 10 * a / c);
    p34C.setY(pCenter.y() - 10 * b / c);

    a = pCenter.x() - pz56.x();
    b = pCenter.y() - pz56.y();
    c = sqrt(a*a + b*b);
    p56C.setX(pCenter.x() - 10 * a / c);
    p56C.setY(pCenter.y() - 10 * b / c);

    p34CConst.setX(p34C.x());
    p34CConst.setY(p34C.y());
    p56CConst.setX(p56C.x());
    p56CConst.setY(p56C.y());
}
void CGDDC::setDevStateToSafe()
{
    //m_nDCWZ = DCSK;
    isDS=false;  //单锁
    isFS=false;  //封锁
    gdColor=Qt::red;          //轨道区段颜色
    this->setQDState(QDZY);
    this->setDCWZ(DCSK);
    this->setDCState(4);
}

void CGDDC::setQDState(int state)
{
    if(m_nQDState != state)
    {
        m_nOldQDState2 = m_nOldQDState;
        m_nOldQDState = m_nQDState;
    }
    m_nQDState = state;
}

bool CGDDC::getQDState(int state)
{
    if((int)(m_nQDState & state) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int CGDDC::getQDState()
{
    return m_nQDState;
}

bool CGDDC::getOldQDState(int state)
{
    if((int)(m_nOldQDState & state) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CGDDC::getOldQDState2(int state)
{
    if((int)(m_nOldQDState2 & state) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

unsigned int CGDDC::getDevType()
{
    return Dev_DC;
}
void CGDDC::setDCColor(QColor *cq_Color, QColor *dw_Color, QColor *fw_Color)
{
    //设置岔前绘制颜色
    if (true == getIsMainGDFlag())
    {
        if((getDisLight() == true)&&(getQDColor() != Qt::red))
        {
            *cq_Color = Qt::yellow;
        }
        else
        {
            *cq_Color = getQDColor();
        }
    }
    else
    {
        *cq_Color = SkyBlue;//Qt::blue;
    }
    //设置定位绘制颜色
    if (true == getIsMainGDFlag())
    {
        if (DCDW == getDCWZ())
        {
            if((getDisLight() == true)&&(getQDColor() != Qt::red))
            {
                *dw_Color = Qt::yellow;
            }
            else
            {
                *dw_Color = getQDColor();
            }
        }
        else
        {
            if ((DCSK == getDCWZ()) && (getQDColor() == Qt::red))
            {
                *dw_Color = getQDColor();
            }
            else
            {
                *dw_Color = SkyBlue;
            }
        }
    }
    else
    {
        *dw_Color = SkyBlue;
    }
    //设置反位绘制颜色
    if (true == getIsMainGDFlag())
    {
        if (DCFW == getDCWZ())
        {
            if((getDisLight() == true)&&(getQDColor() != Qt::red))
            {
                *fw_Color = Qt::yellow;
            }
            else
            {
                *fw_Color = getQDColor();
            }
        }
        else
        {
            if ((DCSK == getDCWZ()) && (getQDColor() == (Qt::red)))
            {
                *fw_Color = getQDColor();
            }
            else
            {
                *fw_Color = SkyBlue;
            }
        }
    }
    else
    {
        *fw_Color = SkyBlue;
    }
}
//设置道岔绘制颜色
void CGDDC::setDCColor()
{
    setDCColor(&CQcolor, &DWcolor, &FWcolor); //设置岔前、定位、反位绘制颜色
}
//获取道岔绘制颜色.lwm
QColor CGDDC::getDCColor()
{
    if(CQcolor == Qt::red || DWcolor == Qt::red || FWcolor == Qt::red)
    {
        return Qt::red;
    }
    else if(CQcolor == Qt::white || DWcolor == Qt::white || FWcolor == Qt::white)
    {
        return Qt::white;
    }
    else if(CQcolor == SkyBlue /*|| DWcolor == SkyBlue || FWcolor == SkyBlue*/)
    {
        return SkyBlue;
    }
    else
    {
        return gdColor;
    }
}
int CGDDC::moveCursor(QPoint p)
{
    QPoint pt;
    QRect rect_CX;
    rect_CX.setRect(pCenter.x()-6,pCenter.y()-3,12,6);
    pt.setX(p.x());
    pt.setY(p.y()-Off_Global);
    if (m_textRect.contains(pt))
    {
        return 1;
    }
    else if(rect_CX.contains(pt))
    {
        return 1;
    }
    return 0;
}
//void CGDDC::gddc_StatePro()
//{
//    if((true == getIsQDZY()) || true == getIsQDGZ())
//    {
//        setQDColor("RED");
//    }
//    else
//    {
//        setQDColor("BLUE");
//    }
//}
//成员变量封装函数
void CGDDC::setQDCode(unsigned int code)
{
    m_nQDCode = code;
}
unsigned int CGDDC::getQDCode()
{
    return m_nQDCode;
}
void CGDDC::setCxjy(unsigned int nCxjy)
{
    m_nCxjy = nCxjy;
}
bool CGDDC::getCxjy(unsigned int nCxjy)
{
    if ((m_nCxjy & nCxjy)> 0)
        return true;
    else
        return false;
}
void CGDDC::setZ(unsigned int nZ)
{
    m_nZ = nZ;
}
bool CGDDC::getZ(unsigned int nZ)
{
    if ((m_nZ & nZ)> 0)
        return true;
    else
        return false;
}
void CGDDC::setJyj(unsigned int nJyj)
{
    m_nJyj = nJyj;
}
bool CGDDC::getJyj(unsigned int nJyj)
{
    if ((m_nJyj & nJyj)> 0)
        return true;
    else
        return false;
}
void CGDDC::setCQdev(int dev)
{
    m_nQ = dev;
}
int CGDDC::getCQdev()
{
    return m_nQ;
}
void CGDDC::setDWdev(int dev)
{
    m_nD = dev;
}
int CGDDC::getDWdev()
{
    return m_nD;
}
void CGDDC::setFWdev(int dev)
{
    m_nF = dev;
}
int CGDDC::getFWdev()
{
    return m_nF;
}
//void CGDDC::setCQCode(int code)
//{
//    m_nCQ_Code=code;
//}
//int CGDDC::getCQCode()
//{
//    return m_nCQ_Code;
//}
//void CGDDC::setDWCode(int code)
//{
//    m_nDW_Code=code;
//}
//int CGDDC::getDWCode()
//{
//    return m_nDW_Code;
//}
//void CGDDC::setFWCode(int code)
//{
//    m_nFW_Code=code;
//}
//int CGDDC::getFWCode()
//{
//    return m_nFW_Code;
//}
void CGDDC::setIsMainGD(bool nFlag)
{
    isMainGD = nFlag;
}
bool CGDDC::getIsMainGD()
{
    return isMainGD;
}
bool CGDDC::getIsMainGDFlag()
{
    if (m_bDrawGD || isMainGD)
    {
        return true;
    }
    return false;
}
void CGDDC::setDCWZ(unsigned int dcwz)
{
    m_nDCWZ = dcwz;
}
unsigned int  CGDDC::getDCWZ()
{
    return m_nDCWZ;
}
//void CGDDC::setDCWZ_Old(unsigned int dcwz)
//{
//    m_nDCWZ_Old = dcwz;
//}
//unsigned int CGDDC::getDCWZ_Old()
//{
//    return m_nDCWZ_Old;
//}
//void CGDDC::setSDDCCode(int sddc_code)
//{
//    m_nSDDCCode = sddc_code;
//}
//int CGDDC::getSDDCCode()
//{
//    return m_nSDDCCode;
//}
int CGDDC::setDrawGD(bool bDrawGD)
{
    m_bDrawGD = bDrawGD;
    if (getDCWZ() == DCDW)
        return m_nD;
    if (getDCWZ() == DCFW)
        return m_nF;
    return -1;
}
//void CGDDC::setIsSDDC(bool flag)
//{
//    isSDDC=flag;
//}
//bool CGDDC::getIsSDDC()
//{
//    return isSDDC;
//}
//void CGDDC::setOneToMore(int onetomore)
//{
//    m_nOneToMore=onetomore;
//}
//int CGDDC::getOneToMore()
//{
//    return m_nOneToMore;
//}
void CGDDC::setp1(QPointF pt)
{
    p1=pt;
    p1Const=pt;
}
QPointF CGDDC::getp1()
{
    return p1Const;
}
void CGDDC::setp2(QPointF pt)
{
    p2=pt;
    p2Const=pt;
}
QPointF CGDDC::getp2()
{
    return p2Const;
}
void CGDDC::setp3(QPointF pt)
{
    p3=pt;
    p3Const=pt;
}
QPointF CGDDC::getp3()
{
    return p3Const;
}
void CGDDC::setp4(QPointF pt)
{
    p4=pt;
    p4Const=pt;
}
QPointF CGDDC::getp4()
{
    return p4Const;
}
void CGDDC::setp5(QPointF pt)
{
    p5=pt;
    p5Const=pt;
}
QPointF CGDDC::getp5()
{
    return p5Const;
}
void CGDDC::setp6(QPointF pt)
{
    p6=pt;
    p6Const=pt;
}
QPointF CGDDC::getp6()
{
    return p6Const;
}
void CGDDC::setp12(QPointF pt)
{
    p12=pt;
    p12Const=pt;
}
QPointF CGDDC::getp12()
{
    return p12Const;
}
void CGDDC::setp34(QPointF pt)
{
    p34=pt;
    p34Const=pt;
}
QPointF CGDDC::getp34()
{
    return p34Const;
}
void CGDDC::setp56(QPointF pt)
{
    p56=pt;
    p56Const=pt;
}
QPointF CGDDC::getp56()
{
    return p56Const;
}
void CGDDC::setpz12(QPointF pt)
{
    pz12=pt;
    pz12Const=pt;
}
QPointF CGDDC::getpz12()
{
    return pz12Const;
}
void CGDDC::setpz34(QPointF pt)
{
    pz34=pt;
    pz34Const=pt;
}
QPointF CGDDC::getpz34()
{
    return pz34Const;
}
void CGDDC::setpz56(QPointF pt)
{
    pz56=pt;
    pz56Const=pt;
}
QPointF CGDDC::getpz56()
{
    return pz56Const;
}
//void CGDDC::setSameQDDCCode(int code)
//{
//    m_nSameQDDCCode=code;
//}
//int CGDDC::getSameQDDCCode()
//{
//    return m_nSameQDDCCode;
//}
//void CGDDC::setIsWGDW(bool flag)
//{
//    isWGDW=flag;
//}
//bool CGDDC::getIsWGDW()
//{
//    return isWGDW;
//}
//void CGDDC::setGLB_DCCQ(int glb)
//{
//    GLB_DCCQ=glb;
//}
//int CGDDC::getGLB_DCCQ()
//{
//    return GLB_DCCQ;
//}
//void CGDDC::setGLB_DCCX(int glb)
//{
//    GLB_DCCX=glb;
//}
//int CGDDC::getGLB_DCCX()
//{
//    return GLB_DCCX;
//}
//void CGDDC::setGLB_DCDW(int glb)
//{
//    GLB_DCDW=glb;
//}
//int CGDDC::getGLB_DCDW()
//{
//    return GLB_DCDW;
//}
//void CGDDC::setGLB_DCFW(int glb)
//{
//    GLB_DCFW=glb;
//}
//int CGDDC::getGLB_DCFW()
//{
//    return GLB_DCFW;
//}
//void CGDDC::setIsTSDC(bool flag)
//{
//    isTSDC=flag;
//}
//bool CGDDC::getIsTSDC()
//{
//    return isTSDC;
//}
//void CGDDC::setCarrierFrequency(QString str)
//{
//    CarrierFrequency=str;
//}
//QString CGDDC::getCarrierFrequency()
//{
//    return CarrierFrequency;
//}
//void CGDDC::setDir_DMH(bool flag)
//{
//    if(flag == 1)
//    {
//        Dir_DMH=LEFT;
//    }
//    else
//    {
//        Dir_DMH=RIGHT;
//    }
//}
//int CGDDC::getDir_DMH()
//{
//    return Dir_DMH;
//}
//void CGDDC::setIsDrawDPZPFlag(bool flag)
//{
//    isDrawDPZPFlag=flag;
//}
//bool CGDDC::getIsDrawDPZPFlag()
//{
//    return isDrawDPZPFlag;
//}
void CGDDC::setIsFLBL_CQ(bool flag)
{
    isFLBL_CQ = flag;
}
bool CGDDC::getIsFLBL_CQ()
{
    return isFLBL_CQ;
}
void CGDDC::setIsFLBL_DW(bool flag)
{
    isFLBL_DW = flag;
}
bool CGDDC::getIsFLBL_DW()
{
    return isFLBL_DW;
}
void CGDDC::setIsFLBL_FW(bool flag)
{
    isFLBL_FW = flag;
}
bool CGDDC::getIsFLBL_FW()
{
    return isFLBL_FW;
}
void CGDDC::setDisDCWZ(bool flag)
{
    isDisDCWZ = flag;
}
bool CGDDC::getDisDCWZ()
{
    return isDisDCWZ;
}
void CGDDC::setDisLight(bool flag)
{
    isDisLight = flag;
}
bool CGDDC::getDisLight()
{
    return isDisLight;
}
void CGDDC::setIsDS(bool flag)
{
    isDS=flag;
}
bool CGDDC::getIsDS()
{
    return isDS;
}
void CGDDC::setIsFS(bool flag)
{
    isFS=flag;
}
bool CGDDC::getIsFS()
{
    return isFS;
}
