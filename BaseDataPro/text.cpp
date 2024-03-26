#include "text.h"
CText::CText()
{
    m_nDCNode = 0xffff;         //关联道岔名称
    //m_nQDNode = 0xffff;
    m_nSize = 10;     //文本大小
    textColor = Qt::white;       //文本显示颜色
    m_nTextType = TEXT;
}

//成员功能函数实现
void CText::TextInit()
{
    m_textRectConst.setLeft(m_textRect.left());
    m_textRectConst.setTop(m_textRect.top());
    m_textRectConst.setRight(m_textRect.right());
    m_textRectConst.setBottom(m_textRect.bottom());
}
void CText::Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type)
{
    QFont font;
    //坐标变换 2021.1.11 BJT
    if (STATIONNAMETEXT == getTextType())
    {
        m_textRect.setLeft(m_textRectConst.left() * nDiploid);
        m_textRect.setTop(m_textRectConst.top() * nDiploid);
        m_textRect.setRight(m_textRectConst.right() * nDiploid);
        m_textRect.setBottom(m_textRectConst.bottom() * nDiploid);
    }
    else
    {
        m_textRect.setLeft(m_textRectConst.left() * nDiploid+offset.x()*nDiploid);
        m_textRect.setTop(m_textRectConst.top() * nDiploid+offset.y()*nDiploid);
        m_textRect.setRight(m_textRectConst.right() * nDiploid+offset.x()*nDiploid);
        m_textRect.setBottom(m_textRectConst.bottom() * nDiploid+offset.y()*nDiploid);
    }
    if(false == getDisplayName())
    {
        return;
    }
    if (STATIONNAMETEXT == getTextType())
    {
        font.setFamily("宋体");
        font.setPointSize(getTextSize() * nDiploid);//字号
        font.setItalic(false);//斜体
        painter->setFont(font);//设置字体
        painter->setPen(Qt::green);//设置画笔颜色
        //绘制文本
        painter->drawText(m_textRect.left(), m_textRect.top(), getName());
    }
    else if (DCQDNAMETEXT == getTextType())
    {
        font.setFamily("宋体");
        font.setPointSize(getTextSize() * nDiploid);//字号
        font.setItalic(false);//斜体
        painter->setFont(font);//设置字体
        painter->setPen(Qt::white);//设置画笔颜色
        //绘制文本
        painter->drawText(m_textRect.left(), m_textRect.top(), getName());
    }
    else if(TEXT == getTextType())
    {
        font.setFamily("宋体");
        font.setPointSize(getTextSize() * nDiploid);//字号
        font.setItalic(false);//斜体
        painter->setFont(font);//设置字体
        painter->setPen(Qt::white);//设置画笔颜色
        //绘制文本
        painter->drawText(m_textRect.left(), m_textRect.top()+10*nDiploid, getName());
    }
}
void CText::setVollover(QPoint pt_Base)
{
    if(getTextType() == STATIONNAMETEXT)
    {
        return;  //站名不需要翻转
    }
    else if(getTextType() == DCQDNAMETEXT)
    {
        setTextRect(QRectF(pt_Base.x() - (getTextRect().right()-pt_Base.x()),pt_Base.y() - (getTextRect().top()-pt_Base.y()),getTextRect().width(),getTextRect().height()));
        setCenterPt(QPointF(pt_Base.x() - (getCenterPt().x()-pt_Base.x()),pt_Base.y() - (getCenterPt().y() - pt_Base.y())));
    }
    else
    {
        setTextRect(QRectF(pt_Base.x() - (getTextRect().right()-pt_Base.x()),pt_Base.y() - (getTextRect().bottom()-pt_Base.y()),getTextRect().width(),getTextRect().height()));
        setCenterPt(QPointF(pt_Base.x() - (getCenterPt().x()-pt_Base.x()),pt_Base.y() - (getCenterPt().y() - pt_Base.y())));
    }
}
unsigned int CText::getDevType()
{
    return Dev_TEXT;
}
int CText::moveCursor(QPoint p)
{
    QPoint pt;
    pt.setX(p.x());
    pt.setY(p.y()-Off_Global);
    QRect rect;
    rect.setRect(m_textRect.left(),m_textRect.top()-10,m_textRect.width(),m_textRect.height());
    if (rect.contains(pt))
    {
        if(getTextType() == DCQDNAMETEXT)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}
//成员变量封装函数实现
void CText::setGLDCNode(int node)
{
    m_nDCNode = node;
}
int CText::getGLDCNode()
{
    return m_nDCNode;
}
//void CText::setGLQDNode(unsigned int node)
//{
//    m_nQDNode = node;
//}
//unsigned int CText::getGLQDNode()
//{
//    return m_nQDNode;
//}
void CText::setTextSize(unsigned int size)
{
    m_nSize = size;
}
unsigned int CText::getTextSize()
{
    return m_nSize;
}
void CText::setTextColor(QString strColor)
{
    if ("WHITE" == strColor)
    {
        textColor = Qt::white;
    }
    else if ("GREEN" == strColor)
    {
        textColor = Qt::green;
    }
    else if ("BLUE" == strColor)
    {
        textColor = Qt::blue;
    }
    else if ("YELLOW" == strColor)
    {
        textColor = Qt::yellow;
    }
    else if ("BLACK" == strColor)
    {
        textColor = Qt::black;
    }
    else if ("RED" == strColor)
    {
        textColor = Qt::red;
    }
    else if ("GRAY" == strColor)
    {
        textColor = Qt::gray;
    }
    else
    {
        textColor = Qt::white;
    }
}
QColor CText::getTextColor()
{
    return textColor;
}
void CText::setTextType(QString type)
{
    if(type == "STATIONNAMETEXT")
    {
        m_nTextType = STATIONNAMETEXT;
    }
    else if(type == "DCQDNAMETEXT")
    {
        m_nTextType = DCQDNAMETEXT;
    }
    else if(type == "TEXT")
    {
        m_nTextType = TEXT;
    }
}
unsigned int CText::getTextType()
{
    return m_nTextType;
}
