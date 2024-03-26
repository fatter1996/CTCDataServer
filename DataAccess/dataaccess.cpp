#include "dataaccess.h"
#include <QDebug>
#include "MyStation/mystation.h"
#include "Log/log.h"

DataAccess::DataAccess()
{
    m_mySql = new CMySQLCon;
}
DataAccess::~DataAccess()
{
    m_mySql->close();
}

bool DataAccess::openDataBase(QString ipadd, int port, QString dbName, QString user, QString pwd)
{
    m_mySql->setDabaBaseInfo(ipadd, port, dbName, user, pwd);
    if(m_mySql->open())
    {
        return true;
    }
    else
    {
        QLOG_ERROR()<< "openDataBase() failed!" ;
        return false;
    }
}
//数据库是否打开，没打开则重新打开
bool DataAccess::isDataBaseOpen()
{
    if(m_mySql->isOpen())
    {
        return true;
    }
    else
    {
        //return m_mySql->open();
        bool bOpen = m_mySql->open();
        if(bOpen)
        {
            qDebug()<< "Database reopen succeed!";
        }
        else
        {
            QLOG_ERROR()<< "Database reopen failed!";
        }
        return bOpen;
    }
}
//查找车站的个数
int DataAccess::SelectStationCount()
{
    QString strSql = QString("SELECT COUNT(station_id) FROM station_list");
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return 0;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectStationCount() failed!" << m_mySql->query->lastError();
        return -1;
    }
    while (m_mySql->next())
    {
        return m_mySql->value(0).toInt();
    }
    return 0;
}
//根据登录名获取用户信息
SysUser DataAccess::getSysUser(QString _loginName)
{
    SysUser sysuser;
    QString strSql = QString("SELECT user_id, user_name, login_name, password, salt FROM sys_user WHERE login_name='%1'")
            .arg(_loginName);
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return sysuser;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "getSysUser() failed!" << m_mySql->query->lastError();
        return sysuser;
    }
    while (m_mySql->next())
    {
        SysUser sysuser;
        sysuser.userId = m_mySql->value(0).toInt();
        sysuser.userName = m_mySql->value(1).toString();
        sysuser.loginName = m_mySql->value(2).toString();
        sysuser.password = m_mySql->value(3).toString();
        sysuser.salt = m_mySql->value(4).toString();
        return sysuser;
    }
    return sysuser;
}

//查找车站进路权限
int DataAccess::SelectStationRoutePermit(MyStation *pMyStation)
{
    QString strSql = QString("SELECT route_permit FROM station_list WHERE station_id = %1")
            .arg(pMyStation->getStationID());
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectStationInfoByStaId() failed!" << m_mySql->query->lastError();
        return -1;
    }
    while (m_mySql->next())
    {
        pMyStation->RoutePermit = m_mySql->value(0).toInt();
        return 1;
    }
    return -1;
}
//更新车站进路权限等信息
int DataAccess::UpdateStationInfo(MyStation *pMyStation)
{
    QString strSql = QString("UPDATE station_list SET route_permit = %1, \
                     auto_sign_stage = %2, fczk_mode = %3, fszl_mode = %4 \
                     ,plan_mode = %5 ,plan_ctrl = %6 \
                     WHERE station_id = %7")
            .arg(pMyStation->RoutePermit)
            .arg(pMyStation->AutoSignStage==true?1:0)
            .arg(pMyStation->m_nFCZKMode)
            .arg(pMyStation->ModalSelect.nModeState)
            .arg(pMyStation->ModalSelect.nStateSelect)
            .arg(pMyStation->ModalSelect.nPlanCtrl==true?1:0)
            .arg(pMyStation->getStationID());
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateStationInfo() failed!" << m_mySql->query->lastError();
        return -1;
    }
}
//获取车站状态信息
int DataAccess::SelectStationInfo(MyStation *pMyStation)
{
    QString strSql = QString("SELECT route_permit,auto_sign_stage, fczk_mode, fszl_mode \
                     ,plan_mode,plan_ctrl FROM station_list WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectStationInfo() failed!";
        return -1;
    }
    while (m_mySql->next())
    {
        pMyStation->RoutePermit = m_mySql->value(0).toInt();
        pMyStation->AutoSignStage = m_mySql->value(1).toInt()==1?true:false;
        pMyStation->m_nFCZKMode = m_mySql->value(2).toInt()==1?true:false;
        pMyStation->ModalSelect.nModeState = m_mySql->value(3).toInt();
        pMyStation->ModalSelect.nStateSelect = m_mySql->value(4).toInt();
        pMyStation->ModalSelect.nPlanCtrl = m_mySql->value(5).toInt()==1?true:false;
        return 1;
    }
    return -1;
}

//插入阶段计划并返回该计划的id（父级计划，类似一批计划的批号）
int DataAccess::InsetStage(Stage* _stage) //CStagePlan* _stagePlan
{
    QString strSql = QString("INSERT INTO plan_stageplan(station_id, plan_num, plan_timerecv, dispatch, dispatcher) VALUES (%1, %2, '%3', '%4', '%5')")
            .arg(_stage->station_id)
            .arg(_stage->m_nPlanNum)
            .arg(_stage->m_timRecv.toString().size()?_stage->m_timRecv.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stage->m_strDispatch)
            .arg(_stage->m_strDispatcher);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        strSql = "SELECT MAX(plan_id) FROM plan_stageplan;";
        //qDebug() <<strSql;
        if (!m_mySql->exec(strSql))
        {
            QLOG_ERROR()<< "InsetStage()-->SELECT exec failed!" << m_mySql->query->lastError();
            return -1;
        }

        while (m_mySql->next())
        {
            //发送更新数据消息
            //pObjDoc->GetMyStationByStaIDInStaArray(_stage->station_id)->SendUpdateDataMsg(UPDATETYPE_JDJH);
            return m_mySql->value(0).toInt();
        }
    }
    else
    {
        QLOG_ERROR()<< "InsetStage() failed!";
    }
    return -1;
}
//查找计划id
int DataAccess::SelectPlanId(Stage* _stage)
{
    QString strSql = QString("SELECT plan_id, plan_num, plan_timerecv, dispatch, dispatcher FROM plan_stageplan WHERE plan_num = %1")
            .arg(_stage->m_nPlanNum);
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectPlanId() failed!" << m_mySql->query->lastError();
        return -1;
    }
    while (m_mySql->next())
    {
        QDateTime timeNow = QDateTime::currentDateTime();
        if (m_mySql->value(2).toString() != "")
        {
            QDateTime tm = QDateTime::fromString(m_mySql->value(2).toString(), "yyyy-MM-dd hh:mm:ss");
            //1秒内的计划认为是同一批计划
            if (timeNow.toTime_t() - tm.toTime_t() <= 1)
            {
                return m_mySql->value(0).toInt();
            }
        }
        return m_mySql->value(0).toInt();
    }
    return -1;
}

//读取该站所有的阶段计划详情
void DataAccess::SelectAllStagePlanDetail(MyStation* pMyStation)
{
    QString strSql = QString("SELECT detail_id, plan_id, plan_num, plan_timerecv, plan_kind, plan_jfctype,\
                             dispatch, signstate, reach_trainnum, reach_trainnumold, reach_track, reach_timeplan, reach_timereal, \
                             reach_xhd, depart_trainnum, depart_trainnumold, depart_track, depart_timeplan, depart_timereal,\
                             depart_xhd, isdelete, iselectric, ultralimitlevel, kehuoflag, traintype, runningtype, isxianlusuo \
                             FROM plan_stageplan_detail WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllStagePlanDetail() failed!" << m_mySql->query->lastError();
        return;
    }
    pMyStation->m_ArrayStagePlan.clear();
    while (m_mySql->next())
    {
        StagePlan* stagePlan = new StagePlan();
        stagePlan->station_id = pMyStation->getStationID();
        stagePlan->detail_id = m_mySql->value(0).toInt();
        stagePlan->plan_id = m_mySql->value(1).toInt();
        stagePlan->m_nPlanNumber = m_mySql->value(2).toInt();
        stagePlan->m_timJHRcv = m_mySql->value(3).toDateTime();
        stagePlan->m_btStagePlanKind = m_mySql->value(4).toInt();
        stagePlan->m_btBeginOrEndFlg = m_mySql->value(5).toInt();
        stagePlan->m_strDispatchDesk = m_mySql->value(6).toString();
        stagePlan->m_nStateSignPlan = m_mySql->value(7).toInt();

        stagePlan->m_strReachTrainNum = m_mySql->value(8).toString();
        stagePlan->m_strReachTrainNumOld = m_mySql->value(9).toString();
        stagePlan->m_strRecvTrainTrack = m_mySql->value(10).toString();
        stagePlan->m_nRecvTrainTrack = pMyStation->GetCodeByStrName(stagePlan->m_strRecvTrainTrack);
        stagePlan->m_timProvReachStation = m_mySql->value(11).toDateTime();
//      _pTrainRouteOrder.m_timProvReachStation = m_mySql->value(12).toDateTime();
        stagePlan->m_strXHD_JZk = m_mySql->value(13).toString();
        stagePlan->m_nCodeReachStaEquip = pMyStation->GetCodeByStrName(stagePlan->m_strXHD_JZk);

        stagePlan->m_strDepartTrainNum = m_mySql->value(14).toString();
        stagePlan->m_strDepartTrainNumOld = m_mySql->value(15).toString();
        stagePlan->m_strDepartTrainTrack = m_mySql->value(16).toString();
        stagePlan->m_nDepartTrainTrack = pMyStation->GetCodeByStrName(stagePlan->m_strDepartTrainTrack);
        stagePlan->m_timProvDepaTrain = m_mySql->value(17).toDateTime();
//      _pTrainRouteOrder.m_timProvDepaTrain = m_mySql->value(18).toDateTime();
        stagePlan->m_strXHD_CZk = m_mySql->value(19).toString();
        stagePlan->m_nCodeDepartStaEquip = pMyStation->GetCodeByStrName(stagePlan->m_strXHD_CZk);

        stagePlan->m_bDeleteFlag = m_mySql->value(20).toInt();
        stagePlan->m_bElectric = m_mySql->value(21).toInt();
        stagePlan->m_nLevelCX = m_mySql->value(22).toInt();
        stagePlan->m_nLHFlg = m_mySql->value(23).toInt();
        stagePlan->m_strLCLX = m_mySql->value(24).toString();
        stagePlan->m_strYXLX = m_mySql->value(25).toString();
        stagePlan->bXianLuSuo = m_mySql->value(26).toInt();

        pMyStation->m_ArrayStagePlan.append(stagePlan);
    }
}

