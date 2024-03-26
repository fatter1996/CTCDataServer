#include "mydoc.h"
#include <QtDebug>
#include <QSettings>
#include <QMessageBox>
#include <windows.h>
#include "GlobalHeaders/GlobalFuntion.h"

//解析LS+CTC+占线板的socket通道数据
void MyDoc::receiveAllLSCTCDataSlot(QByteArray dataArray, QString clientAdd, int clientPort)
{
    unsigned char RecvArray[2048]={0};
    int nLength=0;
    if((dataArray.size() < 6) || (dataArray.size() >= 2048))
    {
        return;   //防止越界
    }
    memset(RecvArray,0,2048);
    memcpy(RecvArray,dataArray,dataArray.size());  //转换数据类型
    nLength=(int)(RecvArray[4] | (RecvArray[5]<<8));
    if(nLength != dataArray.size())
    {
        qDebug()<<" 接收长度与数据解析长度不符！ "<<nLength<<dataArray.size()<<ByteArrayToString(dataArray);
        return;
    }
    if((RecvArray[0] == 0xEF) && (RecvArray[1] == 0xEF) && (RecvArray[2] == 0xEF) && (RecvArray[3] == 0xEF)
       && (RecvArray[nLength-1] == 0xFE) && (RecvArray[nLength-2] == 0xFE) && (RecvArray[nLength-3] == 0xFE) && (RecvArray[nLength-4] == 0xFE))//帧头帧尾校验成功
    {
        //来源地址和端口
        //qDebug()<<"收到[主通道]数据"<<QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM)<<"client_add="<<client_add<<"client_port="<<client_port;
        //QHostAddress qHostAdd = QHostAddress(client_add);
        //数据类型
        int dataType = (int)RecvArray[8];
        if(dataType == DATATYPE_LS)
        {
            //qDebug()<<"收到[联锁]数据.";
            //各站联锁数据
            //StatusChangeNew(dataArray, nLength, client_add, client_port);
            recvAllLSData(dataArray, nLength, clientAdd, clientPort);
        }
        else if(dataType == DATATYPE_CTC)
        {
            //qDebug()<<"收到[CTC]数据.";
            //各站CTC数据
            recvAllCTCData(RecvArray, nLength, clientAdd, clientPort);
        }
        else if(dataType == DATATYPE_BOARD)
        {
            //qDebug()<<"收到[占线板]数据.";
            //各站占线板数据
            recvAllBoardData(RecvArray, nLength, clientAdd, clientPort);
        }
        else if(dataType == DATATYPE_JK)
        {
            //qDebug()<<"收到[集控台]数据.";
            //各站集控台数据
            recvAllJKData(RecvArray, nLength, clientAdd, clientPort);
        }
        else if(dataType == DATATYPE_ZXT)
        {
            //qDebug()<<"收到[占线图]数据.";
            //各站占线图数据
            recvAllZXTData(RecvArray, nLength, clientAdd, clientPort);
        }
    }
}
//解析教师机的socket通道数据
void MyDoc::receiveTeacherDataSlot(QByteArray array, QString client_add, int client_port)
{
    unsigned char RecvArray[2048]={0};
    int nLength=0;
    int arrSize = array.size();

    if((arrSize < 6) || (arrSize >= 2048))
    {
        return;   //防止越界
    }
    memset(RecvArray,0,2048);
    memcpy(RecvArray,array,array.size());  //转换数据类型
    nLength=(int)(RecvArray[4] | (RecvArray[5]<<8));
    if(nLength != array.size())
    {
        QLOG_ERROR()<<"教师机 接收长度与数据解析长度不符！ "<<nLength<<array.size();
        return;
    }

    if((RecvArray[0] == 0xEF) && (RecvArray[1] == 0xEF) && (RecvArray[2] == 0xEF) && (RecvArray[3] == 0xEF)
       && (RecvArray[nLength-1] == 0xFE) && (RecvArray[nLength-2] == 0xFE) && (RecvArray[nLength-3] == 0xFE) && (RecvArray[nLength-4] == 0xFE))//帧头帧尾校验成功
    {
        //来源地址和端口
        //qDebug()<<"收到[教师机]数据"<<QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM)<<"client_add="<<client_add<<"client_port="<<client_port;
        //防错，教师机数据错误
        if(0x76 == RecvArray[16])
        {
            RecvArray[16] = 0x00;
            array[16] = 0x00;//防错，教师机数据错误
        }
        recvTeacherData((unsigned char *)array.data(), nLength);
    }
}
//解析培训软件的socket通道数据
void MyDoc::receiveTrainingDataSlot(QByteArray array, QString client_add, int client_port)
{
    unsigned char RecvArray[2048]={0};
    int nLength=0;
    if((array.size() < 6) || (array.size() >= 2048))
    {
        return;   //防止越界
    }
    memset(RecvArray,0,2048);
    memcpy(RecvArray,array,array.size());  //转换数据类型
    nLength=(int)(RecvArray[4] | (RecvArray[5]<<8));
    if(nLength != array.size())
    {
        qDebug()<<" 接收长度与数据解析长度不符！ "<<nLength<<array.size();
        return;
    }

    if((RecvArray[0] == 0xEF) && (RecvArray[1] == 0xEF) && (RecvArray[2] == 0xEF) && (RecvArray[3] == 0xEF)
       && (RecvArray[nLength-1] == 0xFE) && (RecvArray[nLength-2] == 0xFE) && (RecvArray[nLength-3] == 0xFE) && (RecvArray[nLength-4] == 0xFE))//帧头帧尾校验成功
    {
//        //来源地址和端口
//        //qDebug()<<"收到[培训软件]数据"<<QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM)<<"client_add="<<client_add<<"client_port="<<client_port;
//        //qDebug()<<"UDP="<<ByteArrayToString(array);
//        //QHostAddress qHostAdd = QHostAddress(client_add);
//        //数据类型
//        //unsigned int dataType = RecvArray[8];
//        //if(dataType == DATATYPE_TRAIN)
        {
            RecvTrainningData(RecvArray, nLength, client_add, client_port);
        }
    }
}
//槽-发送数据给教师机
void MyDoc::sendDataToTeacherSlot(MyStation *pMyStation, QByteArray dataArray, int len)
{
    //发送数据至教师机
    sendDataToJSJ(dataArray, len);
}
//槽-发送数据给联锁
void MyDoc::sendDataToLSSlot(MyStation *pMyStation, QByteArray dataArray, int len)
{
    sendDataToLS(pMyStation, dataArray, len);
}
//槽-发送数据给CTC
void MyDoc::sendDataToCTCSlot(MyStation *pMyStation, QByteArray dataArray, int len, int currCtcIndex)
{
    int revcType = (int)(dataArray[9]&0xFF);
    //站场状态 || 列车车次信息
    if(revcType == 0x11 || revcType == 0x66)
    {
        //发送给所有车站的CTC(本站、其他站站间透明使用)
        for (int s = 0; s < vectMyStation.size(); s++)
        {
            MyStation* pStation = vectMyStation.at(s);
            sendDataToCTC(pStation, dataArray, len);
        }
    }
    else
    {
        sendDataToCTC(pMyStation, dataArray, len, currCtcIndex);
    }
}
//槽-发送数据给占线板
void MyDoc::sendDataToBoardSlot(MyStation *pMyStation, QByteArray dataArray, int len)
{
    sendDataToBoard(pMyStation, dataArray, len);
}
//槽-发送数据给集控台
void MyDoc::sendDataToJKSlot(MyStation *pMyStation, QByteArray dataArray, int len, int currSoftIndex)
{
    sendDataToJK(pMyStation, dataArray, len, currSoftIndex);
}
//槽-发送数据给占线图
void MyDoc::sendDataToZXTSlot(MyStation *pMyStation, QByteArray dataArray, int len, int currSoftIndex)
{
    sendDataToZXT(pMyStation, dataArray, len, currSoftIndex);
}
//槽--发送更新数据消息（给所有连接终端-CTC、占线板）
void MyDoc::sendUpdateDataMsgSlot(MyStation *pStation, int _type, int currCtcIndex, int currZxbIndex)
{
    sendUpdateDataMsg(pStation, _type, currCtcIndex, currZxbIndex);
}
//槽--更新邻站报点(邻站id，车次，报点类型-0x22到达，0x11出发，0x33通过，报点时间)
void MyDoc::UpdateLZReportTimeSlot(int lzId, QString checi, int type, QDateTime dateTime)
{
    for (int s = 0; s < vectMyStation.size(); s++)
    {
        MyStation* pStation = vectMyStation.at(s);
        if(lzId == (int)pStation->getStationID())
        {
            pStation->UpdateLZReportTime(checi,type,dateTime);
            break;
        }
    }
}

