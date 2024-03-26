#include "dispatchorderdispatcher.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DispatchOrderDispatcher::DispatchOrderDispatcher()
{

}
//调度台接收信息转JSON字符串
QString DispatchOrderDispatcher::ToJsonString()
{
    QJsonObject outJsonObject;
    QJsonArray outJsonArray;
    for(auto ement:vectDispathInfo)
    {
        QJsonObject outJson;
        outJson.insert("Dispatcher",  ement.strDispatcher.toUtf8().data());
        outJson.insert("nRecvState", ement.nRecvState);
        outJson.insert("timRecv", ement.timRecv.toString().size()?ement.timRecv.toString("yyyy-MM-dd hh:mm:ss"):"NULL");
        outJsonArray.append(outJson);
    }
    outJsonObject.insert("vectDispathInfo", QJsonValue(outJsonArray));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(outJsonObject);
    QByteArray byteArray = jsonDoc.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    qDebug()<<"strJson="<<strJson;
    return strJson;
}
//JSON字符串转调度台接收信息
void DispatchOrderDispatcher::JsonStringToData(QString strData)
{
    QByteArray byteArray = strData.toUtf8();//.toLocal8Bit();//toLatin1();
    QJsonParseError jsonParseError;     //JSON错误代码
    QJsonDocument jsonDoc=QJsonDocument::fromJson( byteArray, &jsonParseError);
    QJsonObject jsonObject=jsonDoc.object();

    vectDispathInfo.clear();
    if(jsonObject.contains("vectDispathInfo"))
    {
        QJsonValue value_Data = jsonObject.value("vectDispathInfo");
        if (value_Data.isArray())
        {
            QJsonArray JsonArray_Data = value_Data.toArray();
            int nSize = JsonArray_Data.size();
            for (int i = 0; i < nSize; ++i)
            {
                QJsonValue jc_value = JsonArray_Data.at(i);
                if(jc_value.isObject())
                {
                    QJsonObject jc_Jobject = jc_value.toObject();
                    DispatcherInfo ddtInfo;
                    ddtInfo.strDispatcher = jc_Jobject["Dispatcher"].toString();
                    ddtInfo.nRecvState = jc_Jobject["nRecvState"].toInt();
                    QString strTime = jc_Jobject["timRecv"].toString();
                    if(strTime != "NULL")
                    {
                        ddtInfo.timRecv = QDateTime::fromString(strTime, "yyyy-MM-dd hh:mm:ss");
                    }
                    vectDispathInfo.append(ddtInfo);
                }
            }
        }
    }
}