//插入阶段计划并返回该计划的id
int DataAccess::InsetStagePlanDetail(StagePlan* _stagePlan)
{
    QString strSql = QString("INSERT INTO plan_stageplan_detail(station_id, plan_id, plan_num, plan_timerecv, plan_kind, plan_jfctype,\
                     dispatch, signstate, reach_trainnum, reach_trainnumold, reach_track, reach_timeplan, reach_timereal, \
                     reach_xhd, depart_trainnum, depart_trainnumold, depart_track, depart_timeplan, depart_timereal, \
                     depart_xhd, isdelete, iselectric, ultralimitlevel, kehuoflag, traintype, runningtype, isxianlusuo)\
                     VALUES (%1, %2, %3, '%4', %5, %6, '%7', %8, '%9', '%10', '%11', '%12', '%13', \
                     '%14', '%15', '%16', '%17', '%18', '%19', '%20', %21, %22, %23, %24, '%25', '%26',  %27);")
            .arg(_stagePlan->station_id)
            .arg(_stagePlan->plan_id)
            .arg(_stagePlan->m_nPlanNumber)
            //.arg(_stagePlan->m_timJHRcv.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_stagePlan->m_timJHRcv.toString().size()?_stagePlan->m_timJHRcv.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stagePlan->m_btStagePlanKind)
            .arg(_stagePlan->m_btBeginOrEndFlg)
            .arg(_stagePlan->m_strDispatchDesk)
            .arg(_stagePlan->m_nStateSignPlan)
            .arg(_stagePlan->m_strReachTrainNum)
            .arg(_stagePlan->m_strReachTrainNumOld)
            .arg(_stagePlan->m_strRecvTrainTrack)
            //.arg(_stagePlan->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_stagePlan->m_timProvReachStation.toString().size()?_stagePlan->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            //.arg(_stagePlan->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_stagePlan->m_timProvReachStation.toString().size()?_stagePlan->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stagePlan->m_strXHD_JZk)
            .arg(_stagePlan->m_strDepartTrainNum)
            .arg(_stagePlan->m_strDepartTrainNumOld)
            .arg(_stagePlan->m_strDepartTrainTrack)
            //.arg(_stagePlan->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_stagePlan->m_timProvDepaTrain.toString().size()?_stagePlan->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            //.arg(_stagePlan->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_stagePlan->m_timProvDepaTrain.toString().size()?_stagePlan->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stagePlan->m_strXHD_CZk)
            .arg(_stagePlan->m_bDeleteFlag)
            .arg(_stagePlan->m_bElectric)
            .arg(_stagePlan->m_nLevelCX)
            .arg(_stagePlan->m_nLHFlg)
            .arg(_stagePlan->m_strLCLX)
            .arg(_stagePlan->m_strYXLX)
            .arg(_stagePlan->bXianLuSuo);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        strSql = "SELECT MAX(detail_id) FROM plan_stageplan_detail;";
        //qDebug() <<strSql;
        if (!m_mySql->exec(strSql))
            return -1;

        while (m_mySql->next())
        {
            //发送更新数据消息
            //pObjDoc->GetMyStationByStaIDInStaArray(_stagePlan->station_id)->SendUpdateDataMsg(UPDATETYPE_JDJH);
            _stagePlan->detail_id = m_mySql->value(0).toInt();
            return _stagePlan->detail_id;
        }
    }
    else
    {
        QLOG_ERROR()<< "InsetStagePlanDetail() failed!" << m_mySql->query->lastError();
    }
    return -1;
}

//更新阶段计划详情并返回更新成功标志
int DataAccess::UpdateStagePlanDetail(StagePlan* _stagePlan)
{
//    QString strSql = QString("UPDATE plan_stageplan_detail SET signstate = %1 WHERE detail_id = %2")
//            .arg(_stagePlan->m_nStateSignPlan).arg(_stagePlan->detail_id);
    QString strSql = QString("UPDATE plan_stageplan_detail SET \
             plan_timerecv='%1', plan_kind=%2, plan_jfctype=%3, dispatch='%4', \
             reach_trainnum='%5', reach_trainnumold='%6', reach_track='%7', \
             reach_timeplan='%8', reach_timereal='%9', reach_xhd='%10', \
             depart_trainnum='%11', depart_trainnumold='%12', depart_track='%13', \
             depart_timeplan='%14', depart_timereal='%15', depart_xhd='%16', \
             isdelete=%17, iselectric=%18, ultralimitlevel=%19, kehuoflag=%20, \
             traintype='%21', runningtype='%22', isxianlusuo=%23, plan_num=%24, \
             signstate=%25 \
             WHERE detail_id = %26")
            .arg(_stagePlan->m_timJHRcv.toString().size()?_stagePlan->m_timJHRcv.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stagePlan->m_btStagePlanKind)
            .arg(_stagePlan->m_btBeginOrEndFlg)
            .arg(_stagePlan->m_strDispatchDesk)
            .arg(_stagePlan->m_strReachTrainNum)
            .arg(_stagePlan->m_strReachTrainNumOld)
            .arg(_stagePlan->m_strRecvTrainTrack)
            .arg(_stagePlan->m_timProvReachStation.toString().size()?_stagePlan->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stagePlan->m_timProvReachStation.toString().size()?_stagePlan->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stagePlan->m_strXHD_JZk)
            .arg(_stagePlan->m_strDepartTrainNum)
            .arg(_stagePlan->m_strDepartTrainNumOld)
            .arg(_stagePlan->m_strDepartTrainTrack)
            .arg(_stagePlan->m_timProvDepaTrain.toString().size()?_stagePlan->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stagePlan->m_timProvDepaTrain.toString().size()?_stagePlan->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_stagePlan->m_strXHD_CZk)
            .arg(_stagePlan->m_bDeleteFlag)
            .arg(_stagePlan->m_bElectric)
            .arg(_stagePlan->m_nLevelCX)
            .arg(_stagePlan->m_nLHFlg)
            .arg(_stagePlan->m_strLCLX)
            .arg(_stagePlan->m_strYXLX)
            .arg(_stagePlan->bXianLuSuo)
            .arg(_stagePlan->m_nPlanNumber)
            .arg(_stagePlan->m_nStateSignPlan)
            .arg(_stagePlan->detail_id);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateStagePlanDetail() failed!" << m_mySql->query->lastError();
    }
    return -1;
}

