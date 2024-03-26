#ifndef CTEXT_H
#define CTEXT_H

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include "BaseData.h"
class CText : public CBaseData
{
public:
    explicit CText();

    //成员变量声明
private:
    int m_nDCNode;   //关联道岔设备号
    //unsigned int m_nQDNode;      //关联道岔设备编号
    unsigned int m_nSize;     //文本大小
    QColor textColor;       //文本显示颜色
    unsigned int m_nTextType;
public:
    QString m_DCname;//道岔区段关联道岔名称

    //成员变量封装函数声明
public:
    void setGLDCNode(int node);
    int getGLDCNode();
    //void setGLQDNode(unsigned int node);
    //unsigned int getGLQDNode();
    void setTextSize(unsigned int size);
    unsigned int getTextSize();
    void setTextColor(QString strColor);
    QColor getTextColor();
    void setTextType(QString type);
    unsigned int getTextType();

    //成员功能函数声明
public:
    void TextInit(); //初始化
    virtual void Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type);
    virtual void setVollover(QPoint pt_Base);
    virtual unsigned int getDevType();
    virtual int moveCursor(QPoint p);
};

#endif // CTEXT_H
