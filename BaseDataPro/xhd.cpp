#include <QtDebug>
#include "xhd.h"

CXHD::CXHD()
{
    m_nXHDType=JZ_XHJ;    //信号机类型
    isSignalType = false;   //是否是虚拟信号
    m_nSafeLamp=XHD_HD;   //安全灯光
    isHigh = false;       //是否为高柱
    m_nXHDState = m_nSafeLamp;
    isLCBt_Down = false;
    isDCBt_Down = false;
    isYDBt_Down = false;
    nXHANSize = 15;
    nXHANSizeConst = nXHANSize;
    m_rectLC.setRect(0, 0, 0, 0);       //列车按钮区域
    m_rectDC.setRect(0, 0, 0, 0);       //调车按钮区域
    m_rectYD.setRect(0, 0, 0, 0);       //引导按钮区域
    m_rectLCConst.setRect(0, 0, 0, 0);
    m_rectDCConst.setRect(0, 0, 0, 0);
    m_rectYDConst.setRect(0, 0, 0, 0);
    isYDSD=false;
    signalDCAN=false;
    xhd_Rect.setRect(0,0,0,0);
    isMD=false;
    isLCANFB=false;
    isDCANFB=false;
    isYDANFB=false;
    m_nTimeType=0x11;
    m_nTimeCount=0;

    XHD_ds_HD=false;
    XHD_ds_LD=false;
    XHD_ds_UD=false;
    XHD_ds_BD=false;
    XHD_ds_AD=false;
    XHD_ds_YBD=false;
    isLCANFlash=false;
    isDCANFlash=false;
    isBTANFlash=false;

    m_nFuncLockState = 0;
    p1 = QPoint(0,0);
    p2 = QPoint(0,0);
    p12 = QPoint(0,0);
    p34 = QPoint(0,0);
    p56 = QPoint(0,0);
    center = QPoint(0,0);
    m_nRadius = 7;//XHD_RADIUS;
    m_nHeight = 10;
}

//成员功能函数实现
void CXHD::XHDInit(int type)
{
    pCenterConst.setX(pCenter.x());
    pCenterConst.setY(pCenter.y());
    m_textRectConst.setLeft(m_textRect.left());
    m_textRectConst.setTop(m_textRect.top());
    m_textRectConst.setRight(m_textRect.right());
    m_textRectConst.setBottom(m_textRect.bottom());

    m_nXHDState = XHD_DS;
    if(type == 0x55)
    {
        XHDInit_Th_kb();
    }

    center.setX(pCenter.x());
    center.setY(pCenter.y());
    if(m_nType==31||m_nType==32){
        center.setX(center.x() - m_nHeight);
    }
    else
    {
        center.setX(center.x() + m_nHeight);
    }

    if(m_nType==31||m_nType==32){
        if(isHigh)
        {
            p1 = QPoint(center.x(),center.y()-m_nRadius);
            p2 = QPoint(center.x(),center.y()+m_nRadius);
            p12 = QPoint(center.x(),center.y());
            p34 = QPoint(center.x()+m_nRadius+2,center.y());
            p56 = QPoint(center.x()+m_nRadius*2+2,center.y());
        }
        else
        {
            p1 = QPoint(center.x(),center.y()-m_nRadius);
            p2 = QPoint(center.x(),center.y()+m_nRadius);
            p12 = QPoint(center.x(),center.y());
            p34 = QPoint(center.x()+2,center.y());
            p56 = QPoint(center.x()+2+m_nRadius,center.y());
        }
    }
    else{
        if(isHigh)
        {
            p1 = QPoint(center.x(),center.y()-m_nRadius);
            p2 = QPoint(center.x(),center.y()+m_nRadius);
            p12 = QPoint(center.x(),center.y());
            p34 = QPoint(center.x()-m_nRadius,center.y());
            p56 = QPoint(center.x()-m_nRadius*2,center.y());
        }
        else
        {
            p1 = QPoint(center.x(),center.y()-m_nRadius);
            p2 = QPoint(center.x(),center.y()+m_nRadius);
            p12 = QPoint(center.x(),center.y());
            p34 = QPoint(center.x(),center.y());
            p56 = QPoint(center.x()-m_nRadius,center.y());
        }
    }
}
void CXHD::XHDInit_Th_kb()
{
    nXHANSize = 12;
    nXHANSizeConst = nXHANSize;

    if(getType() == 32)
    {
        m_rectLC.setRect(pCenter.x()-28,pCenter.y()-5,nXHANSize,nXHANSize);
    }
    else if(getType() == 34)
    {
        m_rectLC.setRect(pCenter.x()+16,pCenter.y()-5,nXHANSize,nXHANSize);
    }
    if(getIsYDSD() == true)
    {
        if(getType() == 32)
        {
            m_rectYD.setRect(pCenter.x()-28-16,pCenter.y()-5,nXHANSize,nXHANSize);
        }
        else if(getType() == 34)
        {
            m_rectYD.setRect(pCenter.x()+16+16,pCenter.y()-5,nXHANSize,nXHANSize);
        }
    }
    m_rectLCConst=m_rectLC;
    m_rectDCConst=m_rectDC;
    m_rectYDConst=m_rectYD;
}