//读取该站所有的行车日志
void DataAccess::SelectAllTrafficLog(MyStation* pMyStation)//int _staId, CObArray *_objArr
{
    QString strSql = QString("SELECT log_id, station_id, plan_num, staName, plan_jfctype, reach_trainnum, \
                     reach_trainnumold, reach_track, reach_timeplan, reach_timereal, reach_xhd, depart_trainnum, \
                     depart_trainnumold, depart_track, depart_timeplan, depart_timereal, depart_xhd, isdelete, \
                     iselectric, ultralimit_level, kehuoflag, traintype, runningtype, isxianlusuo, executeflag, \
                     adjtstationfrom, adjtstationto, agradjtdepat_time, adjtdepat_time, adjtagrdepat_time, reachadjt_time, \
                     reach_ultralimitlevel, depart_ultralimitlevel, notes, transportpassenger, allowtracknotmatch, \
                     allowEntrnotmatch, isArmy, isImportant, plan_checkState, flowstatus_jiaoling, flowstatus_liejian, \
                     flowstatus_shangshui, flowstatus_xiwu, flowstatus_jiaopiao, flowstatus_chengjiang, flowstatus_zhaigua, \
                     flowstatus_liewei, flowstatus_huojian, flowstatus_huancheng, flowstatus_zhuangxie, flowstatus_jiche, \
                     flowstatus_daokou, flowstatus_chehao, flowstatus_zongkong, flowstatus_zhanwu, train_posistatus, \
                     train_nextproc FROM plan_trafficlog WHERE station_id = %1").arg(pMyStation->getStationID());
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllTrafficLog() failed!" << m_mySql->query->lastError();
        return;
    }
    pMyStation->m_ArrayTrafficLog.clear();
    while (m_mySql->next())
    {
        TrafficLog* _pTrafficLog = new TrafficLog();
        _pTrafficLog->log_id = m_mySql->value(0).toInt();
        _pTrafficLog->station_id = pMyStation->getStationID();//_staId;
        _pTrafficLog->m_nPlanNumber = m_mySql->value(2).toInt();
        _pTrafficLog->m_strStaName = m_mySql->value(3).toString();
        _pTrafficLog->m_btBeginOrEndFlg = m_mySql->value(4).toInt();
        //添加到达信息
        if (_pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_SF)
        {
            _pTrafficLog->m_strReachTrainNum = m_mySql->value(5).toString();
            _pTrafficLog->m_strReachTrainNumOld = m_mySql->value(6).toString();
            _pTrafficLog->m_strRecvTrainTrack = m_mySql->value(7).toString();
            _pTrafficLog->m_timProvReachStation = m_mySql->value(8).toDateTime();
            _pTrafficLog->m_timChartReachStation = _pTrafficLog->m_timProvReachStation;
            _pTrafficLog->m_timRealReachStation = m_mySql->value(9).toDateTime();
            _pTrafficLog->m_strXHD_JZk = m_mySql->value(10).toString();
            _pTrafficLog->m_timAgrFromAdjtStaDepaTrain = m_mySql->value(27).toDateTime();
            _pTrafficLog->m_timFromAdjtStaDepaTrain = m_mySql->value(28).toDateTime();
        }
        //添加出发信息
        if (_pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_ZD)
        {
            _pTrafficLog->m_strDepartTrainNum = m_mySql->value(11).toString();
            _pTrafficLog->m_strDepartTrainNumOld = m_mySql->value(12).toString();
            _pTrafficLog->m_strDepartTrainTrack = m_mySql->value(13).toString();
            _pTrafficLog->m_timProvDepaTrain = m_mySql->value(14).toDateTime();
            _pTrafficLog->m_timChartDepaTrain = _pTrafficLog->m_timProvDepaTrain;
            _pTrafficLog->m_timRealDepaTrain = m_mySql->value(15).toDateTime();
            _pTrafficLog->m_strXHD_CZk = m_mySql->value(16).toString();
            _pTrafficLog->m_timToAdjtStaAgrDepaTrain = m_mySql->value(29).toDateTime();
            _pTrafficLog->m_timtoAdjtStation = m_mySql->value(30).toDateTime();
        }
        //其他信息
        if (_pTrafficLog->m_btBeginOrEndFlg == JFC_TYPE_SF)//0xBB
        {
            _pTrafficLog->m_strTypeFlag = "始发";
        }
        else if (_pTrafficLog->m_btBeginOrEndFlg == JFC_TYPE_ZD)//0xCC
        {
            _pTrafficLog->m_strTypeFlag = "终到";
        }
        else if (_pTrafficLog->m_btBeginOrEndFlg == JFC_TYPE_TG)//0xDD
        {
            _pTrafficLog->m_strTypeFlag = "通过";
        }
        else
        {
            _pTrafficLog->m_strTypeFlag = "接发";
        }

        _pTrafficLog->m_strTrainNum = _pTrafficLog->m_strReachTrainNum != "" ? _pTrafficLog->m_strReachTrainNum : _pTrafficLog->m_strDepartTrainNum;
        _pTrafficLog->m_strTrainTrack = _pTrafficLog->m_strRecvTrainTrack != "" ? _pTrafficLog->m_strRecvTrainTrack : _pTrafficLog->m_strDepartTrainTrack;//new
        _pTrafficLog->m_bDeleteFlag = m_mySql->value(17).toInt();
        _pTrafficLog->m_bElectric = m_mySql->value(18).toInt();
        _pTrafficLog->m_nLevelCX = m_mySql->value(19).toInt();
        _pTrafficLog->m_nLHFlg = m_mySql->value(20).toInt();
        _pTrafficLog->m_strLCLX = m_mySql->value(21).toString();
        _pTrafficLog->m_strYXLX = m_mySql->value(22).toString();
        _pTrafficLog->m_nIndexLCLX = pMyStation->GetTrainTypeIndex(_pTrafficLog->m_strLCLX);//列车类型序号（管内动车组、通勤列车等）
        _pTrafficLog->m_nIndexYXLX = pMyStation->GetTrainRunTypeIndex(_pTrafficLog->m_strYXLX);//运行类型序号（动车组、快速旅客列车等）
        _pTrafficLog->bXianLuSuo = m_mySql->value(23).toInt();
        if (_pTrafficLog->bXianLuSuo)
        {
            _pTrafficLog->m_strTypeFlag = "通过";
        }
        _pTrafficLog->m_nExecuteFlag = m_mySql->value(24).toInt();
        _pTrafficLog->m_strFromAdjtStation = m_mySql->value(25).toString();
        _pTrafficLog->m_strToAdjtStation = m_mySql->value(26).toString();
        _pTrafficLog->m_strDDCX = pMyStation->GetChaoXianLevel(m_mySql->value(31).toInt());
        _pTrafficLog->m_strCFCX = pMyStation->GetChaoXianLevel(m_mySql->value(32).toInt());
        _pTrafficLog->m_strNotes = m_mySql->value(33).toString();
        _pTrafficLog->m_bBLKY = m_mySql->value(34).toInt();
        _pTrafficLog->m_bAllowGDNotMatch = m_mySql->value(35).toInt();
        _pTrafficLog->m_bAllowCRKNotMatch = m_mySql->value(36).toInt();
        _pTrafficLog->m_bArmy = m_mySql->value(37).toInt();
        _pTrafficLog->m_bImportant = m_mySql->value(38).toInt();
        _pTrafficLog->m_nCheckState = m_mySql->value(39).toInt();
        _pTrafficLog->m_btJALStatus = m_mySql->value(40).toInt();
        _pTrafficLog->m_btLJStatus = m_mySql->value(41).toInt();
        _pTrafficLog->m_btSSStatus = m_mySql->value(42).toInt();
        _pTrafficLog->m_btXWStatus = m_mySql->value(43).toInt();
        _pTrafficLog->m_btJPStatus = m_mySql->value(44).toInt();
        _pTrafficLog->m_btCJStatus = m_mySql->value(45).toInt();
        _pTrafficLog->m_btZGStatus = m_mySql->value(46).toInt();
        _pTrafficLog->m_btLWStatus = m_mySql->value(47).toInt();
        _pTrafficLog->m_btHJStatus = m_mySql->value(48).toInt();
        _pTrafficLog->m_btHCStatus = m_mySql->value(49).toInt();
        _pTrafficLog->m_btZXStatus = m_mySql->value(50).toInt();
        _pTrafficLog->m_btJCStatus = m_mySql->value(51).toInt();
        _pTrafficLog->m_btDKStatus = m_mySql->value(52).toInt();
        _pTrafficLog->m_btCHStatus = m_mySql->value(53).toInt();
        _pTrafficLog->m_btZKStatus = m_mySql->value(54).toInt();
        _pTrafficLog->m_btZWStatus = m_mySql->value(55).toInt();
        _pTrafficLog->m_strTrainPosStatus = m_mySql->value(56).toString();
        _pTrafficLog->m_strTrainPosStatusOld = _pTrafficLog->m_strTrainPosStatus;
        _pTrafficLog->m_strProc = m_mySql->value(57).toString();
        _pTrafficLog->m_nCodeReachStaEquip = pMyStation->GetCodeByStrName(_pTrafficLog->m_strXHD_JZk);
        _pTrafficLog->m_nCodeDepartStaEquip = pMyStation->GetCodeByStrName(_pTrafficLog->m_strXHD_CZk);
        _pTrafficLog->m_bReachTrainNumSX = pMyStation->GetSXByCode(_pTrafficLog->m_nCodeReachStaEquip, 0);//NEW
        _pTrafficLog->m_bDepartTrainNumSX = pMyStation->GetSXByCode(_pTrafficLog->m_nCodeDepartStaEquip, 1);//NEW
        pMyStation->m_ArrayTrafficLog.append(_pTrafficLog);
    }
}

//插入行车日志并返回该日志的id
int DataAccess::InsetTrafficLog(TrafficLog* _trafficLog)
{
    QString strSql = QString("INSERT INTO plan_trafficlog(station_id, plan_num, staName, plan_jfctype, reach_trainnum, reach_trainnumold, \
                     reach_track, reach_timeplan, reach_xhd, depart_trainnum, depart_trainnumold, depart_track, depart_timeplan, depart_xhd, \
                     iselectric, ultralimit_level, kehuoflag, traintype, runningtype, executeflag, reach_ultralimitlevel, depart_ultralimitlevel, \
                     reach_direction, depart_direction, adjtstationfrom, adjtstationto, train_posistatus, train_nextproc,transportpassenger, \
                     allowtracknotmatch, allowEntrnotmatch, isArmy, isImportant, isxianlusuo) VALUES \
                     (%1, %2, '%3', %4, '%5', '%6', '%7', '%8', '%9', '%10', '%11', '%12', '%13', '%14', %15, %16, %17, \
                     '%18', '%19', 0, %20, %21, '%22', '%23', '%24', '%25', '%26', '%27', %28, %29, %30, %31, %32, %33);")
            .arg(_trafficLog->station_id)
            .arg(_trafficLog->m_nPlanNumber)
            .arg(_trafficLog->m_strStaName)
            .arg(_trafficLog->m_btBeginOrEndFlg)
            .arg(_trafficLog->m_strReachTrainNum)
            .arg(_trafficLog->m_strReachTrainNumOld)
            .arg(_trafficLog->m_strRecvTrainTrack)
            //.arg(_trafficLog->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"))
            //.arg(DataTimeToMySQLString(_trafficLog->m_timProvReachStation)) //替换上一行代码
            .arg(_trafficLog->m_timProvReachStation.toString().size()?_trafficLog->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_trafficLog->m_strXHD_JZk)
            .arg(_trafficLog->m_strDepartTrainNum)
            .arg(_trafficLog->m_strDepartTrainNumOld)
            .arg(_trafficLog->m_strDepartTrainTrack)
            //.arg(_trafficLog->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"))
            //.arg(DataTimeToMySQLString(_trafficLog->m_timProvDepaTrain)) //替换上一行代码
            .arg(_trafficLog->m_timProvDepaTrain.toString().size()?_trafficLog->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_trafficLog->m_strXHD_CZk)
            .arg(_trafficLog->m_bElectric)
            .arg(_trafficLog->m_nLevelCX)
            .arg(_trafficLog->m_nLHFlg)
            .arg(_trafficLog->m_strLCLX)
            .arg(_trafficLog->m_strYXLX)
            .arg(_trafficLog->m_nLevelCX) //到达超限
            .arg(_trafficLog->m_nLevelCX) //出发超限
            .arg(_trafficLog->m_strFromAdjtStation)
            .arg(_trafficLog->m_strToAdjtStation)
            .arg(_trafficLog->m_strFromAdjtStation)
            .arg(_trafficLog->m_strToAdjtStation)
            .arg(_trafficLog->m_strTrainPosStatus)
            .arg(_trafficLog->m_strProc)
            .arg(_trafficLog->m_bBLKY)
            .arg(_trafficLog->m_bAllowGDNotMatch)
            .arg(_trafficLog->m_bAllowCRKNotMatch)
            .arg(_trafficLog->m_bArmy)
            .arg(_trafficLog->m_bImportant)
            .arg(_trafficLog->bXianLuSuo);

    strSql.replace("'NULL'","NULL");//空时间处理
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        strSql = "SELECT MAX(log_id) FROM plan_trafficlog";
        //qDebug() <<strSql;
        if (!m_mySql->exec(strSql))
            return -1;
        while (m_mySql->next())
        {
            _trafficLog->log_id = m_mySql->value(0).toInt();
            //发送更新数据消息
            //UPDATETYPE_XCRZ
            return _trafficLog->log_id;
        }
    }
    else
    {
        QLOG_ERROR()<< "InsetTrafficLog() failed!" << m_mySql->query->lastError();
    }
    return -1;
}

