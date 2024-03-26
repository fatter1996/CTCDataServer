#include "qd.h"
#include <QtDebug>

CQD::CQD()
{
    m_nChildNum=0;
    for(int i=0;i<4;i++)
    {
        m_nChild[i]=0xffff;
    }
//    QD_LCode=0xffff;
//    QD_RCode=0xffff;
//    FQJ=DOWN;
    DMH_StateLeft=DMH_JC;  //0xffff
    DMH_StateRight=DMH_JC;  //0xffff
//    DMH_ShowType=1;
//    isDisDPZPFlag=false;
//    DPM_Dir=LEFT;
//    DPM_slope=0;
//    DPM_DCSlopeType=1;
    m_nBelongToTCC=0;
//    m_nSubDevType=Dev_WCQD;
//    isQDZY=false;         //是否区段占用
//    isQDGZ=false;         //是否区段故障
    gdColor = SkyBlue;              //轨道区段颜色
    //gdColor = Qt::red;              //轨道区段颜色
//    nDPM_ComTime=0;        //低频码未接收到有效通信计数，超过两秒未收到则导向安全测
//    isNeedToAQC=true;         //低频码是否需要导向安全测标识
}

void CQD::Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type)
{
//    if(getIsDisDPZPFlag() == false)
//    {
//        return;
//    }
//    //坐标计算
//    QFont font;
//    DPMpt.setX(DPMptConst.x()*nDiploid+offset.x()*nDiploid);
//    DPMpt.setY(DPMptConst.y()*nDiploid+offset.y()*nDiploid);
//    m_DrawRect.setLeft(m_DrawRectConst.left()+offset.x()*nDiploid);
//    m_DrawRect.setTop(m_DrawRectConst.top()+offset.y()*nDiploid);
//    m_DrawRect.setRight(m_DrawRectConst.right()+offset.x()*nDiploid);
//    m_DrawRect.setBottom(m_DrawRectConst.bottom()+offset.y()*nDiploid);
//    QString str_Dmh="";
//    QColor color1=Qt::black;
//    QColor color2=Qt::black;
//    QRect rect1;
//    QRect rect2;
//    rect1.setLeft(m_DrawRect.left());
//    rect1.setTop(m_DrawRect.top());
//    rect1.setRight(m_DrawRect.right());
//    rect1.setBottom(m_DrawRect.bottom()-5);
//    rect2.setLeft(m_DrawRect.left());
//    rect2.setTop(m_DrawRect.top()+5);
//    rect2.setRight(m_DrawRect.right());
//    rect2.setBottom(m_DrawRect.bottom());
//    if(0xffff == DMH_State)
//    {
//        return;
//    }
//    if(getDMHShowType() == 0)  //条状方框显示
//    {
//        font.setFamily("Times New Roman");
//        font.setPointSize(8 * nDiploid);//字号
//        font.setItalic(false);//斜体
//        font.setBold(true);//加粗
//        painter->setFont(font);//设置字体
//        if(getDMHState() == DMH_H)
//        {
//            color1=Qt::red;
//            color2=Qt::red;
//            str_Dmh="H";
//        }
//        else if(getDMHState() == DMH_HB)
//        {
//            color1=Qt::red;
//            color2=Qt::white;
//            str_Dmh="HB";
//        }
//        else if(getDMHState() == DMH_HU)
//        {
//            color1=Qt::red;
//            color2=Qt::yellow;
//            str_Dmh="HU";
//        }
//        else if(getDMHState() == DMH_UU)
//        {
//            color1=Qt::yellow;
//            color2=Qt::yellow;
//            str_Dmh="UU";
//        }
//        else if(getDMHState() == DMH_UUS)
//        {
//            color1=Qt::yellow;
//            if((nElapsed%2)==0)
//            {
//                color2=Qt::black;
//            }
//            else
//            {
//               color2=Qt::yellow;
//            }
//            str_Dmh="UUS";
//        }
//        else if(getDMHState() == DMH_U2)
//        {
//            color1=Qt::yellow;
//            color2=Qt::yellow;
//            str_Dmh="U2";
//        }
//        else if(getDMHState() == DMH_U2S)
//        {
//            color1=Qt::black;
//            if((nElapsed%2)==0)
//            {
//                color2=Qt::black;
//            }
//            else
//            {
//               color2=Qt::yellow;
//            }
//            str_Dmh="U2S";
//        }
//        else if(getDMHState() == DMH_U)
//        {
//            color1=Qt::yellow;
//            color2=Qt::yellow;
//            str_Dmh="U";
//        }
//        else if(getDMHState() == DMH_LU)
//        {
//            color1=Qt::green;
//            color2=Qt::yellow;
//            str_Dmh="LU";
//        }
//        else if(getDMHState() == DMH_L)
//        {
//            color1=Qt::green;
//            color2=Qt::green;
//            str_Dmh="L";
//        }
//        else if(getDMHState() == DMH_L2)
//        {
//            color1=QColor(0, 233, 0);//颜色
//            color2=QColor(0, 233, 0);
//            str_Dmh="L2";
//        }
//        else if(getDMHState() == DMH_L3)
//        {
//            color1=QColor(0, 190, 0);//颜色
//            color2=QColor(0, 190, 0);
//            str_Dmh="L3";
//        }
//        else if(getDMHState() == DMH_L4)
//        {
//            color1=QColor(0, 159, 0);//颜色
//            color2=QColor(0, 159, 0);
//            str_Dmh="L4";
//        }
//        else if(getDMHState() == DMH_L5)
//        {
//            color1=QColor(0, 127, 0);//颜色
//            color2=QColor(0, 127, 0);
//            str_Dmh="L5";
//        }
//        else if(getDMHState() == DMH_JC)
//        {
//            color1=Qt::black;
//            color2=Qt::black;
//            str_Dmh="JC";
//        }
//        else if(getDMHState() == DMH_SP)
//        {
//            color1=Qt::black;
//            color2=Qt::black;
//            str_Dmh="SP";
//        }
//        painter->setPen(QPen(Qt::gray, 0.3));
//        painter->setBrush(color1);
//        painter->drawRect(rect1);
//        painter->setBrush(color2);
//        painter->drawRect(rect2);
//        QFontMetrics fm(font);
//        QRect rec = fm.boundingRect(str_Dmh);
//        painter->setPen(Qt::black);//设置画笔颜色
//        painter->drawText((rect1.left()+rect1.right())/2-rec.width()/2, rect2.bottom(), str_Dmh); //绘制文本
//    }
//    else if(getDMHShowType() == 1)  //字母显示
//    {
//        font.setFamily("Times New Roman");
//        font.setPointSize(8 * nDiploid);//字号
//        font.setItalic(false);//斜体
//        painter->setFont(font);//设置字体
//        if(getDMHState() == DMH_H)
//        {
//            painter->setPen(Qt::red);//设置画笔颜色
//            str_Dmh="H";
//        }
//        else if(getDMHState() == DMH_HB)
//        {
//            painter->setPen(Qt::red);//设置画笔颜色
//            str_Dmh="HB";
//        }
//        else if(getDMHState() == DMH_HU)
//        {
//            painter->setPen(Qt::red);//设置画笔颜色
//            str_Dmh="HU";
//        }
//        else if(getDMHState() == DMH_UU)
//        {
//            painter->setPen(Qt::yellow);//设置画笔颜色
//            str_Dmh="UU";
//        }
//        else if(getDMHState() == DMH_UUS)
//        {
//            painter->setPen(Qt::yellow);//设置画笔颜色
//            str_Dmh="UUS";
//        }
//        else if(getDMHState() == DMH_U2)
//        {
//            painter->setPen(Qt::yellow);//设置画笔颜色
//            str_Dmh="U2";
//        }
//        else if(getDMHState() == DMH_U2S)
//        {
//            painter->setPen(Qt::yellow);//设置画笔颜色
//            str_Dmh="U2S";
//        }
//        else if(getDMHState() == DMH_U)
//        {
//            painter->setPen(Qt::yellow);//设置画笔颜色
//            str_Dmh="U";
//        }
//        else if(getDMHState() == DMH_LU)
//        {
//            painter->setPen(Qt::yellow);//设置画笔颜色
//            str_Dmh="LU";
//        }
//        else if(getDMHState() == DMH_L)
//        {
//            painter->setPen(Qt::green);//设置画笔颜色
//            str_Dmh="L";
//        }
//        else if(getDMHState() == DMH_L2)
//        {
//            painter->setPen(Qt::green);//设置画笔颜色
//            str_Dmh="L2";
//        }
//        else if(getDMHState() == DMH_L3)
//        {
//            painter->setPen(Qt::green);//设置画笔颜色
//            str_Dmh="L3";
//        }
//        else if(getDMHState() == DMH_L4)
//        {
//            painter->setPen(Qt::green);//设置画笔颜色
//            str_Dmh="L4";
//        }
//        else if(getDMHState() == DMH_L5)
//        {
//            painter->setPen(Qt::green);//设置画笔颜色
//            str_Dmh="L5";
//        }
//        else if(getDMHState() == DMH_JC)
//        {
//            painter->setPen(Qt::white);//设置画笔颜色
//            str_Dmh="JC";
//        }
//        else if(getDMHState() == DMH_SP)
//        {
//            painter->setPen(Qt::white);//设置画笔颜色
//            str_Dmh="SP";
//        }
//        painter->drawText(DPMpt.x(), DPMpt.y(), str_Dmh); //绘制文本
//    }
//    else if(getDMHShowType() == 2)
//    {
//        QPainterPath path1,path2;
//        str_Dmh="";
//        if(getDPM_slope()==0)//斜率为0时，低频码绘制
//        {
//            if(RIGHT==getDPM_Dir())//低频码箭头向右
//            {
//                //大三角型的绘制
//                path1.moveTo(DPMpt.x()-6*nDiploid, DPMpt.y()-6*nDiploid);
//                path1.lineTo(DPMpt.x()-6*nDiploid, DPMpt.y()+6*nDiploid);
//                path1.lineTo(DPMpt.x()+6*nDiploid, DPMpt.y());
//                path1.lineTo(DPMpt.x()-6*nDiploid, DPMpt.y()-6*nDiploid);
//                //小三角型的绘制
//                path2.moveTo(DPMpt.x(), DPMpt.y()-3*nDiploid);
//                path2.lineTo(DPMpt.x(), DPMpt.y()+3*nDiploid);
//                path2.lineTo(DPMpt.x()+7*nDiploid, DPMpt.y());
//                path2.lineTo(DPMpt.x(), DPMpt.y()-3*nDiploid);
//            }
//            else if(LEFT==getDPM_Dir())//低频码箭头向左
//            {
//                //大三角型的绘制
//                path1.moveTo(DPMpt.x()+6*nDiploid, DPMpt.y()-6*nDiploid);
//                path1.lineTo(DPMpt.x()+6*nDiploid, DPMpt.y()+6*nDiploid);
//                path1.lineTo(DPMpt.x()-6*nDiploid, DPMpt.y());
//                path1.lineTo(DPMpt.x()+6*nDiploid, DPMpt.y()-6*nDiploid);
//                //小三角型的绘制
//                path2.moveTo(DPMpt.x(), DPMpt.y()-3*nDiploid);
//                path2.lineTo(DPMpt.x(), DPMpt.y()+3*nDiploid);
//                path2.lineTo(DPMpt.x()-7*nDiploid, DPMpt.y());
//                path2.lineTo(DPMpt.x(), DPMpt.y()-3*nDiploid);
//            }
//        }
//        else//斜率不等于0时，低频码绘制
//        {
//            float k=fabsf(getDPM_slope());
//            float a=((6*(1+k))*sqrt(k*k+1))/(k*k+1);
//            float b=((6*(k-1))*sqrt(k*k+1))/(k*k+1);
//            float c=(6*sqrt(k*k+1))/(k*k+1);
//            float d=(6*k)*sqrt(k*k+1)/(k*k+1);
//            if(RIGHT==getDPM_Dir())//低频码箭头向右
//            {
//                if(((0 == getDPM_DCSlopeType()) && (getDPM_slope()<0)) || ((1 == getDPM_DCSlopeType()) && (getDPM_slope()>0)))
//                {
//                    //大三角型的绘制
//                    path1.moveTo(DPMpt.x()+b*nDiploid, DPMpt.y()+a*nDiploid);
//                    path1.lineTo(DPMpt.x()-a*nDiploid, DPMpt.y()+b*nDiploid);
//                    path1.lineTo(DPMpt.x()+c*nDiploid, DPMpt.y()-d*nDiploid);
//                    path1.lineTo(DPMpt.x()+b*nDiploid, DPMpt.y()+a*nDiploid);
//                    //小三角型的绘制
//                    path2.moveTo((DPMpt.x()+b*nDiploid+DPMpt.x()+c*nDiploid)/2, (DPMpt.y()+a*nDiploid+DPMpt.y()-d*nDiploid)/2);
//                    path2.lineTo((DPMpt.x()-a*nDiploid+DPMpt.x()+c*nDiploid)/2, (DPMpt.y()+b*nDiploid+DPMpt.y()-d*nDiploid)/2);
//                    path2.lineTo(DPMpt.x()+c*nDiploid, DPMpt.y()-d*nDiploid);
//                    path2.lineTo((DPMpt.x()+b*nDiploid+DPMpt.x()+c*nDiploid)/2, (DPMpt.y()+a*nDiploid+DPMpt.y()-d*nDiploid)/2);
//                    //qDebug()<<getDPM_slope()<<DPMpt.x(),DPMpt.y();
//                }
//                else
//                {
//                    //大三角型的绘制
//                    path1.moveTo(DPMpt.x()-a*nDiploid, DPMpt.y()-b*nDiploid);
//                    path1.lineTo(DPMpt.x()+b*nDiploid, DPMpt.y()-a*nDiploid);
//                    path1.lineTo(DPMpt.x()+c*nDiploid, DPMpt.y()+d*nDiploid);
//                    path1.lineTo(DPMpt.x()-a*nDiploid, DPMpt.y()-b*nDiploid);
//                    //小三角型的绘制
//                    path2.moveTo((DPMpt.x()-a*nDiploid+DPMpt.x()+c*nDiploid)/2, (DPMpt.y()-b*nDiploid+DPMpt.y()+d*nDiploid)/2);
//                    path2.lineTo((DPMpt.x()+b*nDiploid+DPMpt.x()+c*nDiploid)/2, (DPMpt.y()-a*nDiploid+DPMpt.y()+d*nDiploid)/2);
//                    path2.lineTo(DPMpt.x()+c*nDiploid, DPMpt.y()+d*nDiploid);
//                    path2.lineTo((DPMpt.x()-a*nDiploid+DPMpt.x()+c*nDiploid)/2, (DPMpt.y()-b*nDiploid+DPMpt.y()+d*nDiploid)/2);
//                    //qDebug()<<getDPM_slope()<<DPMpt.x(),DPMpt.y();
//                }
//            }
//            if(LEFT==getDPM_Dir())//低频码箭头向左
//            {
//                if(((0 == getDPM_DCSlopeType()) && (getDPM_slope()<0)) || ((1 == getDPM_DCSlopeType()) && (getDPM_slope()>0)))
//                {
//                    //大三角型的绘制
//                    path1.moveTo(DPMpt.x()+a*nDiploid, DPMpt.y()-b*nDiploid);
//                    path1.lineTo(DPMpt.x()-b*nDiploid, DPMpt.y()-a*nDiploid);
//                    path1.lineTo(DPMpt.x()-c*nDiploid, DPMpt.y()+d*nDiploid);
//                    path1.lineTo(DPMpt.x()+a*nDiploid, DPMpt.y()-b*nDiploid);
//                    //小三角型的绘制
//                    path2.moveTo((DPMpt.x()+a*nDiploid+DPMpt.x()-c*nDiploid)/2, (DPMpt.y()-b*nDiploid+DPMpt.y()+d*nDiploid)/2);
//                    path2.lineTo((DPMpt.x()-b*nDiploid+DPMpt.x()-c*nDiploid)/2, (DPMpt.y()-a*nDiploid+DPMpt.y()+d*nDiploid)/2);
//                    path2.lineTo(DPMpt.x()-c*nDiploid, DPMpt.y()+d*nDiploid);
//                    path2.lineTo((DPMpt.x()+a*nDiploid+DPMpt.x()-c*nDiploid)/2, (DPMpt.y()-b*nDiploid+DPMpt.y()+d*nDiploid)/2);
//                    //qDebug()<<getDPM_slope()<<DPMpt.x(),DPMpt.y();
//                }
//                else
//                {
//                    //大三角型的绘制
//                    path1.moveTo(DPMpt.x()-b*nDiploid, DPMpt.y()+a*nDiploid);
//                    path1.lineTo(DPMpt.x()+a*nDiploid, DPMpt.y()+b*nDiploid);
//                    path1.lineTo(DPMpt.x()-c*nDiploid, DPMpt.y()-d*nDiploid);
//                    path1.lineTo(DPMpt.x()-b*nDiploid, DPMpt.y()+a*nDiploid);
//                    //小三角型的绘制
//                    path2.moveTo((DPMpt.x()-b*nDiploid+DPMpt.x()-c*nDiploid)/2, (DPMpt.y()+a*nDiploid+DPMpt.y()-d*nDiploid)/2);
//                    path2.lineTo((DPMpt.x()+a*nDiploid+DPMpt.x()-c*nDiploid)/2, (DPMpt.y()+b*nDiploid+DPMpt.y()-d*nDiploid)/2);
//                    path2.lineTo(DPMpt.x()-c*nDiploid, DPMpt.y()-d*nDiploid);
//                    path2.lineTo((DPMpt.x()-b*nDiploid+DPMpt.x()-c*nDiploid)/2, (DPMpt.y()+a*nDiploid+DPMpt.y()-d*nDiploid)/2);
//                    //qDebug()<<getDPM_slope()<<DPMpt.x(),DPMpt.y();
//                }
//            }
//        }
//        font.setFamily("Times New Roman");
//        font.setPointSize(8 * nDiploid);//字号
//        font.setItalic(false);//斜体
//        font.setBold(true);//加粗
//        painter->setFont(font);//设置字体
//        if(getDMHState() == DMH_H)
//        {
//            color1=Qt::red;
//            color2=Qt::red;
//        }
//        else if(getDMHState() == DMH_HB)
//        {
//            color1=Qt::red;
//            color2=Qt::white;
//        }
//        else if(getDMHState() == DMH_HU)
//        {
//            color1=Qt::red;
//            color2=Qt::yellow;
//        }
//        else if(getDMHState() == DMH_UU)
//        {
//            color1=Qt::yellow;
//            color2=Qt::yellow;
//        }
//        else if(getDMHState() == DMH_UUS)
//        {
//            color1=Qt::yellow;
//            if((nElapsed%2)==0)
//            {
//                color2=Qt::black;
//            }
//            else
//            {
//               color2=Qt::yellow;
//            }
//        }
//        else if(getDMHState() == DMH_U2)
//        {
//            color1=Qt::yellow;
//            color2=Qt::yellow;
//        }
//        else if(getDMHState() == DMH_U2S)
//        {
//            color1=Qt::black;
//            if((nElapsed%2)==0)
//            {
//                color2=Qt::black;
//            }
//            else
//            {
//               color2=Qt::yellow;
//            }
//        }
//        else if(getDMHState() == DMH_U)
//        {
//            color1=Qt::yellow;
//            color2=Qt::yellow;
//        }
//        else if(getDMHState() == DMH_LU)
//        {
//            color1=Qt::green;
//            color2=Qt::yellow;
//        }
//        else if(getDMHState() == DMH_L)
//        {
//            color1=Qt::green;
//            color2=Qt::green;
//        }
//        else if(getDMHState() == DMH_L2)
//        {
//            color1=Qt::green;
//            color2=Qt::green;
//            str_Dmh="2";
//        }
//        else if(getDMHState() == DMH_L3)
//        {
//            color1=Qt::green;
//            color2=Qt::green;
//            str_Dmh="3";
//        }
//        else if(getDMHState() == DMH_L4)
//        {
//            color1=Qt::green;
//            color2=Qt::green;
//            str_Dmh="4";
//        }
//        else if(getDMHState() == DMH_L5)
//        {
//            color1=Qt::green;
//            color2=Qt::green;
//            str_Dmh="5";
//        }
//        else if(getDMHState() == DMH_JC)
//        {
//            color1=Qt::black;
//            color2=Qt::black;
//        }
//        else if(getDMHState() == DMH_SP)
//        {
//            color1=Qt::black;
//            color2=Qt::black;
//        }

//        painter->setPen(QPen(Qt::white, 0.3));
//        painter->setBrush(color1);
//        painter->drawPath(path1);
//        painter->fillPath(path1,color1);
//        if((getDMHState()==DMH_HU) || (getDMHState()==DMH_HB) || (getDMHState()==DMH_UU) || (getDMHState()==DMH_UUS) || (getDMHState()==DMH_U2) || (getDMHState()==DMH_U2S) || (getDMHState()==DMH_LU))
//        {
//            painter->setBrush(color2);
//            painter->drawPath(path2);
//            painter->fillPath(path2,color2);
//        }
//        painter->setPen(Qt::black);//设置画笔颜色
//        if(str_Dmh != "")
//        {
//            if(LEFT==getDPM_Dir())
//            {
//                painter->drawText(DPMpt.x(),DPMpt.y()+4*nDiploid, str_Dmh); //绘制文本
//            }
//            if(RIGHT==getDPM_Dir())
//            {
//                painter->drawText(DPMpt.x()-5*nDiploid,DPMpt.y()+4*nDiploid, str_Dmh); //绘制文本
//            }
//        }
//    }
}
//void CQD::qdTime_Pro()
//{
//    int count=getDPM_ComTime();
//    if(count < 14)
//    {
//        setisNeedToAQC(false);
//        count++;
//        setDPM_ComTime(count);
//    }
//    else
//    {
//        setisNeedToAQC(true);
//        setDMHState(DMH_JC);
//    }
//}
void CQD::setDevStateToSafe()
{
    DMH_StateLeft=DMH_WM;  //0xffff
    DMH_StateRight=DMH_WM;  //0xffff
    gdColor = Qt::red;              //轨道区段颜色
}