void CXHD::Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type)
{
    if(type==0x55)
    {
        Draw_Th_kb(painter,nElapsed, nDiploid,offset);
    }
}
void CXHD::Draw_Th_kb(QPainter *painter,long nElapsed, double nDiploid,QPoint offset)
{
    QPointF p_Line1;  //信号机竖线绘制坐标
    QPointF p_Line2;  //信号机竖线绘制坐标
    QPointF p_Line3;  //信号机高柱横线绘制坐标
    QPointF p_Line4;  //信号机高柱横线绘制坐标
    QPointF p_Xhd;    //信号机靠近信号柱灯位坐标
    QPointF p_Xhd2;   //信号机远离信号柱灯位坐标
    QColor xhd1_Color = Qt::black;  //信号机近端信号灯位显示颜色
    QColor xhd2_Color = Qt::black;  //信号机远端信号灯位显示颜色
    QFont font;

    //坐标变换 2021.1.11 BJT
    pCenter.setX(pCenterConst.x() * nDiploid+offset.x()*nDiploid);
    pCenter.setY(pCenterConst.y() * nDiploid+offset.y()*nDiploid);
    m_textRect.setLeft(m_textRectConst.left()* nDiploid+offset.x()*nDiploid);
    m_textRect.setTop(m_textRectConst.top()* nDiploid+offset.y()*nDiploid);
    m_textRect.setRight(m_textRectConst.right()* nDiploid+offset.x()*nDiploid);
    m_textRect.setBottom(m_textRectConst.bottom()* nDiploid+offset.y()*nDiploid);
    m_rectLC.setLeft(m_rectLCConst.left()*nDiploid+offset.x()*nDiploid);
    m_rectLC.setTop(m_rectLCConst.top()*nDiploid+offset.y()*nDiploid);
    m_rectLC.setRight(m_rectLCConst.right()*nDiploid+offset.x()*nDiploid);
    m_rectLC.setBottom(m_rectLCConst.bottom()*nDiploid+offset.y()*nDiploid);
    m_rectDC.setLeft(m_rectDCConst.left()*nDiploid+offset.x()*nDiploid);
    m_rectDC.setTop(m_rectDCConst.top()*nDiploid+offset.y()*nDiploid);
    m_rectDC.setRight(m_rectDCConst.right()*nDiploid+offset.x()*nDiploid);
    m_rectDC.setBottom(m_rectDCConst.bottom()*nDiploid+offset.y()*nDiploid);
    m_rectYD.setLeft(m_rectYDConst.left()*nDiploid+offset.x()*nDiploid);
    m_rectYD.setTop(m_rectYDConst.top()*nDiploid+offset.y()*nDiploid);
    m_rectYD.setRight(m_rectYDConst.right()*nDiploid+offset.x()*nDiploid);
    m_rectYD.setBottom(m_rectYDConst.bottom()*nDiploid+offset.y()*nDiploid);


    nXHANSize = nXHANSizeConst * nDiploid;

    //反走样,防止出现锯齿状线条
    painter->setRenderHint(QPainter::Antialiasing, true);

    if(getXHDType() == XHP_XHJ)//绘制信号牌 2021.7.29 BJT
    {
        painter->setPen(QPen(Qt::white, 1));
        painter->setBrush(Qt::black);
        if ((31 == getType()) || (32 == getType()))
        {
            p_Line1.setX(pCenter.x() - 10 * nDiploid);
            p_Line1.setY(pCenter.y() - 16 * nDiploid);
            p_Line2.setX(pCenter.x() - 10 * nDiploid);
            p_Line2.setY(pCenter.y());
            painter->drawLine(p_Line1, p_Line2); //绘制信号牌竖线

            p_Xhd.setX(pCenter.x() - 10 * nDiploid);
            p_Xhd.setY(pCenter.y()-8* nDiploid);
            p_Xhd2.setX(p_Xhd.x() + 20 * nDiploid);
            p_Xhd2.setY(p_Xhd.y());
            painter->drawRect(p_Xhd2.x()-12 * nDiploid,p_Xhd2.y(), 12 * nDiploid, 12 * nDiploid);
            QPointF points[3] = {QPointF(p_Xhd2.x()-11 * nDiploid, p_Xhd2.y()+2* nDiploid), QPointF(p_Xhd2.x()-1* nDiploid, p_Xhd2.y()+2* nDiploid), QPointF(p_Xhd2.x()-6* nDiploid, p_Xhd2.y()+11* nDiploid)};
            painter->setPen(QPen(Qt::black, 1));
            painter->setBrush(Qt::yellow);
            painter->drawPolygon(points, 3);
            painter->setPen(QPen(Qt::white, 1));
            painter->drawLine(p_Xhd, p_Xhd2);
        }
        else
        {
            p_Xhd.setX(pCenter.x() + 10 * nDiploid);
            p_Xhd.setY(pCenter.y() + 8* nDiploid);
            p_Xhd2.setX(p_Xhd.x() -20 * nDiploid);
            p_Xhd2.setY(p_Xhd.y());
            painter->setPen(QPen(Qt::white, 1));
            painter->setBrush(Qt::black);
            painter->drawRect(p_Xhd2.x(),p_Xhd2.y()-12* nDiploid, 12 * nDiploid, 12 * nDiploid);
            QPointF points[3] = {QPointF(p_Xhd2.x()+1 * nDiploid, p_Xhd2.y()-2* nDiploid), QPointF(p_Xhd2.x()+11* nDiploid, p_Xhd2.y()-2* nDiploid), QPointF(p_Xhd2.x()+6* nDiploid, p_Xhd2.y()-11* nDiploid)};
            painter->setPen(QPen(Qt::black, 1));
            painter->setBrush(Qt::yellow);
            painter->drawPolygon(points, 3);
            painter->setPen(QPen(Qt::white, 1));
            painter->drawLine(p_Xhd, p_Xhd2);

            p_Line1.setX(pCenter.x() + 10 * nDiploid);
            p_Line1.setY(pCenter.y());
            p_Line2.setX(pCenter.x() + 10 * nDiploid);
            p_Line2.setY(pCenter.y() + 16 * nDiploid);
            painter->drawLine(p_Line1, p_Line2); //绘制信号牌竖线
        }
        xhd_Rect.setRect(-100,-100,-80,-80);
    }
    else
    {
        if (false == getSignalType())   //虚拟按钮不绘制信号灯位
        {
            //绘制信号机柱 2021.1.12 BJT
            if ((31 == getType()) || (32 == getType()))
            {
                p_Line1.setX(pCenter.x() - 10 * nDiploid);
                p_Line1.setY(pCenter.y() - 8 * nDiploid);
                p_Line2.setX(pCenter.x() - 10 * nDiploid);
                p_Line2.setY(pCenter.y() + 8 * nDiploid);
            }
            else
            {
                p_Line1.setX(pCenter.x() + 10 * nDiploid);
                p_Line1.setY(pCenter.y() - 8 * nDiploid);
                p_Line2.setX(pCenter.x() + 10 * nDiploid);
                p_Line2.setY(pCenter.y() + 8 * nDiploid);
            }
            painter->setPen(QPen(SkyBlue, 2));
            painter->drawLine(p_Line1, p_Line2); //绘制信号机柱竖线

            if (true == getIsHigh())
            {
                p_Line3.setX(p_Line1.x());
                p_Line3.setY((p_Line1.y() + p_Line2.y()) / 2);
                if ((31 == getType()) || (32 == getType()))
                {
                    p_Line4.setX(p_Line1.x() + 6 * nDiploid);
                    p_Line4.setY(p_Line3.y());
                }
                else
                {
                    p_Line4.setX(p_Line1.x() - 6 * nDiploid);
                    p_Line4.setY(p_Line3.y());
                }
                painter->setPen(QPen(SkyBlue, 2));
                painter->drawLine(p_Line3, p_Line4); //绘制信号机柱高柱短横线
            }
            //绘制信号机灯位 2021.7.29 BJT
            if ((31 == getType()) || (32 == getType()))
            {
                if (true == getIsHigh())
                {
                    p_Xhd.setX(pCenter.x() + 4 * nDiploid);
                    p_Xhd.setY(pCenter.y());
                }
                else
                {
                    p_Xhd.setX(pCenter.x() - 1 * nDiploid);
                    p_Xhd.setY(pCenter.y());
                }
                p_Xhd2.setX(p_Xhd.x() + 16 * nDiploid);
                p_Xhd2.setY(p_Xhd.y());
            }
            else
            {
                if (true == getIsHigh())
                {
                    p_Xhd.setX(pCenter.x() - 4 * nDiploid);
                    p_Xhd.setY(pCenter.y());
                }
                else
                {
                    p_Xhd.setX(pCenter.x() + 1 * nDiploid);
                    p_Xhd.setY(pCenter.y());
                }
                p_Xhd2.setX(p_Xhd.x() - 16 * nDiploid);
                p_Xhd2.setY(p_Xhd.y());
            }
            setXHD_Color(&xhd1_Color, &xhd2_Color, nElapsed);   //根据信号机状态实时设置信号机灯位颜色
            painter->setPen(QPen(SkyBlue, 1));
            painter->setBrush(xhd1_Color);
            painter->drawEllipse(p_Xhd, 8 * nDiploid, 8 * nDiploid);

            if(getIsMD() == true)
            {
                painter->drawLine(QPoint(p_Xhd.x()-4 * nDiploid,p_Xhd.y()-5 * nDiploid), QPoint(p_Xhd.x()+6 * nDiploid,p_Xhd.y()+5 * nDiploid)); //绘制灭灯状态
                painter->drawLine(QPoint(p_Xhd.x()+6 * nDiploid,p_Xhd.y()-5 * nDiploid), QPoint(p_Xhd.x()-4 * nDiploid,p_Xhd.y()+5 * nDiploid)); //绘制灭灯状态
            }
            m_rectDC.setRect(p_Xhd.x()-8*nDiploid,p_Xhd.y()-8*nDiploid,16* nDiploid,16* nDiploid);
            //xhd_Rect.setRect(p_Xhd.x()-8* nDiploid,p_Xhd.y()-8* nDiploid,16* nDiploid,16* nDiploid);
            if ((34 == getType()) || (32 == getType()))
            {
                painter->setPen(QPen(SkyBlue, 1));
                painter->setBrush(xhd2_Color);
                painter->drawEllipse(p_Xhd2, 8* nDiploid, 8* nDiploid);
                if(getIsMD() == true)
                {
                    painter->drawLine(QPoint(p_Xhd2.x()-4 * nDiploid,p_Xhd2.y()-5 * nDiploid), QPoint(p_Xhd2.x()+6 * nDiploid,p_Xhd2.y()+5 * nDiploid)); //绘制灭灯状态
                    painter->drawLine(QPoint(p_Xhd2.x()+6 * nDiploid,p_Xhd2.y()-5 * nDiploid), QPoint(p_Xhd2.x()-4 * nDiploid,p_Xhd2.y()+5 * nDiploid)); //绘制灭灯状态
                }
                if(p_Xhd.x()>p_Xhd2.x())
                {
                    m_rectDC.setRect(p_Xhd2.x()-8*nDiploid,p_Xhd2.y()-8*nDiploid,32* nDiploid,16* nDiploid);
                }
                else
                {
                    m_rectDC.setRect(p_Xhd.x()-8*nDiploid,p_Xhd.y()-8*nDiploid,32* nDiploid,16* nDiploid);
                }
            }
            else if (((XHD_UU == getXHDState())) || (XHD_LU == getXHDState()) || (XHD_LL == getXHDState()) || (XHD_YD == getXHDState()) || (XHD_USU == getXHDState()))
            {
                painter->setPen(QPen(SkyBlue, 1));
                painter->setBrush(xhd2_Color);
                painter->drawEllipse(p_Xhd2, 8* nDiploid, 8* nDiploid);
                if(getIsMD() == true)
                {
                    painter->drawLine(QPoint(p_Xhd2.x()-4 * nDiploid,p_Xhd2.y()-5 * nDiploid), QPoint(p_Xhd2.x()+6 * nDiploid,p_Xhd2.y()+5 * nDiploid)); //绘制灭灯状态
                    painter->drawLine(QPoint(p_Xhd2.x()+6 * nDiploid,p_Xhd2.y()-5 * nDiploid), QPoint(p_Xhd2.x()-4 * nDiploid,p_Xhd2.y()+5 * nDiploid)); //绘制灭灯状态
                }
                if(p_Xhd.x()>p_Xhd2.x())
                {
                    m_rectDC.setRect(p_Xhd2.x()-8*nDiploid,p_Xhd2.y()-8*nDiploid,32* nDiploid,16* nDiploid);
                }
                else
                {
                    m_rectDC.setRect(p_Xhd.x()-8*nDiploid,p_Xhd.y()-8*nDiploid,32* nDiploid,16* nDiploid);
                }
            }
        }
        //绘制信号机按钮
        painter->setPen(Qt::white);
        if(isLCANFlash&&((nElapsed%2)==0))
        {
            //终端闪烁-黄色
            painter->setBrush(Qt::yellow);
        }
        else if((getIsLCDown() == true)&&((nElapsed%2)==0))
        {
            painter->setBrush(Qt::black);
        }
        else
        {
            painter->setBrush(Qt::green);
        }
        painter->drawRect(m_rectLC);

        if(getIsLCANFB() == true)
        {
            painter->setPen(QPen(Qt::red, 2));
            painter->drawLine(m_rectLC.topLeft(), m_rectLC.bottomRight()); //绘制按钮封锁
            painter->drawLine(m_rectLC.topRight(), m_rectLC.bottomLeft()); //绘制按钮封锁
        }

        painter->setPen(Qt::white);
        if((getIsYDDown() == true)&&((nElapsed%2)==0))
        {
            painter->setBrush(Qt::black);
        }
        else
        {
            painter->setBrush(Qt::blue);
        }
        painter->drawRect(m_rectYD);
        if(getIsYDANFB() == true)
        {
            painter->setPen(QPen(Qt::red, 2));
            painter->drawLine(m_rectYD.topLeft(), m_rectYD.bottomRight()); //绘制按钮封锁
            painter->drawLine(m_rectYD.topRight(), m_rectYD.bottomLeft()); //绘制按钮封锁
        }

        if(getIsDCANFB() == true)
        {
            painter->setPen(QPen(Qt::red, 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(m_rectDC); //绘制按钮封锁
        }
    }
    //绘制信号机名称 2021.1.11 BJT
    if(getTimeType() != 0x11)
    {
        font.setFamily("宋体");
        font.setPointSize(10 * nDiploid);//字号
        font.setItalic(false);//斜体
        painter->setFont(font);//设置字体
        painter->setPen(Qt::red);//设置画笔颜色
        //绘制文本
        painter->drawText(m_textRect.left(), m_textRect.top()+10* nDiploid, QString("%1").arg(getTimeCount()));
    }
    if ((true == getDisplayName()) && (getTimeType() == 0x11))
    {
        font.setFamily("宋体");
        font.setPointSize(10 * nDiploid);//字号
        font.setItalic(false);//斜体
        painter->setFont(font);//设置字体
        painter->setPen(Qt::white);//设置画笔颜色
        if(isDCANFlash&&((nElapsed%2)==0))
        {
            painter->setPen(Qt::yellow);//设置画笔颜色
        }
        //绘制文本
        painter->drawText(m_textRect.left(), m_textRect.top()+10* nDiploid, getName());
    }

}
void CXHD::Draw_ToolTip(QPainter *painter, double nDiploid)
{
    QFont font;
    QPoint pt;
    //反走样,防止出现锯齿状线条
    painter->setRenderHint(QPainter::Antialiasing, true);
    //绘制鼠标进入文字提示信息
    if((true == getToolTipFlag()) && (""!=getToolTipStr()))
    {
        pt.setX(pCenter.x()+5*nDiploid);
        pt.setY(pCenter.y()-10*nDiploid);
        font.setFamily("宋体");
        font.setPointSize(9 * nDiploid);//字号
        font.setItalic(false);//斜体

        QFontMetrics fm(font);
        QRect rec = fm.boundingRect(getToolTipStr());

        painter->setPen(QPen(QColor(38, 38, 38), 1));
        painter->setBrush(QColor(252, 245, 221));
        painter->drawRect(pt.x(), pt.y(), rec.width()+10*nDiploid, rec.height()+4*nDiploid);

        painter->setFont(font);//设置字体
        painter->setPen(QColor(38, 38, 38));//设置画笔颜色
        //绘制文本
        painter->drawText(pt.x()+5*nDiploid, pt.y()+12* nDiploid, getToolTipStr());
    }
}
void CXHD::setDevStateToSafe()
{
    m_nXHDState = XHD_DS;
    isLCBt_Down = false;
    isDCBt_Down = false;
    isYDBt_Down = false;
    isMD=false;
    m_nTimeType=0x11;
    m_nTimeCount=0xFF;
}
void CXHD::setVollover(QPoint pt_Base)
{
    double x1=0;
    double x2=0;
    double y1=0;
    double y2=0;
    double y3=0;
    if(getType() == 32)
    {
        setType(34);
    }
    else if(getType() == 34)
    {
        setType(32);
    }
    else if(getType() == 31)
    {
        setType(33);
    }
    else if(getType() == 33)
    {
        setType(31);
    }
    setTextRect(QRectF(pt_Base.x() - (getTextRect().right()-pt_Base.x()),pt_Base.y() - (getTextRect().bottom()-pt_Base.y()),getTextRect().width(),getTextRect().height()));
    setCenterPt(QPointF(pt_Base.x() - (getCenterPt().x()-pt_Base.x()),pt_Base.y() - (getCenterPt().y() - pt_Base.y())));
    setDCAN_Rect(QRectF(pt_Base.x() - (m_rectDC.right()-pt_Base.x()),pt_Base.y() - (m_rectDC.bottom()-pt_Base.y()),m_rectDC.width(),m_rectDC.height()));
    setLCAN_Rect(QRectF(pt_Base.x() - (m_rectLC.right()-pt_Base.x()),pt_Base.y() - (m_rectLC.bottom()-pt_Base.y()),m_rectLC.width(),m_rectLC.height()));
    setYDAN_Rect(QRectF(pt_Base.x() - (m_rectYD.right()-pt_Base.x()),pt_Base.y() - (m_rectYD.bottom()-pt_Base.y()),m_rectYD.width(),m_rectYD.height()));
}
unsigned int CXHD::getDevType()
{
    return Dev_XH;
}
void CXHD::setXHD_Color(QColor *xhd1_Color, QColor *xhd2_Color, int nElapsed)
{
    *xhd1_Color = Qt::black;
    *xhd2_Color = Qt::black;
    if (XHD_HD == getXHDState())
    {
        *xhd1_Color = Qt::red;
        *xhd2_Color = Qt::black;
    }
    else if (XHD_DS == getXHDState())
    {
        if (0 == nElapsed % 2)
        {
            if(XHD_AD == getSafeLamp())
            {
                *xhd1_Color = Qt::blue;
            }
            else
            {
                *xhd1_Color = Qt::red;
            }
        }
        else
        {
            *xhd1_Color = Qt::black;
        }
        *xhd2_Color = Qt::black;
    }
    else if (XHD_AD == getXHDState())
    {
        *xhd1_Color = Qt::blue;
        *xhd2_Color = Qt::black;
    }
    else if (XHD_BD == getXHDState())
    {
        *xhd1_Color = Qt::white;
        *xhd2_Color = Qt::black;
    }
    else if (XHD_LD == getXHDState())
    {
        *xhd1_Color = Qt::black;
        *xhd2_Color = Qt::green;
    }
    else if (XHD_UD == getXHDState())
    {
        *xhd1_Color = Qt::black;
        *xhd2_Color = Qt::yellow;
    }
    else if (XHD_UU == getXHDState())
    {
        *xhd1_Color = Qt::yellow;
        *xhd2_Color = Qt::yellow;
    }
    else if (XHD_LL == getXHDState())
    {
        *xhd1_Color = Qt::green;
        *xhd2_Color = Qt::green;
    }
    else if (XHD_YD == getXHDState())
    {
        *xhd1_Color = Qt::white;
        *xhd2_Color = Qt::red;
    }
    else if (XHD_LU == getXHDState())
    {
        *xhd1_Color = Qt::yellow;
        *xhd2_Color = Qt::green;
    }
    else if (XHD_2U == getXHDState())
    {
        *xhd1_Color = Qt::yellow;
        *xhd2_Color = Qt::black;
    }
    else if (XHD_BS == getXHDState())
    {
        if (0 == nElapsed % 2)
        {
            *xhd1_Color = Qt::white;
        }
        else
        {
            *xhd1_Color = Qt::black;
        }
        *xhd2_Color = Qt::black;
    }
    else if (XHD_US == getXHDState())
    {
        *xhd1_Color = Qt::black;
        if (0 == nElapsed % 2)
        {
            *xhd2_Color = Qt::yellow;
        }
        else
        {
            *xhd2_Color = Qt::black;
        }
    }
    else if (XHD_LS == getXHDState())
    {
        *xhd1_Color = Qt::black;
        if (0 == nElapsed % 2)
        {
            *xhd2_Color = Qt::green;
        }
        else
        {
            *xhd2_Color = Qt::black;
        }
    }
    else if (XHD_USU == getXHDState())
    {
        *xhd1_Color = Qt::yellow;
        if (0 == nElapsed % 2)
        {
            *xhd2_Color = Qt::yellow;
        }
        else
        {
            *xhd2_Color = Qt::black;
        }
    }
    else if (XHD_MD == getXHDState())
    {
        *xhd1_Color = Qt::black;
        *xhd2_Color = Qt::black;
    }
}
int CXHD::moveCursor(QPoint p)
{
    if (m_rectDC.contains(p))
    {
        return 1;
    }
    else if(m_rectLC.contains(p))
    {
        return 2;
    }
    else if(m_rectYD.contains(p))
    {
        return 3;
    }
    return 0;
}
void CXHD::xhd_StatePro()
{
//    if (XHD_HD == getXHDState())
//    {
//        if(true == getDSDS_H())
//        {
//            setXHDState(XHD_DS);
//        }
//    }
//    else if (XHD_DS == getXHDState())
//    {
//        if((true == getDSDS_H()) && (XHD_HD == getSafeLamp()))
//        {
//            setXHDState(XHD_DS);
//        }
//        else if((false == getDSDS_H()) && (XHD_HD == getSafeLamp()))
//        {
//            setXHDState(XHD_HD);
//        }
//        else if((true == getDSDS_A()) && (XHD_AD == getSafeLamp()))
//        {
//            setXHDState(XHD_DS);
//        }
//        else if((false == getDSDS_A()) && (XHD_AD == getSafeLamp()))
//        {
//            setXHDState(XHD_AD);
//        }
//    }
//    else if (XHD_AD == getXHDState())
//    {
//        if(true == getDSDS_A())
//        {
//            setXHDState(XHD_DS);
//        }
//    }
//    else if (XHD_BD == getXHDState())
//    {
//    }
//    else if (XHD_LD == getXHDState())
//    {
//    }
//    else if (XHD_UD == getXHDState())
//    {
//    }
//    else if (XHD_UU == getXHDState())
//    {
//    }
//    else if (XHD_LL == getXHDState())
//    {
//    }
//    else if (XHD_YD == getXHDState())
//    {
//    }
//    else if (XHD_LU == getXHDState())
//    {
//    }
//    else if (XHD_2U == getXHDState())
//    {
//    }
//    else if (XHD_BS == getXHDState())
//    {
//    }
//    else if (XHD_US == getXHDState())
//    {
//    }
//    else if (XHD_LS == getXHDState())
//    {
//    }
//    else if (XHD_USU == getXHDState())
//    {
//    }
//    else if (XHD_MD == getXHDState())
//    {
//    }
}
//成员变量封装函数
void CXHD::setXHDType(QString strType)
{
    unsigned int nType=JZ_XHJ;
    if (strType == "JZ_XHJ")
    {
        nType = JZ_XHJ;
    }
    else if (strType == "DCJL_XHJ")
    {
        nType = DCJL_XHJ;
    }
    else if (strType == "JZFS_XHJ")
    {
        nType = JZFS_XHJ;
    }
    else if (strType == "SXCZ_XHJ")
    {
        nType = SXCZ_XHJ;
    }
    else if (strType == "YG_XHJ")
    {
        nType = YG_XHJ;
    }
    else if (strType == "CZ_XHJ")
    {
        nType = CZ_XHJ;
    }
    else if (strType == "DC_XHJ")
    {
        nType = DC_XHJ;
    }
    else if (strType == "DCFS_XHJ")
    {
        nType = DCFS_XHJ;
    }
    else if (strType == "FCJL_XHJ")
    {
        nType = FCJL_XHJ;
    }
    else if (strType == "JLFS_XHJ")
    {
        nType = JLFS_XHJ;
    }
    else if (strType == "XHP_XHJ")
    {
        nType = XHP_XHJ;
    }
    m_nXHDType = nType;
}
unsigned int CXHD::getXHDType()
{
    return m_nXHDType;
}
void CXHD::setSignalType(bool flag)
{
    isSignalType = flag;
}
bool CXHD::getSignalType()
{
    return isSignalType;
}
void CXHD::setSafeLamp(QString strSafelamp)
{
    unsigned int safelamp=XHD_HD;
    if (strSafelamp == "XHD_HD")
    {
        safelamp = XHD_HD;
    }
    else
    {
        safelamp = XHD_AD;
    }
    m_nSafeLamp = safelamp;
}
unsigned int CXHD::getSafeLamp()
{
    return m_nSafeLamp;
}
void CXHD::setIsHigh(bool ishigh)
{
    isHigh = ishigh;
}
bool CXHD::getIsHigh()
{
    return isHigh;
}
void CXHD::setXHDState(unsigned int state)
{
    m_nXHDState = state;
}
unsigned int CXHD::getXHDState()
{
    return m_nXHDState;
}
//*取得信号机状态   * @param nState 所给信号机状态   * @return 如果信号机状态包含所给状态返回真，否则返回假*/
bool CXHD::getXHDState(int nState)
{
    if ((int)m_nXHDState == nState)//m_nState
        return true;
    else
        return false;
}
/*void CXHD::setXHD_lsState(unsigned int state)
{
    m_nXHD_lsState = state;
}
unsigned int CXHD::getXHD_lsState()
{
    return m_nXHD_lsState;
}*/
void CXHD::setIsLCDown(bool nFlag)
{
    isLCBt_Down = nFlag;
}
bool CXHD::getIsLCDown()
{
    return isLCBt_Down;
}
void CXHD::setIsDCDown(bool nFlag)
{
    isDCBt_Down = nFlag;
}
bool CXHD::getIsDCDown()
{
    return isDCBt_Down;
}
void CXHD::setIsYDDown(bool nFlag)
{
    isYDBt_Down = nFlag;
}
bool CXHD::getIsYDDown()
{
    return isYDBt_Down;
}
void CXHD::setIsYDSD(bool flag)
{
    isYDSD=flag;
}
bool CXHD::getIsYDSD()
{
    return isYDSD;
}
void CXHD::setLCAN_Rect(QRectF rect)
{
    m_rectLC=rect;
    m_rectLCConst=rect;
}
void CXHD::setDCAN_Rect(QRectF rect)
{
    m_rectDC=rect;
    m_rectDCConst=rect;
}
void CXHD::setYDAN_Rect(QRectF rect)
{
    m_rectYD=rect;
    m_rectYDConst=rect;
}
void CXHD::setSignalDCAN(bool flag)
{
    signalDCAN=flag;
}
bool CXHD::getSignalDCAN()
{
    return signalDCAN;
}
void CXHD::setIsMD(bool flag)
{
    isMD=flag;
}
bool CXHD::getIsMD()
{
    return isMD;
}
void CXHD::setIsLCANFB(bool flag)
{
    isLCANFB=flag;
}
bool CXHD::getIsLCANFB()
{
    return isLCANFB;
}
void CXHD::setIsDCANFB(bool flag)
{
    isDCANFB=flag;
}
bool CXHD::getIsDCANFB()
{
    return isDCANFB;
}
void CXHD::setIsYDANFB(bool flag)
{
    isYDANFB=flag;
}
bool CXHD::getIsYDANFB()
{
    return isYDANFB;
}
void CXHD::setTimeType(int x)
{
    m_nTimeType=x;
}
int CXHD::getTimeType()
{
    return m_nTimeType;
}
void CXHD::setTimeCount(int x)
{
    m_nTimeCount=x;
}
int CXHD::getTimeCount()
{
    return m_nTimeCount;
}