////发送心跳信息给教师机
//void MyDoc::sendHeartBeatToJSJ()
//{
//    int nLength = 60;
//    unsigned int stationID=0x00;
//    QByteArray byArrayUDPDate;
//    //byArrayUDPDate.resize(nLength);
//    byArrayUDPDate.append(nLength, char(0));//添加20个字节并全部置零
//    for (int i = 0; i < 4; i++)
//    {
//        byArrayUDPDate[i] = 0xEF;
//        byArrayUDPDate[nLength - i - 1] = 0xFE;
//    }
//    byArrayUDPDate[9] = 0x23;//分类信息码
//    byArrayUDPDate[10] = CTC_TYPE_CTC2_CASCO;//CTC制式
//    //每个站都发送心跳
//    for (int i = 0; i < vectMyStation.size(); i++)
//    {
//        MyStation* pStation = vectMyStation.at(i);
//        if (pStation)
//        {
//            stationID = pStation->getStationID();
//            //memcpy(&byArrayUDPDate[6], &stationID, 1);//车站id
//            byArrayUDPDate[6] = stationID;
//            sendDataToJSJ(byArrayUDPDate, nLength);
//        }
//    }
//}
//发送数据至教师机
void MyDoc::sendDataToJSJ(QByteArray pSendDate, int nLength)
{
    emit sendDataToTeacherSignal(pSendDate, teacherAddr, teacherPort, nLength);
    //qDebug()<<"[向教师机发送数据]"<<ByteArrayToString0x(pSendDate);
}
//发送状态给所有车站CTC（各终端的站间透明使用）
void MyDoc::sendStatusToAllStationCTC(QByteArray pSendDate, int nLength)
{
    //车站id
    int nStaid = pSendDate[6];
    int revcType = (int)(pSendDate[9]&0xFF);
    //站场状态不发送，只发送其他类型的数据
    if(0x11 == revcType)
    {
        return;
    }
    pSendDate[8]=DATATYPE_LS;//联锁类型
    for (int s = 0; s < vectMyStation.size(); s++)
    {
        MyStation* pStation = vectMyStation.at(s);
        //发送给所有车站的CTC(本站、其他站站间透明使用)
        //sendDataToCTC(pStation, pSendDate, nLength);
        //仅发送给本车站的占线板
        if(nStaid == (int)pStation->getStationID())
        {
            //发送给车站的所有占线板
            sendDataToBoard(pStation, pSendDate, nLength);
        }

        //站场状态单独发送
//        QByteArray sendArray = pStation->packStationStatusToCTC();
//        qDebug()<<"sendArray="<<ByteArrayToString(sendArray);
//        if(sendArray.size() > 14)
//        {
//            int sendLenth = (int)(sendArray[4]&0xFF | (sendArray[5]&0xFF)<<8);
//            //发送给所有车站的CTC(本站、其他站站间透明使用)
//            sendDataToCTC(pStation, sendArray, sendLenth);
//            //仅发送给本车站的占线板
//            if(nStaid == (int)pStation->getStationID())
//            {
//                //发送给车站的所有占线板
//                sendDataToBoard(pStation, sendArray, sendLenth);
//            }
//        }
    }
}
//发送状态给所有车站的Soft
void MyDoc::sendStatusToAllStationSoft(QByteArray pSendDate, int nLength)
{
    //车站id
    int nStaid = pSendDate[6];
    int revcType = (int)(pSendDate[9]&0xFF);
    //站场状态+实时车次信息
    if(/*0x11 == revcType ||*/ 0x66 == revcType)
    {
        pSendDate[8]=DATATYPE_LS;//联锁类型
        for (int s = 0; s < vectMyStation.size(); s++)
        {
            MyStation* pStation = vectMyStation.at(s);
            //仅发送给本车站的Soft
            if(nStaid == (int)pStation->getStationID())
            {
                //发送给车站的所有集控
                sendDataToJK(pStation, pSendDate, nLength);
                //发送给车站的所有占线图
                //sendDataToZXT(pStation, pSendDate, nLength);
            }
        }
    }
}
//向pStation车站的联锁发送数据
void MyDoc::sendDataToLS(MyStation *pStation, QByteArray pSendDate, int nLength)
{
    if(pStation->isLSConnected)
    {
        emit sendDataToMainSignal(pSendDate, pStation->lsAddr, pStation->lsPort, nLength);
    }
}
//向pStation车站所有的CTC终端发送数据
void MyDoc::sendDataToCTC(MyStation *pStation, QByteArray pSendDate, int nLength, int currClientIndex)
{
    if(-1 == currClientIndex)
    {
        //发给本站所有CTC终端
        for (int j=0; j<10; j++)
        {
            //if(pStation->ctcAddr[j] != "" && pStation->ctcPort[j] != 0)
            if(pStation->isCTCConnected[j])
            {
                emit sendDataToMainSignal(pSendDate, pStation->ctcAddr[j], pStation->ctcPort[j], nLength);
//                qDebug()<<QString("发送站场状态：id=%1,%2,%3,%4")
//                          .arg(pStation->getStationID()).arg(pStation->getStationName())
//                          .arg(pStation->ctcAddr[j]).arg(pStation->ctcPort[j]);
            }
        }
    }
    else
    {
        for (int j=0; j<10; j++)
        {
            //if(currClientIndex != j)//发给本站当前CTC终端以外的其余终端
            if(currClientIndex == j)//发给本站当前CTC终端
            {
                //if(pStation->ctcAddr[j] != "" && pStation->ctcPort[j] != 0)
                if(pStation->isCTCConnected[j])
                {
                    emit sendDataToMainSignal(pSendDate, pStation->ctcAddr[j], pStation->ctcPort[j], nLength);
                }
            }
        }
    }
}
//向pStation车站所有的占线板终端发送数据
void MyDoc::sendDataToBoard(MyStation *pStation, QByteArray pSendDate, int nLength, int currClientIndex)
{
    if(-1 == currClientIndex)
    {
        //发给本站所有占线板终端
        for (int j=0; j<10; j++)
        {
            //if(pStation->zxbAddr[j] != "" && pStation->zxbPort[j] != 0)
            if(pStation->isBoardConnected[j])
            {
                emit sendDataToMainSignal(pSendDate, pStation->zxbAddr[j], pStation->zxbPort[j], nLength);
            }
        }
    }
    else
    {
        //发给本站当前CTC终端以外的其余终端
        for (int j=0; j<10; j++)
        {
            if(currClientIndex != j)
            {
                //if(pStation->zxbAddr[j] != "" && pStation->zxbPort[j] != 0)
                if(pStation->isBoardConnected[j])
                {
                    emit sendDataToMainSignal(pSendDate, pStation->zxbAddr[j], pStation->zxbPort[j], nLength);
                }
            }
        }
    }
}
//向pStation车站所有的集控终端发送数据
void MyDoc::sendDataToJK(MyStation *pStation, QByteArray pSendDate, int nLength, int currClientIndex)
{
    if(-1 == currClientIndex)
    {
        //发给本站所有JK终端
        for (int j=0; j<10; j++)
        {
            //if(pStation->jkAddr[j] != "" && pStation->jkPort[j] != 0)
            if(pStation->isJKConnected[j])
            {
                emit sendDataToMainSignal(pSendDate, pStation->jkAddr[j], pStation->jkPort[j], nLength);
            }
        }
    }
    else
    {
        //发给本站当前JK终端以外的其余终端
        for (int j=0; j<10; j++)
        {
            if(currClientIndex != j)
            {
                //if(pStation->jkAddr[j] != "" && pStation->jkPort[j] != 0)
                if(pStation->isJKConnected[j])
                {
                    emit sendDataToMainSignal(pSendDate, pStation->jkAddr[j], pStation->jkPort[j], nLength);
                }
            }
        }
    }
}
//向pStation车站所有的占线图终端发送数据
void MyDoc::sendDataToZXT(MyStation *pStation, QByteArray pSendDate, int nLength, int currClientIndex)
{
    if(-1 == currClientIndex)
    {
        //发给本站所有ZXT终端
        for (int j=0; j<10; j++)
        {
            //if(pStation->jkAddr[j] != "" && pStation->jkPort[j] != 0)
            if(pStation->isZXTConnected[j])
            {
                emit sendDataToMainSignal(pSendDate, pStation->zxtAddr[j], pStation->zxtPort[j], nLength);
            }
        }
    }
    else
    {
        //发给本站当前ZXT终端以外的其余终端
        for (int j=0; j<10; j++)
        {
            if(currClientIndex != j)
            {
                //if(pStation->zxtAddr[j] != "" && pStation->zxtPort[j] != 0)
                if(pStation->isZXTConnected[j])
                {
                    emit sendDataToMainSignal(pSendDate, pStation->zxtAddr[j], pStation->zxtPort[j], nLength);
                }
            }
        }
    }
}
//发送更新数据消息（给所有连接终端-CTC、占线板）
void MyDoc::sendUpdateDataMsg(MyStation *pStation, int _type, int currCtcIndex, int currZxbIndex)
{
    //不采用，直接返回，使用TCP通道发送数据
    return;

    int nLength = 30;//60;
    QByteArray byArrayUDPDate;
    byArrayUDPDate.append(nLength, char(0));//添加60个字节并全部置零
    for(int i = 0; i < 4; i++)
    {
        byArrayUDPDate[i] = 0xEF;
        byArrayUDPDate[nLength - i -1] = 0xFE;
    }
    byArrayUDPDate[4] = nLength;//帧长度
    byArrayUDPDate[5] = nLength<<8;//帧长度
    byArrayUDPDate[6] = pStation->getStationID();
    byArrayUDPDate[8] = DATATYPE_SERVER;//DATATYPE_CTRL;
    byArrayUDPDate[9] = FUNCTYPE_UPDATE;//分类信息码
    byArrayUDPDate[10] = _type;//子分类信息码
    //发给本站所有的CTC、占线板终端
    {
        sendDataToCTC(pStation, byArrayUDPDate, nLength, currCtcIndex);
        sendDataToBoard(pStation, byArrayUDPDate, nLength, currZxbIndex);
    }
}
//联锁接口数据处理
void MyDoc::recvAllLSData(QByteArray recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)recvArray[6];
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    int revcType = (int)(recvArray[9]&0xFF);
    //qDebug()<<QString("接收到【联锁终端%1】的数据").arg(nStaid);

    //保存接收到数据的socket信息，用于向对方发送数据
    pStation->lsAddr = clientAdd;
    pStation->lsPort = clientPort;
    pStation->m_nTimeLSBreak = 0;
    if(pStation->isLSConnected==false)
    {
        QLOG_INFO()<<QString("%1与联锁通信成功！").arg(pStation->getStationName());
    }
    pStation->isLSConnected = true;
    //pStation->IsCommunicationsNormal = true;  //有效通信数据解析后置相关标志量为真
    ////pStation->IrregularCycle = 0;  //有效通信数据解析后置相关非正常通信周期计数为零
    //qDebug()<<QString("接收到【联锁终端%1】的数据").arg(nStaid);
