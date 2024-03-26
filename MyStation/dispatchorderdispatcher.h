#ifndef DISPATCHORDERDISPATCHER_H
#define DISPATCHORDERDISPATCHER_H

#include <QString>
#include <QDateTime>


//调度台信息
struct DispatcherInfo{
    QString strDispatcher;//调度台
    QDateTime timRecv;//调度台接收时间
    int nRecvState = 0;//接收状态 1:接收 0:未接收
};

//调度台调度命令
class DispatchOrderDispatcher
{
public:
    DispatchOrderDispatcher();

public:
    int station_id = 0;
    int order_id = 0;//id
    int uNumber = 0;//命令号
    QString strType;//命令类型
    QString strContent;//命令内容
    QString strStation;//车站
    QString strDutyName;//值班人
    QDateTime timCreate;//创建时间
    bool bSend = false;//发送状态，0保存未发送，1已发送
    bool bDel=false;//删除指令
    QDateTime timSend;//发送时间
    QVector<DispatcherInfo> vectDispathInfo;//调度台接收信息
public:
    QString ToJsonString();//调度台接收信息转JSON字符串
    void JsonStringToData(QString strData);//JSON字符串转调度台接收信息

};

#endif // DISPATCHORDERDISPATCHER_H
