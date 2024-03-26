#ifndef DISPATCHORDERSTATION_H
#define DISPATCHORDERSTATION_H

#include <QString>
#include <QDateTime>

//车站调度命令（从调度台接收到的）
class DispatchOrderStation
{
public:
    DispatchOrderStation();
    ~DispatchOrderStation();

public:
    int station_id = 0;
    int order_id = 0;//id
    int uNumber = 0;//命令号
    QString strNumber;//命令号
    QString strDisCenter;//调度中心
    QString strDisName;//调度员
    QString strType;//命令类型
    QString strContent;//命令内容
    QString strSignName;//签收人
    QDateTime timOrder;//接收时间
    QDateTime timSign;//签收时间
    QStringList listRecvPlace;//接收单位
    QStringList listReadName;//阅读人
    int nStateDisOrder = 2;//调度命令签收状态 1:保存 2:发送未签收 3:STPC签收 4:拒签 0:信息无效
};

#endif // DISPATCHORDERSTATION_H