//    qDebug()<<QString("收到联锁站场状态：id=%1,%2,%3,%4")
//              .arg(pStation->getStationID()).arg(pStation->getStationName())
//              .arg(pStation->lsAddr).arg(pStation->lsPort);
    if(revcType != 0x00)
    {
        QByteArray sendArrayNew;
        sendArrayNew.append(recvArray);
        //打包CTC分散自律状态
        if(revcType == 0x11)
        {
            int dataLength=(int)((sendArrayNew[4]&0xFF) | (sendArrayNew[5]&0xFF)<<8);
            if( (int)(sendArrayNew[dataLength-5]&0xFF) != 0x22)//不是联锁非常站控模式
            {
                //修改状态帧，增加CTC的分散自律状态
                //0 中心控制 1 车站控制 2 分散自律
                if(0 == pStation->ModalSelect.nModeState)
                {
                    sendArrayNew[dataLength-5] = 0x00;
                }
                else if(1 == pStation->ModalSelect.nModeState)
                {
                    sendArrayNew[dataLength-5] = 0x01;
                }
                else if(2 == pStation->ModalSelect.nModeState)
                {
                    sendArrayNew[dataLength-5] = 0x02;
                }
            }
        }
        //发送给Qt老版本占线板
        sendStatusToAllStationCTC(sendArrayNew, recvlen);
        //发送给MFC集控终端
        sendStatusToAllStationSoft(sendArrayNew, recvlen);
    }

    Mutex.lock();
    //站场平面图信息
    if(revcType == 0x11)
    {
        pStation->updateDevStateOfLS((unsigned char *)recvArray.data());
        if(pStation->m_bModeChanged)
        {
            pStation->m_bModeChanged = false;
            m_pDataAccess->UpdateStationInfo(pStation);
            qDebug()<<QString("非常站控模式转换！id=%1，%2").arg(pStation->getStationID()).arg(pStation->getStationName());
        }
    }
    //行车日志列车报点信息
    else if(revcType == 0x55)
    {
        //RecvLSData_ReportTime(pStation, (unsigned char *)recvArray.data(), recvlen);
        pStation->recvLSData_ReportTime((unsigned char *)recvArray.data(), recvlen);
    }
    //联锁列车实时位置报点
    else if(revcType == 0x66)
    {
        //RecvLSData_TrainPos(pStation, (unsigned char *)recvArray.data(), recvlen);
        pStation->recvLSData_TrainPos((unsigned char *)recvArray.data(), recvlen);
    }
    //非常站控
    else if(revcType == 0x77)
    {
        //RecvLSData_FCZK(pStation, (unsigned char *)recvArray.data(), recvlen);
        pStation->recvLSData_FCZK((unsigned char *)recvArray.data(), recvlen);
    }
    //调度命令
    else if(revcType == 0x0c)
    {
        //RecvLSData_DDML(pStation, (unsigned char *)recvArray.data(), recvlen);
        pStation->recvLSData_DDML((unsigned char *)recvArray.data(), recvlen);
    }
    //阶段计划
    else if(revcType == 0x0b)
    {
        //RecvLSData_JDJH(pStation, (unsigned char *)recvArray.data(), recvlen);
        pStation->recvLSData_JDJH((unsigned char *)recvArray.data(), recvlen);
    }
    //邻站预告
    else if(revcType == 0x0d)
    {
        //RecvLSData_LZYG(pStation, (unsigned char *)recvArray.data(), recvlen);
        pStation->recvLSData_LZYG((unsigned char *)recvArray.data(), recvlen);
    }
    //进路办理回执
    else if(revcType == 0x1B)
    {
        //RecvLSData_RouteReturn(pStation, (unsigned char *)recvArray.data(), recvlen);
        pStation->recvLSData_RouteReturn((unsigned char *)recvArray.data(), recvlen);
    }
    Mutex.unlock();
}
//接受的CTC+占线板数据处理
void MyDoc::AnalyCTCData(MyStation* pStation, unsigned char *recvArray, int recvlen, int nClientIndex)
{
    //方法1
    // unsigned char *转为QByteArray
    //QByteArray qRecvArray;
    //qRecvArray.append(recvlen, char(0));//添加60个字节并全部置零
    //memcpy(qRecvArray.data(), recvArray, recvlen);//copy数据
    //方法2（取为公共方法）
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);

    //数据分类码
    int revcType = (int)(recvArray[9]&0xFF);

    Mutex.lock();
    //用户登录信息
    if(FUNCTYPE_USERLOGIN == revcType)
    {
        recvCTCDataUserLogin(pStation, recvArray, recvlen, nClientIndex);
    }
    //进路按钮按下（终端闪烁）
    else if(FUNCTYPE_BTNCLICK == revcType)
    {
        //recvCTCDataRouteBtnClick(pStation, recvArray, recvlen, nClientIndex);
        pStation->recvCTCData_RouteBtnClick(recvArray, recvlen);
    }
    //设备操作开始计时记录
    else if(FUNCTYPE_DEVOPERA == revcType)
    {
        pStation->setCmdCountDown();
    }
    //设备命令清除
    else if(FUNCTYPE_CMDCLEAR == revcType)
    {
        pStation->clearCmd();
    }
    //调度命令
    else if(FUNCTYPE_DISPTCH == revcType)
    {
        //RecvCTCData_RecvDisOrderInfo(pStation, recvArray, recvlen);
        pStation->recvCTCData_DisOrderInfo(recvArray, recvlen);
    }
    //阶段计划签收
    else if(FUNCTYPE_STAGEPL == revcType)
    {
        //签收
        if(0x01 == (int)recvArray[10])
        {
            if(0xAA == (int)recvArray[13])//签收-阶段计划
            {
                pStation->SignStagePlan(true);
            }
        }
        //发送计划
        else if(0x03 == (int)recvArray[10])
        {
            qDebug() << "收到发送计划功能";
            pStation->SendPlan();
        }
    }
    //修改后的阶段计划
    else if(FUNCTYPE_STAGEPLNEW == revcType)//0x33
    {
        //阶段计划发送给联锁
        sendDataToLS(pStation, qRecvArray, recvlen);
        qDebug()<<"CTC-STAGEPLAN-UDP="<<ByteArrayToString(qRecvArray);
        //阶段计划详情
        StagePlan* stagePlan = pStation->updateStagePlan(recvArray, recvlen);
        stagePlan->m_bModified = true;
        //阶段计划大计划
        Stage* stage = new Stage();
        stage->station_id = pStation->getStationID();
        stage->m_nPlanNum = stagePlan->m_nPlanNumber;
        stage->m_timRecv = QDateTime::currentDateTime();
        pStation->m_ArrayStagePlan.append(stagePlan);
        //qDebug()<<"bAutoSign="<<bAutoSign;
        //自动签收
        pStation->SignStagePlan(true);//false
    }
    //模式转换
    else if(FUNCTYPE_CHGMODE == revcType)
    {
        //控制模式转换
        if(0x01 == (int)recvArray[10])
        {
            int newMode = (int)recvArray[11];
            //
            pStation->CancleFlblKXFalg();
            //需要中心模拟同意
            if(pStation->StaConfigInfo.bChgModeNeedJSJ)
            {
                int currMode = pStation->ModalSelect.nModeState;
                int targtMode = newMode;
                pStation->ModalSelect.nRequestMode = targtMode;
                int nLength = 60;
                unsigned char byArrayUDPDate[60];
                unsigned int stationID=0x00;
                memset(byArrayUDPDate,0,nLength);
                for(int i = 0; i < 4; i++)
                {
                    byArrayUDPDate[i] = 0xEF;
                    byArrayUDPDate[nLength - i -1] = 0xFE;
                }
                memcpy(&byArrayUDPDate[4], &nLength, 2);//帧长度
                stationID = pStation->getStationID();
                memcpy(&byArrayUDPDate[6], &stationID, 1);//帧长度
                byArrayUDPDate[8] = 0xCC;//分类信息码
                byArrayUDPDate[9] = 0x2C;//分类信息码

                if((currMode != targtMode) && (targtMode != -1))
                {
                    if(currMode == 0)
                    {
                        byArrayUDPDate[10] = CTC_MODE_CENTER;
                    }
                    else if(currMode == 1)
                    {
                        byArrayUDPDate[10] = CTC_MODE_STATION;
                    }
                    else if(currMode == 2)
                    {
                        byArrayUDPDate[10] = CTC_MODE_NORMAL;
                    }

                    if(targtMode == 0)
                    {
                        byArrayUDPDate[11] = CTC_MODE_CENTER;
                    }
                    else if(targtMode == 1)
                    {
                        byArrayUDPDate[11] = CTC_MODE_STATION;
                    }
                    else if(targtMode == 2)
                    {
                        byArrayUDPDate[11] = CTC_MODE_NORMAL;
                    }
                    //pFrame->StationModalSelect[iRow-1].nRequestMode = targtMode;
                    //pDoc->SendUDPDataToJSJ(byArrayUDPDate, nLength);
                    //数据帧转换
                    QByteArray qSendArray = UnsignedCharToQByteArray(byArrayUDPDate, nLength);
                    sendDataToJSJ(qSendArray, nLength);
                }
            }
            else
            {
                int oldMode = pStation->ModalSelect.nModeState;
                pStation->ModalSelect.nModeState = newMode;
                pStation->ModalSelect.nRequestMode = -1;
                m_pDataAccess->UpdateStationInfo(pStation);
                ////GetStationInfo(pStation);
                //发送同步消息--随站场状态实时发送
                //sendUpdateDataMsg(pStation, UPDATETYPE_KZMS);
                //当前模式-0 中心控制 1 车站控制 2 分散自律
                qDebug() << "oldMode = " << oldMode << "newMode = " << newMode;
                if(0==oldMode && 1==newMode)
                {
                    QString msg = QString("%1的车站控制模式由中心控制转为车站控制").arg(pStation->getStationName());
                    //发送报警提示信息
                    pStation->sendWarningMsgToCTC(3,1,msg);
                    //进路序列自触标志全部清除
                    pStation->SetRouteAutoTouchState(false);
                    qDebug() << msg;
                }
                else if(2==oldMode && 1==newMode)
                {
                    QString msg = QString("%1的车站控制模式由车站调车转为车站控制").arg(pStation->getStationName());
                    //发送报警提示信息
                    pStation->sendWarningMsgToCTC(3,1,msg);
                    //进路序列自触标志全部清除
                    pStation->SetRouteAutoTouchState(false);
                    qDebug() << msg;
                }
                else if(1==oldMode && 0==newMode)
                {
                    QString msg = QString("%1的车站控制模式由车站控制转为中心控制").arg(pStation->getStationName());
                    qDebug() << msg;
                }
                else if(1!=newMode)
                {
                    QString msg = QString("%1的车站控制模式由车站控制转为%2").arg(pStation->getStationName())
                            .arg(0==newMode?"中心控制":"车站调车");
                    qDebug() << msg;
                    //发送报警提示信息
                    pStation->sendWarningMsgToCTC(3,1,msg);
                    //进路序列满足条件的列车全部自动自触标志
                    pStation->SetRouteAutoTouchState(true);
                }
            }
        }
        //计划控制转换
        else if(0x02 == (int)recvArray[10])
        {
            bool oldCtrl = pStation->ModalSelect.nPlanCtrl;
            int oldState = pStation->ModalSelect.nStateSelect;
            int newState = (int)recvArray[11];
            bool newCtrl = ((int)recvArray[12])==1?true:false;
            pStation->ModalSelect.nStateSelect = newState;
            pStation->ModalSelect.nPlanCtrl = newCtrl;//((int)recvArray[12])==1?true:false;
            m_pDataAccess->UpdateStationInfo(pStation);
            //发送同步消息--随站场状态实时发送
            //sendUpdateDataMsg(pStation, UPDATETYPE_KZMS);
            //0 手工排路 1 按图排路（生成进路序列）
            if(1==oldState && 0==newState)
            {
                //清除进路序列
                pStation->m_ArrayRouteOrder.clear();
                //清除数据库
                m_pDataAccess->DeleteStationRouteOrder(pStation->getStationID());
                //发送清除同步命令
                pStation->sendOneTrainRouteOrderToSoft(DATATYPE_ALL, nullptr, SYNC_FLAG_DELALL);
                QString msg = QString("%1的进路控制模式由按图排路转为完全手工").arg(pStation->getStationName());
                //发送报警提示信息
                pStation->sendWarningMsgToCTC(3,1,msg);
            }
            if(1==newState && (true==newCtrl && false==oldCtrl))
            {
                QString msg = QString("%1启用计划控制").arg(pStation->getStationName());
                //发送报警提示信息
                pStation->sendWarningMsgToCTC(3,1,msg);
            }
        }
        //自动签收阶段计划选项操作
        else if(0x03 == (int)recvArray[10])
        {
            pStation->AutoSignStage = ((int)recvArray[11])==1?true:false;
            m_pDataAccess->UpdateStationInfo(pStation);
            //发送同步消息
            //sendUpdateDataMsg(pStation, UPDATETYPE_KZMS);
            //站场状态中实时发送
        }
    }
    //列车进路序列
    else if(FUNCTYPE_ROUTE == revcType)
    {
        //RecvCTCData_RecvTrainRouteInfo(pStation, recvArray, recvlen);
        pStation->recvCTCData_TrainRouteOrder(recvArray, recvlen);
    }
    //行车日志
    else if(FUNCTYPE_TRAFFIC == revcType)
    {
        //RecvCTCData_RecvTrafficLogInfo(pStation, recvArray, recvlen);
        pStation->recvCTCData_TrafficLogInfo(recvArray, recvlen);
    }
    //区间逻辑检查
    else if(FUNCTYPE_SECTION == revcType)
    {
        //空
    }
    //作业流程处理
    else if(FUNCTYPE_FLOWS == revcType)
    {
        //RecvCTCData_RecvJobFlowInfo(pStation, recvArray, recvlen, nClientIndex, -1);
        pStation->recvCTCData_JobFlowInfo(recvArray, recvlen, nClientIndex, -1);
    }
    //功能按钮操作+站场操作
    else if(FUNCTYPE_FUNCS == revcType) //0x88
    {
        //股道封锁、股道和道岔分路不良及空闲、有电无电，不能发送给联锁！
        if(pStation->dealFuncBtnCmd(recvArray, recvlen))
        {
            qRecvArray[8] = DATATYPE_SERVER;//DATATYPE_CTRL;
            //站场状态中已打包状态， 不用再转发给各终端
            ////转发给占线板
            //sendDataToBoard(pStation, qRecvArray, recvlen);
            ////转发给CTC多终端
            //sendDataToCTC(pStation, qRecvArray, recvlen, nClientIndex);
        }
        else
        {
            //处理进路取消
            pStation->DealRouteCancleCmd(recvArray, recvlen);
            //进路判断-含防错办逻辑判定
            if(0==pStation->dealFuncBtnCmdToLS(recvArray, recvlen, nClientIndex))
            {
                //给联锁发送的命令都以CTC的标志发送
                qRecvArray[8]=DATATYPE_CTC;
                //命令发送给联锁
                sendDataToLS(pStation, qRecvArray, recvlen);
                qDebug()<<"发送联锁命令！"<<ByteArrayToString0x(qRecvArray);
            }
        }
        //清除计数
        //pStation->clearCmdCountDown();
        pStation->clearCmd();
    }
    //车次计划修改/CTC人工新增计划
    else if(FUNCTYPE_CHGCC == revcType)
    {
        //RecvCTCData_RecvCTCChgPlan(pStation, recvArray, recvlen);
        pStation->recvCTCData_ChangePlan(recvArray, recvlen);
    }
    //车次操作
    else if(FUNCTYPE_MDYCC == revcType)//车次操作
    {
        if(pStation->recvCheciCmd(recvArray, recvlen))
        {
            //个别操作命令需要发给联锁
            sendDataToLS(pStation, qRecvArray, recvlen);
        }
    }
    //调度命令签收
    else if(FUNCTYPE_DDMLQS == revcType)
    {
        //sendDataToLS(pStation, qRecvArray, recvlen);
        //按照CTC的签收协议执行，回看上述的类型：FUNCTYPE_DISPTCH。
    }
    //行车日志人工报点+临时限速
    else if(FUNCTYPE_XCRZ == revcType
         || FUNCTYPE_LIMITCC == revcType
         )
    {
        sendDataToLS(pStation, qRecvArray, recvlen);
    }
    //更新数据消息
    else if(FUNCTYPE_UPDATE == revcType)
    {
        #if 0
        if(UPDATETYPE_ALL == (int)recvArray[10])
        {
            m_pDataAccess->SelectAllStagePlanDetail(pStation);
            m_pDataAccess->SelectAllTrafficLog(pStation);
            m_pDataAccess->SelectAllRouteOrder(pStation);
            m_pDataAccess->SelectStationInfo(pStation);
            m_pDataAccess->SelectAllDisOrderRecv(pStation);
            {
                m_pDataAccess->SelectAllGDAntiSlip(pStation);
                pStation->SetAllGDAntiSlip();
            }
        }
        else if(UPDATETYPE_JDJH == (int)recvArray[10])
        {
            m_pDataAccess->SelectAllStagePlanDetail(pStation);
        }
        else if(UPDATETYPE_XCRZ == (int)recvArray[10])
        {
            m_pDataAccess->SelectAllTrafficLog(pStation);
        }
        else if(UPDATETYPE_JLXL == (int)recvArray[10])
        {
            m_pDataAccess->SelectAllRouteOrder(pStation);
        }
        else if(UPDATETYPE_GDFL == (int)recvArray[10])
        {
            m_pDataAccess->SelectAllGDAntiSlip(pStation);
            pStation->SetAllGDAntiSlip();
        }
        else if(UPDATETYPE_DDML == (int)recvArray[10])
        {
            m_pDataAccess->SelectAllDisOrderRecv(pStation);
        }
        else if(UPDATETYPE_KZMS == (int)recvArray[10])
        {
            //m_pDataAccess->SelectStationInfoByStaId(pStation->StationID, pStation->pStaInfo);
            //pStation->m_nFCZKMode = pStation->pStaInfo->FCZKMode;
            //pStation->StationModalSelect.nModeState = pStation->pStaInfo->FSZLMode;
            m_pDataAccess->SelectStationInfo(pStation);
        }
        int type = (int)recvArray[10];
        //发送同步消息
        sendUpdateDataMsg(pStation, type, nClientIndex, -1);
        #endif
    }
    //行车日志车次计划操作
    else if(PLAN_CMD_TYPE == revcType)
    {
        //RecvBoard1Data(pStation, recvArray, recvlen);
        pStation->recvBoradData_PlanCmdInfo(recvArray, recvlen);
    }
    //防错办强制执行操作
    else if(FUNCTYPE_CHECK == revcType)
    {
        pStation->recvCTCData_ForceExecute(qRecvArray);
    }
    else
    {
        //0x04 调度命令签收 - 见上述相应功能，采用CTC协议
        //0x61 占线板修改后的阶段计划 - 见上述相应功能，采用CTC协议
        //0x88 自动/人工办理进路 - 见上述相应功能，采用CTC协议
        //0x71 调度命令转发信号员 - 见上述相应功能，采用CTC协议
        ;
    }
    Mutex.unlock();
}
//接收CTC数据处理
void MyDoc::recvAllCTCData(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)recvArray[6];
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }

    int revcType = (int)(recvArray[9]&0xFF);
    //保存接收到数据的socket信息，用于向对方发送数据
    int nCTCDevType = (int)((recvArray[7]&0xF0)>>4);//岗位码
    int nABNum = (int)(recvArray[7]&0x0F);//终端序号
    int nClientIndex = nABNum - 1;
    if(0 <= nClientIndex && nClientIndex<10)
    {
        //保存接收到数据的socket信息，用于向对方发送数据
        pStation->ctcAddr[nClientIndex] = clientAdd;
        pStation->ctcPort[nClientIndex] = clientPort;
        pStation->m_nTimeCTCBreak[nClientIndex] = 0;//通信正常
        if(pStation->isCTCConnected[nClientIndex]==false)
        {
            QLOG_INFO()<<QString("%1服务端与CTC通信1成功！").arg(pStation->getStationName());
        }
        pStation->isCTCConnected[nClientIndex] = true;
        //pStation->StaConfigInfo.nCTCDevType = nCTCDevType;//岗位
        //qDebug()<<QString("接收到【CTC车务终端%1-%2】的数据").arg(nStaid).arg(nABNum);
    }
    if(0x23 == revcType)
    {
        return;//心跳包
    }
    //集中处理数据
    AnalyCTCData(pStation, recvArray, recvlen, nClientIndex);
}
//接收所有占线板终端数据
void MyDoc::recvAllBoardData(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)recvArray[6];
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);

    int revcType = (int)(recvArray[9]&0xFF);
    //保存接收到数据的socket信息，用于向对方发送数据
    int nCTCDevType = (recvArray[7]&0xF0)>>4;//岗位码
    int nABNum = recvArray[7]&0x0F;//终端序号
    int nClientIndex = nABNum - 1;
    if(0 <= nClientIndex && nClientIndex<10)
    {
        //保存接收到数据的socket信息，用于向对方发送数据
        pStation->zxbAddr[nClientIndex] = clientAdd;
        pStation->zxbPort[nClientIndex] = clientPort;
        pStation->m_nTimeBoardBreak[nClientIndex] = 0;//通信正常
        if(pStation->isBoardConnected[nClientIndex]==false)
        {
            QLOG_INFO()<<QString("%1服务端与占线板通信成功！").arg(pStation->getStationName());
        }
        pStation->isBoardConnected[nClientIndex] = true;
        //pStation->StaConfigInfo.nCTCDevType = nCTCDevType;//岗位
        //qDebug()<<QString("接收到【占线板终端%1-%2】的数据").arg(nStaid).arg(nABNum);
    }
    if(0x23 == revcType)
    {
        return;//心跳包
    }
    //集中处理数据
    AnalyCTCData(pStation, recvArray, recvlen, nClientIndex);
}
//接收所有集控终端数据
void MyDoc::recvAllJKData(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)recvArray[6];
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);

    int revcType = (int)(recvArray[9]&0xFF);
    //保存接收到数据的socket信息，用于向对方发送数据
    //int nJKDevType = (recvArray[7]&0xF0)>>4;//岗位码
    int nABNum = recvArray[7]&0x0F;//终端序号
    int nClientIndex = nABNum - 1;
    if(0 <= nClientIndex && nClientIndex<10)
    {
        //保存接收到数据的socket信息，用于向对方发送数据
        pStation->jkAddr[nClientIndex] = clientAdd;
        pStation->jkPort[nClientIndex] = clientPort;
        pStation->m_nTimeJKBreak[nClientIndex] = 0;//通信正常
        if(pStation->isJKConnected[nClientIndex]==false)
        {
            QLOG_INFO()<<QString("%1服务端与集控通信成功！").arg(pStation->getStationName());
            qDebug()<<QString("接收到【集控终端%1-%2】的数据").arg(nStaid).arg(nABNum);
        }
        pStation->isJKConnected[nClientIndex] = true;
        //pStation->StaConfigInfo.nCTCDevType = nCTCDevType;//岗位
        //qDebug()<<QString("接收到【集控终端%1-%2】的数据").arg(nStaid).arg(nABNum);
    }
    if(0x23 == revcType)
    {
        return;//心跳包
    }
    //集中处理数据
    AnalyCTCData(pStation, recvArray, recvlen, nClientIndex);
}
//接收所有占线图终端数据
void MyDoc::recvAllZXTData(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)recvArray[6];
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);

    int revcType = (int)(recvArray[9]&0xFF);
    //保存接收到数据的socket信息，用于向对方发送数据
    //int nJKDevType = (recvArray[7]&0xF0)>>4;//岗位码
    int nABNum = recvArray[7]&0x0F;//终端序号
    int nClientIndex = nABNum - 1;
    if(0 <= nClientIndex && nClientIndex<10)
    {
        //保存接收到数据的socket信息，用于向对方发送数据
        pStation->zxtAddr[nClientIndex] = clientAdd;
        pStation->zxtPort[nClientIndex] = clientPort;
        pStation->m_nTimeZXTBreak[nClientIndex] = 0;//通信正常
        if(pStation->isZXTConnected[nClientIndex]==false)
        {
            QLOG_INFO()<<QString("%1服务端与占线图通信成功！").arg(pStation->getStationName());
            qDebug()<<QString("接收到【占线图%1-%2】的数据").arg(nStaid).arg(nABNum);
        }
        pStation->isZXTConnected[nClientIndex] = true;
        //qDebug()<<QString("接收到【占线图%1-%2】的数据").arg(nStaid).arg(nABNum);
    }
    if(0x23 == revcType)
    {
        return;//心跳包
    }
    //集中处理数据
    AnalyCTCData(pStation, recvArray, recvlen, nClientIndex);
}
//接收CTC数据-用户登录
void MyDoc::recvCTCDataUserLogin(MyStation *pStation, unsigned char *recvArray, int recvlen, int currCtcIndex)
{
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);

    int dataType = (int)(recvArray[10]&0xFF);
    //用户登录请求 || 用户注销操作
    if(0x01 == dataType || 0x03 == dataType)
    {
        int nCount = 11;
        //登录名长度
        int len = (int)(qRecvArray[nCount]&0xFF);
        nCount++;
        //登录名
        QString inputLoginName;
        inputLoginName = ByteArrayToUnicode(qRecvArray.mid(nCount,len));
        nCount += len;

        //用户登录请求
        if(0x01 == dataType)
        {
            //登录密码长度
            len = (int)(qRecvArray[nCount]&0xFF);
            nCount++;          
            //登录密码
            QString inputPassword;
            inputPassword = ByteArrayToUnicode(qRecvArray.mid(nCount,len));
            nCount += len;

            if(m_pDataAccess)
            {
                int loginStatus = 0;
                SysUser userGet = m_pDataAccess->getSysUser(inputLoginName);
                //找到用户
                if(userGet.userName!="")
                {
                    //输入的密码加密处理
                    QString inputPwdEncrypted = inputLoginName + inputPassword + userGet.salt;
                    //密码MD5加密
                    inputPwdEncrypted = this->GetMd5(inputPwdEncrypted.trimmed());
                    //密码正确
                    if(inputPwdEncrypted == userGet.password)
                    {
                        SysUser* pUser = new SysUser;
                        pUser->salt = userGet.salt;
                        pUser->staId = userGet.staId;
                        pUser->roleId = userGet.roleId;
                        pUser->userId = userGet.userId;
                        pUser->password = userGet.password;
                        pUser->userName = userGet.userName;
                        pUser->loginName = userGet.loginName;
                        //pUser->loginStatus = userGet.loginStatus;
                        pUser->loginStatus = 1;
                        //是否新登录用户
                        bool bNewUser = true;
                        //todo
                        for(int i=0; i<pStation->vectLoginUser.size(); i++)
                        {
                            if(pUser->loginName == pStation->vectLoginUser[i]->loginName)
                            {
                                bNewUser = false;
                                pStation->vectLoginUser[i]->loginStatus = 1;
                                break;
                            }
                        }
                        if(bNewUser)
                        {
                            //增加
                            pStation->vectLoginUser.append(pUser);
                        }
                        else
                        {
                            int count = pStation->vectLoginUser.size();
                            //更新最后登录的用户
                            if(count>0)
                            {
                                //当前用户重新登录
                                if(pUser->loginName == pStation->vectLoginUser[count-1]->loginName)
                                {
                                    pStation->vectLoginUser[count-1]->loginStatus = 1;
                                }
                                //非当前用户登录，则增加
                                else
                                {
                                    //增加
                                    pStation->vectLoginUser.append(pUser);
                                }
                            }
                        }
                        loginStatus = 0x01;//登录成功
                    }
                    //密码错误
                    else
                    {
                        loginStatus = 0x02;//密码错误
                    }
                }
                //未找到用户
                else
                {
                    loginStatus = 0x03;//用户名错误
                }

                //数据组帧
                int nLength = 0;
                int nCount=10;
                QByteArray dataArray;
                //添加4096个字节并全部置零
                dataArray.append(nCount, char(0));
                //功能码
                //nCount = 9;
                dataArray[dataArray.size()-1] = FUNCTYPE_USERLOGIN;
                //子分类码
                dataArray.append(1, char(0));
                dataArray[dataArray.size()-1] = 0x02;
                //登录名长度
                dataArray.append(1, char(0));
                dataArray[dataArray.size()-1] = inputLoginName.toLocal8Bit().length();
                //登录名内容
                dataArray.append(inputLoginName.toLocal8Bit());
                //登录校验状态
                dataArray.append(1, char(0));
                dataArray[dataArray.size()-1] = loginStatus;
                //帧尾,预留2个空白
                dataArray.append(6, char(0));
                nLength = dataArray.size();
                //打包帧头帧尾
                pStation->packHeadAndTail(&dataArray, nLength);
                //向CTC发送登录回执信息
                this->sendDataToCTC(pStation, dataArray, nLength);
            }
        }
        //用户注销操作
        else if(0x03 == dataType)
        {
            for(int i=0; i<pStation->vectLoginUser.size(); i++)
            {
                if(inputLoginName == pStation->vectLoginUser[i]->loginName)
                {
                    pStation->vectLoginUser[i]->loginStatus = 2;
                    //不发送回执，在站场状态中打包发送
                    break;
                }
            }
        }
    }
}

