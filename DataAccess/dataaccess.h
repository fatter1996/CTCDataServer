#ifndef DATAACCESS_H
#define DATAACCESS_H

#include "MySQLCon.h"
//#include "MyStation/mystation.h"
#include "MyStation/stage.h"
#include "MyStation/stageplan.h"
#include "MyStation/trainrouteorder.h"
#include "MyStation/trafficlog.h"
#include "MyStation/sysuser.h"
#include "MyStation/dispatchorderstation.h"
#include "MyStation/dispatchorderdispatcher.h"
#include "MyStation/dispatchorderlocomotive.h"
#include "BaseDataPro/gd.h"

class MyStation;
//数据库访问接口
class DataAccess
{
public:
    DataAccess();
    ~DataAccess();

    //友元类，可访问本类的所有方法和变量
    friend class MyStation;

public:
    CMySQLCon *m_mySql;
    //连接和打开数据库
    bool openDataBase(QString ipadd, int port, QString dbName, QString user, QString pwd);
    //数据库是否打开
    bool isDataBaseOpen();

public:
    //查找车站的个数
    int SelectStationCount();

public:
    //根据登录名获取用户信息
    SysUser getSysUser(QString _loginName);


public:
    //获取车站进路权限
    int SelectStationRoutePermit(MyStation* pMyStation);
    //更新车站进路权限等信息
    int UpdateStationInfo(MyStation* pMyStation);
    //获取车站状态信息
    int SelectStationInfo(MyStation* pMyStation);

public:
    //插入批计划并返回该批计划的id（父级计划）
    int InsetStage(Stage* _stage);
    //查找计划id
    int SelectPlanId(Stage* _stage);

    //读取该站所有的阶段计划详情
    void SelectAllStagePlanDetail(MyStation* pMyStation);
    //插入阶段计划详情并返回该计划的id
    int InsetStagePlanDetail(StagePlan* _stagePlan);
    //更新阶段计划详情并返回更新成功标志
    int UpdateStagePlanDetail(StagePlan* _stagePlan);

    //读取该站所有的行车日志
    void SelectAllTrafficLog(MyStation* pMyStation);//int _staId, CObArray *_objArr
    //插入行车日志并返回该日志的id
    int InsetTrafficLog(TrafficLog* _trafficLog);
    //更新行车日志并返回更新成功标志
    int UpdateTrafficLog(TrafficLog* _trafficLog);
    //更新行车日志股道信息并返回更新成功标志
    int UpdateTrafficLogTrack(TrafficLog* _trafficLog);
    //更新行车日志其他信息并返回更新成功标志
    int UpdateTrafficLogOtherInfo(TrafficLog* _trafficLog);
    //删除行车日志并返回删除成功标志
    int DeleteTrafficLog(TrafficLog* _trafficLog);

    //读取该站所有的进路序列
    void SelectAllRouteOrder(MyStation* pMyStation);
    //插入进路序列并返回该序列的id
    int InsetRouteOrder(TrainRouteOrder* _pRouteOrder);
    //更新进路序列并返回更新成功标志
    int UpdateRouteOrder(TrainRouteOrder* _pRouteOrder);
    //删除进路序列并返回删除成功标志
    int DeleteRouteOrder(TrainRouteOrder* _pRouteOrder);//int _routeid
    //删除本站所有进路序列并返回删除成功标志
    int DeleteStationRouteOrder(int statId);

public:
    //重置阶段计划表(截断表 清空表数据)
    bool ResetTableStagePlan();
    //重置本站阶段计划表(截断表 清空表数据)
    bool ResetStationStagePlan(MyStation* pMyStation);
    //重置行车日志表
    bool ResetTableTrafficLog();
    //重置本站行车日志表(截断表 清空表数据)
    bool ResetStationTrafficLog(MyStation* pMyStation);
    //重置进路序列表
    bool ResetTableRouteOrder();
    //重置本站进路序列表
    bool ResetStationRouteOrder(MyStation* pMyStation);

    //重置所有表
    bool ResetAllTable();
    //重置本站数据库及表
    bool ResetStationTable(MyStation* pMyStation);

public:
    //读取该站所有的接收调度命令
    void SelectAllDisOrderRecv(MyStation* pMyStation);
    //插入接收的调度命令返回该内容的id
    int InsertDisOrderRecv(DispatchOrderStation* _pDisOrderRecv);
    //更新接收的调度命令返回更新成功标志
    int UpdateDisOrderRecv(DispatchOrderStation* _pDisOrderRecv);
    //重置调度命令表
    bool ResetTableDisOrderRecv();
    //重置本站调度命令表
    bool ResetStationDisOrderRecv(MyStation* pMyStation);

    //读取该站所有的请求/调度台调度命令
    void SelectAllDisOrderDisp(MyStation* pMyStation);
    //插入请求/调度台的调度命令返回该内容的id
    int InsertDisOrderDisp(DispatchOrderDispatcher* _pDisOrderDDT);
    //更新请求/调度台的调度命令返回更新成功标志
    int UpdateDisOrderDisp(DispatchOrderDispatcher* _pDisOrderDDT);
    //删除调度台的调度命令，返回删除成功标志
    int DeleteDisOrderDisp(DispatchOrderDispatcher* _pDisOrderDDT);
    //重置请求/调度台调度命令表
    bool ResetTableDisOrderDisp();
    //重置本站请求/调度台调度命令表
    bool ResetStationDisOrderDisp(MyStation* pMyStation);

    //读取该站所有的机车调度命令
    void SelectAllDisOrderLocom(MyStation* pMyStation);
    //插入机车的调度命令返回该内容的id
    int InsertDisOrderLocom(DispatchOrderLocomotive* _pDisOrderJC);
    //更新机车的调度命令返回更新成功标志
    int UpdateDisOrderLocom(DispatchOrderLocomotive* _pDisOrderJC);
    //删除机车的调度命令，返回删除成功标志
    int DeleteDisOrderLocom(DispatchOrderLocomotive* _pDisOrderJC);
    //重置机车调度命令表
    bool ResetTableDisOrderLocom();
    //重置本站机车调度命令表
    bool ResetStationDisOrderLocom(MyStation* pMyStation);

public:
    //读取该站所有的股道防溜
    void SelectAllGDAntiSlip(MyStation* pMyStation);
    //更新股道防溜返回更新成功标志
    int UpdateGDAntiSlip(int _staId, CGD* _pGD);
    //清除股道表中所有股道的防溜信息
    bool ClearTableGDAntiSlip();
    //清除股道表中本站所有股道的防溜信息
    bool ClearStationGDAntiSlip(MyStation* pMyStation);


public:
    //========== 防错办基础数据读取接口 ==========
    //读取该站所有的股道基础属性
    void SelectAllGDAttribute(MyStation* pMyStation);
    //读取该站所有的出入口基础属性
    void SelectAllGatewayAttribute(MyStation* pMyStation);
    //读取该站所有的列车固定径路基础属性
    void SelectAllFixedRoute(MyStation* pMyStation);
    //读取该站所有的车次股道列表信息
    void SelectAllTrainNumTrack(MyStation* pMyStation);

public:
    //检查表是否是空表
    bool CheckTableIsNull(QString strTableName);

};

#endif // DATAACCESS_H
