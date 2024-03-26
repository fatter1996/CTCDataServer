#include "tg.h"

CTG::CTG()
{
    nXHANSize = 12;
    nXHANSizeConst = nXHANSize;
    isTGANDown=false;
}
void CTG::Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type)
{
    if(type==0x55)
    {
        Draw_Th_kb(painter,nElapsed, nDiploid,offset);
    }
}
void CTG::Draw_Th_kb(QPainter *painter,long nElapsed, double nDiploid,QPoint offset)
{
    //坐标变换 2021.1.11 BJT
    pCenter.setX(pCenterConst.x() * nDiploid+offset.x()*nDiploid);
    pCenter.setY(pCenterConst.y() * nDiploid+offset.y()*nDiploid);
    m_textRect.setLeft(m_textRectConst.left()* nDiploid+offset.x()*nDiploid);
    m_textRect.setTop(m_textRectConst.top()* nDiploid+offset.y()*nDiploid);
    m_textRect.setRight(m_textRectConst.right()* nDiploid+offset.x()*nDiploid);
    m_textRect.setBottom(m_textRectConst.bottom()* nDiploid+offset.y()*nDiploid);

    nXHANSize = nXHANSizeConst * nDiploid;
    m_rectTGAN.setLeft(pCenter.x()-nXHANSize/2);
    m_rectTGAN.setTop(pCenter.y()-nXHANSize/2);
    m_rectTGAN.setRight(pCenter.x()+nXHANSize/2);
    m_rectTGAN.setBottom(pCenter.y()+nXHANSize/2);

    QFont font;
    //反走样,防止出现锯齿状线条
    painter->setRenderHint(QPainter::Antialiasing, true);

    //绘制信号机按钮
    painter->setPen(Qt::white);
    if((getIsTGANDown() == true)&&((nElapsed%2)==0))
    {
        painter->setBrush(Qt::black);
    }
    else
    {
        painter->setBrush(Qt::green);
    }
    painter->drawRect(m_rectTGAN);

    //绘制信号机名称 2021.1.11 BJT
    if (true == getDisplayName())
    {
        font.setFamily("宋体");
        font.setPointSize(9 * nDiploid);//字号
        font.setItalic(false);//斜体
        painter->setFont(font);//设置字体
        painter->setPen(Qt::white);//设置画笔颜色
        //绘制文本
        painter->drawText(m_textRect.left(), m_textRect.top()+10*nDiploid, getName());
    }
}
void CTG::Draw_ToolTip(QPainter *painter, double nDiploid)
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
        font.setPointSize(10 * nDiploid);//字号
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
int CTG::moveCursor(QPoint p)
{
    if (m_rectTGAN.contains(p))
    {
        return 1;
    }
    return 0;
}
void CTG::setVollover(QPoint pt_Base)
{
    if(getCenterPt().y()>getTextRect().bottom())
    {
        setTextRect(QRectF(pt_Base.x() - (getTextRect().right()-pt_Base.x())-7,pt_Base.y() - (getTextRect().bottom()-pt_Base.y())-37,getTextRect().width(),getTextRect().height()));
    }
    else
    {
        setTextRect(QRectF(pt_Base.x() - (getTextRect().right()-pt_Base.x())-7,pt_Base.y() - (getTextRect().bottom()-pt_Base.y())-5,getTextRect().width(),getTextRect().height()));
    }
    setCenterPt(QPointF(pt_Base.x() - (getCenterPt().x()-pt_Base.x())-7,pt_Base.y() - (getCenterPt().y() - pt_Base.y())-2));
}
void CTG::setDevStateToSafe()
{
    isTGANDown=false;
}
unsigned int CTG::getDevType()
{
    return Dev_TG;
}
void CTG::setGLXH1(int x)
{
    m_nGLXH1=x;
}
int CTG::getGLXH1()
{
    return m_nGLXH1;
}
void CTG::setGLXH2(int x)
{
    m_nGLXH2=x;
}
int CTG::getGLXH2()
{
    return m_nGLXH2;
}
void CTG::setGLXH3(int x)
{
    m_nGLXH3=x;
}
int CTG::getGLXH3()
{
    return m_nGLXH3;
}
void CTG::setGLXH4(int x)
{
    m_nGLXH4=x;
}
int CTG::getGLXH4()
{
    return m_nGLXH4;
}
void CTG::setIsTGANDown(bool flag)
{
    isTGANDown=flag;
}
bool CTG::getIsTGANDown()
{
    return isTGANDown;
}