//接收的教师机数据处理（阶段计划，调度命令等）（是否自动签收）
void MyDoc::recvTeacherData(unsigned char *recvArray, int recvlen, bool bAutoSign)
{
    //车站id
    int nStaid = (int)recvArray[6];
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据类型
    int revcType = (int)recvArray[9];
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);

    //调度命令 0x99
    if(revcType == TCHTYPE_DISPTCH)
    {
        //Mutex.lock();//培训软件下发计划时会卡死
        DispatchOrderStation* disOrderRecv = pStation->updateDisorderSta(recvArray, recvlen);
        //CDisOrderRecv disOrderRecv = pStation->m_ArrayDisOrderRecv[pStation->m_ArrayDisOrderRecv.count()-1];
        disOrderRecv->order_id = m_pDataAccess->InsertDisOrderRecv(disOrderRecv);
        //全局添加
        vectRecvDisOrder.append(disOrderRecv);
        //本站添加
        pStation->m_ArrayDisOrderRecv.append(disOrderRecv);

        //发送同步数据消息
        sendUpdateDataMsg(pStation, UPDATETYPE_DDML);
        //Mutex.unlock();
        //处理和发送1个数据
        pStation->sendOneDisOrderToSoft(DATATYPE_ALL,disOrderRecv,SYNC_FLAG_ADD,1,1);
        //报警信息
        pStation->sendWarningMsgToCTC(3,2,"收到调度命令");
        //语音播报
        pStation->SendSpeachText("调度命令下达请签收");
    }
    //阶段计划 0x33
    else if(revcType == TCHTYPE_STAGEPL)
    {
        //Mutex.lock();
        //阶段计划详情
        StagePlan* stagePlan = pStation->updateStagePlan(recvArray, recvlen);
        //阶段计划大计划
        Stage* stage = new Stage();
        stage->station_id = pStation->getStationID();
        stage->m_nPlanNum = stagePlan->m_nPlanNumber;
        stage->m_timRecv = QDateTime::currentDateTime();
        //查数据库
        stagePlan->plan_id = m_pDataAccess->SelectPlanId(stage);
        //还没有父表内容，则插入
        if(stagePlan->plan_id <= 0)
        {
            stagePlan->plan_id = m_pDataAccess->InsetStage(stage);
        }
        stagePlan->detail_id = m_pDataAccess->InsetStagePlanDetail(stagePlan);
        pStation->m_ArrayStagePlan.append(stagePlan);
        //阶段计划回执发送
        SendStagePlanReceipt(pStation, stagePlan);

        //发送同步数据消息
        sendUpdateDataMsg(pStation, UPDATETYPE_JDJH);
        //qDebug()<<"bAutoSign="<<bAutoSign;
        //自动签收
        pStation->SignStagePlan(bAutoSign);//false
        //没签收
        if(!pStation->AutoSignStage && !bAutoSign)
        {
            //处理和发送1个数据
            pStation->sendOneStagePlanToSoft(DATATYPE_ALL,stagePlan,SYNC_FLAG_ADD,1,1);
        }
        //Mutex.unlock();

        //报警信息
        pStation->sendWarningMsgToCTC(3,2,"收到列车计划");
        //语音播报
        pStation->SendSpeachText("阶段计划下达请签收");
    }
    //站场重置 0xEA
    else if(revcType == TCHTYPE_RESET)
    {
        int isSingle = (int)recvArray[10];
        if(0 == isSingle)
        {
            qDebug()<<"重置所有站";
            //重置所有站
            resetStationInfo(pStation, true);
            //用线程运行
            //QFuture<void> future = QtConcurrent::run(this, &MyDoc::resetStationInfo,pStation,true);
        }
        else
        {
            //重置单站
            resetStationInfo(pStation, false);
        }
    }
    //控制模式转换申请结果 0x2A
    else if(revcType == TCHTYPE_CHGMODE)
    {
        int targtMode = (int)recvArray[11];
        int result    = (int)recvArray[12];
        int ctcMode;
        switch(pStation->ModalSelect.nModeState)
        {
        case 0: ctcMode = CTC_MODE_CENTER; break;
        case 1: ctcMode = CTC_MODE_STATION; break;
        case 2: ctcMode = CTC_MODE_NORMAL; break;
        //default: nMode = 1; break;
        }

        if(/*(currMode == ctcMode) && (currMode != targtMode) &&*/
            (ctcMode != targtMode) && (result == AGREE) )
        {
            int nNewMode;
            switch(targtMode)
            {
            case CTC_MODE_CENTER:  nNewMode = 0; break;
            case CTC_MODE_STATION: nNewMode = 1; break;
            case CTC_MODE_NORMAL:  nNewMode = 2; break;
            default: nNewMode = 1; break;
            }
            //0 中心控制 1 车站控制 2 分散自律
            pStation->ModalSelect.nModeState = nNewMode;
            pStation->ModalSelect.nRequestMode = -1;
            //pStation->pStaInfo->FSZLMode = nMode;
            m_pDataAccess->UpdateStationInfo(pStation);
            //发送同步消息-站场状态中同步
            //sendUpdateDataMsg(pStation, UPDATETYPE_KZMS);
            //当前模式-0 中心控制 1 车站控制 2 分散自律
            if(ctcMode==CTC_MODE_CENTER && 1==nNewMode)
            {
                QString msg = QString("%1的车站控制模式由中心控制转为车站控制").arg(pStation->getStationName());
                //发送报警提示信息
                pStation->sendWarningMsgToCTC(3,1,msg);
                //进路序列自触标志全部清除
                pStation->SetRouteAutoTouchState(false);
                //语音播报
                pStation->SendSpeachText("控制模式已转换");
            }
            else if(ctcMode==CTC_MODE_NORMAL && 1==nNewMode)
            {
                QString msg = QString("%1的车站控制模式由车站调车转为车站控制").arg(pStation->getStationName());
                //发送报警提示信息
                pStation->sendWarningMsgToCTC(3,1,msg);
                //进路序列自触标志全部清除
                pStation->SetRouteAutoTouchState(false);
                //语音播报
                pStation->SendSpeachText("控制模式已转换");
            }
            else if(1!=nNewMode)
            {
                QString msg = QString("%1的车站控制模式由车站控制转为%2").arg(pStation->getStationName())
                        .arg(0==nNewMode?"中心控制":"车站调车");
                //发送报警提示信息
                pStation->sendWarningMsgToCTC(3,1,msg);
                //进路序列满足条件的列车全部自动自触标志
                pStation->SetRouteAutoTouchState(true);
                //语音播报
                pStation->SendSpeachText("控制模式已转换");
            }
        }
    }
    //时钟同步 0xAA
    else if(revcType == TCHTYPE_TIMSYNC)
    {
        int nCount = 10;
        SYSTEMTIME sysTime;
        GetSystemTime(&sysTime);
        sysTime.wYear = (recvArray[nCount] | recvArray[nCount+1]<<8);
        sysTime.wMonth = recvArray[nCount+2];
        sysTime.wDay  = recvArray[nCount+3];
        sysTime.wHour = recvArray[nCount+4];
        sysTime.wMinute = recvArray[nCount+5];
        sysTime.wSecond = recvArray[nCount+6];
        sysTime.wDayOfWeek = -1;
        sysTime.wMilliseconds = 0;
        SetLocalTime(&sysTime);
    }
    //一键关机 0xAB
    else if(revcType == TCHTYPE_SHUTDN)
    {
        //system("shutdown -s -t 00");
        system("shutdown -s -t 3");//3秒后关机
    }
    //临时限速命令 0x0E
    else if(revcType == TCHTYPE_LIMIT)
    {
        //参照CTC济南东版本开发
        pStation->recvTeacherData_LimitSpeed(recvArray, recvlen);
    }
    //临时限速命令(新版教师机) 0x0F
    else if(revcType == TCHTYPE_LIMITSP)
    {
        //参照新协议开发
        pStation->recvTeacherData_LimitSpeedNew(qRecvArray, recvlen);
    }
    //邻站模拟进出站 0x10
    else if(revcType == TCHTYPE_LZMNJCZ)
    {
        if(AutoTouchMinitesWhenLZMNJCZ>=0)
        {
            //参照新增加教师机协议开发
            pStation->recvTeacherData_LZNMJCZ(qRecvArray, recvlen);
        }
    }

    //(必须在存入数据库后再发送，否则CTC从数据库获取不到数据)
    //向车站CTC转发教师机命令信息
    {
        unsigned char newArray[2048];
        memset(newArray,0,2048);
        memcpy(newArray, recvArray, recvlen);
        newArray[8] = DATATYPE_TCC;
        //数据帧转换
        QByteArray qSendArray = UnsignedCharToQByteArray(newArray, recvlen);
        sendDataToCTC(pStation, qSendArray, recvlen);
        sendDataToBoard(pStation, qSendArray, recvlen);
        sendDataToJK(pStation, qSendArray, recvlen);
        sendDataToZXT(pStation, qSendArray, recvlen);
    }
}
//清除车站的所有数据（是否全部车站）
void MyDoc::resetStationInfo(MyStation *pStation, bool bAll)
{
    //互斥锁
    //QMutexLocker locker(&Mutex);
    //Mutex.lock();
    //清除所有数据
    vectRecvDisOrder.clear();//调度命令
    //清除所有站数据
    if(bAll)
    {
        QDateTime currTime = QDateTime::currentDateTime();
        //上一次重置的时间差（当前时间比上一次时间晚，则为正值）
        qint64 secondsVSLast = m_timeLastResetStationAll.secsTo(currTime);
        qDebug()<<"ResetStationAll secondsVSLast="<<secondsVSLast;
        if(secondsVSLast<=3)
        {
            qDebug()<<"站场重置间隔小于3秒，本次退出！";
            //间隔3秒，否则退出本次执行
            return;
        }

        //保持操作数据库一致，所有站都清空
        for(int i=0; i<vectMyStation.count(); i++)
        {
            MyStation *pStation1 = vectMyStation[i];
            pStation1->m_ArrayRouteOrder.clear();
            pStation1->m_ArrayTrafficLog.clear();
            pStation1->m_ArrayStagePlan.clear();
            pStation1->m_ArrayStagePlanChg.clear();
            pStation1->m_ArrayTrain.clear();
            pStation1->m_ArrayTrainManType.clear();
            pStation1->m_ArrayDisOrderRecv.clear();
            pStation1->m_ArrayDisOrderDisp.clear();
            pStation1->m_ArrayDisOrderLocomot.clear();
            pStation1->ResetStationDevStatus();
            pStation1->clearCmdCountDown();//清除终端闪烁
            pStation1->vectCheckResult.clear();
            pStation1->ClearAllGDAntiSlip();//清除铁鞋
            pStation1->vectMsgRecord.clear();
            //发送同步消息
            //sendUpdateDataMsg(pStation1, UPDATETYPE_ALL);
        }
        //数据库访问
        //m_pDataAccess->ResetAllTable();//数据库清除全部
        //用线程运行
        QFuture<void> future = QtConcurrent::run(this, &MyDoc::resetDataBase);
        //记录时间
        m_timeLastResetStationAll = QDateTime::currentDateTime();
    }
    //清除单站数据
    else
    {
        pStation->m_ArrayRouteOrder.clear();
        pStation->m_ArrayTrafficLog.clear();
        pStation->m_ArrayStagePlan.clear();
        pStation->m_ArrayStagePlanChg.clear();
        pStation->m_ArrayTrain.clear();
        pStation->m_ArrayTrainManType.clear();
        pStation->m_ArrayDisOrderRecv.clear();
        pStation->m_ArrayDisOrderDisp.clear();
        pStation->m_ArrayDisOrderLocomot.clear();
        pStation->ResetStationDevStatus();
        pStation->clearCmdCountDown();//清除终端闪烁
        pStation->vectCheckResult.clear();
        pStation->ClearAllGDAntiSlip();//清除铁鞋
        pStation->vectMsgRecord.clear();
        //数据库访问
        m_pDataAccess->ResetStationTable(pStation);//数据库清除本站
        //发送同步消息
        //sendUpdateDataMsg(pStation, UPDATETYPE_ALL);
    }
    //Mutex.unlock();
}
//重置数据库
void MyDoc::resetDataBase()
{
    //互斥锁
    QMutexLocker locker(&Mutex);
    m_pDataAccess->ResetAllTable();//数据库清除全部
}

