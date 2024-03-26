#ifndef LIMITSPEED_H
#define LIMITSPEED_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include "BaseDataPro/gddc.h"
#include "BaseDataPro/gd.h"

//临时限速类
class LimitSpeed : public QObject
{
    Q_OBJECT
public:
    explicit LimitSpeed(QObject *parent = nullptr);

signals:

public:
    int cmdNum;//命令号
    QString strCmdNum;//命令号
    int speed;//限速值
    QDateTime startTime;//开始时间
    QDateTime finishTime;//结束时间
    QVector<CGD*> vectGD;//区段列表
    QVector<CGDDC*> vectGDDC;//道岔列表
    QVector<CGD*> vectQJ;//区间列表
    bool bSet;//设置or取消
    bool bSetStatus;//设置状态，true设置，false没设置
    bool bStartNow;//是否立即设置


};

#endif // LIMITSPEED_H
