#ifndef CGDDC_H
#define CGDDC_H
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <QDateTime>
#include "qd.h"

class CGDDC : public CQD
{
public:
    explicit CGDDC();

    //成员变量声明
private:
    unsigned int m_nQDCode;    //关联区段设备编号
    unsigned int m_nCxjy;      //超限绝缘节
    unsigned int m_nZ;         //折点
    unsigned int m_nJyj;       //绝缘节
    QPointF p1, p2, p3, p4, p5, p6, p12, p34, p56, pz12, pz34, pz56;
    QPointF p1Const, p2Const, p3Const, p4Const, p5Const, p6Const, p12Const, p34Const, p56Const, pz12Const, pz34Const, pz56Const;
    int m_nQ;        //岔前绘制连接设备
    int m_nD;        //定位绘制连接设备
    int m_nF;        //反位绘制连接设备
    //int m_nCQ_Code;  //岔前关联设备
    //int m_nDW_Code;  //定位关联设备
    //int m_nFW_Code;  //反位关联设备
    bool isMainGD;
    unsigned int m_nDCWZ;//道岔位置
    //unsigned int m_nDCWZ_Old;
    QPointF p34C, p56C;   //用于存放岔心关键坐标，不从数据读取，而是再init中计算得到
    QPointF p34CConst, p56CConst;
    //int m_nSDDCCode;
    //QString m_TrainNum;
    //bool m_TrainDown;
    bool m_bDrawGD;
    //bool isSDDC;
    //int m_nOneToMore;
    //int m_nSameQDDCCode;
    //bool isWGDW;
    //int GLB_DCCQ;    //公里标
    //int GLB_DCCX;    //公里标
    //int GLB_DCDW;    //公里标
    //int GLB_DCFW;    //公里标
    //bool isTSDC;
    //QString CarrierFrequency;
    //int Dir_DMH;
    //bool isDrawDPZPFlag;  //判断本道岔是否绘制低频载频信息（一个区段包含多个道岔时仅一个道岔绘制低频载频信息）
    bool isFLBL_CQ;
    bool isFLBL_DW;
    bool isFLBL_FW;
    bool isDisDCWZ;
    bool isDisLight;

    QColor CQcolor = Qt::red;
    QColor DWcolor = Qt::red;
    QColor FWcolor = Qt::red;

public:
//    bool getDWFLBL();
//    bool getFWFLBL();
//    bool getCQFLBL();
//    bool isFLBL[3];
//    bool m_nCQFLBLKX;
//    bool m_nFWFLBLKX;
//    bool m_nDWFLBLKX;
//    void setDWFLBL(bool flag);
//    void setFWFLBL(bool flag);
//    void setCQFLBL(bool flag);
    void setDCState(unsigned char btState);

    bool isPowerCutDW;      //定位是否停电
    bool isPowerCutFW;      //反位是否停电
    bool isPowerCutCQ;      //岔前是否停电

    bool isDS;  //单锁
    bool isFS;  //封锁

public:
    QString m_strCheCiNum;//车次
    bool m_nCheciLost;//车次是否丢失
    bool m_bElectric;//电力牵引
    int m_nSXCheCi;//1右行 0左行
    bool m_bLCTW;//列车挺稳标志
    int m_nKHType;//客货类型：客车/货车
    int m_nSpeed;

    bool isJGGZ;//尖轨故障
    bool isXGGZ;//心轨故障
    int flblStatusDW;//定位分路不良,0无，1分路，2确认空闲
    int flblStatusFW;//反位分路不良,0无，1分路，2确认空闲
    int flblStatusCQ;//岔前分路不良,0无，1分路，2确认空闲
//    bool isPowerDW;//定位是否有电
//    bool isPowerFW;//反位是否有电
//    bool isPowerCQ;//岔前是否有电
    int  speedLimitStatus;//限速状态,0无限速,1定位限速表示稳定,2反位限速表示稳定

    bool isSpeedLimit;    //是否限速（临时限速）
    int  LimitSpeed;      //限速值
    //引导总锁状态(包含上下行)
    int m_nDCSXAllLock;	//下锁: |GDDC_X_LOCK		上锁 |GDDC_S_LOCK

public:
    bool m_bWarning = false;//道岔四开报警
    QDateTime m_TimeDCSK;//道岔四开的时间

