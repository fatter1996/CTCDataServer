#ifndef STAGEPLAN_H
#define STAGEPLAN_H

#include <QString>
#include <QDateTime>

//阶段计划详情
class StagePlan
{
public:
    StagePlan();

public:
    int detail_id = 0;//阶段计划详情id
    int station_id = 0;//车站id
    int plan_id = 0;//批计划id
    int m_nPlanNumber = 0;//计划号
    QDateTime m_timJHRcv;//计划接收时间
    int m_btStagePlanKind = 0;//阶段计划类型（增加或删除）
    int m_btBeginOrEndFlg = 0;//始发（终到）标志位  0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
    QString m_strDispatchDesk;//调度台名称
    int m_nStateSignPlan = 0;//签收状态（大于0认为已签收，1签收为行车日志，2签收为进路序列）
    int m_nDeleteReal = 0;//删除计划（物理）

    QString m_strReachTrainNum;//列车接车车次
    QString m_strReachTrainNumOld;//列车接车车次
    QString m_strRecvTrainTrack;//接车股道
    int m_nRecvTrainTrack = 0;//接车股道设备号
    QDateTime m_timProvReachStation;//规定到站时间
    QString m_strXHD_JZk;//进站口信号机
    int m_nCodeReachStaEquip = 0;//进站口信号机设备号

    QString m_strDepartTrainNum;//列车发车车次
    QString m_strDepartTrainNumOld;//列车发车车次
    QString m_strDepartTrainTrack;//发车股道
    int m_nDepartTrainTrack = 0;//发车股道设备号
    QDateTime m_timProvDepaTrain;//规定出发时间
    QString m_strXHD_CZk;//出站口信号机
    int m_nCodeDepartStaEquip = 0;//出站口信号机设备号

    int m_bDeleteFlag = 0;//删除标志
    int m_bElectric = 0;//电力
    int m_nLevelCX = 0;//超限等级
    int m_nLHFlg = 0;//列车的客货标志
    int m_nIndexLCLX = 0;//列车类型序号（管内动车组、通勤列车等）
    QString m_strLCLX;
    int m_nIndexYXLX = 0;//运行类型序号（动车组、快速旅客列车等）
    QString m_strYXLX;
    int bXianLuSuo = 0;//线路所
    int m_bBLKY = 0;//办理客运
    int m_bAllowGDNotMatch = 0;//允许股道与固定径路不同
    int m_bAllowCRKNotMatch = 0;//允许出入口与固定进路不同
    int m_bArmy = 0;//军运
    int m_bImportant = 0;//重点

    int m_btLJStatus = 0;//列检状态（占线板）
    int m_btJALStatus = 0;//交令状态（占线板）
    int m_btJPStatus = 0;//交票状态（占线板）
    int m_btLWStatus = 0;//列尾状态（占线板）
    int m_btJCStatus = 0;//机车状态（占线板）
    int m_btHJStatus = 0;//货检状态（占线板）
    int m_btCJStatus = 0;//乘降状态（占线板）
    int m_btSSStatus = 0;//上水状态（占线板）
    int m_btZGStatus = 0;//摘挂状态（占线板）
    int m_btHCStatus = 0;//换乘状态（占线板）
    int m_btZXStatus = 0;//装卸状态（占线板）
    int m_btXWStatus = 0;//吸污状态（占线板）
    int m_btDKStatus = 0;//道口状态（占线板）
    int m_btCHStatus = 0;//车号状态（占线板）
    int m_btZWStatus = 0;//站务状态（占线板）
    int m_btZKStatus = 0;//综控状态（占线板）

    bool m_bModified = false;//修改过的计划，默认否
};

#endif // STAGEPLAN_H
