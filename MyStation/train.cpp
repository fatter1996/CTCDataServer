#include "train.h"
#include "GlobalHeaders/Global.h"
#include <QColor>

Train::Train(QObject *parent) : QObject(parent)
{
    m_strCheCi = "";//车次
    m_bElectric = true;//电力牵引
    m_bRight = 0;//0 左行 1右行
    m_bLost = false; //车次是否丢失
    m_bActive = false;
    m_nType = LCMN;
    m_bStop = false;
    m_nPosCode = -1;
    m_nPosCodeReal = -1;

    m_nKHType = LCTYPE_KC;
    m_nLevelCX = 0;
    m_nLengh = 16;
    m_nSpeed = 0;
    m_nIndexLCLX = -1;

    m_bManType = false;

    m_nOvertime = 0;
    pCenter.setX(0.0);
    pCenter.setY(0.0);
    m_RectCheci.setRect(0,0,0,0);
}
//绘制
void Train::Draw(QPainter *painter, long nElapsed, double nDiploid, QPoint offset)
{
    //早晚点矩形
    QRect rectOvertime;
    //方向三角形坐标
    QPoint pointSJX[3];
    //停稳标志矩形
    QRect rectTW;
    //QColor green = QColor(0,255,0);//绿色
    //QColor white = QColor(255,255,255);//白色
    //字体（显示规范标准字体）
    int fontStandardHigh = 15;
    //停稳标志宽度
    int twWid = 8;//10;

    //字体
    QFont font;
    font.setFamily("宋体");
    font.setPointSize(fontStandardHigh * nDiploid);//字号
    font.setItalic(false);//斜体
    font.setBold(true);//加粗
    QFontMetrics fm(font);
    QString strStandard = "88888";//标准5位字符
    QRect recsStandardSize = fm.boundingRect(strStandard);
    QRect recChiCiSize = fm.boundingRect(this->m_strCheCi);
    //背景范围
    int width = recsStandardSize.width()+4;//宽
    int heigh = fontStandardHigh+5;//4;//高
    //车次号显示，车次号居中显示，至少占5位字符宽，
    //当车次号超出5位字符时自动扩大车次窗长度
    if(recChiCiSize.width()>=recsStandardSize.width())
    {
        width = recChiCiSize.width()+4;//宽
    }
    m_RectCheci.setRect(pCenter.x()-width/2, pCenter.y()-heigh/2, width, heigh);
    //背景颜色
    //电力机车-绿色
    QColor colorBk = Qt::green;
    //非电力机车-白色
    if(!this->m_bElectric)
    {
        colorBk = Qt::white;
    }
    QPen pen1;
    pen1.setWidth(1);
    pen1.setColor(colorBk);
    painter->setPen(pen1);
    QBrush brush1;
    brush1.setStyle(Qt::SolidPattern);
    brush1.setColor(colorBk);
    //绘制底色
    //painter->drawRect(m_RectCheci);
    painter->fillRect(m_RectCheci, brush1);

    //绘制方向箭头
    if(!this->m_bManType)
    {
        QPen pen2;
        pen2.setWidth(1);
        pen2.setColor(Qt::white);
        painter->setPen(pen2);
        QBrush brush2;
        brush2.setStyle(Qt::SolidPattern);
        brush2.setColor(Qt::white);
        painter->setBrush(brush2);

        if(this->m_bRight)
        {
            //向右
            pointSJX[0] = QPoint(m_RectCheci.right()+1, m_RectCheci.top());
            pointSJX[1] = QPoint(m_RectCheci.right()+1+6, m_RectCheci.top()+heigh/2);
            pointSJX[2] = QPoint(m_RectCheci.right()+1, m_RectCheci.bottom());
            //向右，在左边
            rectTW.setRect(m_RectCheci.left()-twWid, m_RectCheci.top(), twWid, m_RectCheci.height());
        }
        else
        {
            //向左
            pointSJX[0] = QPoint(m_RectCheci.left(), m_RectCheci.top());
            pointSJX[1] = QPoint(m_RectCheci.left()-6, m_RectCheci.top()+heigh/2);
            pointSJX[2] = QPoint(m_RectCheci.left(), m_RectCheci.bottom());
            //向左，在右边
            rectTW.setRect(m_RectCheci.right(), m_RectCheci.top(), twWid, m_RectCheci.height());
        }
        painter->drawPolygon(pointSJX, 3);
    }

    //绘制文字-车次
    {
        painter->setFont(font);//设置字体
        if(m_nKHType == (int)LCTYPE_KC)
        {
            painter->setPen(QPen(Qt::red, 2));//设置画笔颜色
        }
        else
        {
            painter->setPen(QPen(Qt::blue, 2));//设置画笔颜色
        }
        painter->drawText(m_RectCheci,Qt::AlignHCenter|Qt::AlignVCenter,this->m_strCheCi);
    }
    //早晚点超过99分钟时显示为“小时:分钟”。正点时不显示早晚点窗口.
    //if(this->m_nOvertime != 0)
    //非人工标注且（列车早晚点||货车晚点）
    if(!this->m_bManType
        && ((m_nKHType==LCTYPE_KC && this->m_nOvertime != 0)|| (m_nKHType != LCTYPE_KC && this->m_nOvertime>0))
      )
    {
        QFont font2;
        font2.setFamily("宋体");
        font2.setPointSize(12 * nDiploid);//字号
        font2.setItalic(false);//斜体
        font2.setBold(true);//加粗
        QFontMetrics fm2(font2);
        QString strOvertime = "99";//标准5位字符
        //早晚点颜色
        QColor colorOvertime = Qt::red;
        //早点
        if(this->m_nOvertime<0)
        {
            //早点颜色-红色
            colorOvertime = Qt::red;
            if(this->m_nOvertime>=-99)
            {
                //显示格式：03
                strOvertime = QString("%1").arg(0-this->m_nOvertime,2,10,QChar('0'));
            }
            else
            {
                //显示格式：01:03
                strOvertime = QString("%1:%2").arg((0-this->m_nOvertime)/60,2,10,QChar('0')).arg((0-this->m_nOvertime)%60,2,10,QChar('0'));
            }
        }
        //晚点
        else if(this->m_nOvertime>0)
        {
            //晚点颜色-蓝色
            colorOvertime = Qt::blue;
            if(this->m_nOvertime<=99)
            {
                //显示格式：03
                strOvertime = strOvertime = QString("%1").arg(this->m_nOvertime,2,10,QChar('0'));
            }
            else
            {
                //显示格式：01:03
                strOvertime = QString("%1:%2").arg(this->m_nOvertime/60,2,10,QChar('0')).arg(this->m_nOvertime%60,2,10,QChar('0'));
            }
        }
        QRect recsOvertimeSize = fm2.boundingRect(strOvertime);

        if(this->m_bRight)
        {
            //向右，在左边
            rectOvertime.setRect(m_RectCheci.left()-recsOvertimeSize.width(), m_RectCheci.top(), recsOvertimeSize.width(), m_RectCheci.height());
            rectTW.setRect(rectOvertime.left()-twWid, m_RectCheci.top(), twWid, m_RectCheci.height());
        }
        else
        {
            //向左，在右边
            rectOvertime.setRect(m_RectCheci.right(), m_RectCheci.top(), recsOvertimeSize.width(), m_RectCheci.height());
            rectTW.setRect(rectOvertime.right(), m_RectCheci.top(), twWid, m_RectCheci.height());
        }
        QBrush brush2;
        brush2.setStyle(Qt::SolidPattern);
        brush2.setColor(colorOvertime);
        //绘制底色
        painter->fillRect(rectOvertime, brush2);
        //设置字体
        painter->setFont(font2);
        //设置画笔颜色-白色
        painter->setPen(QPen(Qt::white, 2));
        //显示文字
        painter->drawText(rectOvertime, Qt::AlignHCenter|Qt::AlignVCenter, strOvertime);
    }
    //显示停稳标志
    if(!this->m_bRunning && this->m_bStop)
    {
        QBrush brush3;
        brush3.setStyle(Qt::SolidPattern);
        brush3.setColor(Qt::yellow);
        //绘制底色
        painter->fillRect(rectTW, brush3);
    }
}
