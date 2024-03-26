#ifndef TRAIN_H
#define TRAIN_H

#include <QObject>
#include <QPainter>

//列车
class Train : public QObject
{
    Q_OBJECT
public:
    explicit Train(QObject *parent = nullptr);

signals:

public:
    QString m_strCheCi;//车次
    bool m_bElectric;//电力牵引
    bool m_bRight;//0 左行 1右行
    bool m_bLost; //车次是否丢失
    int  m_nPosCode;//显示位置的设备号
    int  m_nPosCodeReal;//实际位置的设备号
    bool m_bActive;//活跃的车次
    int  m_nType;//车次属性：列车/调车
    bool m_bStop;//车次停稳

    int  m_nKHType;//客货类型：客车/货车
    bool m_bRunning;//正在运行、停止
    int  m_nLevelCX;//超限（等级）
    int  m_nLengh;//编组长度
    int  m_nSpeed;//速度
    int  m_nIndexLCLX;//列车类型序号（管内动车组、通勤列车等）

    bool m_bManType;//人工标记车次(非联锁车次)

    int m_nOvertime;//早晚点超时，大于0表示晚点，小于0表示早点
    QPoint pCenter;      //设备中心点，不被修改的，
    //QPointF pCenterConst; //设备中心点，不被修改的，缩放及偏移使用
    QRect m_RectCheci;//车次的矩形框

    int nLastGDCode = -1;
    int nNowGDCode = -1;
public:
    //绘制
    void Draw(QPainter *painter, long nElapsed, double nDiploid, QPoint offset);
};

#endif // TRAIN_H
