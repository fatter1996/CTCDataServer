#include "mydoc.h"
#include <QtDebug>
#include <QSettings>
#include <QMessageBox>
#include <windows.h>
#include "GlobalHeaders/GlobalFuntion.h"
#include "Log/log.h"

//将接收到的数据根据帧头帧位进行分割并返回分割后的二维数组
QVector<QByteArray> MyDoc::SplitReceiveData_SameHeadTail(QByteArray recvArray)
{
    QVector<QByteArray> dataArray;//用于存放分割后的数据
    QByteArray tempArray;//临时数组

    //长度不够则退出
    if(recvArray.size()<14)
    {
        return dataArray;
    }

    int head = 0xEF;//帧头
    int tail = 0xFE;//帧尾
    QVector<int> header;
    header.append(head);
    header.append(head);
    header.append(head);
    header.append(head);
    QVector<int> ender;
    ender.append(tail);
    ender.append(tail);
    ender.append(tail);
    ender.append(tail);
    int index_Header=0;
    int dataLen=0;//数据长度包含帧头和帧尾

    //从头开始遍历数据帧
    for(int i=0; i<recvArray.size(); i++)
    {
        //比对帧头
        bool headFlag=true;
        for(int j=0;j<header.size();j++)
        {
            if(i+j+ender.size() >= recvArray.size())//越界防护
            {
                headFlag=false;
                break;
            }
            //qDebug()<<"(int)header[j]="<<(int)header[j]<<",(int)recvArray[i+j]="<<(int)(recvArray[i+j]&0xFF);
            if((int)header[j] != (int)(recvArray[i+j]&0xFF))
            {
                headFlag=false;
                break;
            }
        }
        if(!headFlag)
        {
            continue;
        }

        //记录帧头开始位置
        index_Header = i;
        //数据长度包含帧头和帧尾
        dataLen = (int)(recvArray[i+header.size()] | recvArray[i+header.size()+1]<<8);
        //帧头+内容+帧尾
        if(i+dataLen>recvArray.size())//越界防护
        {
            break;
        }
        //将i指向帧尾开始的位置，避免不必要的循环
        i+= (dataLen-ender.size());

        //比对帧尾
        bool endFlag=true;
        for(int j=0; j<ender.size(); j++)
        {
            if(i+j>=recvArray.size())  //简单防护
            {
                endFlag=false;
                break;
            }
            if(ender[j] != (int)(recvArray[i+j]&0xFF))
            {
                endFlag=false;
                break;
            }
        }
        if(endFlag == false)
        {
            continue;//帧尾校验失败则不作为帧尾处理
        }

        //加入整包数据
        tempArray.append(recvArray.data()+index_Header, dataLen);
        dataArray.append(tempArray);
        tempArray.clear();

        i+= (ender.size());//到帧尾结束的位置
        i--;
    }

    return dataArray;
}

