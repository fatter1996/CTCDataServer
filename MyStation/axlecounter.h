#ifndef AXLECOUNTER_H
#define AXLECOUNTER_H

#include <QObject>

//计轴
class AxleCounter : public QObject
{
    Q_OBJECT
public:
    explicit AxleCounter(QObject *parent = nullptr);

public:
    //关联信号机名称
    QString strJuncXHD;
    //关联信号机设备号
    unsigned int codejuncXHD;

    //按钮状态-复零（0弹起,1按下）
    int btnStateFL;
    //按钮状态-使用（0弹起,1按下）
    int btnStateSY;
    //按钮状态-停止（0弹起,1按下）
    int btnStateTZ;

    //指示灯的状态-复零灯
    int lightStateFL;
    //指示灯的状态-使用灯
    int lightStateSY;
    //指示灯的状态-停止灯
    int lightStateTZ;
    //指示灯的状态-区轨灯
    int lightStateQG;

    //指示灯的状态-区间灯
    int lightStateQJ;
    //指示灯的状态-计轴报警灯
    int lightStateJZBJ;

signals:

};

#endif // AXLECOUNTER_H
