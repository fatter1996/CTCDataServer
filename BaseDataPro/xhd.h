#ifndef CXHD_H
#define CXHD_H

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include "BaseData.h"
class CXHD : public CBaseData
{
public:
    explicit CXHD();

    //成员变量声明
private:
public:
    unsigned int m_nXHDType;    //信号机类型
    bool isSignalType;   //是否是虚拟信号
    unsigned int m_nSafeLamp;   //安全灯光
    bool isHigh;       //是否为高柱
    unsigned int m_nXHDState;  //信号机状态
    //unsigned int m_nXHD_lsState;  //信号机状态
    bool isLCBt_Down;
    bool isDCBt_Down;
    bool isYDBt_Down;
    double nXHANSize;   //信号按钮大小
    double nXHANSizeConst;   //信号按钮大小
    QRectF m_rectLC;       //列车按钮区域
    QRectF m_rectDC;       //调车按钮区域
    QRectF m_rectYD;       //引导按钮区域
    QRectF m_rectLCConst;
    QRectF m_rectDCConst;
    QRectF m_rectYDConst;
    bool isYDSD;
    bool signalDCAN;    //是否有单独的调车按钮
    QRect xhd_Rect;
    bool isMD;
    bool isLCANFB;
    bool isDCANFB;
    bool isYDANFB;
    int m_nTimeType;   //0x11-无倒计时  0x22-人解30s  0x33-人解180s   0x44-坡道解锁180s  0x55-引导首区段故障保持15s
    int m_nTimeCount;  //0xFF-无倒计时  其他-实时值

public:
    bool XHD_ds_HD;//信号灯灯丝断丝-红灯
    bool XHD_ds_LD;//信号灯灯丝断丝-绿灯
    bool XHD_ds_UD;//信号灯灯丝断丝-黄灯
    bool XHD_ds_BD;//信号灯灯丝断丝-白灯
    bool XHD_ds_AD;//信号灯灯丝断丝-蓝灯
    bool XHD_ds_YBD;//信号灯灯丝断丝-引白灯
    bool isLCANFlash;
    bool isDCANFlash;
    bool isBTANFlash;

public:
    int m_nFuncLockState;//封锁状态
    QPoint p1, p2, p12, p34, p56, center;
    int m_nRadius;
    int m_nHeight;

    //防错办基础属性
public:
    QString enexName;//出入口名称
    int direct;//方向 0双向/1下行进站/2上行出站/3下行出站/4上行进站。双线自动闭塞区段，按进站信号机的行别、属性 选择“下行进站/上行出站/下行出站/上行进站”。 单线自动闭塞、半自动闭塞区段，选择“双向”
    int allowOverLimit;//允许超限等级1允许一级超限 2允许二级超限 3允许三级超限 4-允许超级超限 0不允许超限列车
    int allowPassenger;//允许旅客列车1 允许旅客列车 0不允许旅客列车
    int allowFreight;//允许货物列车1 允许货物列车  0不允许货物列车


    //成员变量封装函数声明
public:
    void setXHDType(QString strType);
    unsigned int getXHDType();
    void setSignalType(bool flag);
    bool getSignalType();
    void setSafeLamp(QString strSafelamp);
    unsigned int getSafeLamp();
    void setIsHigh(bool ishigh);
    bool getIsHigh();
    void setXHDState(unsigned int state);
    unsigned int getXHDState();
    bool getXHDState(int nState);
    void setIsLCDown(bool nFlag);
    bool getIsLCDown();
    void setIsDCDown(bool nFlag);
    bool getIsDCDown();
    void setIsYDDown(bool nFlag);
    bool getIsYDDown();
    void setIsYDSD(bool flag);
    bool getIsYDSD();
    void setLCAN_Rect(QRectF rect);
    void setDCAN_Rect(QRectF rect);
    void setYDAN_Rect(QRectF rect);
    void setSignalDCAN(bool flag);
    bool getSignalDCAN();
    void setIsMD(bool flag);
    bool getIsMD();
    void setIsLCANFB(bool flag);
    bool getIsLCANFB();
    void setIsDCANFB(bool flag);
    bool getIsDCANFB();
    void setIsYDANFB(bool flag);
    bool getIsYDANFB();
    void setTimeType(int x);
    int getTimeType();
    void setTimeCount(int x);
    int getTimeCount();

    //功能函数声明
public:
    void XHDInit(int type); //信号灯初始化
    void XHDInit_Th_kb(); //标准界面信号灯初始化
    virtual void Draw(QPainter *painter,long nElapsed, double nDiploid,QPoint offset,int type);
    void Draw_Th_kb(QPainter *painter,long nElapsed, double nDiploid,QPoint offset);
    virtual void Draw_ToolTip(QPainter *painter, double nDiploid);
    virtual unsigned int getDevType();
    virtual int moveCursor(QPoint p);
    virtual void setVollover(QPoint pt_Base);
    void setXHD_Color(QColor *xhd1_Color, QColor *xhd2_Color, int nElapsed);
    void xhd_StatePro();
    virtual void setDevStateToSafe();
};

#endif // CXHD_H