//TCP断开槽
void MyDoc::tcpClientDisconnectedSlot(QString clientAdd, int clientPort)
{
    for(int i=0; i<vectMyStation.size(); i++)
    {
        MyStation* pMyStation = vectMyStation[i];
        for(int j=0;j<10;j++)
        {
            if(clientAdd==pMyStation->ctcAddr2[j] && clientPort==pMyStation->ctcPort2[j])
            {
                pMyStation->ctcAddr2[j] = "";//IP地址
                pMyStation->ctcPort2[j] = 0;//端口
                pMyStation->m_nTimeCTCBreak2[j]++;//本站CTC终端通信中断计时器
                pMyStation->isCTCConnected2[j] = false;//本站CTC终端是否连接
                //qDebug()<<QString("%1链接断开【CTC2车务终端%2-%3】").arg(QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM)).arg(pMyStation->getStationID()).arg(j+1);
                QLOG_WARN()<<QString("TCP链接断开【CTC2车务终端-%1-%2】").arg(pMyStation->getStationName()).arg(j+1);
                return;
            }
        }
        for(int j=0;j<10;j++)
        {
            if(clientAdd==pMyStation->zxbAddr2[j] && clientPort==pMyStation->zxbPort2[j])
            {
                pMyStation->zxbAddr2[j] = "";//IP地址
                pMyStation->zxbPort2[j] = 0;//端口
                pMyStation->m_nTimeBoardBreak2[j]++;//本站占线板终端通信中断计时器
                pMyStation->isBoardConnected2[j] = false;//本站占线板终端是否连接
                //qDebug()<<QString("%1链接断开【占线板%2-%3】").arg(QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM)).arg(pMyStation->getStationID()).arg(j+1);
                QLOG_WARN()<<QString("TCP链接断开【占线板-%1-%2】").arg(pMyStation->getStationName()).arg(j+1);
                return;
            }
        }
        for(int j=0;j<10;j++)
        {
            if(clientAdd==pMyStation->jkAddr2[j] && clientPort==pMyStation->jkPort2[j])
            {
                pMyStation->jkAddr2[j] = "";//IP地址
                pMyStation->jkPort2[j] = 0;//端口
                pMyStation->m_nTimeJKBreak2[j]++;//本站CTC终端通信中断计时器
                pMyStation->isJKConnected2[j] = false;//本站CTC终端是否连接
                //qDebug()<<QString("%1链接断开【集控终端%2-%3】").arg(QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM)).arg(pMyStation->getStationID()).arg(j+1);
                QLOG_WARN()<<QString("TCP链接断开【集控终端-%1-%2】").arg(pMyStation->getStationName()).arg(j+1);
                return;
            }
        }
        for(int j=0;j<10;j++)
        {
            if(clientAdd==pMyStation->zxtAddr2[j] && clientPort==pMyStation->zxtPort2[j])
            {
                pMyStation->zxtAddr2[j] = "";//IP地址
                pMyStation->zxtPort2[j] = 0;//端口
                pMyStation->m_nTimeZXTBreak2[j]++;//本站占线图终端通信中断计时器
                pMyStation->isZXTConnected2[j] = false;//本站占线图终端是否连接
                //qDebug()<<QString("%1链接断开【占线图%2-%3】").arg(QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM)).arg(pMyStation->getStationID()).arg(j+1);
                QLOG_WARN()<<QString("TCP链接断开【占线图-%1-%2】").arg(pMyStation->getStationName()).arg(j+1);
                return;
            }
        }
    }
}
//解析所有终端的socket通道数据-TCP
void MyDoc::receiveAllDataSlot2(QByteArray dataArray, QString clientAdd, int clientPort)
{
    int recvLength=dataArray.size();
    if(recvLength < 14)
    {
        return;   //无效数据
    }
    //来源地址和端口
    //qDebug()<<"收到[主通道2]数据"<<QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM)<<"client_add="<<clientAdd<<"client_port="<<clientPort;
    //校验和分割收到的数据
    QVector<QByteArray> recvSpitArr = SplitReceiveData_SameHeadTail(dataArray);
    //遍历数据
    for(int i=0; i<recvSpitArr.size(); i++)
    {
        QByteArray dataArray = recvSpitArr[i];
//        int dataLength=(int)(dataArray[4] | (dataArray[5]<<8));
//        if(dataLength != recvLength)
//        {
//            qDebug()<<" 接收长度与数据解析长度不符2！ "<<dataLength<<recvLength;
//            return;
//        }
//        //帧头帧尾校验成功
//        if((((int)dataArray[0]) == 0xEF) && (((int)dataArray[1]) == 0xEF)
//           && (((int)dataArray[2]) == 0xEF) && (((int)dataArray[3]) == 0xEF)
//           && (((int)dataArray[dataLength-1]) == 0xFE) && (((int)dataArray[dataLength-2]) == 0xFE)
//           && (((int)dataArray[dataLength-3]) == 0xFE) && (((int)dataArray[dataLength-4]) == 0xFE)
//           )
        {

            //数据类型
            int dataType = (int)(dataArray[8]&0xFF);
            //int dataLen = (int)(dataArray[4] | dataArray[5]<<8);
            //CTC车务终端
            if(dataType == DATATYPE_CTC)
            {
                //qDebug()<<"收到[CTC2]数据.";
                //各站CTC数据
                recvAllCTCData2((unsigned char *)dataArray.data(), dataArray.size(), clientAdd, clientPort);
            }
            //占线板
            else if(dataType == DATATYPE_BOARD)
            {
                //qDebug()<<"收到[占线板]数据.";
                //各站占线板数据
                recvAllBoardData2((unsigned char *)dataArray.data(), dataArray.size(), clientAdd, clientPort);
            }
            //集控台
            else if(dataType == DATATYPE_JK)
            {
                //qDebug()<<"收到[JK]数据.";
                //各站集控数据
                recvAllJKData2((unsigned char *)dataArray.data(), dataArray.size(), clientAdd, clientPort);
            }
            //占线图
            else if(dataType == DATATYPE_ZXT)
            {
                //qDebug()<<"收到[JK]数据.";
                //各站占线图数据
                recvAllZXTData2((unsigned char *)dataArray.data(), dataArray.size(), clientAdd, clientPort);
            }
        }
    }
}
//槽-发送数据给CTC-TCP通道
void MyDoc::sendDataToCTCSlot2(MyStation *pMyStation, QByteArray dataArray, int len, int currCtcIndex)
{
    int i = currCtcIndex;
    if(0<=i && i<=10)
    {
        //单个终端
        if(pMyStation->isCTCConnected2[i])
        {
            emit sendDataToMainSignal2(dataArray, pMyStation->ctcAddr2[i], pMyStation->ctcPort2[i], len);
        }
    }
    else if(-1 == i)
    {
        //全部终端
        for(int j=0; j<10; j++)
        {
            if(pMyStation->isCTCConnected2[j])
            {
                emit sendDataToMainSignal2(dataArray, pMyStation->ctcAddr2[j], pMyStation->ctcPort2[j], len);
            }
        }
    }
}
//槽-发送数据给占线板-TCP通道
void MyDoc::sendDataToBoardSlot2(MyStation *pMyStation, QByteArray dataArray, int len, int currIndex)
{
    int i = currIndex;
    if(0<=i && i<=10)
    {
        //单个终端
        if(pMyStation->isBoardConnected2[i])
        {
            emit sendDataToMainSignal2(dataArray, pMyStation->zxbAddr2[i], pMyStation->zxbPort2[i], len);
        }
    }
    else if(-1 == i)
    {
        //全部终端
        for(int j=0; j<10; j++)
        {
            if(pMyStation->isBoardConnected2[j])
            {
                emit sendDataToMainSignal2(dataArray, pMyStation->zxbAddr2[j], pMyStation->zxbPort2[j], len);
            }
        }
    }
}
//槽-发送数据给JK-TCP通道
void MyDoc::sendDataToJKSlot2(MyStation *pMyStation, QByteArray dataArray, int len, int currIndex)
{
    int i = currIndex;
    if(0<=i && i<=10)
    {
        //单个终端
        if(pMyStation->isJKConnected2[i])
        {
            emit sendDataToMainSignal2(dataArray, pMyStation->jkAddr2[i], pMyStation->jkPort2[i], len);
        }
    }
    else if(-1 == i)
    {
        //全部终端
        for(int j=0; j<10; j++)
        {
            if(pMyStation->isJKConnected2[j])
            {
                emit sendDataToMainSignal2(dataArray, pMyStation->jkAddr2[j], pMyStation->jkPort2[j], len);
            }
        }
    }
}
//槽-发送数据给占线图ZXT-TCP通道
void MyDoc::sendDataToZXTSlot2(MyStation *pMyStation, QByteArray dataArray, int len, int currIndex)
{
    int i = currIndex;
    if(0<=i && i<=10)
    {
        //单个终端
        if(pMyStation->isZXTConnected2[i])
        {
            emit sendDataToMainSignal2(dataArray, pMyStation->zxtAddr2[i], pMyStation->zxtPort2[i], len);
        }
    }
    else if(-1 == i)
    {
        //全部终端
        for(int j=0; j<10; j++)
        {
            if(pMyStation->isZXTConnected2[j])
            {
                emit sendDataToMainSignal2(dataArray, pMyStation->zxtAddr2[j], pMyStation->zxtPort2[j], len);
            }
        }
    }
}