//更新行车日志并返回更新成功标志
int DataAccess::UpdateTrafficLog(TrafficLog* _trafficLog)
{
     QString strSql = QString("UPDATE plan_trafficlog SET plan_jfctype = %1, reach_trainnum = '%2', reach_trainnumold = '%3', \
                     reach_track = '%4', reach_timeplan = '%5', reach_xhd = '%6', depart_trainnum = '%7', depart_trainnumold = '%8', \
                     depart_track = '%9', depart_timeplan = '%10', depart_xhd = '%11', isdelete = %12, iselectric = %13, executeflag = %14, \
                     reach_timereal = '%15', depart_timereal = '%16', agradjtdepat_time = '%17', adjtdepat_time = '%18', adjtagrdepat_time = '%19', \
                     reachadjt_time = '%20', allowtracknotmatch = %21, allowEntrnotmatch = %22, isArmy = %23, isImportant = %24, \
                     plan_checkState = %25, flowstatus_jiaoling = %26, flowstatus_liejian = %27, flowstatus_shangshui = %28, \
                     flowstatus_xiwu = %29, flowstatus_jiaopiao = %30, flowstatus_chengjiang = %31, flowstatus_zhaigua = %32, \
                     flowstatus_liewei = %33, flowstatus_huojian = %34, flowstatus_huancheng = %35, flowstatus_zhuangxie = %36, \
                     flowstatus_jiche = %37, flowstatus_daokou = %38, flowstatus_chehao = %39, flowstatus_zongkong = %40, flowstatus_zhanwu = %41, \
                     reach_routestatus = %42, depart_routestatus= %43, train_posistatus = '%44', train_nextproc = '%45', \
                     ultralimit_level = %46, reach_ultralimitlevel = %47, depart_ultralimitlevel = %48, transportpassenger=%49, \
                     traintype = '%50', runningtype = '%51', kehuoflag = %52 ,notes='%53' WHERE log_id = %54")
             .arg(_trafficLog->m_btBeginOrEndFlg)
             .arg(_trafficLog->m_strReachTrainNum)
             .arg(_trafficLog->m_strReachTrainNumOld)
             .arg(_trafficLog->m_strRecvTrainTrack)
             //.arg(_trafficLog->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"))
             .arg(_trafficLog->m_timProvReachStation.toString().size()?_trafficLog->m_timProvReachStation.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
             .arg(_trafficLog->m_strXHD_JZk)
             .arg(_trafficLog->m_strDepartTrainNum)
             .arg(_trafficLog->m_strDepartTrainNumOld)
             .arg(_trafficLog->m_strDepartTrainTrack)
             //.arg(_trafficLog->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"))
             .arg(_trafficLog->m_timProvDepaTrain.toString().size()?_trafficLog->m_timProvDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
             .arg(_trafficLog->m_strXHD_CZk)
             .arg(_trafficLog->m_bDeleteFlag)
             .arg(_trafficLog->m_bElectric)
             .arg(_trafficLog->m_nExecuteFlag)
             .arg(_trafficLog->m_timRealReachStation.toString().size()?_trafficLog->m_timRealReachStation.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
             .arg(_trafficLog->m_timRealDepaTrain.toString().size()?_trafficLog->m_timRealDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
             .arg(_trafficLog->m_timAgrFromAdjtStaDepaTrain.toString().size()?_trafficLog->m_timAgrFromAdjtStaDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
             .arg(_trafficLog->m_timFromAdjtStaDepaTrain.toString().size()?_trafficLog->m_timFromAdjtStaDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
             .arg(_trafficLog->m_timToAdjtStaAgrDepaTrain.toString().size()?_trafficLog->m_timToAdjtStaAgrDepaTrain.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
             .arg(_trafficLog->m_timtoAdjtStation.toString().size()?_trafficLog->m_timtoAdjtStation.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
             .arg(_trafficLog->m_bAllowGDNotMatch)
             .arg(_trafficLog->m_bAllowCRKNotMatch)
             .arg(_trafficLog->m_bArmy)
             .arg(_trafficLog->m_bImportant)
             .arg(_trafficLog->m_nCheckState)
             .arg(_trafficLog->m_btJALStatus)
             .arg(_trafficLog->m_btLJStatus)
             .arg(_trafficLog->m_btSSStatus)
             .arg(_trafficLog->m_btXWStatus)
             .arg(_trafficLog->m_btJPStatus)
             .arg(_trafficLog->m_btCJStatus)
             .arg(_trafficLog->m_btZGStatus)
             .arg(_trafficLog->m_btLWStatus)
             .arg(_trafficLog->m_btHJStatus)
             .arg(_trafficLog->m_btHCStatus)
             .arg(_trafficLog->m_btZXStatus)
             .arg(_trafficLog->m_btJCStatus)
             .arg(_trafficLog->m_btDKStatus)
             .arg(_trafficLog->m_btCHStatus)
             .arg(_trafficLog->m_btZKStatus)
             .arg(_trafficLog->m_btZWStatus)
             .arg(_trafficLog->m_btJCJLStatus)
             .arg(_trafficLog->m_btFCJLStatus)
             .arg(_trafficLog->m_strTrainPosStatus)
             .arg(_trafficLog->m_strProc)
             .arg(_trafficLog->m_nLevelCX)
             .arg(_trafficLog->m_nLevelCX)
             .arg(_trafficLog->m_nLevelCX)
             .arg(_trafficLog->m_bBLKY)
             .arg(_trafficLog->m_strLCLX)
             .arg(_trafficLog->m_strYXLX)
             .arg(_trafficLog->m_nLHFlg)
             .arg(_trafficLog->m_strNotes)
             .arg(_trafficLog->log_id);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        //UPDATETYPE_XCRZ
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateTrafficLog() failed!" << m_mySql->query->lastError();
    }
    return -1;
}
//更新行车日志股道信息并返回更新成功标志
int DataAccess::UpdateTrafficLogTrack(TrafficLog *_trafficLog)
{
    QString strSql = QString("UPDATE plan_trafficlog SET reach_track = '%1', depart_track = '%2' WHERE log_id = %3")
            .arg(_trafficLog->m_strRecvTrainTrack)
            .arg(_trafficLog->m_strDepartTrainTrack)
            .arg(_trafficLog->log_id);
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
       return 1;
    }
    else
    {
       QLOG_ERROR()<< "UpdateTrafficLogTrack() failed!" << m_mySql->query->lastError();
    }
    return -1;
}

//更新行车日志其他信息并返回更新成功标志
int DataAccess::UpdateTrafficLogOtherInfo(TrafficLog* _trafficLog)
{
    //列车位置和下一流程信息
    QString strSql = QString("UPDATE plan_trafficlog SET train_posistatus = '%1', train_nextproc = '%2' WHERE log_id = %3")
            .arg(_trafficLog->m_strTrainPosStatus)
            .arg(_trafficLog->m_strProc)
            .arg(_trafficLog->log_id);
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        //UPDATETYPE_XCRZ
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateTrafficLogOtherInfo() failed!" << m_mySql->query->lastError();
    }
    return -1;
}
//删除行车日志并返回删除成功标志
int DataAccess::DeleteTrafficLog(TrafficLog *_trafficLog)
{
    QString strSql = QString("DELETE FROM plan_trafficlog WHERE log_id = %1")
            .arg(_trafficLog->log_id);
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "DeleteTrafficLog() failed!" << m_mySql->query->lastError();
    }
    return -1;
}

//读取该站所有的进路序列
void DataAccess::SelectAllRouteOrder(MyStation* pMyStation)
{
    QString strSql = QString("SELECT route_id, plan_num, plan_jfctype, route_type, trainnum, trainnumold,\
                     track, timeplan, timereal, timeClear, entrance_xhd, autotouch, iselectric, ultralimitlevel,\
                     kehuoflag, isxianlusuo, descrip, descripreal, route_state, route_stateDescrip, successed, \
                     direction, isyxjl, xhdstart, xhdend, xhdyxend, isbtjl, arraybtjl, confirmed, lsbRouteIndex,\
                     tempXhdArray, tempXhdBtnArray, isdelete, father_id, iszhjl, iscreatebyman \
                     FROM plan_routeorder \
                     WHERE station_id = %1")
                     .arg(pMyStation->getStationID());
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllRouteOrder() failed!" << m_mySql->query->lastError();
        return;
    }
    pMyStation->m_ArrayRouteOrder.clear();
    while (m_mySql->next())
    {
        TrainRouteOrder* _pTrainRouteOrder = new TrainRouteOrder();
        _pTrainRouteOrder->station_id = pMyStation->getStationID();
        _pTrainRouteOrder->route_id = m_mySql->value(0).toInt();
        _pTrainRouteOrder->m_nPlanNumber = m_mySql->value(1).toInt();
        _pTrainRouteOrder->m_btBeginOrEndFlg = m_mySql->value(2).toInt();
        _pTrainRouteOrder->m_btRecvOrDepart = m_mySql->value(3).toInt();
        _pTrainRouteOrder->m_strTrainNum = m_mySql->value(4).toString();
        _pTrainRouteOrder->m_strTrainNumOld = m_mySql->value(5).toString();
        _pTrainRouteOrder->m_strTrack = m_mySql->value(6).toString();
        _pTrainRouteOrder->m_nTrackCode = pMyStation->GetCodeByStrName(_pTrainRouteOrder->m_strTrack);
        _pTrainRouteOrder->m_nGDPos = pMyStation->GetGDPosInzcArray(_pTrainRouteOrder->m_nTrackCode);
        _pTrainRouteOrder->m_timPlanned = m_mySql->value(7).toDateTime();
        _pTrainRouteOrder->m_timBegin = m_mySql->value(8).toDateTime();
        _pTrainRouteOrder->m_timClean = m_mySql->value(9).toDateTime();
        _pTrainRouteOrder->m_strXHD_JZk = m_mySql->value(10).toString();
        _pTrainRouteOrder->m_strXHD_CZk = _pTrainRouteOrder->m_strXHD_JZk;
        _pTrainRouteOrder->m_nCodeReachStaEquip = pMyStation->GetCodeByStrName(_pTrainRouteOrder->m_strXHD_JZk);
        _pTrainRouteOrder->m_nCodeDepartStaEquip = _pTrainRouteOrder->m_nCodeReachStaEquip;
        _pTrainRouteOrder->m_nAutoTouch = m_mySql->value(11).toInt();
        _pTrainRouteOrder->m_bElectric = m_mySql->value(12).toInt();
        _pTrainRouteOrder->m_nLevelCX = m_mySql->value(13).toInt();
        _pTrainRouteOrder->m_nLHFlg = m_mySql->value(14).toInt();
        _pTrainRouteOrder->bXianLuSuo = m_mySql->value(15).toInt();
        _pTrainRouteOrder->m_strRouteDescrip = m_mySql->value(16).toString();
        _pTrainRouteOrder->m_strRouteDescripReal =  m_mySql->value(17).toString();
        _pTrainRouteOrder->m_nOldRouteState = m_mySql->value(18).toInt();
        _pTrainRouteOrder->m_strRouteState = m_mySql->value(19).toString();
        _pTrainRouteOrder->m_bSuccessed = m_mySql->value(20).toInt();
        _pTrainRouteOrder->m_strDirection = m_mySql->value(21).toString();
        _pTrainRouteOrder->m_bYXJL = m_mySql->value(22).toInt();
        _pTrainRouteOrder->m_strXHDBegin = m_mySql->value(23).toString();
        _pTrainRouteOrder->m_strXHDEnd   = m_mySql->value(24).toString();
        _pTrainRouteOrder->m_strXHDYXEnd   = m_mySql->value(25).toString();
        _pTrainRouteOrder->m_bBTJL = m_mySql->value(26).toInt();
        //_pTrainRouteOrder->strArrayBTJL   = m_mySql->value(27).toString();
        _pTrainRouteOrder->m_bConfirmed = m_mySql->value(28).toInt()==1?true:false;
        _pTrainRouteOrder->m_nIndexRoute = m_mySql->value(29).toInt();
        //_pTrainRouteOrder->tempXhdArray   = m_mySql->value(30).toString();
        //_pTrainRouteOrder->tempRouteBtnArray   = m_mySql->value(31).toString();
        _pTrainRouteOrder->m_bDeleteFlag = m_mySql->value(32).toInt();
        _pTrainRouteOrder->father_id = m_mySql->value("father_id").toInt();
        _pTrainRouteOrder->m_bZHJL = m_mySql->value("iszhjl").toInt();
        _pTrainRouteOrder->m_bCreateByMan = m_mySql->value("iscreatebyman").toInt();
        //线路所无股道信息，将接车口和发车口组成进路描述
        if (_pTrainRouteOrder->bXianLuSuo)
        {
            _pTrainRouteOrder->m_nTrackCode = 0xFFFF;
            //_pTrainRouteOrder->m_strTrack = "";
            _pTrainRouteOrder->m_nGDPos = -1;
            _pTrainRouteOrder->m_strDirection = "通过";
        }
        //此处不再重复初始化
        ////判断延续进路
        //pMyStation->InitRouteYXJL(_pTrainRouteOrder);
        //初始化变通进路
        pMyStation->InitRouteBtjl(_pTrainRouteOrder);
        pMyStation->m_ArrayRouteOrder.append(_pTrainRouteOrder);
    }
    for(int i=0; i<pMyStation->m_ArrayRouteOrder.size();i++)
    {
        TrainRouteOrder* _pTrainRouteOrder = (TrainRouteOrder*)pMyStation->m_ArrayRouteOrder[i];
        //检查发车进路相应的接车进路是否是延续进路
        pMyStation->CheckJCRouteIsYXJL(_pTrainRouteOrder);
    }
}

//插入进路序列并返回该序列的id
int DataAccess::InsetRouteOrder(TrainRouteOrder* _pRouteOrder)
{
    QString strSql = QString("INSERT INTO plan_routeorder(station_id, plan_num, plan_jfctype, route_type, trainnum, \
                     trainnumold, track, timeplan, timereal, entrance_xhd, autotouch, iselectric, ultralimitlevel, \
                     kehuoflag, isxianlusuo, descrip, descripreal, route_state, route_stateDescrip, direction, isyxjl, \
                     xhdstart, xhdend, xhdyxend, isbtjl, arraybtjl, confirmed, lsbRouteIndex, successed, father_id, \
                     iszhjl,iscreatebyman) VALUES \
                     (%1, %2, %3, %4, '%5', '%6', '%7', '%8', '%9', '%10', \
                     %11, %12, %13, %14, %15, '%16', '%17', %18, '%19', '%20', \
                     %21, '%22', '%23', '%24', %25, NULL, %26, %27, %28, %29, %30, %31);")
            .arg(_pRouteOrder->station_id)
            .arg(_pRouteOrder->m_nPlanNumber)
            .arg(_pRouteOrder->m_btBeginOrEndFlg)
            .arg(_pRouteOrder->m_btRecvOrDepart)
            .arg(_pRouteOrder->m_strTrainNum)
            .arg(_pRouteOrder->m_strTrainNum)
            .arg(_pRouteOrder->m_strTrack)
            //.arg(_pRouteOrder->m_timPlanned.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_pRouteOrder->m_timPlanned.toString().size()?_pRouteOrder->m_timPlanned.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            //.arg(_pRouteOrder->m_timBegin.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_pRouteOrder->m_timBegin.toString().size()?_pRouteOrder->m_timBegin.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pRouteOrder->m_btRecvOrDepart==ROUTE_JC ? _pRouteOrder->m_strXHD_JZk : _pRouteOrder->m_strXHD_CZk)
            .arg(_pRouteOrder->m_nAutoTouch)
            .arg(_pRouteOrder->m_bElectric)
            .arg(_pRouteOrder->m_nLevelCX)
            .arg(_pRouteOrder->m_nLHFlg)
            .arg(_pRouteOrder->bXianLuSuo)
            .arg(_pRouteOrder->m_strRouteDescrip)
            .arg(_pRouteOrder->m_strRouteDescripReal)
            .arg(_pRouteOrder->m_nOldRouteState)
            .arg(_pRouteOrder->m_strRouteState)
            .arg(_pRouteOrder->m_strDirection)
            .arg(_pRouteOrder->m_bYXJL)
            .arg(_pRouteOrder->m_strXHDBegin)
            .arg(_pRouteOrder->m_strXHDEnd)
            .arg(_pRouteOrder->m_strXHDYXEnd)
            .arg(_pRouteOrder->m_bBTJL)
          //.arg(_pRouteOrder->strArrayBTJL)
            .arg(_pRouteOrder->m_bConfirmed?1:0)
            .arg(_pRouteOrder->m_nIndexRoute)
            .arg(_pRouteOrder->m_bSuccessed)
            .arg(_pRouteOrder->father_id)
            .arg(_pRouteOrder->m_bZHJL)
            .arg(_pRouteOrder->m_bCreateByMan);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        strSql = "SELECT MAX(route_id) FROM plan_routeorder;";
        //qDebug() <<strSql;

        if (!m_mySql->exec(strSql))
        {
            QLOG_ERROR()<< "InsetRouteOrder() SELECT failed!" << m_mySql->query->lastError();
            return -1;
        }
        while (m_mySql->next())
        {
            _pRouteOrder->route_id = m_mySql->value(0).toInt();
            //发送更新数据消息
            //pObjDoc->GetMyStationByStaIDInStaArray(_pRouteOrder->station_id)->SendUpdateDataMsg(UPDATETYPE_JLXL);
            return _pRouteOrder->route_id;
        }
    }
    else
    {
        QLOG_ERROR()<< "InsetRouteOrder() failed!";
    }
    return -1;
}

//更新进路序列并返回更新成功标志
int DataAccess::UpdateRouteOrder(TrainRouteOrder* _pRouteOrder)
{
    QString strSql = QString("UPDATE plan_routeorder SET track = '%1', plan_num = %2, plan_jfctype = %3, route_type = %4, \
                     timeplan = '%5', timereal = '%6', entrance_xhd = '%7', iselectric = %8, ultralimitlevel = %9, \
                     kehuoflag = %10, direction = '%11', autotouch = %12, descrip = '%13', descripreal = '%14', route_state = %15, \
                     route_stateDescrip = '%16', xhdstart = '%17', xhdend = '%18', xhdyxend = '%19', isbtjl = %20, \
                     arraybtjl = NULL, confirmed = %21, lsbRouteIndex = %22, successed = %23, isdelete=%24 WHERE route_id = %25")
            .arg(_pRouteOrder->m_strTrack)
            .arg(_pRouteOrder->m_nPlanNumber)
            .arg(_pRouteOrder->m_btBeginOrEndFlg)
            .arg(_pRouteOrder->m_btRecvOrDepart)
            //.arg(_pRouteOrder->m_timPlanned.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_pRouteOrder->m_timPlanned.toString().size()?_pRouteOrder->m_timPlanned.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            //.arg(_pRouteOrder->m_timBegin.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_pRouteOrder->m_timBegin.toString().size()?_pRouteOrder->m_timBegin.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pRouteOrder->m_btRecvOrDepart==ROUTE_JC ? _pRouteOrder->m_strXHD_JZk : _pRouteOrder->m_strXHD_CZk)
            .arg(_pRouteOrder->m_bElectric)
            .arg(_pRouteOrder->m_nLevelCX)
            .arg(_pRouteOrder->m_nLHFlg)
            .arg(_pRouteOrder->m_strDirection)
            .arg(_pRouteOrder->m_nAutoTouch)
            .arg(_pRouteOrder->m_strRouteDescrip)
            .arg(_pRouteOrder->m_strRouteDescripReal)
            .arg(_pRouteOrder->m_nOldRouteState)
            .arg(_pRouteOrder->m_strRouteState)
            .arg(_pRouteOrder->m_strXHDBegin)
            .arg(_pRouteOrder->m_strXHDEnd)
            .arg(_pRouteOrder->m_strXHDYXEnd)
            .arg(_pRouteOrder->m_bBTJL)
          //.arg(_pRouteOrder->strArrayBTJL)
            .arg(_pRouteOrder->m_bConfirmed?1:0)
            .arg(_pRouteOrder->m_nIndexRoute)
            .arg(_pRouteOrder->m_bSuccessed)
            .arg(_pRouteOrder->m_bDeleteFlag)
            .arg(_pRouteOrder->route_id);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateRouteOrder() failed!" << m_mySql->query->lastError();
    }
    return -1;
}

//删除进路序列并返回删除成功标志
int DataAccess::DeleteRouteOrder(TrainRouteOrder* _pRouteOrder) //int _routeid
{
    QString strSql = QString("DELETE FROM plan_routeorder WHERE route_id = %1")
            .arg(_pRouteOrder->route_id);
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        //pObjDoc->GetMyStationByStaIDInStaArray(_pRouteOrder->station_id)->SendUpdateDataMsg(UPDATETYPE_JLXL);
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "DeleteRouteOrder() failed!" << m_mySql->query->lastError();
    }
    return -1;
}
//删除本站所有进路序列并返回删除成功标志
int DataAccess::DeleteStationRouteOrder(int statId)
{
    QString strSql = QString("DELETE FROM plan_routeorder WHERE station_id = %1")
            .arg(statId);
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "DeleteStationRouteOrder() failed!" << m_mySql->query->lastError();
    }
    return -1;
}

