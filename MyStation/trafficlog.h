#ifndef TRAFFICLOG_H
#define TRAFFICLOG_H

#include <QString>
#include <QDateTime>

//行车日志
class TrafficLog
{
public:
    TrafficLog();

public:
    int log_id = 0;//行车日志id
    int station_id = 0;//车站id
    int m_nPlanNumber = 0;//计划号
    QString m_strStaName;//车站名
    int m_btBeginOrEndFlg = 0;//（本条进路所属的阶段计划类型）始发（终到）标志位--0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
    //到达信息
    QString m_strReachTrainNum;//到达车次
    QString m_strReachTrainNumOld;
    QString m_strRecvTrainTrack;//到达股道
    QDateTime m_timProvReachStation;//计划到站时间
    QDateTime m_timRealReachStation;//实际到站时间
    QString m_strXHD_JZk;//进站口信号机
    QDateTime m_timAgrFromAdjtStaDepaTrain;//同意邻站发车时间
    QDateTime m_timFromAdjtStaDepaTrain;//邻站发车时间
    //出发信息
    QString m_strDepartTrainNum;//出发车次
    QString m_strDepartTrainNumOld;
    QString m_strDepartTrainTrack;//出发股道
    QDateTime m_timProvDepaTrain;//计划出发时间
    QDateTime m_timRealDepaTrain;//实际出发时间
    QString m_strXHD_CZk;//出站口信号机
    QDateTime m_timToAdjtStaAgrDepaTrain;//邻站同意发车时间
    QDateTime m_timtoAdjtStation;//到达邻站时间
    bool m_bReachTrainNumSX; //到达车次是上行车次
    bool m_bDepartTrainNumSX;//出发车次是上行车次
    //其他信息
    QString m_strTypeFlag;//计划类型文字-始发、终到、通过
    QString m_strTrainNum;//车次
    QString m_strTrainTrack;//股道
    int m_bDeleteFlag = 0;//删除标志
    int m_bElectric = 0;//电力
    int m_nLevelCX = 0;//超限等级
    int m_nLHFlg = 0;//列车货车标识 1列车 0货车  默认1
    QString m_strLCLX;
    QString m_strYXLX;
    int m_nIndexLCLX;//列车类型序号（管内动车组、通勤列车等）
    int m_nIndexYXLX;//运行类型序号（动车组、快速旅客列车等）
    int bXianLuSuo = 0;//线路所
    int m_nExecuteFlag = 0;//计划执行完毕标志
    QString m_strFromAdjtStation;//来向
    QString m_strToAdjtStation;//去向
    QString m_strDDCX;//到达超限
    QString m_strCFCX;//出发超限
    QString m_strNotes;//记事
    int m_bBLKY = 0;//办理客运
    int m_bAllowGDNotMatch = 0;//允许股道与固定径路不同
    int m_bAllowCRKNotMatch = 0;//允许出入口与固定进路不同
    int m_bArmy = 0;//军运
    int m_bImportant = 0;//重点
    int m_nCheckState = 0;//计划检查校验状态  ?？？待确定
    int m_btJALStatus = 0;//交令状态（占线板）
    int m_btLJStatus = 0;//列检状态（占线板）
    int m_btSSStatus = 0;//上水状态（占线板）
    int m_btXWStatus = 0;//吸污状态（占线板）
    int m_btJPStatus = 0;//交票状态（占线板）
    int m_btCJStatus = 0;//乘降状态（占线板）
    int m_btZGStatus = 0;//摘挂状态（占线板）
    int m_btLWStatus = 0;//列尾状态（占线板）
    int m_btHJStatus = 0;//货检状态（占线板）
    int m_btHCStatus = 0;//换乘状态（占线板）
    int m_btZXStatus = 0;//装卸状态（占线板）
    int m_btJCStatus = 0;//机车状态（占线板）
    int m_btDKStatus = 0;//道口状态（占线板）
    int m_btCHStatus = 0;//车号状态（占线板）
    int m_btZKStatus = 0;//综控状态（占线板）
    int m_btZWStatus = 0;//站务状态（占线板）
    int m_btJCJLStatus = 0;//接车进路状态（占线板）
    int m_btFCJLStatus = 0;//发车进路状态（占线板）
    QString m_strTrainPosStatus;//列车位置信息（占线板）
    QString m_strTrainPosStatusOld;//上一次列车位置信息（占线板）
    QString m_strProc;//计划下一个流程
    int m_nCodeReachStaEquip = 0;//进站口信号设备号
    int m_nCodeDepartStaEquip = 0;//出站口信号设备号
    bool m_bModify = false;//是否修改了
    bool m_bDelete = false;//是否修改了

    QDateTime m_timChartReachStation;//图定到站时间
    QDateTime m_timChartDepaTrain;//图定出发时间
};

#endif // TRAFFICLOG_H
