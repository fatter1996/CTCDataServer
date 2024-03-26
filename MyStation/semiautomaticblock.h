#ifndef SEMIAUTOMATICBLOCK_H
#define SEMIAUTOMATICBLOCK_H

#include <QObject>

//半自动闭塞
class SemiAutomaticBlock : public QObject
{
    Q_OBJECT
public:
    explicit SemiAutomaticBlock(QObject *parent = nullptr);

public:
    //关联信号机名称
    QString strJuncXHD;
    //关联信号机设备号
    unsigned int codejuncXHD;
    //半自动闭塞设备号
    unsigned int code;

    //接车箭头状态
    int arrowStateReach;
    //发车箭头状态
    int arrowStateDepart;

signals:

};

#endif // SEMIAUTOMATICBLOCK_H
