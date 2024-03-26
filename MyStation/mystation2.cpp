#include "mystation.h"

#include <QTextCodec>
#include <QtDebug>
#include <windows.h>
#include <qstring.h>
//#include <QTest>
#include <QThread>
#include <QtConcurrent>
#include <QFuture>


//打包阶段计划为数据帧数组
QByteArray MyStation::packStagePlanToArray(StagePlan *pStagePlan)
{
    QByteArray dataArray;
    int nCount = 0;
    //阶段计划详情id
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data(),&pStagePlan->detail_id,2);
    //计划号
    nCount+=4;
    dataArray.append(4, char(0));
    //dataArray.append(4, pStagePlan->m_nPlanNumber);
    memcpy(dataArray.data()+(nCount-4),&pStagePlan->m_nPlanNumber,4);
    //列车接车车次
    QByteArray byteArray = pStagePlan->m_strReachTrainNum.toLocal8Bit();
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
    //列车发车车次
    byteArray = pStagePlan->m_strDepartTrainNum.toLocal8Bit();
    ccLen = byteArray.count();
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
    //接车股道设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2),&pStagePlan->m_nRecvTrainTrack,2);
    //规定到站时间
    nCount+=7;
    dataArray.append(7, char(0));
    int Year1 = pStagePlan->m_timProvReachStation.date().year();
    int Month1 = pStagePlan->m_timProvReachStation.date().month();
    int Day1 = pStagePlan->m_timProvReachStation.date().day();
    int Hour1 = pStagePlan->m_timProvReachStation.time().hour();
    int Minu1 = pStagePlan->m_timProvReachStation.time().minute();
    int Secd1 = pStagePlan->m_timProvReachStation.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //发车股道设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2), &pStagePlan->m_nDepartTrainTrack, 2);
    //规定出发时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pStagePlan->m_timProvDepaTrain.date().year();
    Month1 = pStagePlan->m_timProvDepaTrain.date().month();
    Day1 = pStagePlan->m_timProvDepaTrain.date().day();
    Hour1 = pStagePlan->m_timProvDepaTrain.time().hour();
    Minu1 = pStagePlan->m_timProvDepaTrain.time().minute();
    Secd1 = pStagePlan->m_timProvDepaTrain.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //始发（终到）标志位
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->m_btBeginOrEndFlg;
    //进站口信号机设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2), &pStagePlan->m_nCodeReachStaEquip, 2);
    //出站口信号机设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2), &pStagePlan->m_nCodeDepartStaEquip, 2);
    //电力机车
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->m_bElectric?0x11:0x00;
    //超限
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->m_nLevelCX;
    //列车类型（客车/货车）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->m_nLHFlg==LCTYPE_KC?0x01:0x00;
    //列车类型（管内动车组、通勤列车等）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->m_nIndexLCLX;
    //运行类型序号（动车组、快速旅客列车等）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->m_nIndexYXLX;
    //线路所
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->bXianLuSuo?0x01:0x00;
    //计划接收时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pStagePlan->m_timJHRcv.date().year();
    Month1 = pStagePlan->m_timJHRcv.date().month();
    Day1 = pStagePlan->m_timJHRcv.date().day();
    Hour1 = pStagePlan->m_timJHRcv.time().hour();
    Minu1 = pStagePlan->m_timJHRcv.time().minute();
    Secd1 = pStagePlan->m_timJHRcv.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //调度台名称
    byteArray = pStagePlan->m_strDispatchDesk.toLocal8Bit();
    ccLen = byteArray.count();
    //调度台名称长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //调度台名称
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //阶段计划类型（增加或删除）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->m_btStagePlanKind;
    //签收状态
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pStagePlan->m_nStateSignPlan;

    //qDebug()<<"StagePlan dataArray="<<ByteArrayToString(dataArray);
    return dataArray;
}
//打包行车日志为数据帧数组
QByteArray MyStation::packTrafficLogToArray(TrafficLog *pTrafficLog)
{
    QByteArray dataArray;
    int nCount = 0;
    //行车日志ID
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data(),&pTrafficLog->log_id,2);
    //计划号
    nCount+=4;
    dataArray.append(4, char(0));
    memcpy(dataArray.data()+(nCount-4),&pTrafficLog->m_nPlanNumber,4);
    //始发（终到）标志位
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btBeginOrEndFlg;
    //到达列车车次
    QByteArray byteArray = pTrafficLog->m_strReachTrainNum.toLocal8Bit();
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
    //接车股道设备号
    nCount+=2;
    dataArray.append(2, char(0));
    int devCode = this->GetCodeByStrName(pTrafficLog->m_strRecvTrainTrack);
    memcpy(dataArray.data()+(nCount-2),&devCode,2);
    //计划到站时间
    nCount+=7;
    dataArray.append(7, char(0));
    int Year1 = pTrafficLog->m_timProvReachStation.date().year();
    int Month1 = pTrafficLog->m_timProvReachStation.date().month();
    int Day1 = pTrafficLog->m_timProvReachStation.date().day();
    int Hour1 = pTrafficLog->m_timProvReachStation.time().hour();
    int Minu1 = pTrafficLog->m_timProvReachStation.time().minute();
    int Secd1 = pTrafficLog->m_timProvReachStation.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //实际到站时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timRealReachStation.date().year();
    Month1 = pTrafficLog->m_timRealReachStation.date().month();
    Day1 = pTrafficLog->m_timRealReachStation.date().day();
    Hour1 = pTrafficLog->m_timRealReachStation.time().hour();
    Minu1 = pTrafficLog->m_timRealReachStation.time().minute();
    Secd1 = pTrafficLog->m_timRealReachStation.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //同意邻站发车时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timAgrFromAdjtStaDepaTrain.date().year();
    Month1 = pTrafficLog->m_timAgrFromAdjtStaDepaTrain.date().month();
    Day1 = pTrafficLog->m_timAgrFromAdjtStaDepaTrain.date().day();
    Hour1 = pTrafficLog->m_timAgrFromAdjtStaDepaTrain.time().hour();
    Minu1 = pTrafficLog->m_timAgrFromAdjtStaDepaTrain.time().minute();
    Secd1 = pTrafficLog->m_timAgrFromAdjtStaDepaTrain.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //邻站发车时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timFromAdjtStaDepaTrain.date().year();
    Month1 = pTrafficLog->m_timFromAdjtStaDepaTrain.date().month();
    Day1 = pTrafficLog->m_timFromAdjtStaDepaTrain.date().day();
    Hour1 = pTrafficLog->m_timFromAdjtStaDepaTrain.time().hour();
    Minu1 = pTrafficLog->m_timFromAdjtStaDepaTrain.time().minute();
    Secd1 = pTrafficLog->m_timFromAdjtStaDepaTrain.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //进站口信号机设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2), &pTrafficLog->m_nCodeReachStaEquip, 2);
    //到达车次是上行车次
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bReachTrainNumSX?0x01:0x00;

    //出发列车车次
    byteArray = pTrafficLog->m_strDepartTrainNum.toLocal8Bit();
    ccLen = byteArray.count();
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
    //出发股道设备号
    nCount+=2;
    dataArray.append(2, char(0));
    devCode = this->GetCodeByStrName(pTrafficLog->m_strDepartTrainTrack);
    memcpy(dataArray.data()+(nCount-2),&devCode,2);
    //计划出发时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timProvDepaTrain.date().year();
    Month1 = pTrafficLog->m_timProvDepaTrain.date().month();
    Day1 = pTrafficLog->m_timProvDepaTrain.date().day();
    Hour1 = pTrafficLog->m_timProvDepaTrain.time().hour();
    Minu1 = pTrafficLog->m_timProvDepaTrain.time().minute();
    Secd1 = pTrafficLog->m_timProvDepaTrain.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //实际出发时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timRealDepaTrain.date().year();
    Month1 = pTrafficLog->m_timRealDepaTrain.date().month();
    Day1 = pTrafficLog->m_timRealDepaTrain.date().day();
    Hour1 = pTrafficLog->m_timRealDepaTrain.time().hour();
    Minu1 = pTrafficLog->m_timRealDepaTrain.time().minute();
    Secd1 = pTrafficLog->m_timRealDepaTrain.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //邻站同意发车时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timToAdjtStaAgrDepaTrain.date().year();
    Month1 = pTrafficLog->m_timToAdjtStaAgrDepaTrain.date().month();
    Day1 = pTrafficLog->m_timToAdjtStaAgrDepaTrain.date().day();
    Hour1 = pTrafficLog->m_timToAdjtStaAgrDepaTrain.time().hour();
    Minu1 = pTrafficLog->m_timToAdjtStaAgrDepaTrain.time().minute();
    Secd1 = pTrafficLog->m_timToAdjtStaAgrDepaTrain.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //到达邻站时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timtoAdjtStation.date().year();
    Month1 = pTrafficLog->m_timtoAdjtStation.date().month();
    Day1 = pTrafficLog->m_timtoAdjtStation.date().day();
    Hour1 = pTrafficLog->m_timtoAdjtStation.time().hour();
    Minu1 = pTrafficLog->m_timtoAdjtStation.time().minute();
    Secd1 = pTrafficLog->m_timtoAdjtStation.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //出站口信号机设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2), &pTrafficLog->m_nCodeDepartStaEquip, 2);
    //出发车次是上行车次
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bDepartTrainNumSX?0x01:0x00;

    //删除标志
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bDeleteFlag?0x01:0x00;
    //电力
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bElectric?0x01:0x00;
    //超限等级
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_nLevelCX;
    //列车类型（客车/货车）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_nLHFlg==LCTYPE_KC?0x01:0x00;
    //列车类型（管内动车组、通勤列车等）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_nIndexLCLX;
    //运行类型序号（动车组、快速旅客列车等）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_nIndexYXLX;
    //线路所标志
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->bXianLuSuo?0x01:0x00;

    //计划执行完毕标志
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_nExecuteFlag;

    //办理客运
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bBLKY?0x01:0x00;
    //允许股道与固定径路不同
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bAllowGDNotMatch?0x01:0x00;
    //允许出入口与固定进路不同
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bAllowCRKNotMatch?0x01:0x00;
    //军运
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bArmy?0x01:0x00;
    //重点
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_bImportant?0x01:0x00;

    //来向
    byteArray = pTrafficLog->m_strFromAdjtStation.toLocal8Bit();
    ccLen = byteArray.count();
    //来向长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //来向内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }

    //去向
    byteArray = pTrafficLog->m_strToAdjtStation.toLocal8Bit();
    ccLen = byteArray.count();
    //去向长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //去向内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }

    //记事
    byteArray = pTrafficLog->m_strNotes.toLocal8Bit();
    ccLen = byteArray.count();
    //记事长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //记事内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }

    //计划类型
    byteArray = pTrafficLog->m_strTypeFlag.toLocal8Bit();
    ccLen = byteArray.count();
    //计划类型长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //计划类型内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }

    //计划检查校验状态
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_nCheckState;

    //交令状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btJALStatus;
    //列检状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btLJStatus;
    //上水状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btSSStatus;
    //吸污状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btXWStatus;
    //交票状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btJPStatus;
    //乘降状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btCJStatus;
    //摘挂状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btZGStatus;
    //列尾状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btLWStatus;
    //货检状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btHJStatus;
    //换乘状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btHCStatus;
    //装卸状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btZXStatus;
    //机车状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btJCStatus;
    //道口状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btDKStatus;
    //车号状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btCHStatus;
    //综控状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btZKStatus;
    //站务状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btZWStatus;
    //接车进路状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btJCJLStatus;
    //发车进路状态（占线板）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrafficLog->m_btFCJLStatus;

    //列车位置信息（占线板）
    byteArray = pTrafficLog->m_strTrainPosStatus.toLocal8Bit();
    ccLen = byteArray.count();
    //列车位置信息长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //列车位置信息内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }

    //计划下一个流程（占线板）
    byteArray = pTrafficLog->m_strProc.toLocal8Bit();
    ccLen = byteArray.count();
    //长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }

    //图定到站时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timChartReachStation.date().year();
    Month1 = pTrafficLog->m_timChartReachStation.date().month();
    Day1 = pTrafficLog->m_timChartReachStation.date().day();
    Hour1 = pTrafficLog->m_timChartReachStation.time().hour();
    Minu1 = pTrafficLog->m_timChartReachStation.time().minute();
    Secd1 = pTrafficLog->m_timChartReachStation.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);

    //图定出发时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrafficLog->m_timChartDepaTrain.date().year();
    Month1 = pTrafficLog->m_timChartDepaTrain.date().month();
    Day1 = pTrafficLog->m_timChartDepaTrain.date().day();
    Hour1 = pTrafficLog->m_timChartDepaTrain.time().hour();
    Minu1 = pTrafficLog->m_timChartDepaTrain.time().minute();
    Secd1 = pTrafficLog->m_timChartDepaTrain.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);

    //qDebug()<<"TrafficLog dataArray="<<ByteArrayToString(dataArray);
    return dataArray;
}
//打包进路序列为数据帧数组
QByteArray MyStation::packTrainRouteOrderToArray(TrainRouteOrder *pTrainRouteOrder)
{
    QByteArray dataArray;
    int nCount = 0;
    //进路序列id
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data(),&pTrainRouteOrder->route_id,2);
    //计划号
    nCount+=4;
    dataArray.append(4, char(0));
    memcpy(dataArray.data()+(nCount-4),&pTrainRouteOrder->m_nPlanNumber,4);
    //始发（终到）标志位
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_btBeginOrEndFlg;
    //到达/出发标志
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_btRecvOrDepart;
    //车次号
    QByteArray byteArray = pTrainRouteOrder->m_strTrainNum.toLocal8Bit();
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
    //股道设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2),&pTrainRouteOrder->m_nTrackCode,2);
    //计划时间
    nCount+=7;
    dataArray.append(7, char(0));
    int Year1 = pTrainRouteOrder->m_timPlanned.date().year();
    int Month1 = pTrainRouteOrder->m_timPlanned.date().month();
    int Day1 = pTrainRouteOrder->m_timPlanned.date().day();
    int Hour1 = pTrainRouteOrder->m_timPlanned.time().hour();
    int Minu1 = pTrainRouteOrder->m_timPlanned.time().minute();
    int Secd1 = pTrainRouteOrder->m_timPlanned.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //开始时间
    nCount+=7;
    dataArray.append(7, char(0));
    Year1 = pTrainRouteOrder->m_timBegin.date().year();
    Month1 = pTrainRouteOrder->m_timBegin.date().month();
    Day1 = pTrainRouteOrder->m_timBegin.date().day();
    Hour1 = pTrainRouteOrder->m_timBegin.time().hour();
    Minu1 = pTrainRouteOrder->m_timBegin.time().minute();
    Secd1 = pTrainRouteOrder->m_timBegin.time().second();
    memcpy(dataArray.data()+(nCount-7), &Year1, 2);
    memcpy(dataArray.data()+(nCount-5), &Month1, 1);
    memcpy(dataArray.data()+(nCount-4), &Day1, 1);
    memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
    memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
    memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    //进站口信号机设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2), &pTrainRouteOrder->m_nCodeReachStaEquip, 2);
    //出站口信号机设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2), &pTrainRouteOrder->m_nCodeDepartStaEquip, 2);
    //自触模式
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_nAutoTouch?0x01:0x00;
    //电力
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_bElectric?0x01:0x00;
    //超限等级
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_nLevelCX;
    //列车类型（客车/货车）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_nLHFlg==LCTYPE_KC?0x01:0x00;
    //线路所标志
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->bXianLuSuo?0x01:0x00;
    //进路描述（"-"分割）*************
    byteArray = pTrainRouteOrder->m_strRouteDescrip.toLocal8Bit();
    ccLen = byteArray.count();
    //进路描述长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //进路描述内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //实际触发进路描述（","分割）*************
    byteArray = pTrainRouteOrder->m_strRouteDescripReal.toLocal8Bit();
    ccLen = byteArray.count();
    //实际触发进路描述长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //实际触发进路描述内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //进路当前状态
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_nOldRouteState;
    //进路当前状态描述*************
    byteArray = pTrainRouteOrder->m_strRouteState.toLocal8Bit();
    ccLen = byteArray.count();
    //进路当前状态描述长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //进路当前状态描述内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //进路方向*************
    byteArray = pTrainRouteOrder->m_strDirection.toLocal8Bit();
    ccLen = byteArray.count();
    //进路方向长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //进路方向内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //是延续进路
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_bYXJL?0x01:0x00;
    //是变通进路
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_bBTJL?0x01:0x00;
    //变通进路数组，用“|”连接*************
    QString strArray = pTrainRouteOrder->strArrayBTJL.join("|");
    byteArray = strArray.toLocal8Bit();
    ccLen = byteArray.count();
    //变通进路数组
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //变通进路数组内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //删除标志
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_bDeleteFlag?0x01:0x00;
    //组合进路
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_bZHJL?0x01:0x00;
    //组合进路父id
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2),&pTrainRouteOrder->father_id,2);

    //是否人工办理进路时生成
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrainRouteOrder->m_bCreateByMan?0x01:0x00;


    //qDebug()<<"TrainRouteOrder dataArray="<<ByteArrayToString(dataArray);
    return dataArray;
}
//打包调度命令为数据帧数组
QByteArray MyStation::packDisOrderToArray(DispatchOrderStation *pDisOrderSta)
{
    QByteArray dataArray;
    int nCount = 0;
    //子分类码
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = 0x01;//车站调度命令
    //id
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2),&pDisOrderSta->order_id,2);
    //命令号
    nCount+=4;
    dataArray.append(4, char(0));
    memcpy(dataArray.data()+(nCount-4),&pDisOrderSta->uNumber,4);
    //接收时间
    {
        nCount+=7;
        dataArray.append(7, char(0));
        int Year1 = pDisOrderSta->timOrder.date().year();
        int Month1 = pDisOrderSta->timOrder.date().month();
        int Day1 = pDisOrderSta->timOrder.date().day();
        int Hour1 = pDisOrderSta->timOrder.time().hour();
        int Minu1 = pDisOrderSta->timOrder.time().minute();
        int Secd1 = pDisOrderSta->timOrder.time().second();
        memcpy(dataArray.data()+(nCount-7), &Year1, 2);
        memcpy(dataArray.data()+(nCount-5), &Month1, 1);
        memcpy(dataArray.data()+(nCount-4), &Day1, 1);
        memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
        memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
        memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    }
    //调度中心
    {
        QByteArray byteArray = pDisOrderSta->strDisCenter.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //调度员/发令人
    {
        QByteArray byteArray = pDisOrderSta->strDisName.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //命令类型
    {
        QByteArray byteArray = pDisOrderSta->strType.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //命令内容
    {
        QByteArray byteArray = pDisOrderSta->strContent.toLocal8Bit();
        int len = byteArray.count();
        //长度
//        nCount++;
//        dataArray.append(1, char(0));
        nCount+=2;
        dataArray.append(2, char(0));
        dataArray[nCount-2] = len;//低位在前
        dataArray[nCount-1] = len>>8;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //接收单位
    {
        QString strArray = pDisOrderSta->listRecvPlace.join("|");
        QByteArray byteArray = strArray.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //签收状态
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pDisOrderSta->nStateDisOrder;
    qDebug()<<"车站调度命令"<<pDisOrderSta->strType<<pDisOrderSta->nStateDisOrder;
    //签收人
    {
        QByteArray byteArray = pDisOrderSta->strSignName.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //签收时间
    {
        nCount+=7;
        dataArray.append(7, char(0));
        int Year1 = pDisOrderSta->timSign.date().year();
        int Month1 = pDisOrderSta->timSign.date().month();
        int Day1 = pDisOrderSta->timSign.date().day();
        int Hour1 = pDisOrderSta->timSign.time().hour();
        int Minu1 = pDisOrderSta->timSign.time().minute();
        int Secd1 = pDisOrderSta->timSign.time().second();
        memcpy(dataArray.data()+(nCount-7), &Year1, 2);
        memcpy(dataArray.data()+(nCount-5), &Month1, 1);
        memcpy(dataArray.data()+(nCount-4), &Day1, 1);
        memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
        memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
        memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    }
    //阅读人数组
    {
        QString strArray = pDisOrderSta->listReadName.join("|");
        QByteArray byteArray = strArray.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }

    //qDebug()<<"DispatchOrderStation dataArray="<<ByteArrayToString(dataArray);
    return dataArray;
}
//打包[调度台调度命令]为数据帧数组
QByteArray MyStation::packDisOrderDDTToArray(DispatchOrderDispatcher *pDisOrderDDT)
{
    QByteArray dataArray;
    int nCount = 0;
    //子分类码
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = 0x02;//调度台调度命令
    //id
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2),&pDisOrderDDT->order_id,2);
    //命令号
    nCount+=4;
    dataArray.append(4, char(0));
    memcpy(dataArray.data()+(nCount-4),&pDisOrderDDT->uNumber,4);
    //命令类型
    {
        QByteArray byteArray = pDisOrderDDT->strType.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //命令内容
    {
        QByteArray byteArray = pDisOrderDDT->strContent.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount+=2;
        dataArray.append(2, char(0));
        dataArray[nCount-2] = len;//低位在前
        dataArray[nCount-1] = len>>8;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //车站
    {
        QByteArray byteArray = pDisOrderDDT->strStation.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //值班人
    {
        QByteArray byteArray = pDisOrderDDT->strDutyName.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //创建时间
    {
        nCount+=7;
        dataArray.append(7, char(0));
        QDateTime tim = pDisOrderDDT->timCreate;
        int Year1 = tim.date().year();
        int Month1 = tim.date().month();
        int Day1 = tim.date().day();
        int Hour1 = tim.time().hour();
        int Minu1 = tim.time().minute();
        int Secd1 = tim.time().second();
        memcpy(dataArray.data()+(nCount-7), &Year1, 2);
        memcpy(dataArray.data()+(nCount-5), &Month1, 1);
        memcpy(dataArray.data()+(nCount-4), &Day1, 1);
        memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
        memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
        memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    }
    //发送状态
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pDisOrderDDT->bSend?0x01:0x00;
    //发送时间
    {
        nCount+=7;
        dataArray.append(7, char(0));
        QDateTime tim = pDisOrderDDT->timSend;
        int Year1 = tim.date().year();
        int Month1 = tim.date().month();
        int Day1 = tim.date().day();
        int Hour1 = tim.time().hour();
        int Minu1 = tim.time().minute();
        int Secd1 = tim.time().second();
        memcpy(dataArray.data()+(nCount-7), &Year1, 2);
        memcpy(dataArray.data()+(nCount-5), &Month1, 1);
        memcpy(dataArray.data()+(nCount-4), &Day1, 1);
        memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
        memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
        memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    }
    //调度台个数
    int ddtCount = pDisOrderDDT->vectDispathInfo.size();
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ddtCount;
    //调度台内容
    for(int i=0; i<ddtCount; i++)
    {
        //调度台
        {
            QByteArray byteArray = pDisOrderDDT->vectDispathInfo[i].strDispatcher.toLocal8Bit();
            int len = byteArray.count();
            //长度
            nCount++;
            dataArray.append(1, char(0));
            dataArray[nCount-1] = len;
            //内容
            nCount+=len;
            dataArray.append(len, char(0));
            for(int u=0; u<len; u++)
            {
                dataArray[nCount-len+u] = byteArray[u];
            }
        }
        //接收状态
        {
            nCount++;
            dataArray.append(1, char(0));
            dataArray[nCount-1] = pDisOrderDDT->vectDispathInfo[i].nRecvState;
        }
        //调度台接收时间
        {
            nCount+=7;
            dataArray.append(7, char(0));
            QDateTime tim = pDisOrderDDT->vectDispathInfo[i].timRecv;
            int Year1 = tim.date().year();
            int Month1 = tim.date().month();
            int Day1 = tim.date().day();
            int Hour1 = tim.time().hour();
            int Minu1 = tim.time().minute();
            int Secd1 = tim.time().second();
            memcpy(dataArray.data()+(nCount-7), &Year1, 2);
            memcpy(dataArray.data()+(nCount-5), &Month1, 1);
            memcpy(dataArray.data()+(nCount-4), &Day1, 1);
            memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
            memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
            memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
        }
    }

    //qDebug()<<"DispatchOrderLocomotive dataArray="<<ByteArrayToString(dataArray);
    return dataArray;
}
//打包[机车调度命令]为数据帧数组
QByteArray MyStation::packDisOrderJCToArray(DispatchOrderLocomotive *pDisOrderJC)
{
    QByteArray dataArray;
    int nCount = 0;
    //子分类码
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = 0x03;//机车调度命令
    //id
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2),&pDisOrderJC->order_id,2);
    //命令种类
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pDisOrderJC->orderType;
    //命令号
    nCount+=4;
    dataArray.append(4, char(0));
    memcpy(dataArray.data()+(nCount-4),&pDisOrderJC->uNumber,4);
    //命令类型
    {
        QByteArray byteArray = pDisOrderJC->strType.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //命令内容
    {
        QByteArray byteArray = pDisOrderJC->strContent.toLocal8Bit();
        int len = byteArray.count();
        //长度
//        nCount++;
        nCount+=2;
        dataArray.append(2, char(0));
        dataArray[nCount-2] = len;//低位在前
        dataArray[nCount-1] = len>>8;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //车站
    {
        QByteArray byteArray = pDisOrderJC->strStation.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //值班人
    {
        QByteArray byteArray = pDisOrderJC->strDutyName.toLocal8Bit();
        int len = byteArray.count();
        //长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = len;
        //内容
        nCount+=len;
        dataArray.append(len, char(0));
        for(int u=0; u<len; u++)
        {
            dataArray[nCount-len+u] = byteArray[u];
        }
    }
    //创建时间
    {
        nCount+=7;
        dataArray.append(7, char(0));
        QDateTime tim = pDisOrderJC->timCreate;
        int Year1 = tim.date().year();
        int Month1 = tim.date().month();
        int Day1 = tim.date().day();
        int Hour1 = tim.time().hour();
        int Minu1 = tim.time().minute();
        int Secd1 = tim.time().second();
        memcpy(dataArray.data()+(nCount-7), &Year1, 2);
        memcpy(dataArray.data()+(nCount-5), &Month1, 1);
        memcpy(dataArray.data()+(nCount-4), &Day1, 1);
        memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
        memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
        memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    }
    //发送状态
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pDisOrderJC->bSend?0x01:0x00;
    //发送时间
    {
        nCount+=7;
        dataArray.append(7, char(0));
        QDateTime tim = pDisOrderJC->timSend;
        int Year1 = tim.date().year();
        int Month1 = tim.date().month();
        int Day1 = tim.date().day();
        int Hour1 = tim.time().hour();
        int Minu1 = tim.time().minute();
        int Secd1 = tim.time().second();
        memcpy(dataArray.data()+(nCount-7), &Year1, 2);
        memcpy(dataArray.data()+(nCount-5), &Month1, 1);
        memcpy(dataArray.data()+(nCount-4), &Day1, 1);
        memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
        memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
        memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
    }
    //车次（机车）个数
    int jcCount = pDisOrderJC->vectLocmtInfo.size();
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = jcCount;
    //车次（机车）内容
    for(int i=0; i<jcCount; i++)
    {
        //车次
        {
            QByteArray byteArray = pDisOrderJC->vectLocmtInfo[i].strCheCi.toLocal8Bit();
            int len = byteArray.count();
            //长度
            nCount++;
            dataArray.append(1, char(0));
            dataArray[nCount-1] = len;
            //内容
            nCount+=len;
            dataArray.append(len, char(0));
            for(int u=0; u<len; u++)
            {
                dataArray[nCount-len+u] = byteArray[u];
            }
        }
        //机车
        {
            QByteArray byteArray = pDisOrderJC->vectLocmtInfo[i].strLocomotive.toLocal8Bit();
            int len = byteArray.count();
            //长度
            nCount++;
            dataArray.append(1, char(0));
            dataArray[nCount-1] = len;
            //内容
            nCount+=len;
            dataArray.append(len, char(0));
            for(int u=0; u<len; u++)
            {
                dataArray[nCount-len+u] = byteArray[u];
            }
        }
        //签收状态
        {
            nCount++;
            dataArray.append(1, char(0));
            dataArray[nCount-1] = pDisOrderJC->vectLocmtInfo[i].nRecvState;
        }
        //机车接收时间
        {
            nCount+=7;
            dataArray.append(7, char(0));
            QDateTime tim = pDisOrderJC->vectLocmtInfo[i].timRecv;
            int Year1 = tim.date().year();
            int Month1 = tim.date().month();
            int Day1 = tim.date().day();
            int Hour1 = tim.time().hour();
            int Minu1 = tim.time().minute();
            int Secd1 = tim.time().second();
            memcpy(dataArray.data()+(nCount-7), &Year1, 2);
            memcpy(dataArray.data()+(nCount-5), &Month1, 1);
            memcpy(dataArray.data()+(nCount-4), &Day1, 1);
            memcpy(dataArray.data()+(nCount-3), &Hour1, 1);
            memcpy(dataArray.data()+(nCount-2), &Minu1, 1);
            memcpy(dataArray.data()+(nCount-1), &Secd1, 1);
        }
    }

    //qDebug()<<"DispatchOrderLocomotive dataArray="<<ByteArrayToString(dataArray);
    return dataArray;
}
//打包[股道防溜]为数据帧数组(所有股道)
QByteArray MyStation::packGDAntiSlipToArray()
{
    int size = m_ArrayGDAntiSlip.size();
    QByteArray dataArray;
    int nCount = 0;
    //股道个数
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = size;
    //遍历逐个增加
    for(int i=0; i<size; i++)
    {
        CGD* pGD = m_ArrayGDAntiSlip[i];
        //股道设备号
        nCount+=2;
        dataArray.append(2, char(0));
        memcpy(dataArray.data()+(nCount-2),&pGD->m_nCode,2);

        //左侧防溜类型（接车口）
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = pGD->m_nLAntiSlipType;
        //左侧铁鞋号
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = pGD->m_nLTxNum;
        //左侧紧固器号
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = pGD->m_nLJgqNum;
        //左侧警内米数
        nCount+=2;
        dataArray.append(2, char(0));
        memcpy(dataArray.data()+(nCount-2),&pGD->m_nLJnMeters,2);

        //右侧防溜类型（接车口）
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = pGD->m_nRAntiSlipType;
        //右侧铁鞋号
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = pGD->m_nRTxNum;
        //右侧紧固器号
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = pGD->m_nRJgqNum;
        //右侧警内米数
        nCount+=2;
        dataArray.append(2, char(0));
        memcpy(dataArray.data()+(nCount-2),&pGD->m_nRJnMeters,2);

        //存车信息 旧版本 int类型
        nCount+=2;
        dataArray.append(2, char(0));
        memcpy(dataArray.data()+(nCount-2),&pGD->m_nTrainNums,2);
        //存车信息 新版本 字符串类型
        nCount+=2;
        dataArray.append(2, char(0));
        QByteArray strTrainInfoZCC = pGD->m_sTrainInfoShow.toLocal8Bit();//toLatin1();
        int nTrainInfoZCClen = strTrainInfoZCC.count();
        memcpy(dataArray.data()+(nCount-2),&nTrainInfoZCClen,2);

        nCount+=nTrainInfoZCClen;
        for(int u=0; u<nTrainInfoZCClen; u++)
        {
            //dataArray[u] = strTrainInfoZCC[u];
            dataArray[nCount-nTrainInfoZCClen+u] = strTrainInfoZCC[u];
        }
        //qDebug()<<"GDAntiSlip dataArray=****"<<i<<pGD->getName()<<pGD->m_nLTxNum<<pGD->m_nRTxNum;
    }
   // qDebug()<<"GDAntiSlip dataArray="<<ByteArrayToString(dataArray);
    return dataArray;
}

//初始化同步数据包[阶段计划]
QByteArray MyStation::initSyncPackStagePlan(StagePlan* pStagePlan,int syncFlag,int packSize,int packNum)
{
    QByteArray qSendArray;
    qSendArray.append(16, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_JDJH;
    //功能码
    qSendArray[11] = syncFlag;//SYNC_FLAG_ADD;
    //总包数
    int size = packSize;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = packNum;
    memcpy(qSendArray.data()+14, &num, 2);
    //具体数据
    if(pStagePlan)
    {
        QByteArray aArray = this->packStagePlanToArray(pStagePlan);
        qSendArray.append(aArray);
    }
    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());
    //返回
    return qSendArray;
}
//初始化同步数据包[行车日志]
QByteArray MyStation::initSyncPackTrafficLog(TrafficLog *pTrafficLog, int syncFlag, int packSize, int packNum)
{
    QByteArray qSendArray;
    qSendArray.append(16, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_XCRZ;
    //功能码
    qSendArray[11] = syncFlag;//SYNC_FLAG_ADD;
    //总包数
    int size = packSize;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = packNum;
    memcpy(qSendArray.data()+14, &num, 2);
    //具体数据
    if(pTrafficLog)
    {
        QByteArray aArray = this->packTrafficLogToArray(pTrafficLog);
        qSendArray.append(aArray);
    }
    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());
    //返回
    return qSendArray;
}
//初始化同步数据包[进路序列]
QByteArray MyStation::initSyncPackTrainRouteOrder(TrainRouteOrder *pTrainRouteOrder, int syncFlag, int packSize, int packNum)
{
    QByteArray qSendArray;
    qSendArray.append(16, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_JLXL;
    //功能码
    qSendArray[11] = syncFlag;//SYNC_FLAG_ADD;
    //总包数
    int size = packSize;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = packNum;
    memcpy(qSendArray.data()+14, &num, 2);
    //具体数据
    if(pTrainRouteOrder)
    {
        QByteArray aArray = this->packTrainRouteOrderToArray(pTrainRouteOrder);
        qSendArray.append(aArray);
    }
    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());
    //返回
    return qSendArray;
}
//初始化同步数据包[调度命令]
QByteArray MyStation::initSyncPackDisOrder(DispatchOrderStation *pDisOrderSta, int syncFlag, int packSize, int packNum)
{
    QByteArray qSendArray;
    qSendArray.append(16, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_DDML;
    //功能码
    qSendArray[11] = syncFlag;//SYNC_FLAG_ADD;
    //总包数
    int size = packSize;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = packNum;
    memcpy(qSendArray.data()+14, &num, 2);
    //具体数据
    if(pDisOrderSta)
    {
        QByteArray aArray = this->packDisOrderToArray(pDisOrderSta);
        qSendArray.append(aArray);
    }
    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());
    //返回
    return qSendArray;
}
//初始化同步数据包[调度台调度命令]
QByteArray MyStation::initSyncPackDisOrderDDT(DispatchOrderDispatcher *pDisOrderDDT, int syncFlag, int packSize, int packNum)
{
    QByteArray qSendArray;
    qSendArray.append(16, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_DDML;
    //功能码
    qSendArray[11] = syncFlag;//SYNC_FLAG_ADD;
    //总包数
    int size = packSize;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = packNum;
    memcpy(qSendArray.data()+14, &num, 2);
    //具体数据
    if(pDisOrderDDT)
    {
        QByteArray aArray = this->packDisOrderDDTToArray(pDisOrderDDT);
        qSendArray.append(aArray);
    }
    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());
    //返回
    return qSendArray;
}
//初始化同步数据包[机车调度命令]
QByteArray MyStation::initSyncPackDisOrderJC(DispatchOrderLocomotive *pDisOrderJC, int syncFlag, int packSize, int packNum)
{
    QByteArray qSendArray;
    qSendArray.append(16, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_DDML;
    //功能码
    qSendArray[11] = syncFlag;//SYNC_FLAG_ADD;
    //总包数
    int size = packSize;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = packNum;
    memcpy(qSendArray.data()+14, &num, 2);
    //具体数据
    if(pDisOrderJC)
    {
        QByteArray aArray = this->packDisOrderJCToArray(pDisOrderJC);
        qSendArray.append(aArray);
    }
    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());
    //返回
    return qSendArray;
}
//初始化同步数据包[股道防溜-所有股道]
QByteArray MyStation::initSyncPackGDAntiSlip(int syncFlag, int packSize, int packNum)
{
    QByteArray aArray = this->packGDAntiSlipToArray();
    QByteArray qSendArray;
    qSendArray.append(16, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_GDFL;
    //功能码
    qSendArray[11] = syncFlag;//SYNC_FLAG_ADD;
    //总包数
    int size = packSize;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = packNum;
    memcpy(qSendArray.data()+14, &num, 2);
    //具体数据
    qSendArray.append(aArray);
    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());
    //返回
    return qSendArray;
}

//发送所有的[阶段计划]给Soft
void MyStation::sendSyncAllStagePlanToSoft(int softType, int currSoftIndex)
{
    int size = this->m_ArrayStagePlan.size();
    for(int i=0; i<size; i++)
    {
        StagePlan* pStagePlan = this->m_ArrayStagePlan[i];
        //处理和发送1个数据
        sendOneStagePlanToSoft(softType, pStagePlan,SYNC_FLAG_ADD,size,i+1,currSoftIndex);
//        //_sleep(1000); //没效果，多包数据还是会被自动拼到一起
//        //QTest::qSleep(10);//ms 有效果！
//        QThread::msleep(50);//ms 有效果！
    }
}
//发送所有的[进路序列]给Soft
void MyStation::sendSyncAllTrainRouteOrderToSoft(int softType, int currSoftIndex)
{
    int size = this->m_ArrayRouteOrder.size();
    for(int i=0; i<size; i++)
    {
        TrainRouteOrder* pTrainRouteOrder = this->m_ArrayRouteOrder[i];
        //处理和发送1个数据
        sendOneTrainRouteOrderToSoft(softType, pTrainRouteOrder,SYNC_FLAG_ADD,size,i+1,currSoftIndex);
//        //_sleep(1000); //没效果，多包数据还是会被自动拼到一起(粘包)
//        //QTest::qSleep(10);//ms 有效果！
//        QThread::msleep(100);//ms 有效果！
    }
}
//发送所有的[行车日志]给Soft
void MyStation::sendSyncAllTrafficLogToSoft(int softType, int currSoftIndex)
{
    int size = this->m_ArrayTrafficLog.size();
    for(int i=0; i<size; i++)
    {
        TrafficLog* pTrafficLog = this->m_ArrayTrafficLog[i];
        //处理和发送1个数据
        sendOneTrafficLogToSoft(softType,pTrafficLog,SYNC_FLAG_ADD,size,i+1,currSoftIndex);
//        //_sleep(1000); //没效果，多包数据还是会被自动拼到一起
//        //QTest::qSleep(10);//ms 有效果！
//        QThread::msleep(50);//ms 有效果！
    }
}
//发送同步数据-所有的[调度命令]给Soft
void MyStation::sendSyncAllDisOrderToSoft(int softType, int currSoftIndex)
{
    //车站调度命令
    {
        int size = this->m_ArrayDisOrderRecv.size();
        for(int i=0; i<size; i++)
        {
            DispatchOrderStation* pDisOrderSta = this->m_ArrayDisOrderRecv[i];
            //处理和发送1个数据
            sendOneDisOrderToSoft(softType,pDisOrderSta,SYNC_FLAG_ADD,size,i+1,currSoftIndex);
//            //_sleep(1000); //没效果，多包数据还是会被自动拼到一起
//            //QTest::qSleep(10);//ms 有效果！
//            QThread::msleep(50);//ms 有效果！
        }
    }
    //调度台调度命令
    {
        int size = this->m_ArrayDisOrderDisp.size();
        for(int i=0; i<size; i++)
        {
            DispatchOrderDispatcher* pDisOrder = this->m_ArrayDisOrderDisp[i];
            //处理和发送1个数据
            sendOneDisOrderDDTToSoft(softType,pDisOrder,SYNC_FLAG_ADD,size,i+1,currSoftIndex);
//            //_sleep(1000); //没效果，多包数据还是会被自动拼到一起
//            //QTest::qSleep(10);//ms 有效果！
//            //QThread::msleep(50);//ms 有效果！
//            QThread::msleep(100);//ms 有效果！
        }
    }
    //接车调度命令
    {
        int size = this->m_ArrayDisOrderLocomot.size();
        for(int i=0; i<size; i++)
        {
            DispatchOrderLocomotive* pDisOrder = this->m_ArrayDisOrderLocomot[i];
            //处理和发送1个数据
            sendOneDisOrderJCToSoft(softType,pDisOrder,SYNC_FLAG_ADD,size,i+1,currSoftIndex);
//            //_sleep(1000); //没效果，多包数据还是会被自动拼到一起
//            //QTest::qSleep(10);//ms 有效果！
//            //QThread::msleep(50);//ms 有效果！
//            QThread::msleep(100);//ms 有效果！
        }
    }
}
//发送同步数据-所有的[股道防溜]给Soft
void MyStation::sendSyncAllGDAntiSlipToSoft(int softType,int currSoftIndex)
{
    //所有的打包到一包发送
    sendAllGDAntiSlipToSoft(softType, SYNC_FLAG_ADD,1,1,currSoftIndex);
}
//发送[同步数据]给Soft
void MyStation::sendSyncDataToSoft(int softType, QByteArray qSendArray, int currSoftIndex)
{
    //发送信号
    if(DATATYPE_ALL == softType)//所有终端（CTC+ZXB+JK+ZXT）
    {
        emit sendDataToCTCSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
        emit sendDataToBoardSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
        emit sendDataToJKSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
        emit sendDataToZXTSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
    else if(DATATYPE_CTC == softType)//CTC终端
    {
        emit sendDataToCTCSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
    else if(DATATYPE_BOARD == softType)//占线板终端
    {
        emit sendDataToBoardSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
    else if(DATATYPE_JK == softType)//集控终端
    {
        emit sendDataToJKSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
    else if(DATATYPE_ZXT == softType)//占线图终端
    {
        emit sendDataToZXTSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
}

void MyStation::sendOneStagePlanToSoft(int softType, StagePlan* pStagePlan,int syncFlag,int packSize,int packNum,int currSoftIndex)
{
    QByteArray qSendArray = initSyncPackStagePlan(pStagePlan,syncFlag,packSize,packNum);
    //发送
    sendSyncDataToSoft(softType, qSendArray, currSoftIndex);
}
//发送一个[进路序列]给Soft
void MyStation::sendOneTrainRouteOrderToSoft(int softType, TrainRouteOrder *pTrainRouteOrder, int syncFlag, int packSize, int packNum, int currSoftIndex)
{
    //非按图排路 且 不是删除全部
    if(0 == this->ModalSelect.nStateSelect && SYNC_FLAG_DELALL!=syncFlag)
    {
        //人工排路模式，不发送同步命令
        return;
    }
    QByteArray qSendArray = initSyncPackTrainRouteOrder(pTrainRouteOrder,syncFlag,packSize,packNum);
    //发送
    sendSyncDataToSoft(softType, qSendArray, currSoftIndex);
}
//发送一个[行车日志]给Soft
void MyStation::sendOneTrafficLogToSoft(int softType, TrafficLog *pTrafficLog, int syncFlag, int packSize, int packNum, int currSoftIndex)
{
    QByteArray qSendArray = initSyncPackTrafficLog(pTrafficLog,syncFlag,packSize,packNum);
    //发送
    sendSyncDataToSoft(softType, qSendArray, currSoftIndex);
}
//发送一个[调度命令]给Soft
void MyStation::sendOneDisOrderToSoft(int softType, DispatchOrderStation *pDisOrderSta, int syncFlag, int packSize, int packNum, int currSoftIndex)
{
    QByteArray qSendArray = initSyncPackDisOrder(pDisOrderSta,syncFlag,packSize,packNum);
    //发送
    sendSyncDataToSoft(softType, qSendArray, currSoftIndex);
}
//发送一个[调度台调度命令]给CTC
void MyStation::sendOneDisOrderDDTToSoft(int softType, DispatchOrderDispatcher *pDisOrderDDT, int syncFlag, int packSize, int packNum, int currSoftIndex)
{
    QByteArray qSendArray = initSyncPackDisOrderDDT(pDisOrderDDT,syncFlag,packSize,packNum);
    //发送
    sendSyncDataToSoft(softType, qSendArray, currSoftIndex);
}
//发送一个[机车调度命令]给Soft
void MyStation::sendOneDisOrderJCToSoft(int softType, DispatchOrderLocomotive *pDisOrderJC, int syncFlag, int packSize, int packNum, int currSoftIndex)
{
    QByteArray qSendArray = initSyncPackDisOrderJC(pDisOrderJC,syncFlag,packSize,packNum);
    //发送
    sendSyncDataToSoft(softType, qSendArray, currSoftIndex);
}
//发送所有[股道防溜]给Soft
void MyStation::sendAllGDAntiSlipToSoft(int softType, int syncFlag, int packSize, int packNum, int currSoftIndex)
{
    QByteArray qSendArray = initSyncPackGDAntiSlip(syncFlag,packSize,packNum);
    //发送
    sendSyncDataToSoft(softType, qSendArray, currSoftIndex);
}

////接收CTC数据2-TCP通道同步数据
//void MyStation::recvCTCData2(unsigned char *recvArray, int recvlen, int currCtcIndex, int currZxbIndex)
//{
//    //同步类型码
//    int syncType = (int)(recvArray[10]&0xFF);
//    //要求功能码(数据处理类型)
//    int requireType = (int)(recvArray[11]&0xFF);
//    //请求
//    if(SYNC_FLAG_REQUEST == requireType)
//    {
//        //全部
//        if(SYNC_ALL == syncType)
//        {
//            //阶段计划
//            QFuture<void> future1 = QtConcurrent::run(this, &MyStation::sendSyncAllStagePlanToSoft, DATATYPE_CTC, currCtcIndex);
//            QFuture<void> future2 = QtConcurrent::run(this, &MyStation::sendSyncAllTrainRouteOrderToSoft, DATATYPE_CTC, currCtcIndex);
//            QFuture<void> future3 = QtConcurrent::run(this, &MyStation::sendSyncAllTrafficLogToSoft, DATATYPE_CTC, currCtcIndex);
//            QFuture<void> future4 = QtConcurrent::run(this, &MyStation::sendSyncAllDisOrderToSoft, DATATYPE_CTC, currCtcIndex);
//            QFuture<void> future5 = QtConcurrent::run(this, &MyStation::sendSyncAllGDAntiSlipToSoft, DATATYPE_CTC, currCtcIndex);
//            //防错办
//            QFuture<void> future6 = QtConcurrent::run(this, &MyStation::sendSyncAllGDAttrToSoft, DATATYPE_CTC, currCtcIndex);
//            QFuture<void> future7 = QtConcurrent::run(this, &MyStation::sendSyncAllGatewayAttrToSoft, DATATYPE_CTC, currCtcIndex);
//        }
//        //阶段计划
//        else if(SYNC_JDJH == syncType)
//        {
//            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllStagePlanToSoft, DATATYPE_CTC, currCtcIndex);
//        }
//        //行车日志
//        else if(SYNC_XCRZ == syncType)
//        {
//            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllTrafficLogToSoft, DATATYPE_CTC, currCtcIndex);
//        }
//        //进路序列
//        else if(SYNC_JLXL == syncType)
//        {
//            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllTrainRouteOrderToSoft, DATATYPE_CTC, currCtcIndex);
//        }
//        //调度命令
//        else if(SYNC_DDML == syncType)
//        {
//            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllDisOrderToSoft, DATATYPE_CTC, currCtcIndex);
//        }
//        //股道防溜
//        else if(SYNC_GDFL == syncType)
//        {
//            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllGDAntiSlipToSoft, DATATYPE_CTC,currCtcIndex);
//        }
//    }
//}
//接收终端数据2-TCP通道同步数据-集控、占线图
void MyStation::recvTerminalData2(int softType, unsigned char *recvArray, int recvlen, int currIndex)
{
    //同步类型码
    int syncType = (int)(recvArray[10]&0xFF);
    //要求功能码(数据处理类型)
    int requireType = (int)(recvArray[11]&0xFF);
    //请求
    if(SYNC_FLAG_REQUEST == requireType)
    {
        //全部
        if(SYNC_ALL == syncType)
        {
            //阶段计划
            QFuture<void> future1 = QtConcurrent::run(this, &MyStation::sendSyncAllStagePlanToSoft, softType, currIndex);
            QFuture<void> future2 = QtConcurrent::run(this, &MyStation::sendSyncAllTrainRouteOrderToSoft, softType, currIndex);
            QFuture<void> future3 = QtConcurrent::run(this, &MyStation::sendSyncAllTrafficLogToSoft, softType, currIndex);
            QFuture<void> future4 = QtConcurrent::run(this, &MyStation::sendSyncAllDisOrderToSoft, softType, currIndex);
            QFuture<void> future5 = QtConcurrent::run(this, &MyStation::sendSyncAllGDAntiSlipToSoft, softType, currIndex);
            //防错办
            QFuture<void> future6 = QtConcurrent::run(this, &MyStation::sendSyncAllGDAttrToSoft, softType, currIndex);
            QFuture<void> future7 = QtConcurrent::run(this, &MyStation::sendSyncAllGatewayAttrToSoft, softType, currIndex);
        }
        //阶段计划
        else if(SYNC_JDJH == syncType)
        {
            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllStagePlanToSoft, softType, currIndex);
        }
        //行车日志
        else if(SYNC_XCRZ == syncType)
        {
            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllTrafficLogToSoft, softType, currIndex);
        }
        //进路序列
        else if(SYNC_JLXL == syncType)
        {
            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllTrainRouteOrderToSoft, softType, currIndex);
        }
        //调度命令
        else if(SYNC_DDML == syncType)
        {
            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllDisOrderToSoft, softType, currIndex);
        }
        //股道防溜
        else if(SYNC_GDFL == syncType)
        {
            QFuture<void> future = QtConcurrent::run(this, &MyStation::sendSyncAllGDAntiSlipToSoft, softType, currIndex);
        }
    }
}

//打包[防错办-单个股道属性]为数据帧数组(股道)
QByteArray MyStation::packGDAttrToArray(CGD* pGD)
{
    QByteArray dataArray;
    int nCount = 0;
    //股道设备号
    nCount+=2;
    dataArray.append(2, char(0));
    int devCode = pGD->getCode();
    memcpy(dataArray.data()+(nCount-2),&devCode,2);
    //股道名称
    QByteArray byteArray = pGD->getName().toLocal8Bit();
    int ccLen = byteArray.count();
    //长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //股道id
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2),&pGD->gdId,2);
    //线路性质
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->gdAttr;
    //接发车方向
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->jfcDir;
    //接发车类型
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->jfcAttr;
    //超限
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->overLimit;
    //站台
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->platform;
    //允许动车组
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->isCRH;
    //上水设备
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->isWater;
    //排污设备
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->isBlowdown;
    //军运
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pGD->army;

    return dataArray;
}
//打包[防错办-单个出入口属性]为数据帧数组(信号机)
QByteArray MyStation::packGatewayAttrToArray(CXHD* pXHD)
{
    QByteArray dataArray;
    int nCount = 0;
    //出入口信号机设备号
    nCount+=2;
    dataArray.append(2, char(0));
    int devCode = pXHD->getCode();
    memcpy(dataArray.data()+(nCount-2),&devCode,2);
    //出入口信号机名称
    QByteArray byteArray = pXHD->getName().toLocal8Bit();
    int ccLen = byteArray.count();
    //长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //出入口名称
    byteArray = pXHD->enexName.toLocal8Bit();
    ccLen = byteArray.count();
    //长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = ccLen;
    //内容
    nCount+=ccLen;
    dataArray.append(ccLen, char(0));
    for(int u=0; u<ccLen; u++)
    {
        dataArray[nCount-ccLen+u] = byteArray[u];
    }
    //方向
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pXHD->direct;
    //允许超限等级
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pXHD->allowOverLimit;
    //允许旅客列车
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pXHD->allowPassenger;
    //允许货物列车
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pXHD->allowFreight;

    return dataArray;
}
//发送同步数据-所有的[股道防溜]给Soft
void MyStation::sendSyncAllGDAttrToSoft(int softType, int currSoftIndex)
{
    QByteArray qSendArray;
    qSendArray.append(16+1+1, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_FCB;//防错办基础数据
    //功能码
    qSendArray[11] = SYNC_FLAG_ADD;
    //总包数
    int size = 1;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = 1;
    memcpy(qSendArray.data()+14, &num, 2);
    //子分类码-股道基础属性
    qSendArray[16] = 0x01;
    //总股道个数
    int count = this->vectGDAttr.size();
    qSendArray[17] = count;
    //具体数据
    for (int i=0; i<count && i<255; i++)
    {
        CGD* pGDAttr = this->vectGDAttr[i];
        QByteArray aArray = this->packGDAttrToArray(pGDAttr);
        qSendArray.append(aArray);
    }

    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());

    //发送信号
    if(DATATYPE_ALL == softType)//所有终端（CTC+JK+ZXT+ZXB）
    {
        emit sendDataToCTCSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
        emit sendDataToJKSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
    else if(DATATYPE_CTC == softType)//CTC终端
    {
        emit sendDataToCTCSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
    else if(DATATYPE_JK == softType)//集控终端
    {
        emit sendDataToJKSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }

}
//发送同步数据-所有的[出入口属性]给Soft
void MyStation::sendSyncAllGatewayAttrToSoft(int softType, int currSoftIndex)
{
    QByteArray qSendArray;
    qSendArray.append(16+1+1, char(0));
    //主功能码-数据同步
    qSendArray[9] = FUNCTYPE_SYNC;
    //同步类型
    qSendArray[10] = SYNC_FCB;//防错办基础数据
    //功能码
    qSendArray[11] = SYNC_FLAG_ADD;
    //总包数
    int size = 1;
    memcpy(qSendArray.data()+12, &size, 2);
    //当前包号
    int num = 1;
    memcpy(qSendArray.data()+14, &num, 2);
    //子分类码-出入口基础属性
    qSendArray[16] = 0x02;
    //总出入口个数
    int count = this->vectGatewayAttr.size();
    qSendArray[17] = count;
    //具体数据
    for (int i=0; i<count && i<255; i++)
    {
        CXHD* pXHDAttr = this->vectGatewayAttr[i];
        QByteArray aArray = this->packGatewayAttrToArray(pXHDAttr);
        qSendArray.append(aArray);
    }

    //帧尾4+预留2
    qSendArray.append(6, char(0));
    //制作数据帧头帧尾和关键信息
    packHeadAndTail(&qSendArray, qSendArray.size());

    //发送信号
    if(DATATYPE_ALL == softType)//所有终端（CTC+JK+ZXT+ZXB）
    {
        emit sendDataToCTCSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
        emit sendDataToJKSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
    else if(DATATYPE_CTC == softType)//CTC终端
    {
        emit sendDataToCTCSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
    else if(DATATYPE_JK == softType)//集控终端
    {
        emit sendDataToJKSignal2(this, qSendArray, qSendArray.size(), currSoftIndex);
    }
}

void MyStation::SetTrafficLogProc(TrafficLog* pTrafficLog)
{
    if(pTrafficLog->m_btBeginOrEndFlg != 0xBB)
    {
        if(pTrafficLog->m_timRealReachStation.isNull())
        {
            if(pTrafficLog->m_timAgrFromAdjtStaDepaTrain.isNull())
            {
                pTrafficLog->m_strProc = PROCESS_JCYG;
                return;
            }

            if(pTrafficLog->m_btJCJLStatus != STATUS_JLBL_SUCC)
            {
                pTrafficLog->m_strProc = PROCESS_BLJL;
                return;
            }

            pTrafficLog->m_strProc = PROCESS_LCDDTG;
            return;
        }
    }

    if(pTrafficLog->m_btBeginOrEndFlg != 0xCC)
    {
        if(pTrafficLog->m_timRealDepaTrain.isNull())
        {
            if(pTrafficLog->m_timToAdjtStaAgrDepaTrain.isNull())
            {
                pTrafficLog->m_strProc = PROCESS_FCYG;
                return;
            }

            if(pTrafficLog->m_btFCJLStatus != STATUS_JLBL_SUCC)
            {
                pTrafficLog->m_strProc = PROCESS_BLFL;
                return;
            }

            pTrafficLog->m_strProc = PROCESS_LCCF;
            return;
        }
    }
    pTrafficLog->m_strProc = PROCESS_FINISH;
    return;

}
