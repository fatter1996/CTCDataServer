#ifndef STAGE_H
#define STAGE_H

#include <QString>
#include <QDateTime>

//阶段计划
class Stage
{
public:
    Stage();

public:
    int     m_nPlanId = 0; //计划id
    int     m_nPlanNum = 0;//计划号
    QDateTime   m_timRecv;//接收时间
    QString m_strDispatch;//调度台
    QString m_strDispatcher;//调度员
    unsigned int station_id = 0;
};

#endif // STAGE_H
