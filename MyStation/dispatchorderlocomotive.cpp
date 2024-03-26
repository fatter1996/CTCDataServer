#include "dispatchorderlocomotive.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DispatchOrderLocomotive::DispatchOrderLocomotive()
{

}

//机车信息转JSON字符串
QString DispatchOrderLocomotive::ToJsonString()
{
    QJsonObject outJsonObject;
    QJsonArray outJsonArray;
    for(auto ement:vectLocmtInfo)
    {
        QJsonObject outJson;
        outJson.insert("checi",  ement.strCheCi.toUtf8().data());
        outJson.insert("Locomotive",  ement.strLocomotive.toUtf8().data());
        outJson.insert("nRecvState", ement.nRecvState);
        outJson.insert("timRecv", ement.timRecv.toString().size()?ement.timRecv.toString("yyyy-MM-dd hh:mm:ss"):"NULL");
        outJsonArray.append(outJson);
    }
    outJsonObject.insert("vectLocmtInfo", QJsonValue(outJsonArray));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(outJsonObject);
    QByteArray byteArray = jsonDoc.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    qDebug()<<"strJson="<<strJson;
    return strJson;
}
//JSON字符串转机车信息
void DispatchOrderLocomotive::JsonStringToData(QString strData)
{
    QByteArray byteArray = strData.toLatin1();
    QJsonParseError jsonParseError;     //JSON错误代码
    QJsonDocument jsonDoc=QJsonDocument::fromJson( byteArray, &jsonParseError);
    QJsonObject jsonObject=jsonDoc.object();

    vectLocmtInfo.clear();
    if(jsonObject.contains("vectLocmtInfo"))
    {
        QJsonValue value_Data = jsonObject.value("vectLocmtInfo");
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
                    LocomotiveInfo locoInfo;
                    locoInfo.strCheCi = jc_Jobject["checi"].toString();
                    locoInfo.strLocomotive = jc_Jobject["Locomotive"].toString();
                    locoInfo.nRecvState = jc_Jobject["nRecvState"].toInt();
                    QString strTime = jc_Jobject["timRecv"].toString();
                    if(strTime != "NULL")
                    {
                        locoInfo.timRecv = QDateTime::fromString(strTime, "yyyy-MM-dd hh:mm:ss");
                    }
                    vectLocmtInfo.append(locoInfo);
                }
            }
        }
    }
}












