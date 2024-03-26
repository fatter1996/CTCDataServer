#ifndef INTERFIELDCONNECTION_H
#define INTERFIELDCONNECTION_H

#include <QObject>

//场联/机务段
class InterfieldConnection : public QObject
{
    Q_OBJECT
public:
    explicit InterfieldConnection(QObject *parent = nullptr);

public:
    //关联信号机名称
    QString strJuncXHD;
    //关联信号机设备号
    unsigned int codejuncXHD;

    //接发车箭头状态
    int arrowState=0;
    //指示灯的状态-接近轨
    int lightStateJGJ=0;
    //指示灯的状态-C灯
    int lightStateCFJ=0;
    //指示灯的状态-A灯
    int lightStateAFJ=0;
    //指示灯的状态-B灯
    int lightStateBFJ=0;

signals:

};

#endif // INTERFIELDCONNECTION_H
