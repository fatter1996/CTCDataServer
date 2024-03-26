#ifndef CSTATION_H
#define CSTATION_H

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include "BaseData.h"
#include "qd.h"
#include "xhd.h"
#include "gd.h"
#include "gddc.h"
#include "text.h"
#include "tg.h"
#include "zdbs.h"
#include "jtx.h"
//#include "Route.h"

class CStation: public QObject
{
    Q_OBJECT
public:
    explicit CStation(QObject *parent = nullptr);

    //成员变量声明
public:
    QVector<CBaseData *>DevArray;   //站场设备数组
    int QdNum;
    int GdNum;
    int DcNum;
    int XhNum;
//private:
public:
    QString m_strStationName;  //站名
    unsigned int m_nStationID; //车站id
    unsigned int m_nStationType;   //0x99-区间
    QString m_strLS_Producer;
    bool isGT;
    bool isZCZX;
    QPoint offsetPt;
    int m_nCTCSoftID;
    int m_nLS_ProcessID;  //联锁进程唯一识别号
    int m_nLS_SoftID;    //联锁软件ID
    //bool isLSProcessFlag;  //联锁进程响应请求标识
    int m_nXLMNJSoftID;  //对应线路模拟机软件ID
    bool isLSComState;  //联锁通信状态标识
    int LSComTime;   //与联锁持续无通信周期计数器

    //成员变量封装函数声明
public:
    void setStationName(QString name);   //设置站名
    QString getStationName();            //获取站名
    void setStationID(unsigned int id);
    unsigned int getStationID();
    void setStationType(unsigned int type);
    unsigned int getStationType();
    void setLSProducer(QString producer);
    QString getLSProducer();
    void setIsGT(bool flag);
    bool getIsGT();
    void setIsZCZX(bool flag);
    bool getIsZCZX();
    void setOffSet(QPoint offset);
    QPoint getOffSet();
    void setCTCSoftID(int x);
    int getCTCSoftID();
    void setLSProcessID(int x);
    int getLSProcessID();
    void setLSSoftID(int x);
    int getLSSoftID();
    //void setLSProcessFlag(bool flag);
    //bool getLSProcessFlag();
    void setXLMNJSoftID(int x);
    int getXLMNJSoftID();
    void setLSComState(bool flag);
    bool getLSComState();
    void setLSComTime(int x);
    int getLSComTime();

    //功能函数声明
public:
    void setShow_Menu(int type);  //响应系统菜单
    void set_ZCReset();
    void setQDZYFun(CQD *qd,bool flag);
    void setQDGZFun(CQD *qd,bool flag);
    CQD* getQDFromGDDCNode(int code);
    int getDCNodeFromDCName(QString name);

    void stationInit(int dymType);
    void volloverPro();
    void setDevStateToSafe();
    void drawStation(QPainter *painter,long nElapsed, double nDiploid,int JmType);
    void setGD();
    void clearDrawGD();
    void setMainGD();


    int packQD_StateForCTC(QByteArray &msg,int index);
    void OperationalOfCTC(int anType,int dev1,int dev2,int dev3,int dev4,int dev5);
    void resolutionDevStateOfLS(unsigned char *array);
    void setGDDC_StateOfLS(CGDDC *gddc,int state);
    void setXHD_StateOfLS(CXHD *xhd,int state1,int state2,int state3);
    void setQD_StateOfLS(CQD *qd,int state1,int state2);
    void setQD_Color(CQD *qd,QColor color);
    void setTG_StateOfLS(CTG *tg,int state);
    void setZDBS_StateOfLS(CZDBS *zdbs,int state1,int state2,int state3,int state4,int state5);
signals:
    void signal_YHZSBState(unsigned char yhzs_State,int stationID);

};

#endif // CSTATION_H