//重置阶段计划表(截断表 清空表数据)
bool DataAccess::ResetTableStagePlan()
{
    QString strSql = "truncate table plan_stageplan_detail;";
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }

    //表中无数据时退出
    if(CheckTableIsNull("plan_stageplan_detail"))
    {
        return true;
    }

    if (m_mySql->exec(strSql))
    {
        strSql = "truncate table plan_stageplan;";
        if (m_mySql->exec(strSql))
        {
            return true;
        }
    }
    else
    {
        QLOG_ERROR()<< "ResetTableStagePlan() failed!" << m_mySql->query->lastError();
    }
    return false;
}

//重置本站阶段计划表(截断表 清空表数据)
bool DataAccess::ResetStationStagePlan(MyStation* pMyStation)
{
    QString strSql = QString("DELETE FROM plan_stageplan_detail WHERE station_id = %1")
            .arg(pMyStation->getStationID());
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        //pObjDoc->GetMyStationByStaIDInStaArray(pMyStation->getStationID())->SendUpdateDataMsg(UPDATETYPE_JLXL);
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ResetStationStagePlan() failed!" << m_mySql->query->lastError();
    }
    return false;
}

//重置行车日志表(截断表 清空表数据)
bool DataAccess::ResetTableTrafficLog()
{
    QString strSql = "truncate table plan_trafficlog;";
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    //表中无数据时退出
    if(CheckTableIsNull("plan_trafficlog"))
    {
        return true;
    }

    if (m_mySql->exec(strSql))
    {
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ResetTableTrafficLog() failed!" << m_mySql->query->lastError();
    }
    return false;
}

