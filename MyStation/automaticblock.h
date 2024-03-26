#ifndef AUTOMATICBLOCK_H
#define AUTOMATICBLOCK_H

#include <QObject>

//自动闭塞
class AutomaticBlock : public QObject
{
    Q_OBJECT
public:
    explicit AutomaticBlock(QObject *parent = nullptr);

public:
    //关联信号机名称
    QString strJuncXHD;
    //关联信号机设备号
    unsigned int codejuncXHD;

    //接发车箭头状态
    int arrowState;

    //按钮状态-总辅助（0弹起,1按下）
    int btnStateZFZ;
    //按钮状态-发辅助（0弹起,1按下）
    int btnStateFFZ;
    //按钮状态-接辅助（0弹起,1按下）
    int btnStateJFZ;
    //按钮状态-改方（0弹起,1按下）
    int btnStateGF;

    //指示灯的状态-监督区间
    int lightStateJDQJ;
    //指示灯的状态-区间灯
    int lightStateQJD;
    //指示灯的状态-辅助灯
    int lightStateFZD;
    //指示灯的状态-区轨灯
    int lightStateQGD;
    //指示灯的状态-允许发车灯
    int lightStateYXFCD;

    //区间逻辑检查状态(0关闭-隐藏；1绿色-启用；2红色-停用；3黄色-异常；)
    int sectionLogicCheckState;

signals:

};

#endif // AUTOMATICBLOCK_H
