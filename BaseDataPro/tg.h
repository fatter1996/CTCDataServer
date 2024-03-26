#ifndef CTG_H
#define CTG_H
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include "BaseData.h"

class CTG: public CBaseData
{
public:
    explicit CTG();

    //成员变量声明
private:
    int m_nGLXH1;
    int m_nGLXH2;
    int m_nGLXH3;
    int m_nGLXH4;
    double nXHANSize;   //信号按钮大小
    double nXHANSizeConst;   //信号按钮大小
    QRect m_rectTGAN;
    bool isTGANDown;

public:
    void setGLXH1(int x);
    int getGLXH1();
    void setGLXH2(int x);
    int getGLXH2();
    void setGLXH3(int x);
    int getGLXH3();
    void setGLXH4(int x);
    int getGLXH4();
    void setIsTGANDown(bool flag);
    bool getIsTGANDown();

public:
    virtual void Draw(QPainter *painter,long nElapsed, double nDiploid,QPoint offset,int type);
    void Draw_Th_kb(QPainter *painter,long nElapsed, double nDiploid,QPoint offset);
    virtual void Draw_ToolTip(QPainter *painter, double nDiploid);
    virtual int moveCursor(QPoint p);
    virtual unsigned int getDevType();
    virtual void setVollover(QPoint pt_Base);
    virtual void setDevStateToSafe();
};

#endif // CTG_H