//接收CTC数据处理-TCP数据同步
void MyDoc::recvAllCTCData2(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)(recvArray[6]&0xFF);
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);
    //qDebug() << qRecvArray;
    //接收类型
    int revcType = (int)(recvArray[9]&0xFF);
    //保存接收到数据的socket信息，用于向对方发送数据
    int nCTCDevType = (int)((recvArray[7]&0xF0)>>4);//岗位码
    int nABNum = (int)(recvArray[7]&0x0F);//终端序号
    int nClientIndex = nABNum - 1;
    if(0 <= nClientIndex && nClientIndex<10)
    {
        QString oldAddr = pStation->ctcAddr2[nClientIndex];
        int     oldPort = pStation->ctcPort2[nClientIndex];
        //保存接收到数据的socket信息，用于向对方发送数据
        pStation->ctcAddr2[nClientIndex] = clientAdd;
        pStation->ctcPort2[nClientIndex] = clientPort;
        pStation->m_nTimeCTCBreak2[nClientIndex] = 0;//通信正常
        pStation->isCTCConnected2[nClientIndex] = true;//本站CTC终端是否连接
        //pStation->StaConfigInfo.nCTCDevType = nCTCDevType;//岗位
        //qDebug()<<QString("接收到【CTC2车务终端%1-%2】的数据").arg(nStaid).arg(nABNum);
        if(oldAddr == "" && oldPort==0)
        {
            QLOG_INFO()<<QString("TCP链接成功【CTC2车务终端-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        }
    }
    if(0x23 == revcType)
    {
        return;//心跳包
    }
    //集中处理数据
    //数据功能分类码
    //数据同步
    if(FUNCTYPE_SYNC == revcType)
    {
        QLOG_INFO()<<QString("收到TCP数据同步请求信息【CTC2车务终端-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        //pStation->recvCTCData2(recvArray, recvlen, nClientIndex);
        pStation->recvTerminalData2(DATATYPE_CTC, recvArray, recvlen, nClientIndex);
    }
    //文字显示,转发到教师机
    if(FUNCTYPE_TEXT == revcType)
    {
        qDebug() << QString("收到TCP-文字复示信息【CTC-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        if(teacherWatchPort > 0)
        {
            emit sendTextDataToJSJ(qRecvArray, recvlen);
        }
    }
}
//接收占线板数据处理-TCP数据同步
void MyDoc::recvAllBoardData2(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)(recvArray[6]&0xFF);
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);
    //qDebug() << qRecvArray;
    //接收类型
    int revcType = (int)(recvArray[9]&0xFF);
    //保存接收到数据的socket信息，用于向对方发送数据
    int nCTCDevType = (int)((recvArray[7]&0xF0)>>4);//岗位码
    int nABNum = (int)(recvArray[7]&0x0F);//终端序号
    int nClientIndex = nABNum - 1;
    if(0 <= nClientIndex && nClientIndex<10)
    {
        QString oldAddr = pStation->zxbAddr2[nClientIndex];
        int     oldPort = pStation->zxbPort2[nClientIndex];
        //保存接收到数据的socket信息，用于向对方发送数据
        pStation->zxbAddr2[nClientIndex] = clientAdd;
        pStation->zxbPort2[nClientIndex] = clientPort;
        pStation->m_nTimeBoardBreak2[nClientIndex] = 0;//通信正常
        pStation->isBoardConnected2[nClientIndex] = true;//本站JK终端是否连接
        //qDebug()<<QString("接收到【占线板%1-%2】的数据").arg(nStaid).arg(nABNum);
        if(oldAddr == "" && oldPort==0)
        {
            QLOG_INFO()<<QString("TCP链接成功【占线板-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        }
    }
    if(0x23 == revcType)
    {
        return;//心跳包
    }
    //集中处理数据
    //数据功能分类码
    //数据同步
    if(FUNCTYPE_SYNC == revcType)
    {
        QLOG_INFO()<<QString("收到TCP数据同步请求信息【占线板-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        pStation->recvTerminalData2(DATATYPE_BOARD, recvArray, recvlen, nClientIndex);
    }
    //文字显示,转发到教师机
    if(FUNCTYPE_TEXT == revcType)
    {
        qDebug() << QString("收到TCP-文字复示信息【占线板-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        if(teacherWatchPort > 0)
        {
            emit sendTextDataToJSJ(qRecvArray, recvlen);
        }
    }
}
//接收集控台数据处理-TCP数据同步
void MyDoc::recvAllJKData2(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)(recvArray[6]&0xFF);
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);
    //qDebug() << qRecvArray;
    //接收类型
    int revcType = (int)(recvArray[9]&0xFF);
    //保存接收到数据的socket信息，用于向对方发送数据
    int nCTCDevType = (int)((recvArray[7]&0xF0)>>4);//岗位码
    int nABNum = (int)(recvArray[7]&0x0F);//终端序号
    int nClientIndex = nABNum - 1;
    if(0 <= nClientIndex && nClientIndex<10)
    {
        QString oldAddr = pStation->jkAddr2[nClientIndex];
        int     oldPort = pStation->jkPort2[nClientIndex];
        //保存接收到数据的socket信息，用于向对方发送数据
        pStation->jkAddr2[nClientIndex] = clientAdd;
        pStation->jkPort2[nClientIndex] = clientPort;
        pStation->m_nTimeJKBreak2[nClientIndex] = 0;//通信正常
        pStation->isJKConnected2[nClientIndex] = true;//本站JK终端是否连接
        //qDebug()<<QString("接收到【集控终端%1-%2】的数据").arg(nStaid).arg(nABNum);
        if(oldAddr == "" && oldPort==0)
        {
            QLOG_INFO()<<QString("TCP链接成功【集控终端-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        }
    }
    if(0x23 == revcType)
    {
        return;//心跳包
    }
    //集中处理数据
    //数据功能分类码
    //数据同步
    if(FUNCTYPE_SYNC == revcType)
    {
        QLOG_INFO()<<QString("收到TCP数据同步请求信息【集控终端-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        pStation->recvTerminalData2(DATATYPE_JK, recvArray, recvlen, nClientIndex);
    }
    //文字显示,转发到教师机
    if(FUNCTYPE_TEXT == revcType)
    {
        qDebug() << QString("收到TCP-文字复示信息【集控终端-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        if(teacherWatchPort > 0)
        {
            emit sendTextDataToJSJ(qRecvArray, recvlen);
        }
    }
}
//接收占线图数据处理-TCP数据同步
void MyDoc::recvAllZXTData2(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort)
{
    //车站id
    int nStaid = (int)(recvArray[6]&0xFF);
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);
    //qDebug() << qRecvArray;
    //接收类型
    int revcType = (int)(recvArray[9]&0xFF);
    //保存接收到数据的socket信息，用于向对方发送数据
    int nCTCDevType = (int)((recvArray[7]&0xF0)>>4);//岗位码
    int nABNum = (int)(recvArray[7]&0x0F);//终端序号
    int nClientIndex = nABNum - 1;
    if(0 <= nClientIndex && nClientIndex<10)
    {
        QString oldAddr = pStation->zxtAddr2[nClientIndex];
        int     oldPort = pStation->zxtPort2[nClientIndex];
        //保存接收到数据的socket信息，用于向对方发送数据
        pStation->zxtAddr2[nClientIndex] = clientAdd;
        pStation->zxtPort2[nClientIndex] = clientPort;
        pStation->m_nTimeZXTBreak2[nClientIndex] = 0;//通信正常
        pStation->isZXTConnected2[nClientIndex] = true;//本站JK终端是否连接
        //qDebug()<<QString("接收到【占线图%1-%2】的数据").arg(nStaid).arg(nABNum);
        if(oldAddr == "" && oldPort==0)
        {
            QLOG_INFO()<<QString("TCP链接成功【占线图-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        }
    }
    if(0x23 == revcType)
    {
        return;//心跳包
    }
    //集中处理数据
    //数据功能分类码
    //数据同步
    if(FUNCTYPE_SYNC == revcType)
    {
        QLOG_INFO()<<QString("收到TCP数据同步请求信息【占线图-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        pStation->recvTerminalData2(DATATYPE_ZXT, recvArray, recvlen, nClientIndex);
    }
    //文字显示,转发到教师机
    if(FUNCTYPE_TEXT == revcType)
    {
        qDebug() << QString("收到TCP-文字复示信息【占线图-%1-%2】").arg(pStation->getStationName()).arg(nABNum);
        if(teacherWatchPort > 0)
        {
            emit sendTextDataToJSJ(qRecvArray, recvlen);
        }
    }
}










