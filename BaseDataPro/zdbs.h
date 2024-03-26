#ifndef CZDBS_H
#define CZDBS_H

#include "BaseData.h"
class CZDBS: public CBaseData
{
public:
    explicit CZDBS();

    //成员变量声明
private:
    int m_nGLXH;
    QVector<int>vect_JJQD;
    bool isNX;
    QPoint ArrowPt;
    QPoint ArrowPtConst;
    int m_nAnNum;
    QPoint p_GFAN;
    QPoint p_GFANConst;
    QPoint p_ZFZAN;
    QPoint p_ZFZANConst;
    QPoint p_FCFZAN;
    QPoint p_FCFZANConst;
    QPoint p_JCFZAN;
    QPoint p_JCFZANConst;
    QPoint p_MN;
    QPoint p_MNConst;

    QRect rect_GF;
    QRect rect_ZFZ;
    QRect rect_FCFZ;
    QRect rect_JCFZ;
    bool isDown_GF;
    bool isDown_ZFZ;
    bool isDown_FCFZ;
    bool isDown_JCFZ;
    int m_nDownTime_FCFZ;   //0xFF时不显示
    int m_nDownTime_JCFZ;

    QPoint p_BSD_YXFC; //允许发车灯
    QPoint p_BSD_YXFCConst; //允许发车灯
    QPoint p_BSD_FZ;  //辅助表示灯
    QPoint p_BSD_FZConst;  //辅助表示灯
    QPoint p_BSD_SG;  //闪光表示灯
    QPoint p_BSD_SGConst;  //闪光表示灯
    QPoint p_BSD_QJ;  //区间表示灯
    QPoint p_BSD_QJConst;  //区间表示灯
    QRect rect_FK;
    QRect rect_FKConst;

    bool isBSDLight_YXFC;
    bool isBSDLight_SG;
    bool isBSDLight_FZ;
    bool isBSDLight_QJ;

    int m_nArrowStateJC;
    int m_nArrowStateFC;

public:
    void setGLXH(int x);
    int getGLXH();
    void addVectJJQD(int x);
    void setVectJJQD(QVector<int> x);
    QVector<int> getVectJJQD();
    void setIsNX(bool flag);
    bool getIsNX();
    void setArrowPt(QPoint pt);
    QPoint getArrowPt();
    void setAnNum(int x);
    int getAnNum();
    void setGFANPt(QPoint pt);
    QPoint getGFANPt();
    void setZFZANPt(QPoint pt);
    QPoint getZFZANPt();
    void setFCFZANPt(QPoint pt);
    QPoint getFCFZANPt();
    void setJCFZANPt(QPoint pt);
    QPoint getJCFZANPt();
    void setMNPt(QPoint pt);
    QPoint getMNPt();
    void setIsDown_GF(bool flag);
    bool getIsDown_GF();
    void setIsDown_ZFZ(bool flag);
    bool getIsDown_ZFZ();
    void setIsDown_FCFZ(bool flag);
    bool getIsDown_FCFZ();
    void setIsDown_JCFZ(bool flag);
    bool getIsDown_JCFZ();
    void setDownTime_FCFZ(int x);
    int getDownTime_FCFZ();
    void setDownTime_JCFZ(int x);
    int getDownTime_JCFZ();
    void setBSD_YXFCPt(QPoint pt);
    QPoint getBSD_YXFCPt();
    void setBSD_SGPt(QPoint pt);
    QPoint getBSD_SGPt();
    void setBSD_FZPt(QPoint pt);
    QPoint getBSD_FZPt();
    void setBSD_QJPt(QPoint pt);
    QPoint getBSD_QJPt();
    void setRect_FK(QRect rect);
    QRect getRect_FK();
    void setIsBSDLight_YXFC(bool flag);
    bool getIsBSDLight_YXFC();
    void setIsBSDLight_SG(bool flag);
    bool getIsBSDLight_SG();
    void setIsBSDLight_FZ(bool flag);
    bool getIsBSDLight_FZ();
    void setIsBSDLight_QJ(bool flag);
    bool getIsBSDLight_QJ();
    void setArrowStateJC(int x);
    int getArrowStateJC();
    void setArrowStateFC(int x);
    int getArrowStateFC();

public:
    virtual void Draw(QPainter *painter,long nElapsed, double nDiploid,QPoint offset,int type);
    virtual void Draw_ToolTip(QPainter *painter, double nDiploid);
    virtual unsigned int getDevType();
    virtual int moveCursor(QPoint p);
    virtual void setVollover(QPoint pt_Base);

    void Draw_Th_kb(QPainter *painter,long nElapsed, double nDiploid,QPoint offset);
};

#endif // CZDBS_H
