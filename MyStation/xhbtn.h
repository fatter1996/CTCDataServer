#ifndef XHBTN_H
#define XHBTN_H

#include <QObject>
#include <QRect>

//信号按钮（独立的）
class XhBtn : public QObject
{
    Q_OBJECT
public:
    explicit XhBtn(QObject *parent = nullptr);

signals:

public:
    int     m_nANTYPE;//信号按钮类型（调车按钮/列车按钮）
    QPoint  m_pCenter;//按钮中心点
    QString m_strName;//按钮名称
    QRect   m_rectBtn;//按钮轮廓坐标
    QRect   m_textRect;//此组件对象的名称区域
    bool    m_RangeVisible;//范围显示
    bool    m_nBtnIsDown;//是否按下
    bool    m_nBtnFlash;//是否闪烁
    bool    m_nFuncLockState;//封锁状态
    int     m_nCode;
    bool    m_bNameUp;//名称是否在上方

};

#endif // XHBTN_H
