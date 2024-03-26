#ifndef CGD_H
#define CGD_H

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include "qd.h"
class CGD : public CQD
{
public:
    explicit CGD();

    //成员变量声明
private:
public:
    QPointF p1, p2, p3, p4, p12, p34, pz12, pz34;
    QPointF p1Const, p2Const, p3Const, p4Const, p12Const, p34Const, pz12Const, pz34Const;   //缩放及偏移使用
    unsigned int m_nGDType;             //轨道区段类型
    unsigned int m_nZ;                  //折点个数

    bool isGDFLBL;    //是否分录不良
    bool m_nGDFLBLKX; //是否分录空闲
    //int GLB_QDleft;
    //int GLB_QDright;
    //bool isSXDC;
    bool isZXGD;
    bool isCXGD;
    bool isJJGD;
    //bool GD_LCode;
    //bool GD_RCode;
    //QString CarrierFrequency;
    //int Dir_DMH;
    //int m_nCodeFHXH;   //区间闭塞分区小轨道电路关联防护信号机
    //bool isNoShow;
    QString m_strMark1;
    QString m_strMark2;

public:
    //防溜信息
    int m_nLAntiSlipType=0;//左侧防溜类型（接车口）
    int m_nLTxNum=0;//左侧铁鞋号
    int m_nLJgqNum=0;//左侧紧固器号
    int m_nLJnMeters=0;//左侧警内米数
    int m_nRAntiSlipType=0;//右侧防溜类型（发车口）
    int m_nRTxNum=0;//右侧铁鞋号
    int m_nRJgqNum=0;//右侧紧固器号
    int m_nRJnMeters=0;//右侧警内米数
    int m_nTrainNums=0;//存车信息 旧版本
    QString m_sTrainInfoShow="";//存车信息 股道栏显示
public:
    QString m_strCheCiNum;//车次
    bool m_nCheciLost;//车次是否丢失
    bool m_bElectric;//电力牵引
    int m_nSXCheCi;//1右行 0左行
    bool m_bLCTW;//列车挺稳标志
    int m_nKHType;//客货类型：客车/货车
    int m_nSpeed;

    bool isLock;          //是否封锁
    bool isPowerCut;      //是否停电
    bool isSpeedLimit;    //是否限速（临时限速）
    int  LimitSpeed;      //限速值

    int flblStatus;//分路不良,0无，1分路，2确认空闲
    int bsqdfmCode;//闭塞区间发送码
    int bsqdfmDirection;//闭塞区间发码方向，0未知，1向左，2向右
    int speedLimitStatus;//限速状态,0无限速,1限速表示稳定,2限速表示闪烁

    //防错办基础属性
public:
    int gdId = NULL;//股道id
    int gdAttr;//线路性质（0 正线 1到发线）
    int jfcDir;//接发车方向（0 上行 1 下行 2 上下行）
    int jfcAttr;//接发车类型（0 客车 1 货车 2客货车）
    int overLimit;//超限（0 不能接发超限列车 1 一级超限 2 二级超限 3 三级超限 4超级超限）
    int platform;//站台（站台 1 高站台 2 低站台 0 无）
    int isCRH;//允许动车组（1 允许动车组 0不允许动车组）
    int isWater;//上水设备（1 上水设备 0无上水设备）
    int isBlowdown;//排污设备（1 排污设备  0无排污设备）
    int army;//军运（1可军用列车  0不能接军用列车）

    //成员变量封装函数声明
public:
    void setGDType(QString str_type);
    unsigned int getGDType();
    void setZ(unsigned int nZ);
    unsigned int getZ();
    void setGDFLBL(bool nFlag);
    bool getGDFLBL();
    void setp1(QPointF pt);
    QPointF getp1();
    void setp2(QPointF pt);
    QPointF getp2();
    void setp3(QPointF pt);
    QPointF getp3();
    void setp4(QPointF pt);
    QPointF getp4();
    void setp12(QPointF pt);
    QPointF getp12();
    void setp34(QPointF pt);
    QPointF getp34();
    void setpz12(QPointF pt);
    QPointF getpz12();
    void setpz34(QPointF pt);
    QPointF getpz34();
    //void setGLB_QDleft(int glb);
    //int getGLB_QDleft();
    //void setGLB_QDright(int glb);
    //int getGLB_QDright();
    //void setIsSXDC(bool flag);
    //bool getIsSXDC();
    void setIsZXGD(bool flag);
    bool getIsZXGD();
    void setIsCXGD(bool flag);
    bool getIsCXGD();
    void setIsJJGD(bool flag);
    bool getIsJJGD();
    //void setGD_LCode(int code);
    //int getGD_LCode();
    //void setGD_RCode(int code);
    //int getGD_RCode();
    //void setCarrierFrequency(QString str);
    //QString getCarrierFrequency();
    //void setDir_DMH(bool flag);
    //int getDir_DMH();

    //void setCodeFHXH(int fhxh);
    //int getCodeFHXH();
    //void setisNoShow(bool flag);
    //bool getisNoShow();
    void setMarkStr1(QString str);
    QString getMarkStr1();
    void setMarkStr2(QString str);
    QString getMarkStr2();

    //成员功能函数声明
public:
    void GDInit(int type); //股道初始化
    virtual void Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type);
    void Draw_Th_kb(QPainter *painter, long nElapsed, double nDiploid,QPoint offset);
    void Draw_FLBL_WBX(QPainter *painter,QPointF pt1,QPointF pt2, double nDiploid);//绘制分录不良外包线
    virtual void Draw_ToolTip(QPainter *painter, double nDiploid);
    virtual unsigned int getDevType();
    virtual int moveCursor(QPoint p);
    virtual void setVollover(QPoint pt_Base);
    virtual void setDevStateToSafe();
    //void gd_StatePro();
};

#endif // CGD_H
