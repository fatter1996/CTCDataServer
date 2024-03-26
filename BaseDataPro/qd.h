#ifndef CQD_H
#define CQD_H
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include "BaseData.h"
class CQD : public CBaseData
{
public:
    explicit CQD();

private:
    //成员变量申明
    int m_nChildNum; //区段的组件个数
    int m_nChild[4]; //区段的组件数组
    //int QD_LCode;  //区段左节点
    //int QD_RCode;  //区段右节点
    //int FQJ;    //区段方向切换继电器-每个区段都有----UP-正向  DOWN-反向
    int DMH_StateLeft;  //低频码状态
    int DMH_StateRight;  //低频码状态
    //QPoint DPMpt;
    //QPoint DPMptConst;
    //int DPM_Dir;//低频码绘制方向，LEFT=左向，RIGHT=右向
    //float DPM_slope;//低频码绘制倾斜程度---第一、第二象限大于零，第三第四象限小于零
    //int DPM_DCSlopeType;//低频码道岔倾斜类型  （以岔心为基准点）0-斜率大于零时向左上倾斜，小于零时向左下倾斜   1-斜率大于零时向右上倾斜，小于零时向右下倾斜  --仅在DMH_ShowType==2时使用
    //int DMH_ShowType;  //0-方框样式显示  1-字母样式显示
    //QRect m_DrawRect;
    //QRect m_DrawRectConst;
    //bool isDisDPZPFlag;  //是否显示低频载频信息
    //int m_nSubDevType;   //设备子类型   有无岔区段和道岔区段之分
    //bool isQDZY;         //是否区段占用
    //bool isQDGZ;         //是否区段故障
    //int nDPM_ComTime;        //低频码未接收到有效通信计数，超过两秒未收到则导向安全测
    //bool isNeedToAQC;         //低频码是否需要导向安全侧标识

public:
    QColor gdColor;          //轨道区段颜色
    //成员变量封装函数
    void setChildNum(int num);
    int getChildNum();
    void setChild(int index,int code);
    int getChild(int index);
//    void setQD_LCode(int code);
//    int getQD_LCode();
//    void setQD_RCode(int code);
//    int getQD_RCode();
//    void setFQJ(int state);
//    int getFQJ();
    void setDMHStateLeft(int state);
    int getDMHStateLeft();
    void setDMHStateRight(int state);
    int getDMHStateRight();
//    void setDPMPt(QPoint p);
//    void setDPM_Dir(int dir);
//    int getDPM_Dir();
//    void setDPM_slope(float slope);
//    float getDPM_slope();
//    void setDPM_DCSlopeType(int type);
//    int getDPM_DCSlopeType();
//    void setDMHShowType(int type);
//    int getDMHShowType();
//    void setDrawRect(QRect rect);
//    void setIsDisDPZPFlag(bool flag);
//    bool getIsDisDPZPFlag();
//    void setSubDevType(int type);
//    int getSubDevType();
//    void setIsQDZY(bool flag);
//    bool getIsQDZY();
//    void setIsQDGZ(bool flag);
//    bool getIsQDGZ();
    void setQDColor(QColor color);
    QColor getQDColor();
//    void setDPM_ComTime(int time);
//    int getDPM_ComTime();
//    void setisNeedToAQC(bool flag);
//    bool getisNeedToAQC();

    //功能实现函数
    //void qdTime_Pro();
    virtual void Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type);
    virtual unsigned int getDevType();
    virtual void setDevStateToSafe();


};

#endif // CQD_H