    //成员封装函数声明
public:
    void setQDCode(unsigned int code);
    unsigned int getQDCode();
    void setCxjy(unsigned int nCxjy);
    bool getCxjy(unsigned int nCxjy);
    void setZ(unsigned int nZ);
    bool getZ(unsigned int nZ);
    void setJyj(unsigned int nJyj);
    bool getJyj(unsigned int nJyj);
    void setCQdev(int dev); //岔前一送多受属性
    int getCQdev();//岔前一送多受属性
    void setDWdev(int dev);//定位一送多受属性
    int getDWdev();//定位一送多受属性
    void setFWdev(int dev);//反位一送多受属性
    int getFWdev();//反位一送多受属性
//    void setCQCode(int code);  //岔前设备
//    int getCQCode();//岔前设备
//    void setDWCode(int code);//定位设备
//    int getDWCode();//定位设备
//    void setFWCode(int code);//反位设备
//    int getFWCode();//反位设备
    void setIsMainGD(bool nFlag);
    bool getIsMainGD();
    bool getIsMainGDFlag();
    void setDCWZ(unsigned int dcwz);
    unsigned int  getDCWZ();
    //void setDCWZ_Old(unsigned int dcwz);
    //unsigned int  getDCWZ_Old();
    //void setSDDCCode(int sddc_code);
    //int getSDDCCode();
    int setDrawGD(bool bDrawGD);
    //void setIsSDDC(bool flag);
    //bool getIsSDDC();
    //void setOneToMore(int onetomore);
    //int getOneToMore();
    void setp1(QPointF pt);
    QPointF getp1();
    void setp2(QPointF pt);
    QPointF getp2();
    void setp3(QPointF pt);
    QPointF getp3();
    void setp4(QPointF pt);
    QPointF getp4();
    void setp5(QPointF pt);
    QPointF getp5();
    void setp6(QPointF pt);
    QPointF getp6();
    void setp12(QPointF pt);
    QPointF getp12();
    void setp34(QPointF pt);
    QPointF getp34();
    void setp56(QPointF pt);
    QPointF getp56();
    void setpz12(QPointF pt);
    QPointF getpz12();
    void setpz34(QPointF pt);
    QPointF getpz34();
    void setpz56(QPointF pt);
    QPointF getpz56();
    //void setSameQDDCCode(int code);
    //int getSameQDDCCode();
    //void setIsWGDW(bool flag);
    //bool getIsWGDW();
//    void setGLB_DCCQ(int glb);
//    int getGLB_DCCQ();
//    void setGLB_DCCX(int glb);
//    int getGLB_DCCX();
//    void setGLB_DCDW(int glb);
//    int getGLB_DCDW();
//    void setGLB_DCFW(int glb);
//    int getGLB_DCFW();
//    void setIsTSDC(bool flag);
//    bool getIsTSDC();
//    void setCarrierFrequency(QString str);
//    QString getCarrierFrequency();
//    void setDir_DMH(bool flag);
//    int getDir_DMH();
//    void setIsDrawDPZPFlag(bool flag);
//    bool getIsDrawDPZPFlag();
    void setIsFLBL_CQ(bool flag);
    bool getIsFLBL_CQ();
    void setIsFLBL_DW(bool flag);
    bool getIsFLBL_DW();
    void setIsFLBL_FW(bool flag);
    bool getIsFLBL_FW();
    void setDisDCWZ(bool flag);
    bool getDisDCWZ();
    void setDisLight(bool flag);
    bool getDisLight();
    void setIsDS(bool flag);
    bool getIsDS();
    void setIsFS(bool flag);
    bool getIsFS();


    //成员功能函数声明
public:
    void GDDCInit(int type); //轨道道岔初始化
    virtual void Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type);
    void Draw_Th_kb(QPainter *painter, long nElapsed, double nDiploid,QPoint offset);
    void Draw_FLBL_WBX(QPainter *painter,QPointF pt1,QPointF pt2, double nDiploid);//绘制分录不良外包线
    virtual void Draw_ToolTip(QPainter *painter, double nDiploid);
    virtual unsigned int getDevType();
    virtual int moveCursor(QPoint p);
    virtual void setVollover(QPoint pt_Base);
    void setDCColor(QColor *cq_Color, QColor *dw_Color, QColor *fw_Color); //设置道岔绘制颜色
    void setDCColor(); //设置道岔绘制颜色.lwm
    QColor getDCColor();//获取道岔绘制颜色.lwm
    //void gddc_StatePro();
    virtual void setDevStateToSafe();

public:
    int m_nQDState;      // 此组件对象的状态
    int m_nOldQDState;   // 此组件对象的上一次状态
    int m_nOldQDState2;  // 此组件对象的上上一次状态
    void setQDState(int state);
    bool getQDState(int state);
    int  getQDState();
    bool getOldQDState(int state);
    bool getOldQDState2(int state);
};

#endif // CGDDC_H