//重置本站行车日志表(截断表 清空表数据)
bool DataAccess::ResetStationTrafficLog(MyStation* pMyStation)
{
    QString strSql = QString("DELETE FROM plan_trafficlog WHERE station_id = %1")
            .arg(pMyStation->getStationID());
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    if (m_mySql->exec(strSql))
    {
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ResetStationTrafficLog() failed!" << m_mySql->query->lastError();
    }
    return false;
}

//重置进路序列表
bool DataAccess::ResetTableRouteOrder()
{
    QString strSql = "truncate table plan_routeorder;";
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    //表中无数据时退出
    if(CheckTableIsNull("plan_routeorder"))
    {
        return true;
    }

    if (m_mySql->exec(strSql))
    {
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ResetTableRouteOrder() failed!" << m_mySql->query->lastError();
    }
    return false;
}

//重置本站进路序列表
bool DataAccess::ResetStationRouteOrder(MyStation* pMyStation)
{
    QString strSql = QString("DELETE FROM plan_routeorder WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        //pObjDoc->GetMyStationByStaIDInStaArray(pMyStation->getStationID())->SendUpdateDataMsg(UPDATETYPE_JLXL);
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ResetStationRouteOrder() failed!" << m_mySql->query->lastError();
    }
    return false;
}

//重置所有表
bool DataAccess::ResetAllTable()
{
    //qDebug()<<QDateTime::currentDateTime().toString(TIME_FORMAT_YMDHMSM);
    QDateTime StartTime = QDateTime::currentDateTime();
    qDebug()<<"StartTime="<<StartTime.toString(Qt::ISODateWithMs);
    ResetTableStagePlan();
    ResetTableTrafficLog();
    ResetTableRouteOrder();
    ResetTableDisOrderRecv();
    ResetTableDisOrderDisp();
    ResetTableDisOrderLocom();
    ClearTableGDAntiSlip();
    QDateTime FinishTime = QDateTime::currentDateTime();
    qDebug()<<"Finish="<<FinishTime.toString(Qt::ISODateWithMs);
    qDebug()<<"Total="<<FinishTime.toMSecsSinceEpoch()-StartTime.toMSecsSinceEpoch()<<"ms";
    return true;
}

//重置本站数据库及表
bool DataAccess::ResetStationTable(MyStation* pMyStation)
{
    ResetStationStagePlan(pMyStation);
    ResetStationTrafficLog(pMyStation);
    ResetStationRouteOrder(pMyStation);
    ResetStationDisOrderRecv(pMyStation);
    ResetStationDisOrderDisp(pMyStation);
    ResetStationDisOrderLocom(pMyStation);
    ClearStationGDAntiSlip(pMyStation);
    return true;
}

//读取该站所有的接收调度命令
void DataAccess::SelectAllDisOrderRecv(MyStation* pMyStation)
{
    QString strSql = QString("SELECT order_id, order_num, time_recv, disCenter, disName, order_type, content,\
                     recv_place, sign_name, sign_time, readName, order_state \
                     FROM disorder_recv WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllDisOrderRecv() failed!" << m_mySql->query->lastError();
        return;
    }

    pMyStation->m_ArrayDisOrderRecv.clear();//清空
    while (m_mySql->next())
    {
        DispatchOrderStation* pDisOrderRecv = new DispatchOrderStation();
        pDisOrderRecv->station_id = pMyStation->getStationID();
        pDisOrderRecv->order_id = m_mySql->value(0).toInt();
        pDisOrderRecv->uNumber = m_mySql->value(1).toInt();
        pDisOrderRecv->strNumber = m_mySql->value(1).toString();
        pDisOrderRecv->timOrder = m_mySql->value(2).toDateTime();
        pDisOrderRecv->strDisCenter = m_mySql->value(3).toString();
        pDisOrderRecv->strDisName = m_mySql->value(4).toString();
        pDisOrderRecv->strType = m_mySql->value(5).toString();
        pDisOrderRecv->strContent = m_mySql->value(6).toString();
        pDisOrderRecv->listRecvPlace.append(m_mySql->value(7).toString().split("|"));
        pDisOrderRecv->strSignName = m_mySql->value(8).toString();
        pDisOrderRecv->timSign = m_mySql->value(9).toDateTime();
        pDisOrderRecv->listReadName.append(m_mySql->value(10).toString().split("|"));
        pDisOrderRecv->nStateDisOrder = m_mySql->value(11).toInt();

        pMyStation->m_ArrayDisOrderRecv.append(pDisOrderRecv);
    }
}

//插入接收调度命令返回该内容的id
int DataAccess::InsertDisOrderRecv(DispatchOrderStation* _pDisOrderRecv)
{
    QString strSql = QString("INSERT INTO disorder_recv(station_id, order_num, time_recv, \
                             disCenter, disName, order_type, content, recv_place, order_state)\
                             VALUES (%1, %2, '%3', '%4','%5', '%6', '%7', '%8', %9)")
            .arg(_pDisOrderRecv->station_id)
            .arg(_pDisOrderRecv->uNumber)
            //.arg(_pDisOrderRecv->timOrder.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(_pDisOrderRecv->timOrder.toString().size()?_pDisOrderRecv->timOrder.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pDisOrderRecv->strDisCenter)
            .arg(_pDisOrderRecv->strDisName)
            .arg(_pDisOrderRecv->strType)
            .arg(_pDisOrderRecv->strContent)
            .arg(_pDisOrderRecv->listRecvPlace.join("|"))
            .arg(_pDisOrderRecv->nStateDisOrder);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        strSql ="SELECT MAX(order_id) FROM disorder_recv;";
        //qDebug() <<strSql;

        if (!m_mySql->exec(strSql))
            return -1;
        while (m_mySql->next())
        {
            return m_mySql->value(0).toInt();
        }
    }
    else
    {
        QLOG_ERROR()<< "InsertDisOrderRecv() failed!" << m_mySql->query->lastError();
        return -1;
    }
}

//更新接收的调度命令返回更新成功标志
int DataAccess::UpdateDisOrderRecv(DispatchOrderStation* _pDisOrderRecv)
{
    QString strSql = QString("UPDATE disorder_recv SET sign_name = '%1', sign_time = '%2', \
                             readName = '%3', order_state = %4 WHERE order_id = %5")
            .arg(_pDisOrderRecv->strSignName)
            .arg(_pDisOrderRecv->timSign.toString().size()?_pDisOrderRecv->timSign.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pDisOrderRecv->listReadName.join("|"))
            .arg(_pDisOrderRecv->nStateDisOrder)
            .arg(_pDisOrderRecv->order_id);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateDisOrderRecv() failed!" << m_mySql->query->lastError();
        return -1;
    }
}

//重置调度命令表
bool DataAccess::ResetTableDisOrderRecv()
{
    QString strSql = QString("TRUNCATE TABLE disorder_recv;");
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    //表中无数据时退出
    if(CheckTableIsNull("disorder_recv"))
    {
        return true;
    }

    if (m_mySql->exec(strSql))
    {
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ResetTableDisOrderRecv() failed!" << m_mySql->query->lastError();
        return false;
    }
}

//重置本站调度命令表
bool DataAccess::ResetStationDisOrderRecv(MyStation* pMyStation)
{
    QString strSql = QString("DELETE FROM disorder_recv WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "ResetStationDisOrderRecv() failed!";
        return -1;
    }
}

