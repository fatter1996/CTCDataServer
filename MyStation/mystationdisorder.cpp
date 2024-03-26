#include "mystation.h"

#include <QTextCodec>
#include <QtDebug>
#include <windows.h>


//根据id获取车站调度命令
DispatchOrderStation *MyStation::GetDisOrderRecvById(int id)
{
    DispatchOrderStation* pDisOrder = nullptr;
    for(int i=0; i<m_ArrayDisOrderRecv.size(); i++)
    {
        if(id == m_ArrayDisOrderRecv[i]->order_id)
        {
            pDisOrder = m_ArrayDisOrderRecv[i];
            break;
        }
    }
    return pDisOrder;
}
//根据id获取调度台调度命令
DispatchOrderDispatcher *MyStation::GetDisOrderDDTById(int id)
{
    DispatchOrderDispatcher* pDisOrder = nullptr;
    for(int i=0; i<m_ArrayDisOrderDisp.size(); i++)
    {
        if(id == m_ArrayDisOrderDisp[i]->order_id)
        {
            pDisOrder = m_ArrayDisOrderDisp[i];
            break;
        }
    }
    return pDisOrder;
}
//根据id获取机车调度命令
DispatchOrderLocomotive *MyStation::GetDisOrderJCById(int id)
{
    DispatchOrderLocomotive* pDisOrder = nullptr;
    for(int i=0; i<m_ArrayDisOrderLocomot.size(); i++)
    {
        if(id == m_ArrayDisOrderLocomot[i]->order_id)
        {
            pDisOrder = m_ArrayDisOrderLocomot[i];
            break;
        }
    }
    return pDisOrder;
}
//更新收到的调度台调度命令信息
DispatchOrderDispatcher *MyStation::updateDisorderDDT(QByteArray recvArray)
{
    //调度命令
    DispatchOrderDispatcher *pDisOrderDDT = pDisOrderDDT = new DispatchOrderDispatcher();
    pDisOrderDDT->station_id = this->getStationID();

    int nCount = 11;
    bool  bDel=false;
    //当前操作(1保存，2发送,3删除)
    int currOprate = (int)(recvArray[nCount]&0xFF);
    if(currOprate==1)
    {
        pDisOrderDDT->bSend=false;
    }
    else if(currOprate==2)
    {
        pDisOrderDDT->bSend=true;
    }
    else if(currOprate==3)
    {
        bDel=true;
    }
    nCount+=1;
    //调度命令ID（数据库中自增）
    int getOrderId = ByteArrayToUInt(recvArray.mid(nCount,2));
    nCount+=2;
    //发送时判断是否已有本条调度命令
     bool border=false;
    if(2==currOprate||3==currOprate)
    {
        if(getOrderId!=0)
        {
            //获取既有的数据
            DispatchOrderDispatcher *pDisOrderDDTExist = this->GetDisOrderDDTById(getOrderId);
            if(pDisOrderDDTExist!=nullptr)
            {
                pDisOrderDDT = pDisOrderDDTExist;
                border=true;
            }
        }
    }
    pDisOrderDDT->bDel=bDel;
    //命令号
    pDisOrderDDT->uNumber = ByteArrayToUInt(recvArray.mid(nCount,4));
    nCount+=4;
    //命令类型
    {
        //长度
        int len = (int)(recvArray[nCount]&0xFF);
        nCount += 1;
        //内容
        QString strTxt;
        strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
        pDisOrderDDT->strType = strTxt;
        nCount += len;
    }
    //命令内容
    {
        //长度
//        int len = (int)(recvArray[nCount]&0xFF);
//        nCount += 1;
        int len = (int)((recvArray[nCount]&0xFF) | ((recvArray[nCount+1]&0xFF)<<8));
        nCount += 2;
        //内容
        QString strTxt;
        strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
        pDisOrderDDT->strContent = strTxt;
        nCount += len;
    }
    //车站
    {
        //长度
        int len = (int)(recvArray[nCount]&0xFF);
        nCount += 1;
        //内容
        QString strTxt;
        strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
        pDisOrderDDT->strStation = strTxt;
        nCount += len;
    }
    //值班人
    {
        //长度
        int len = (int)(recvArray[nCount]&0xFF);
        nCount += 1;
        //内容
        QString strTxt;
        strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
        pDisOrderDDT->strDutyName = strTxt;
        nCount += len;
    }
    //创建时间
    {
        int year  = ByteArrayToUInt(recvArray.mid(nCount,2));//(int)(recvArray[nCount] | recvArray[nCount+1]<<8);
        int mouth = ByteArrayToUInt(recvArray.mid(nCount+2,1));//(int)recvArray[nCount+2];
        int day   = ByteArrayToUInt(recvArray.mid(nCount+3,1));//(int)recvArray[nCount+3];
        int hour  = ByteArrayToUInt(recvArray.mid(nCount+4,1));//(int)recvArray[nCount+4];
        int mini  = ByteArrayToUInt(recvArray.mid(nCount+5,1));//(int)recvArray[nCount+5];
        int second = 0;//(int)recvArray[nCount+6];
        //"2019-03-31 12:24:36";
        //QString strDateTime= QString("%1-%2-%3 %4:%5:%6").arg(year).arg(mouth).arg(day).arg(hour).arg(mini).arg(second);
        QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
        qDebug()<<"创建时间="<<strDateTime;
        QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
        pDisOrderDDT->timCreate = dateTime;
        nCount += 7;
    }
    //调度台个数
    int ddtCount = (int)(recvArray[nCount]&0xFF);
    nCount += 1;
    //调度台内容
    for(int i=0; i<ddtCount; i++)
    {
        DispatcherInfo ddtInfo;
        //调度台
        {
            //长度
            int len = (int)(recvArray[nCount]&0xFF);
            nCount += 1;
            if(len>0)
            {
                //内容
                QString strTxt;
                strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
                ddtInfo.strDispatcher = strTxt;
                nCount += len;
            }
        }
        //增加一条
        if(!border)
        {
            pDisOrderDDT->vectDispathInfo.append(ddtInfo);
        }
    }

    //发送操作，自动接收
    if(2==currOprate)
    {
        QDateTime timeNow = QDateTime::currentDateTime();
        pDisOrderDDT->timSend = timeNow;
        pDisOrderDDT->bSend = true;
        for(int i=0; i<pDisOrderDDT->vectDispathInfo.size(); i++)
        {
            pDisOrderDDT->vectDispathInfo[i].timRecv = timeNow;
            pDisOrderDDT->vectDispathInfo[i].nRecvState = 1;
        }
    }

    return pDisOrderDDT;
}
//更新收到的机车调度命令信息
DispatchOrderLocomotive *MyStation::updateDisorderJC(QByteArray recvArray)
{
    //调度命令
    DispatchOrderLocomotive *pDisOrderJC = new DispatchOrderLocomotive();
    pDisOrderJC->station_id = this->getStationID();

    int nCount = 11;
    //当前操作(1保存，2发送,3删除)
    int currOprate = (int)(recvArray[nCount]&0xFF);
    bool  bDel=false;
    if(currOprate==1)
    {
        pDisOrderJC->bSend=false;
    }
    else if(currOprate==2)
    {
        pDisOrderJC->bSend=true;
    }
    else if(currOprate==3)
    {
        bDel=true;
    }
    nCount+=1;
    //调度命令ID（数据库中自增）
    int getOrderId = ByteArrayToUInt(recvArray.mid(nCount,2));
    nCount+=2;
    //发送时判断是否已有本条调度命令
    bool border=false;
    if(2==currOprate||3==currOprate)
    {
        if(getOrderId!=0)
        {
            //获取既有的数据
            DispatchOrderLocomotive *pDisOrderJCExist = this->GetDisOrderJCById(getOrderId);
            if(pDisOrderJCExist!=nullptr)
            {
                pDisOrderJC = pDisOrderJCExist;
                border=true;
            }
        }
    }
    //
    pDisOrderJC->bDel=bDel;
    //命令种类
    pDisOrderJC->orderType = (int)(recvArray[nCount]&0xFF);
    nCount += 1;
    //命令号
    pDisOrderJC->uNumber = ByteArrayToUInt(recvArray.mid(nCount,4));
    nCount+=4;
    //命令类型
    {
        //长度
        int len = (int)(recvArray[nCount]&0xFF);
        nCount += 1;
        //内容
        QString strTxt;
        strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
//        if(0x2==pDisOrderJC->orderType)
//        {
//            strTxt = "路票";//路票解析后文字乱码，此处特殊处理
//        }
        switch (pDisOrderJC->orderType)
        {
        case 2 : strTxt = "路票"; break;
        case 3 : strTxt = "绿色许可证"; break;
        case 4 : strTxt = "红色许可证"; break;
        case 5 : strTxt = "调车作业单"; break;
        case 6 : strTxt = "书面通知"; break;
        case 7 : strTxt = "半自动闭塞发车进路通知书"; break;
        case 8 : strTxt = "自动站间闭塞发车进路通知书"; break;
        }
        pDisOrderJC->strType = strTxt;
        nCount += len;
    }
    //命令内容
    {
        //长度
//        //int len = (int)(recvArray[nCount]&0xFF);
//        int a = (int)(recvArray[nCount]&0xFF);
//        int b=(int)(recvArray[nCount+1]&0xFF);
//        int len=a<<8;
//        len=len+b;
//        nCount += 1;
//        nCount += 1;
        int len = (int)((recvArray[nCount]&0xFF) | ((recvArray[nCount+1]&0xFF)<<8));
        nCount += 2;
        //内容
        QString strTxt;
        strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
        pDisOrderJC->strContent = strTxt;
        nCount += len;
    }
    //车站
    {
        //长度
        int len = (int)(recvArray[nCount]&0xFF);
        nCount += 1;
        //内容
        QString strTxt;
        strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
        pDisOrderJC->strStation = strTxt;
        nCount += len;
    }
    //值班人
    {
        //长度
        int len = (int)(recvArray[nCount]&0xFF);
        nCount += 1;
        //内容
        QString strTxt;
        strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
        pDisOrderJC->strDutyName = strTxt;
        nCount += len;
    }
    //创建时间
    pDisOrderJC->timCreate = QDateTime::currentDateTime();
    //车次（机车）个数
    int jcCount = (int)(recvArray[nCount]&0xFF);
    nCount += 1;
    //车次（机车）内容
    for(int i=0; i<jcCount; i++)
    {
        LocomotiveInfo jcInfo;
        //车次
        {
            //长度
            int len = (int)(recvArray[nCount]&0xFF);
            nCount += 1;
            if(len>0)
            {
                //内容
                QString strTxt;
                strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
                jcInfo.strCheCi = strTxt;
                nCount += len;
            }
        }
        //机车
        {
            //长度
            int len = (int)(recvArray[nCount]&0xFF);
            nCount += 1;
            if(len>0)
            {
                //内容
                QString strTxt;
                strTxt = ByteArrayToUnicode(recvArray.mid(nCount,len));
                jcInfo.strLocomotive = strTxt;
                nCount += len;
            }
        }
        //增加一条
        if(!border)
        {
            pDisOrderJC->vectLocmtInfo.append(jcInfo);
        }
    }

    //发送操作，自动接收
    if(2==currOprate)
    {
        QDateTime timeNow = QDateTime::currentDateTime();
        pDisOrderJC->timSend = timeNow;
        pDisOrderJC->bSend = true;
        for(int i=0; i<pDisOrderJC->vectLocmtInfo.size(); i++)
        {
            pDisOrderJC->vectLocmtInfo[i].timRecv = timeNow;
            pDisOrderJC->vectLocmtInfo[i].nRecvState = 1;
        }
    }

    return pDisOrderJC;
}


