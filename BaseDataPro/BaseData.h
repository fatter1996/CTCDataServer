#ifndef CBASEDATA_H
#define CBASEDATA_H
#include "GlobalHeaders/GlobalFuntion.h"
class CBaseData : public QObject
{
    Q_OBJECT
public:
    explicit CBaseData(QObject *parent = nullptr);

//封装成员变量
public:
    QPointF pCenter;            //设备中心点
    QPointF pCenterConst;       //缩放及偏移使用
    unsigned int m_nType;       //设备类型
    QString m_strName;          //设备名称
    unsigned int m_nSX;         //上下行咽喉
    unsigned int m_nCode;       //设备编号
    QRectF m_textRect;          //设备名称文本区域
    QRectF m_textRectConst;     //设备名称文本区域
    unsigned int m_nStationID;           //所属站ID
    unsigned int m_nBelongToTCC;         //所属TCC ID
    unsigned int m_nBelongToRBC;         //所属RBC ID
    bool isDisPlayName;         //是否显示名称
    bool m_bToolTip;            //是否显示鼠标提示信息
    QString m_strToolTip;       //鼠标提示信息

    int m_nState;                              // 此组件对象的状态
    int m_nOldState;                           // 此组件对象的上一次状态
    int m_nOldState2;                          // 此组件对象的上上一次状态

public:
    QString strGLB;//公里标字符串
    int GLB;//公里标

public:
    void setState(int nState) ;// * 设置此对象组件的状态  * @param nState 该对象组件的状态参数
    bool getState(int nState) ;// 取得此对象组件是否具有所给状态   * @param nState 所给状态参数
    bool getOldState(int nState) ;// 取得对象上一次状态
    int  getOldState() ;// 取得对象上一次状态
    bool getOldState2(int nState) ;// 取得对象上上一次状态
    int  getState() ;// 取得此对象组件的状态   * @return 返回值即为该组件对象的状态

//封装成员变量接口函数
public:
    void setCenterPt(QPointF pt);  //设置设备中心点
    QPointF getCenterPt();         //获取设备中心点
    void setType(unsigned int nType);   //设置设备类型
    unsigned int getType();             //获取设备类型
    void setName(QString name);         //设置设备名称
    QString getName();                  //获取设备名称
    void setSX(unsigned int nSX);       //设置上下行咽喉
    unsigned int getSX();               //获取上下行咽喉
    void setCode(unsigned int code);    //设置设备编号
    unsigned int getCode();             //获取设备编号
    void setTextRect(QRectF rect);      //设置设备名称文本区域
    QRectF getTextRect();               //获取设备名称文本区域
    void setStationID(unsigned int id); //设置所属站ID
    unsigned int getStationID();        //获取所属站ID
    void setBelongToTCC(unsigned int id);    //设置所属TCC ID
    unsigned int getBelongToTCC();           //获取所属TCC ID
    void setBelongToRBC(unsigned int id);    //设置所属RBC ID
    unsigned int getBelongToRBC();           //获取所属RBC ID
    void setDisplayName(bool nFlag);    //设置是否显示名称
    bool getDisplayName();              //获取是否显示名称
    void setToolTipFlag(bool flag);     //设置是否显示鼠标提示信息
    bool getToolTipFlag();              //获取是否显示鼠标提示信息
    void setToolTipStr(QString str);    //设置鼠标提示信息
    QString getToolTipStr();            //获取鼠标提示信息

//封装虚函数用于实现多态
public:
    virtual void Draw_ToolTip(QPainter *painter, double nDiploid);
    virtual void Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type);
    virtual void setDevStateToSafe();
    virtual unsigned int getDevType();
    virtual int moveCursor(QPoint p);
    virtual void setVollover(QPoint pt_Base);//站场翻转，各子类继承实现

    void draw_Pixmap(QPainter *painter,QString picName,int x,int y,int width,int heigh,int nDiploid);  //绘制图片
};

#endif // CBASEDATA_H
