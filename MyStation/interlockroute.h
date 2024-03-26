#ifndef INTERLOCKROUTE_H
#define INTERLOCKROUTE_H

#include <QObject>

//联锁表进路
class InterlockRoute : public QObject
{
    Q_OBJECT
public:
    explicit InterlockRoute(QObject *parent = nullptr);

signals:

public:
    QString     Number;//序号
    QString     Type;//类型
    QStringList BtnArr;//按钮
    QString     strXHD;//信号机
    QStringList DCArr;//道岔
    QStringList QDArr;//区段
    QStringList DDXHDArr;//敌对信号
};

#endif // INTERLOCKROUTE_H