//读取该站所有的请求/调度台调度命令
void DataAccess::SelectAllDisOrderDisp(MyStation *pMyStation)
{
    QString strSql = QString("SELECT order_id, order_num, order_header, content, rep_name \
                     ,time_create ,time_send ,disp_info \
                     FROM disorder_req WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllDisOrderDisp() failed!" << m_mySql->query->lastError();
        return;
    }

    pMyStation->m_ArrayDisOrderDisp.clear();//清空
    while (m_mySql->next())
    {
        DispatchOrderDispatcher* pDisOrderDDT = new DispatchOrderDispatcher();
        pDisOrderDDT->station_id = pMyStation->getStationID();
        pDisOrderDDT->order_id = m_mySql->value(0).toInt();
        pDisOrderDDT->uNumber = m_mySql->value(1).toInt();//命令号
        pDisOrderDDT->strType = m_mySql->value(2).toString();//命令类型
        pDisOrderDDT->strContent = m_mySql->value(3).toString();//命令内容
        pDisOrderDDT->strDutyName = m_mySql->value(4).toString();//值班人
        pDisOrderDDT->timCreate = m_mySql->value(5).toDateTime();//创建时间
        pDisOrderDDT->timSend = m_mySql->value(6).toDateTime();//发送时间
        pDisOrderDDT->bSend = pDisOrderDDT->timSend.isValid();
        //调度台接收信息
        QString strDispInfoJSON = m_mySql->value(7).toString();
        pDisOrderDDT->JsonStringToData(strDispInfoJSON);
        qDebug()<<"strDispInfoJSON="<<strDispInfoJSON;
        //增加一条
        pMyStation->m_ArrayDisOrderDisp.append(pDisOrderDDT);
    }
}
//插入请求/调度台的调度命令返回该内容的id
int DataAccess::InsertDisOrderDisp(DispatchOrderDispatcher *_pDisOrderDDT)
{
    QString strSql = QString("INSERT INTO disorder_req(station_id, order_num, order_header \
                             ,time_create, time_send, content, rep_name ,disp_info)\
                             VALUES (%1,%2,'%3','%4','%5','%6','%7','%8')")
            .arg(_pDisOrderDDT->station_id)
            .arg(_pDisOrderDDT->uNumber)
            .arg(_pDisOrderDDT->strType)
            .arg(_pDisOrderDDT->timCreate.toString().size()?_pDisOrderDDT->timCreate.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pDisOrderDDT->timSend.toString().size()?_pDisOrderDDT->timSend.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pDisOrderDDT->strContent)
            .arg(_pDisOrderDDT->strDutyName)
            .arg(_pDisOrderDDT->ToJsonString());
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        strSql ="SELECT MAX(order_id) FROM disorder_req;";
        //qDebug() <<strSql;
        if (!m_mySql->exec(strSql))
            return -1;
        while (m_mySql->next())
        {
            return m_mySql->value(0).toInt();
        }
    }
    else
    {
        QLOG_ERROR()<< "InsertDisOrderDisp() failed!" << m_mySql->query->lastError();
        return -1;
    }
}
//更新请求/调度台的调度命令返回更新成功标志
int DataAccess::UpdateDisOrderDisp(DispatchOrderDispatcher *_pDisOrderDDT)
{
    QString strSql = QString("UPDATE disorder_req SET time_send='%1',disp_info='%2'\
                             WHERE order_id = %3")
            .arg(_pDisOrderDDT->timSend.toString().size()?_pDisOrderDDT->timSend.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pDisOrderDDT->ToJsonString())
            .arg(_pDisOrderDDT->order_id);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateDisOrderDisp() failed!" << m_mySql->query->lastError();
        return -1;
    }
}
//删除调度台的调度命令，返回删除成功标志
int DataAccess::DeleteDisOrderDisp(DispatchOrderDispatcher *_pDisOrderDDT)
{
    QString strSql = QString("DELETE FROM disorder_req WHERE order_id = %1")
            .arg(_pDisOrderDDT->order_id);
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "DeleteDisOrderDisp() failed!" << m_mySql->query->lastError();
    }
    return -1;
}
//重置请求/调度台调度命令表
bool DataAccess::ResetTableDisOrderDisp()
{
    QString strSql = QString("TRUNCATE TABLE disorder_req;");
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    //表中无数据时退出
    if(CheckTableIsNull("disorder_req"))
    {
        return true;
    }

    if (m_mySql->exec(strSql))
    {
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ResetTableDisOrderDisp() failed!" << m_mySql->query->lastError();
        return false;
    }
}
//重置本站请求/调度台调度命令表
bool DataAccess::ResetStationDisOrderDisp(MyStation *pMyStation)
{
    QString strSql = QString("DELETE FROM disorder_req WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "ResetStationDisOrderDisp() failed!" << m_mySql->query->lastError();
        return -1;
    }
}

//读取该站所有的机车调度命令
void DataAccess::SelectAllDisOrderLocom(MyStation *pMyStation)
{
    QString strSql = QString("SELECT order_id, order_num, order_type, staName, order_header \
                     ,time_create ,time_send ,content ,send_name,locom_info \
                     FROM disorder_train WHERE station_id = %1")
                     .arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllDisOrderLocom() failed!" << m_mySql->query->lastError();
        return;
    }

    pMyStation->m_ArrayDisOrderLocomot.clear();//清空
    while (m_mySql->next())
    {
        DispatchOrderLocomotive* pDisOrderJC = new DispatchOrderLocomotive();
        pDisOrderJC->station_id = pMyStation->getStationID();
        pDisOrderJC->order_id = m_mySql->value(0).toInt();
        pDisOrderJC->uNumber = m_mySql->value(1).toInt();//命令号
        pDisOrderJC->orderType = m_mySql->value(2).toInt();//命令种类
        pDisOrderJC->strStation = m_mySql->value(3).toString();//车站
        pDisOrderJC->strType = m_mySql->value(4).toString();//命令类型
        pDisOrderJC->timCreate = m_mySql->value(5).toDateTime();//创建时间
        pDisOrderJC->timSend = m_mySql->value(6).toDateTime();//发送时间
        pDisOrderJC->bSend = pDisOrderJC->timSend.isValid();
        pDisOrderJC->strContent = m_mySql->value(7).toString();//命令内容
        pDisOrderJC->strDutyName = m_mySql->value(8).toString();//值班人
        pDisOrderJC->bSend = pDisOrderJC->timSend.isValid();
        //机车接收信息
        QString strLocomInfoJSON = m_mySql->value(9).toString();
        pDisOrderJC->JsonStringToData(strLocomInfoJSON);
        qDebug()<<"strLocomInfoJSON="<<strLocomInfoJSON;
        //增加一条
        pMyStation->m_ArrayDisOrderLocomot.append(pDisOrderJC);
    }
}
//插入机车的调度命令返回该内容的id
int DataAccess::InsertDisOrderLocom(DispatchOrderLocomotive *_pDisOrderJC)
{
    QString strSql = QString("INSERT INTO disorder_train(station_id, order_num \
                            ,order_type ,staName ,order_header \
                            ,time_create, time_send, content, send_name,locom_info)\
                            VALUES (%1,%2,%3,'%4','%5','%6','%7','%8','%9','%10')")
            .arg(_pDisOrderJC->station_id)
            .arg(_pDisOrderJC->uNumber)
            .arg(_pDisOrderJC->orderType)
            .arg(_pDisOrderJC->strStation)
            .arg(_pDisOrderJC->strType)
            .arg(_pDisOrderJC->timCreate.toString().size()?_pDisOrderJC->timCreate.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pDisOrderJC->timSend.toString().size()?_pDisOrderJC->timSend.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pDisOrderJC->strContent)
            .arg(_pDisOrderJC->strDutyName)
            .arg(_pDisOrderJC->ToJsonString());
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        strSql ="SELECT MAX(order_id) FROM disorder_train;";
        //qDebug() <<strSql;
        if (!m_mySql->exec(strSql))
            return -1;
        while (m_mySql->next())
        {
            return m_mySql->value(0).toInt();
        }
    }
    else
    {
        QLOG_ERROR()<< "InsertDisOrderLocom() failed!" << m_mySql->query->lastError();
        return -1;
    }
}
//更新机车的调度命令返回更新成功标志
int DataAccess::UpdateDisOrderLocom(DispatchOrderLocomotive *_pDisOrderJC)
{
    QString strSql = QString("UPDATE disorder_train SET time_send='%1',locom_info='%2' \
                             WHERE order_id = %3")
            .arg(_pDisOrderJC->timSend.toString().size()?_pDisOrderJC->timSend.toString("yyyy-MM-dd hh:mm:ss"):"NULL")
            .arg(_pDisOrderJC->ToJsonString())
            .arg(_pDisOrderJC->order_id);
    strSql.replace("'NULL'","NULL");//空时间处理
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateDisOrderLocom() failed!" << m_mySql->query->lastError();
        return -1;
    }
}
//删除机车的调度命令，返回删除成功标志
int DataAccess::DeleteDisOrderLocom(DispatchOrderLocomotive *_pDisOrderJC)
{
    QString strSql = QString("DELETE FROM disorder_train WHERE order_id = %1")
            .arg(_pDisOrderJC->order_id);
    //qDebug() <<strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "DeleteDisOrderLocom() failed!" << m_mySql->query->lastError();
    }
    return -1;
}
//重置机车调度命令表
bool DataAccess::ResetTableDisOrderLocom()
{
    QString strSql = QString("TRUNCATE TABLE disorder_train;");
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    //表中无数据时退出
    if(CheckTableIsNull("disorder_train"))
    {
        return true;
    }

    if (m_mySql->exec(strSql))
    {
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ResetTableDisOrderLocom() failed!" << m_mySql->query->lastError();
        return false;
    }
}
//重置本站机车调度命令表
bool DataAccess::ResetStationDisOrderLocom(MyStation *pMyStation)
{
    QString strSql = QString("DELETE FROM disorder_train WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "ResetStationDisOrderLocom() failed!" << m_mySql->query->lastError();
        return -1;
    }
}

//读取该站所有的股道防溜
void DataAccess::SelectAllGDAntiSlip(MyStation* pMyStation)
{
    QString strSql = QString("SELECT gdname, l_antislip, l_txnum, l_jgqnum, l_meters, r_antislip, \
                             r_txnum, r_jgqnum, r_meters, trainsnum FROM base_gd_info \
                             WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllGDAntiSlip() failed!" << m_mySql->query->lastError();
        return;
    }
    pMyStation->m_ArrayGDAntiSlip.clear();//清空
    while (m_mySql->next())
    {
        CGD* pGdTemp = new CGD();
        pGdTemp->m_strName = m_mySql->value(0).toString();
        pGdTemp->setCode(pMyStation->GetCodeByStrName(pGdTemp->m_strName));
        pGdTemp->m_nLAntiSlipType = m_mySql->value(1).toInt();
        pGdTemp->m_nLTxNum = m_mySql->value(2).toInt();
        pGdTemp->m_nLJgqNum = m_mySql->value(3).toInt();
        pGdTemp->m_nLJnMeters =m_mySql->value(4).toInt();
        pGdTemp->m_nRAntiSlipType = m_mySql->value(5).toInt();
        pGdTemp->m_nRTxNum = m_mySql->value(6).toInt();
        pGdTemp->m_nRJgqNum = m_mySql->value(7).toInt();
        pGdTemp->m_nRJnMeters = m_mySql->value(8).toInt();
        pGdTemp->m_nTrainNums = m_mySql->value(9).toInt();

        pMyStation->m_ArrayGDAntiSlip.append(pGdTemp);
    }
}

//更新股道防溜返回更新成功标志
int DataAccess::UpdateGDAntiSlip(int _staId, CGD* _pGD)
{
    QString strSql = QString("UPDATE base_gd_info SET l_antislip = %1, l_txnum = %2, \
                     l_jgqnum = %3, l_meters = %4, r_antislip = %5, r_txnum = %6, r_jgqnum = %7, \
                     r_meters = %8, trainsnum = %9 WHERE gdname = '%10' AND station_id = %11")
            .arg(_pGD->m_nLAntiSlipType)
            .arg(_pGD->m_nLTxNum)
            .arg(_pGD->m_nLJgqNum)
            .arg(_pGD->m_nLJnMeters)
            .arg(_pGD->m_nRAntiSlipType)
            .arg(_pGD->m_nRTxNum)
            .arg(_pGD->m_nRJgqNum)
            .arg(_pGD->m_nRJnMeters)
            .arg(_pGD->m_nTrainNums)
            .arg(_pGD->getName())
            .arg(_staId);
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return -1;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息
        // //pObjDoc->GetMyStationByStaIDInStaArray(_staId)->SendUpdateDataMsg(UPDATETYPE_GDFL);
        return 1;
    }
    else
    {
        QLOG_ERROR()<< "UpdateGDAntiSlip() failed!" << m_mySql->query->lastError();
        return -1;
    }
}

