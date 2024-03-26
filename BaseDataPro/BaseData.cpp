#include "BaseData.h"

CBaseData::CBaseData(QObject *parent) : QObject(parent)
{
    pCenter.setX(1);
    pCenter.setY(1);         //设备中心点
    pCenterConst.setX(1);
    pCenterConst.setY(1);    //设备中心点
    m_nType = 1;              //设备类型
    m_strName = "";           //设备名称
    m_nSX = 0;                //上下行咽喉
    m_nCode = 0xffff;         //设备编号
    m_textRect.setRect(1,1,1,1);        //设备名称文本区域
    m_textRectConst.setRect(1,1,1,1);   //设备名称文本区域
    m_nStationID=0;           //所属站ID
    m_nBelongToTCC=0;         //所属TCC ID
    m_nBelongToRBC=0;         //所属RBC ID
    isDisPlayName = true;     //是否显示名称
    m_bToolTip=false;         //是否显示鼠标提示信息
    m_strToolTip="";          //鼠标提示信息

    m_nState = 0;
    m_nOldState = 0;
    m_nOldState2 = 0;

    GLB = 0;
}

/***************设置对象状态******************
*功能：设置组件状态数据
*参数：nState：组件状态数据
**********************************************/
void CBaseData::setState(int nState)
{
    if(m_nState != nState)
    {
        m_nOldState2 = m_nOldState;
        m_nOldState = m_nState;//new,20180510
    }

    m_nState = nState;
}

/***************取得对象状态******************
*功能：判断组件是否包含所给状态
*参数：nState：状态数据
*返回值：包含：TRUE；不包含：FALSE
**********************************************/
bool CBaseData::getState(int nState)
{
    if ( (int) (m_nState & nState) > 0)//判断组件状态是否包含所给状态
        return true;
    else
        return false;
}
//new,lwm,20180510
//***************取得对象旧状态******************
bool CBaseData::getOldState(int nState)
{
    if ( (int) (m_nOldState & nState) > 0)//判断组件状态是否包含所给状态
        return true;
    else
        return false;
}
int CBaseData::getOldState()
{
    return m_nOldState;
}
bool CBaseData::getOldState2(int nState)
{
    if ( (int) (m_nOldState2 & nState) > 0)//判断组件状态是否包含所给状态
        return true;
    else
        return false;
}
/***************取得对象状态******************
*功能：取得组件状态数据
*返回值：组件状态数据
**********************************************/
int CBaseData::getState()
{
    return m_nState;
}

void CBaseData::Draw_ToolTip(QPainter *painter, double nDiploid)
{
}
void CBaseData::Draw(QPainter *painter, long nElapsed, double nDiploid,QPoint offset,int type)
{
}
void CBaseData::setDevStateToSafe()
{

}
void CBaseData::setVollover(QPoint pt_Base)
{

}
unsigned int CBaseData::getDevType()
{
    return 0;
}
int CBaseData::moveCursor(QPoint p)
{
    return 0;
}
void CBaseData::draw_Pixmap(QPainter *painter,QString picName,int x,int y,int width,int heigh,int nDiploid)
{
    QPixmap pix;
    pix.load(picName);
    painter->drawPixmap(x,y,width,heigh,pix);
}
void CBaseData::setCenterPt(QPointF pt)
{
    pCenter=pt;
    pCenterConst=pt;
}
QPointF CBaseData::getCenterPt()
{
    return pCenterConst;
}
void CBaseData::setType(unsigned int nType)
{
    m_nType = nType;
}
unsigned int CBaseData::getType()
{
    return m_nType;
}
void CBaseData::setName(QString name)
{
    m_strName = name;
}
QString CBaseData::getName()
{
    return m_strName;
}
void CBaseData::setSX(unsigned int nSX)
{
    m_nSX = nSX;
}
unsigned int CBaseData::getSX()
{
    return m_nSX;
}
void CBaseData::setCode(unsigned int code)
{
    m_nCode = code;
}
unsigned int CBaseData::getCode()
{
    return m_nCode;
}
void CBaseData::setTextRect(QRectF rect)
{
    m_textRect=rect;
    m_textRectConst=rect;
}
QRectF CBaseData::getTextRect()
{
    return m_textRect;
}
void CBaseData::setStationID(unsigned int id)
{
    m_nStationID=id;
}
unsigned int CBaseData::getStationID()
{
    return m_nStationID;
}
void CBaseData::setBelongToTCC(unsigned int id)
{
    m_nBelongToTCC=id;
}
unsigned int CBaseData::getBelongToTCC()
{
    return m_nBelongToTCC;
}
void CBaseData::setBelongToRBC(unsigned int id)
{
    m_nBelongToRBC=id;
}
unsigned int CBaseData::getBelongToRBC()
{
    return m_nBelongToRBC;
}
void CBaseData::setDisplayName(bool nFlag)
{
    isDisPlayName = nFlag;
}
bool CBaseData::getDisplayName()
{
    return isDisPlayName;
}
void CBaseData::setToolTipFlag(bool flag)
{
    m_bToolTip=flag;
}
bool CBaseData::getToolTipFlag()
{
    return m_bToolTip;
}
void CBaseData::setToolTipStr(QString str)
{
    m_strToolTip=str;
}
QString CBaseData::getToolTipStr()
{
    return m_strToolTip;
}