unsigned int CQD::getDevType()  //重载父类虚函数，可返回Dev_WCQD和Dev_DCQD两个值
{
    return Dev_QD;
}
//成员变量封装函数
void CQD::setChildNum(int num)
{
    m_nChildNum=num;
}
int CQD::getChildNum()
{
    return m_nChildNum;
}
void CQD::setChild(int index,int code)
{
    if(index<4)
    {
        m_nChild[index]=code;
    }
}
int CQD::getChild(int index)
{
    if(index<4)
    {
        return m_nChild[index];
    }
    else
    {
        return 0xffff;
    }
}
//void CQD::setQD_LCode(int code)
//{
//    QD_LCode=code;
//}
//int CQD::getQD_LCode()
//{
//    return QD_LCode;
//}
//void CQD::setQD_RCode(int code)
//{
//    QD_RCode=code;
//}
//int CQD::getQD_RCode()
//{
//    return QD_RCode;
//}
//void CQD::setFQJ(int state)
//{
//    FQJ=state;
//}
//int CQD::getFQJ()
//{
//    return FQJ;
//}
void CQD::setDMHStateLeft(int state)
{
    DMH_StateLeft=state;
}
int CQD::getDMHStateLeft()
{
    return DMH_StateLeft;
}
void CQD::setDMHStateRight(int state)
{
    DMH_StateRight=state;
}
int CQD::getDMHStateRight()
{
    return DMH_StateRight;
}
//void CQD::setDPMPt(QPoint p)
//{
//    DPMpt=p;
//    DPMptConst=p;
//}
//void CQD::setDPM_Dir(int dir)
//{
//    DPM_Dir=dir;
//}
//int CQD::getDPM_Dir()
//{
//    return DPM_Dir;
//}
//void CQD::setDPM_slope(float slope)
//{
//    DPM_slope=slope;
//}
//float CQD::getDPM_slope()
//{
//    return DPM_slope;
//}
//void CQD::setDPM_DCSlopeType(int type)
//{
//    DPM_DCSlopeType=type;
//}
//int CQD::getDPM_DCSlopeType()
//{
//    return DPM_DCSlopeType;
//}
//void CQD::setDMHShowType(int type)
//{
//    DMH_ShowType=type;
//}
//int CQD::getDMHShowType()
//{
//    return DMH_ShowType;
//}
//void CQD::setDrawRect(QRect rect)
//{
//    m_DrawRect=rect;
//    m_DrawRectConst=rect;
//}
//void CQD::setIsDisDPZPFlag(bool flag)
//{
//    isDisDPZPFlag=flag;
//}
//bool CQD::getIsDisDPZPFlag()
//{
//    return isDisDPZPFlag;
//}
//void CQD::setSubDevType(int type)
//{
//    m_nSubDevType=type;
//}
//int CQD::getSubDevType()
//{
//    return m_nSubDevType;
//}
//void CQD::setIsQDZY(bool flag)
//{
//    isQDZY = flag;
//}
//bool CQD::getIsQDZY()
//{
//    return isQDZY;
//}
void CQD::setQDColor(QColor color)
{
    gdColor = color;
}
QColor CQD::getQDColor()
{
    return gdColor;
}
//void CQD::setIsQDGZ(bool flag)
//{
//    isQDGZ = flag;
//}
//bool CQD::getIsQDGZ()
//{
//    return isQDGZ;
//}
//void CQD::setDPM_ComTime(int time)
//{
//    nDPM_ComTime=time;
//}
//int CQD::getDPM_ComTime()
//{
//    return nDPM_ComTime;
//}
//void CQD::setisNeedToAQC(bool flag)
//{
//    isNeedToAQC=flag;
//}
//bool CQD::getisNeedToAQC()
//{
//    return isNeedToAQC;
//}