//清除股道表中所有股道的防溜信息
bool DataAccess::ClearTableGDAntiSlip()
{
    QString strSql = "UPDATE base_gd_info SET l_antislip = 0, l_txnum = 0, l_jgqnum = 0, l_meters = 0, \
                        r_antislip = 0, r_txnum = 0, r_jgqnum = 0, r_meters = 0, trainsnum = 0;";
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息(重置完后发送)
        //((CCTC10Doc*)pObjDoc)->GetMyStationByStaIDInStaArray(_staId)->SendUpdateDataMsg(UPDATETYPE_GDFL);
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ClearTableGDAntiSlip() failed!" << m_mySql->query->lastError();
        return false;
    }
}

//清除股道表中本站所有股道的防溜信息
bool DataAccess::ClearStationGDAntiSlip(MyStation* pMyStation)
{
    QString strSql = QString("UPDATE base_gd_info SET l_antislip = 0, l_txnum = 0, l_jgqnum = 0, \
                     l_meters = 0, r_antislip = 0, r_txnum = 0, r_jgqnum = 0, r_meters = 0, \
                     trainsnum = 0 WHERE station_id = %1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return false;
    }
    if (m_mySql->exec(strSql))
    {
        //发送更新数据消息(重置完后发送)
        //((CCTC10Doc*)pObjDoc)->GetMyStationByStaIDInStaArray(_staId)->SendUpdateDataMsg(UPDATETYPE_GDFL);
        return true;
    }
    else
    {
        QLOG_ERROR()<< "ClearStationGDAntiSlip() failed!" << m_mySql->query->lastError();
        return false;
    }
}

//========== 防错办基础数据读取接口 ==========
//读取该站所有的股道基础属性
void DataAccess::SelectAllGDAttribute(MyStation *pMyStation)
{
    QString strSql = QString("SELECT t1.station_id, t1.gd_id, t1.gdname, t1.gdattr, t1.gddir, t1.gdjfattr, \
                             t1.gdoverlevel, t1.gdplatform, t1.gdisCRH, t1.gdiswater, t1.gdisblowdown, t1.gdArmy \
                             FROM base_gd_info AS t1 WHERE t1.station_id=%1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllGDAttribute() failed!" << m_mySql->query->lastError();
        return;
    }
    pMyStation->vectGDAttr.clear();//清空
    while (m_mySql->next())
    {
        CGD* pGd = new CGD();
        pGd->gdId = m_mySql->value("gd_id").toInt();
        pGd->m_strName = m_mySql->value("gdname").toString();
        pGd->setCode(pMyStation->GetCodeByStrName(pGd->m_strName));
        pGd->gdAttr = m_mySql->value("gdattr").toInt();
        pGd->jfcDir = m_mySql->value("gddir").toInt();
        pGd->jfcAttr = m_mySql->value("gdjfattr").toInt();
        pGd->overLimit = m_mySql->value("gdoverlevel").toInt();
        pGd->platform =m_mySql->value("gdplatform").toInt();
        pGd->isCRH = m_mySql->value("gdisCRH").toInt();
        pGd->isWater = m_mySql->value("gdiswater").toInt();
        pGd->isBlowdown = m_mySql->value("gdisblowdown").toInt();
        pGd->army = m_mySql->value("gdArmy").toInt();
        //add
        pMyStation->vectGDAttr.append(pGd);
    }
}
//读取该站所有的出入口基础属性
void DataAccess::SelectAllGatewayAttribute(MyStation *pMyStation)
{
    QString strSql = QString("SELECT t1.station_id, t1.id, t1.jzxhname, t1.enexname, t1.enexdir,\
                             t1.Isoverlevel, t1.ispassengertrain,t1.isfreighttrain \
                             FROM base_entrance_exit AS t1  WHERE t1.station_id=%1").arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllGatewayAttribute() failed!" << m_mySql->query->lastError();
        return;
    }
    pMyStation->vectGatewayAttr.clear();//清空
    while (m_mySql->next())
    {
        CXHD* pXHD = new CXHD();
        pXHD->m_strName = m_mySql->value("jzxhname").toString();
        pXHD->setCode(pMyStation->GetCodeByStrName(pXHD->m_strName));
        pXHD->enexName = m_mySql->value("enexname").toString();
        pXHD->direct = m_mySql->value("enexdir").toInt();
        pXHD->allowOverLimit = m_mySql->value("Isoverlevel").toInt();
        pXHD->allowPassenger = m_mySql->value("ispassengertrain").toInt();
        pXHD->allowFreight = m_mySql->value("isfreighttrain").toInt();
        //add
        pMyStation->vectGatewayAttr.append(pXHD);
    }
}

//读取该站所有的列车固定径路基础属性
void DataAccess::SelectAllFixedRoute(MyStation *pMyStation)
{
//    QString strSql = QString("SELECT t2.id, t2.station_id, t2.gd_id, \
//                             t2.arrivalnum, t2.departnum, t2.arrivaltime, t2.departtime,\
//                             t2.entraid, t2.exitid, t2.beforestationid, t2.afterstationid,\
//                             t2.stoppoint, t2.deliveryorder, t2.trianflag, t2.cargocheck,\
//                             t2.boardflag, t2.pickhang, t2.transferflag, t2.Loadcargo,\
//                             t2.dirtabsorption, t2.crossflag, t2.wagonnum, t2.waterflag,\
//                             t2.traincheck, t2.Intercontrol, t2.depotflag, t2.payticket, t2.traintail \
//                             FROM base_train_fixedroute AS t2 WHERE t2.station_id=%1")
//                            .arg(pMyStation->getStationID());
    QString strSql = QString("SELECT c.*,d.jzxhname as exitidName \
                             FROM( \
                             SELECT a.*,b.jzxhname as entraidName \
                             FROM base_train_fixedroute as a \
                             LEFT JOIN base_entrance_exit as b \
                             on a.entraid = b.id \
                             ) as c LEFT JOIN base_entrance_exit as d \
                             on c.exitid = d.id \
                             WHERE c.station_id =%1")
                            .arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllFixedRoute() failed!" << m_mySql->query->lastError();
        return;
    }
    pMyStation->vectFixedRoute.clear();//清空
    while (m_mySql->next())
    {
        FixedRoute* pFixedRoute = new FixedRoute();
        pFixedRoute->id = m_mySql->value("id").toInt();
        pFixedRoute->staId = m_mySql->value("station_id").toInt();
        pFixedRoute->gdId = m_mySql->value("gd_id").toInt();
        pFixedRoute->arrivalnum = m_mySql->value("arrivalnum").toString();
        pFixedRoute->arrivaltime = m_mySql->value("arrivaltime").toString();
        pFixedRoute->departnum = m_mySql->value("departnum").toString();
        pFixedRoute->departtime = m_mySql->value("departtime").toString();
        pFixedRoute->entraid = m_mySql->value("entraid").toInt();
        pFixedRoute->entrXHDName = m_mySql->value("entraidName").toString();
        pFixedRoute->exitid = m_mySql->value("exitid").toInt();
        pFixedRoute->exitXHDName = m_mySql->value("exitidName").toString();
        pFixedRoute->beforestationid = m_mySql->value("beforestationid").toInt();
        pFixedRoute->afterstationid = m_mySql->value("afterstationid").toInt();
        pFixedRoute->stoppoint = m_mySql->value("stoppoint").toInt();
        pFixedRoute->deliveryorder = m_mySql->value("deliveryorder").toInt();
        pFixedRoute->trianflag = m_mySql->value("trianflag").toInt();
        pFixedRoute->cargocheck = m_mySql->value("cargocheck").toInt();
        pFixedRoute->boardflag = m_mySql->value("boardflag").toInt();
        pFixedRoute->pickhang = m_mySql->value("pickhang").toInt();
        pFixedRoute->transferflag = m_mySql->value("transferflag").toInt();
        pFixedRoute->Loadcargo = m_mySql->value("Loadcargo").toInt();
        pFixedRoute->dirtabsorption = m_mySql->value("dirtabsorption").toInt();
        pFixedRoute->crossflag = m_mySql->value("crossflag").toInt();
        pFixedRoute->wagonnum = m_mySql->value("wagonnum").toInt();
        pFixedRoute->waterflag = m_mySql->value("waterflag").toInt();
        pFixedRoute->traincheck = m_mySql->value("traincheck").toInt();
        pFixedRoute->Intercontrol = m_mySql->value("Intercontrol").toInt();
        pFixedRoute->depotflag = m_mySql->value("depotflag").toInt();
        pFixedRoute->payticket = m_mySql->value("payticket").toInt();
        pFixedRoute->traintail = m_mySql->value("traintail").toInt();
        //add
        pMyStation->vectFixedRoute.append(pFixedRoute);
    }
}
//读取该站所有的车次股道列表信息
void DataAccess::SelectAllTrainNumTrack(MyStation *pMyStation)
{
    QString strSql = QString("SELECT t1.station_id, t1.id, t1.trainnum, t1.tracks\
                             FROM base_trainnum_tracklist AS t1  WHERE t1.station_id=%1")
                            .arg(pMyStation->getStationID());
    //qDebug() << strSql;
    if(!isDataBaseOpen())
    {
        return;
    }
    if (!m_mySql->exec(strSql))
    {
        QLOG_ERROR()<< "SelectAllTrainNumTrack() failed!" << m_mySql->query->lastError();
        return;
    }
    pMyStation->vectTrainNumTrack.clear();//清空
    while (m_mySql->next())
    {
        TrainNumTrack* pTrainNumTrack = new TrainNumTrack();
        pTrainNumTrack->id = m_mySql->value("id").toInt();
        pTrainNumTrack->staId = m_mySql->value("station_id").toInt();
        pTrainNumTrack->trainnum = m_mySql->value("trainnum").toString();
        pTrainNumTrack->tracksStr = m_mySql->value("tracks").toString();
        pTrainNumTrack->tracks = pTrainNumTrack->Tracks();
        //add
        pMyStation->vectTrainNumTrack.append(pTrainNumTrack);
    }
}

//检查表是否是空表
bool DataAccess::CheckTableIsNull(QString strTableName)
{
    //表中无数据时退出
    QString strSqlCheck = QString("select count(*) as rowCount from %1;").arg(strTableName);
    if (m_mySql->exec(strSqlCheck))
    {
        while (m_mySql->next())
        {
            int rouCount = m_mySql->value(0).toInt();
            if(0 == rouCount)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

