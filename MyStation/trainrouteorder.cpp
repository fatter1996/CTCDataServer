#include "trainrouteorder.h"

#pragma execution_character_set("utf-8")

TrainRouteOrder::TrainRouteOrder()
{
    m_nAutoTouch = false;
    m_nManTouch = false;
    m_bElectric = true;
    m_strRouteState = "等待";
    m_ntime = 0;
    m_nOvertime = 0;

    //初始化UDP
    memset(m_byArrayUDPJLOrderDate, 0, 30);
    for (int i = 0; i < 4; i++)
    {
        m_byArrayUDPJLOrderDate[i] = 0xEF;
        m_byArrayUDPJLOrderDate[30 - i - 1] = 0xFE;
    }
    m_byArrayUDPJLOrderDate[4] = 0x1E;
    m_byArrayUDPJLOrderDate[9] = 0x88;//信息分类码 （CTC车务终端->联锁仿真机）
    m_byArrayUDPJLOrderDate[10] = 0x01;//功能按钮类型-列车进路

    m_nTouchingCount = 0;
}

//0"等待"、1"正在触发"、2"触发完成"、3"占用"、4"出清"、5"取消"
void TrainRouteOrder::SetState(int nState)
{
    m_nOldRouteState = nState;
    switch(nState)
    {
    case 0:
        m_strRouteState = "等待";
        m_nTouchingCount = 0;
        break;
    case 1:
        m_strRouteState = "正在触发";
        break;
    case 2:
        m_strRouteState = "触发完成";
        break;
    case 3:
        m_strRouteState = "占用";
        break;
    case 4:
        {
            m_timClean = QDateTime::currentDateTime();
            m_strRouteState = "出清";
        }
        break;
    case 5:
        m_strRouteState = "取消";
        //m_strRouteState =  "已取消";
        m_nTouchingCount = 0;
        break;
    default:
        break;
    }
}

//复制操作（目标进路序列）
void TrainRouteOrder::MyCopy(TrainRouteOrder *pDstRouteOrder)
{
    pDstRouteOrder->station_id = this->station_id;
    pDstRouteOrder->m_nPlanNumber = this->m_nPlanNumber;
    pDstRouteOrder->m_timPlanned = this->m_timPlanned;
    pDstRouteOrder->m_timBegin = this->m_timBegin;
    pDstRouteOrder->bXianLuSuo = this->bXianLuSuo;
    pDstRouteOrder->m_btRecvOrDepart = this->m_btRecvOrDepart;
    pDstRouteOrder->m_btBeginOrEndFlg = this->m_btBeginOrEndFlg;
    pDstRouteOrder->m_strTrainNum = this->m_strTrainNum;
    pDstRouteOrder->m_strTrainNumOld = this->m_strTrainNumOld;
    pDstRouteOrder->m_nTrackCode = this->m_nTrackCode;
    pDstRouteOrder->m_strTrack = this->m_strTrack;
    pDstRouteOrder->m_nGDPos = this->m_nGDPos;
    pDstRouteOrder->m_nCodeReachStaEquip = this->m_nCodeReachStaEquip;
    pDstRouteOrder->m_nCodeDepartStaEquip = this->m_nCodeDepartStaEquip;
    pDstRouteOrder->m_strXHD_JZk = this->m_strXHD_JZk;
    pDstRouteOrder->m_strXHD_CZk = this->m_strXHD_CZk;
    pDstRouteOrder->m_strDirection = this->m_strDirection;
    pDstRouteOrder->m_bElectric = this->m_bElectric;
    pDstRouteOrder->m_nLevelCX = this->m_nLevelCX;
    pDstRouteOrder->m_nLHFlg = this->m_nLHFlg;
    pDstRouteOrder->m_strRouteState = this->m_strRouteState;
    pDstRouteOrder->m_bDeleteFlag = this->m_bDeleteFlag;
    pDstRouteOrder->m_bOnlyLocomotive = this->m_bOnlyLocomotive;
}