//阶段计划接收回执发送给教师机
void MyDoc::SendStagePlanReceipt(MyStation *pStation, StagePlan *pStagePlan)
{
    QByteArray dataArray;
    int nCount = 0;
    //帧头
    nCount+=10;
    dataArray.append(10, char(0));
    dataArray[6] = pStation->getStationID();
    dataArray[8] = 0xCC;//分类信息码-设备类型
    dataArray[9] = 0x32;//分类信息码-阶段计划回执
    //计划号
    nCount+=4;
    dataArray.append(4, char(0));
    memcpy(dataArray.data()+(nCount-4),&pStagePlan->m_nPlanNumber,4);
    //列车接车车次
    QString strTrainNum = pStagePlan->m_strReachTrainNum != "" ? pStagePlan->m_strReachTrainNum:pStagePlan->m_strDepartTrainNum;
    QByteArray byteArray = strTrainNum.toLocal8Bit();
    int ccLen = byteArray.count();
    //车次号长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //车次号内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }

    //剩余个数，教师机通信固定长度60
    int leftCount = 60-nCount;
    //帧尾
    nCount+=leftCount;
    dataArray.append(leftCount, char(0));

    //制作数据帧头帧尾
    for(int i = 0; i < 4; i++)
    {
        dataArray[i] = 0xEF;
        dataArray[nCount - i -1] = 0xFE;
    }
    //帧长度
    memcpy(dataArray.data()+4, &nCount, 2);
    //发送给教师机
    sendDataToJSJ(dataArray, nCount);
}





















