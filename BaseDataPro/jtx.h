#ifndef CJTX_H
#define CJTX_H

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include "BaseData.h"
class CJTX : public CBaseData
{
public:
    explicit CJTX();

    //成员变量声明
private:
    QPointF p1,p2, p3, p4;
    QPointF p1Const, p2Const, p3Const, p4Const;
    unsigned int m_nJTXType;            //尽头线类型

    //成员变量封装函数
public:
    void setJTXType(unsigned int nType);
    unsigned int getJTXType();
    void setp1(QPointF pt);
    QPointF getp1();
    void setp2(QPointF pt);
    QPointF getp2();
    void setp3(QPointF pt);
    QPointF getp3();
    void setp4(QPointF pt);
    QPointF getp4();

    //成员功能函数实现
public:
    void JTXInit();
    virtual void Draw(QPainter *painter,long nElapsed,float nDiploid,QPoint offset,int type);
    virtual void setVollover(QPoint pt_Base);
    virtual unsigned int getDevType();
};

#endif // CJTX_H
