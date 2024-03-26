#include <QtDebug>
#include "station.h"
//#include "QMessageBox"

CStation::CStation(QObject *parent) : QObject(parent)
{
    m_nStationID=0;
    m_strStationName="";
    m_nStationType=0;
    m_strLS_Producer="";
    isGT=true;
    isZCZX=true;
    offsetPt.setX(1);
    offsetPt.setY(1);
    m_nCTCSoftID=0;
    m_nLS_ProcessID=0xFFFF;  //联锁进程唯一识别号
    m_nLS_SoftID=0;    //联锁软件ID
    //isLSProcessFlag=false;
    m_nXLMNJSoftID=0;
    isLSComState=false;  //联锁通信状态标识
    LSComTime=0;   //与联锁持续无通信周期计数器
    installEventFilter(this);
    startTimer(1000);
    QdNum=0;
    GdNum=0;
    DcNum=0;
    XhNum=0;
}
void CStation::setDevStateToSafe()
{
    for(auto ement:DevArray)
    {
        ement->setDevStateToSafe();
    }
}
void CStation::resolutionDevStateOfLS(unsigned char *array)
{
    int dcNum=(array[10]);
    int xhdNum=(array[11]);
    int qdNum=(array[12]);
    int count=13;
    //道岔状态
    int num=0;
    for(auto ement:DevArray)
    {
        if(num>dcNum)
        {
            break;
        }
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            int state=array[count];
            count++;
            setGDDC_StateOfLS(gddc,state);
            num++;
        }
    }
    //信号机状态
    num=0;
    for(auto ement:DevArray)
    {
        if(num>xhdNum)
        {
            break;
        }
        if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            int state1=array[count];
            count++;
            int state2=array[count];
            count++;
            int state3=array[count];
            count++;
            setXHD_StateOfLS(xhd,state1,state2,state3);
            num++;
        }
    }
    //区段状态
    num=0;
    for(auto ement:DevArray)
    {
        if(num>qdNum)
        {
            break;
        }
        if(ement->getDevType() == Dev_QD)
        {
            CQD *qd=(CQD *)ement;
            int state1=array[count];
            count++;
            int state2=array[count];
            count++;
            setQD_StateOfLS(qd,state1,state2);
            num++;
        }
    }
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_TG)
        {
            CTG *tg=(CTG *)ement;
            int state=array[count];
            count++;
            setTG_StateOfLS(tg,state);
        }
    }

//    count++;  //主副电源一个字节

//    for(auto ement:DevArray)
//    {
//        if(ement->getDevType() == Dev_ZDBS)
//        {
//            CZDBS *zdbs=(CZDBS *)ement;
//            int state1=array[count];
//            count++;
//            int state2=array[count];
//            count++;
//            int state3=array[count];
//            count++;
//            int state4=array[count];
//            count++;
//            int state5=array[count];
//            count++;
//            setZDBS_StateOfLS(zdbs,state1,state2,state3,state4,state5);
//        }
//    }

//    emit signal_YHZSBState(array[count],getStationID());
//    count++;  //咽喉总锁一个字节


}
void CStation::setGDDC_StateOfLS(CGDDC *gddc,int state)
{
    if((state & 0xC0) == 0x80)
    {
        gddc->setDCWZ(DCDW);
    }
    else if((state & 0xC0) == 0x40)
    {
        gddc->setDCWZ(DCFW);
    }
    else
    {
        gddc->setDCWZ(DCSK);
    }
    if((state & 0x20) == 0x20)
    {
        gddc->setIsDS(true);
    }
    else
    {
        gddc->setIsDS(false);
    }
    if((state & 0x10) == 0x10)
    {
        gddc->setIsFS(true);
    }
    else
    {
        gddc->setIsFS(false);
    }
}
void CStation::setXHD_StateOfLS(CXHD *xhd,int state1,int state2,int state3)
{
    if((state1 & 0xF0) == 0x10)
    {
        xhd->setXHDState(XHD_BD);
    }
    else if((state1 & 0xF0) == 0x20)
    {
        xhd->setXHDState(XHD_YD);
    }
    else  if((state1 & 0xF0) == 0x30)
    {
        xhd->setXHDState(XHD_LD);
    }
    else if((state1 & 0xF0) == 0x40)
    {
        xhd->setXHDState(XHD_UD);
    }
    else if((state1 & 0xF0) == 0x50)
    {
        xhd->setXHDState(XHD_LU);
    }
    else if((state1 & 0xF0) == 0x60)
    {
        xhd->setXHDState(XHD_UU);
    }
    else if((state1 & 0xF0) == 0x70)
    {
        xhd->setXHDState(XHD_HD);
    }
    else if((state1 & 0xF0) == 0x80)
    {
        xhd->setXHDState(XHD_AD);
    }
    else if((state1 & 0xF0) == 0x90)
    {
        xhd->setXHDState(XHD_USU);
    }
    else if((state1 & 0xF0) == 0xA0)
    {
        xhd->setXHDState(XHD_LL);
    }
    else if((state1 & 0xF0) == 0xB0)
    {
        xhd->setXHDState(XHD_DS);
    }

    if((state1 & 0x0C) == 0x04)
    {
        xhd->setIsLCDown(true);
    }
    else
    {
        xhd->setIsLCDown(false);
    }

    if((state1 & 0x03) == 0x01)
    {
        xhd->setIsDCDown(true);
    }
    else
    {
        xhd->setIsDCDown(false);
    }

    if((state2 & 0xC0) == 0x40)
    {
        xhd->setIsYDDown(true);
    }
    else
    {
        xhd->setIsYDDown(false);
    }

    if((state2 & 0x30) == 0x00)
    {
        //无外包线
    }
    else if((state2 & 0x30) == 0x10)
    {
        //黄色框线
    }
    else if((state2 & 0x30) == 0x20)
    {
        //绿色框线
    }
    else if((state2 & 0x30) == 0x30)
    {
        //红色框线
    }

    if((state2 & 0x08) == 0x08)
    {
        xhd->setIsMD(false);
    }
    else
    {
        xhd->setIsMD(true);
    }

    if((state2 & 0x07) == 0x00)
    {
        xhd->setTimeType(0x11);
    }
    else if((state2 & 0x07) == 0x01)
    {
        xhd->setTimeType(0x22);
    }
    else if((state2 & 0x07) == 0x02)
    {
        xhd->setTimeType(0x33);
    }
    else if((state2 & 0x07) == 0x03)
    {
        xhd->setTimeType(0x44);
    }
    else if((state2 & 0x07) == 0x04)
    {
        xhd->setTimeType(0x55);
    }

    xhd->setTimeCount(state3);
}
void CStation::setQD_StateOfLS(CQD *qd,int state1,int state2)
{
    if((state1 & 0xE0) == 0x00)
    {
        setQD_Color(qd,Qt::red);
    }
    else if((state1 & 0xE0) == 0x20)
    {
        setQD_Color(qd,Qt::white);
    }
    else if((state1 & 0xE0) == 0x40)
    {
        setQD_Color(qd,SkyBlue);
    }
    else if((state1 & 0xE0) == 0x60)
    {
        setQD_Color(qd,Qt::white);
    }
    else if((state1 & 0xE0) == 0x80)  //深蓝色预锁闭
    {
        setQD_Color(qd,SkyBlue);
    }

    if((state1 & 0x1F) == 0x01)
    {
        qd->setDMHStateLeft(DMH_H);
    }
    else if((state1 & 0x1F) == 0x02)
    {
        qd->setDMHStateLeft(DMH_HU);
    }
    else if((state1 & 0x1F) == 0x03)
    {
        qd->setDMHStateLeft(DMH_HB);
    }
    else if((state1 & 0x1F) == 0x04)
    {
        qd->setDMHStateLeft(DMH_UU);
    }
    else if((state1 & 0x1F) == 0x05)
    {
        qd->setDMHStateLeft(DMH_UUS);
    }
    else if((state1 & 0x1F) == 0x06)
    {
        qd->setDMHStateLeft(DMH_U2);
    }
    else if((state1 & 0x1F) == 0x07)
    {
        qd->setDMHStateLeft(DMH_U2S);
    }
    else if((state1 & 0x1F) == 0x08)
    {
        qd->setDMHStateLeft(DMH_U);
    }
    else if((state1 & 0x1F) == 0x09)
    {
        qd->setDMHStateLeft(DMH_LU);
    }
    else if((state1 & 0x1F) == 0x0A)
    {
        qd->setDMHStateLeft(DMH_L);
    }
    else if((state1 & 0x1F) == 0x0B)
    {
        qd->setDMHStateLeft(DMH_L2);
    }
    else if((state1 & 0x1F) == 0x0C)
    {
        qd->setDMHStateLeft(DMH_L3);
    }
    else if((state1 & 0x1F) == 0x0D)
    {
        qd->setDMHStateLeft(DMH_L4);
    }
    else if((state1 & 0x1F) == 0x0E)
    {
        qd->setDMHStateLeft(DMH_L5);
    }
    else if((state1 & 0x1F) == 0x0F)
    {
        qd->setDMHStateLeft(DMH_JC);
    }
    else if((state1 & 0x1F) == 0x10)
    {
        qd->setDMHStateLeft(DMH_SP);
    }
    else
    {
        qd->setDMHStateLeft(DMH_WM);
    }

    if((state2 & 0x1F) == 0x01)
    {
        qd->setDMHStateRight(DMH_H);
    }
    else if((state2 & 0x1F) == 0x02)
    {
        qd->setDMHStateRight(DMH_HU);
    }
    else if((state2 & 0x1F) == 0x03)
    {
        qd->setDMHStateRight(DMH_HB);
    }
    else if((state2 & 0x1F) == 0x04)
    {
        qd->setDMHStateRight(DMH_UU);
    }
    else if((state2 & 0x1F) == 0x05)
    {
        qd->setDMHStateRight(DMH_UUS);
    }
    else if((state2 & 0x1F) == 0x06)
    {
        qd->setDMHStateRight(DMH_U2);
    }
    else if((state2 & 0x1F) == 0x07)
    {
        qd->setDMHStateRight(DMH_U2S);
    }
    else if((state2 & 0x1F) == 0x08)
    {
        qd->setDMHStateRight(DMH_U);
    }
    else if((state2 & 0x1F) == 0x09)
    {
        qd->setDMHStateRight(DMH_LU);
    }
    else if((state2 & 0x1F) == 0x0A)
    {
        qd->setDMHStateRight(DMH_L);
    }
    else if((state2 & 0x1F) == 0x0B)
    {
        qd->setDMHStateRight(DMH_L2);
    }
    else if((state2 & 0x1F) == 0x0C)
    {
        qd->setDMHStateRight(DMH_L3);
    }
    else if((state2 & 0x1F) == 0x0D)
    {
        qd->setDMHStateRight(DMH_L4);
    }
    else if((state2 & 0x1F) == 0x0E)
    {
        qd->setDMHStateRight(DMH_L5);
    }
    else if((state2 & 0x1F) == 0x0F)
    {
        qd->setDMHStateRight(DMH_JC);
    }
    else if((state2 & 0x1F) == 0x10)
    {
        qd->setDMHStateRight(DMH_SP);
    }
    else
    {
        qd->setDMHStateRight(DMH_WM);
    }
}
void CStation::setQD_Color(CQD *qd,QColor color)
{
    if(qd==NULL)
    {
        return;
    }
    for(int i=0;i<qd->getChildNum();i++)
    {
        for(int j=0;j<DevArray.size();j++)
        {
            if(qd->getChild(i) == DevArray[j]->getCode())
            {
                if(DevArray[j]->getDevType() == Dev_DC)
                {
                    CGDDC *gddc;
                    gddc=(CGDDC*)(DevArray[j]);
                    gddc->setQDColor(color);
                    break;
                }
                else if(DevArray[j]->getDevType() == Dev_GD)
                {
                    CGD *gd;
                    gd=(CGD*)(DevArray[j]);
                    gd->setQDColor(color);
                    break;
                }
            }
        }
    }
}
void CStation::setTG_StateOfLS(CTG *tg,int state)
{
    if((state & 0xC0) == 0x40)
    {
        tg->setIsTGANDown(true);
    }
    else
    {
        tg->setIsTGANDown(false);
    }
}
void CStation::setZDBS_StateOfLS(CZDBS *zdbs,int state1,int state2,int state3,int state4,int state5)
{
    if((state1 & 0xC0) == 0x00)
    {
        zdbs->setArrowStateFC(Arrow_Black);
    }
    else if((state1 & 0xC0) == 0x40)
    {
        zdbs->setArrowStateFC(Arrow_Yellow);
    }
    else if((state1 & 0xC0) == 0x80)
    {
        zdbs->setArrowStateFC(Arrow_Green);
    }
    else if((state1 & 0xC0) == 0xC0)
    {
        zdbs->setArrowStateFC(Arrow_Red);
    }

    if((state1 & 0x30) == 0x00)
    {
        zdbs->setArrowStateFC(Arrow_Black);
    }
    else if((state1 & 0x30) == 0x10)
    {
        zdbs->setArrowStateFC(Arrow_Yellow);
    }
    else if((state1 & 0x30) == 0x20)
    {
        zdbs->setArrowStateFC(Arrow_Green);
    }
    else if((state1 & 0x30) == 0x30)
    {
        zdbs->setArrowStateFC(Arrow_Red);
    }

    if((state1 & 0x08) == 0x08)
    {
        zdbs->setIsDown_GF(false);
    }
    else
    {
        zdbs->setIsDown_GF(true);
    }
    if((state1 & 0x04) == 0x04)
    {
        zdbs->setIsDown_ZFZ(false);
    }
    else
    {
        zdbs->setIsDown_ZFZ(true);
    }
    if((state1 & 0x02) == 0x02)
    {
        zdbs->setIsDown_FCFZ(false);
    }
    else
    {
        zdbs->setIsDown_FCFZ(true);
    }
    if((state1 & 0x01) == 0x01)
    {
        zdbs->setIsDown_JCFZ(false);
    }
    else
    {
        zdbs->setIsDown_JCFZ(true);
    }

    if((state2 & 0xF0) != 0x00)
    {
        zdbs->setIsBSDLight_FZ(true);
    }
    else
    {
        zdbs->setIsBSDLight_FZ(false);
    }
    if((state2 & 0x0F) != 0x00)
    {
        zdbs->setIsBSDLight_QJ(true);
    }
    else
    {
        zdbs->setIsBSDLight_QJ(false);
    }
    if((state3 & 0xF0) != 0x00)
    {
        zdbs->setIsBSDLight_YXFC(true);
    }
    else
    {
        zdbs->setIsBSDLight_YXFC(false);
    }
    if((state3 & 0x0F) != 0x00)
    {
        zdbs->setIsBSDLight_SG(true);
    }
    else
    {
        zdbs->setIsBSDLight_SG(false);
    }
    zdbs->setDownTime_FCFZ(state4);
    zdbs->setDownTime_JCFZ(state5);
}
//打包设备状态到CTC
int CStation::packQD_StateForCTC(QByteArray &msg,int index)
{
    int qdnum=0;
    int dcnum=0;
    int xhdnum=0;
    int devNumCount=index;
    int lenth=index;
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            int state=0x00;
            if(gddc->getDCWZ() == DCDW)
            {
                state |= 0x01;
            }
            if(gddc->getIsDS() == true)
            {
                state |= 0x02;
            }
            if(gddc->getIsFS() == true)
            {
                state |= 0x04;
            }
            if(gddc->getDCWZ() == DCSK)
            {
                state |= 0x08;
            }
            msg[lenth]=state;
            lenth++;
            dcnum++;
        }
    }
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            int state=0x00;
            if(xhd->getXHDState() == XHD_BD)
            {
                state = 0x01;
            }
            else if(xhd->getXHDState() == XHD_YD)
            {
                state = 0x02;
            }
            else if(xhd->getXHDState() == XHD_LD)
            {
                state = 0x03;
            }
            else if(xhd->getXHDState() == XHD_UD)
            {
                state = 0x04;
            }
            else if(xhd->getXHDState() == XHD_LU)
            {
                state = 0x05;
            }
            else if(xhd->getXHDState() == XHD_UU)
            {
                state = 0x06;
            }
            else if(xhd->getXHDState() == XHD_HD)
            {
                state = 0x07;
            }
            else if(xhd->getXHDState() == XHD_AD)
            {
                state = 0x08;
            }
            else if(xhd->getXHDState() == XHD_DS)
            {
                state = 0x09;
            }
            else if(xhd->getXHDState() == XHD_USU)
            {
                state = 0x0A;
            }
            else if(xhd->getXHDState() == XHD_LL)
            {
                state = 0x0B;
            }
            if(xhd->getIsMD() == true)
            {
                state |= 0x10;
            }
            if(xhd->getIsLCANFB() == true)
            {
                state |= 0x20;
            }
            if(xhd->getIsDCANFB() == true)
            {
                state |= 0x40;
            }
            if(xhd->getIsYDANFB() == true)
            {
                state |= 0x80;
            }
            msg[lenth]=state;
            lenth++;
            xhdnum++;
        }
    }
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_QD)
        {
            CQD *qd=(CQD *)ement;
            int state=0x00;
            if(qd->getDMHStateLeft() == DMH_H)
            {
                state=0x04;
            }
            else if(qd->getDMHStateLeft() == DMH_HU)
            {
                state=0x08;
            }
            else if(qd->getDMHStateLeft() == DMH_HB)
            {
                state=0x0C;
            }
            else if(qd->getDMHStateLeft() == DMH_UU)
            {
                state=0x10;
            }
            else if(qd->getDMHStateLeft() == DMH_UUS)
            {
                state=0x14;
            }
            else if(qd->getDMHStateLeft() == DMH_U2)
            {
                state=0x18;
            }
            else if(qd->getDMHStateLeft() == DMH_U2S)
            {
                state=0x1C;
            }
            else if(qd->getDMHStateLeft() == DMH_U)
            {
                state=0x20;
            }
            else if(qd->getDMHStateLeft() == DMH_LU)
            {
                state=0x24;
            }
            else if(qd->getDMHStateLeft() == DMH_L)
            {
                state=0x28;
            }
            else if(qd->getDMHStateLeft() == DMH_L2)
            {
                state=0x2C;
            }
            else if(qd->getDMHStateLeft() == DMH_L3)
            {
                state=0x30;
            }
            else if(qd->getDMHStateLeft() == DMH_L4)
            {
                state=0x34;
            }
            else if(qd->getDMHStateLeft() == DMH_L5)
            {
                state=0x38;
            }
            else if(qd->getDMHStateLeft() == DMH_JC)
            {
                state=0x3C;
            }
            else if(qd->getDMHStateLeft() == DMH_SP)
            {
                state=0x40;
            }
            else if(qd->getDMHStateLeft() == DMH_WM)
            {
                state=0x7C;
            }
            if(qd->getQDColor() == Qt::red)
            {
                state |= 0x01;
            }
            if((qd->getQDColor() == Qt::white) || (qd->getQDColor() == Qt::green))
            {
                state |= 0x02;
            }
            msg[lenth]=state;
            lenth++;

            state=0x00;
            if(qd->getDMHStateRight() == DMH_H)
            {
                state=0x01;
            }
            else if(qd->getDMHStateRight() == DMH_HU)
            {
                state=0x02;
            }
            else if(qd->getDMHStateRight() == DMH_HB)
            {
                state=0x03;
            }
            else if(qd->getDMHStateRight() == DMH_UU)
            {
                state=0x04;
            }
            else if(qd->getDMHStateRight() == DMH_UUS)
            {
                state=0x05;
            }
            else if(qd->getDMHStateRight() == DMH_U2)
            {
                state=0x06;
            }
            else if(qd->getDMHStateRight() == DMH_U2S)
            {
                state=0x07;
            }
            else if(qd->getDMHStateRight() == DMH_U)
            {
                state=0x08;
            }
            else if(qd->getDMHStateRight() == DMH_LU)
            {
                state=0x09;
            }
            else if(qd->getDMHStateRight() == DMH_L)
            {
                state=0x0A;
            }
            else if(qd->getDMHStateRight() == DMH_L2)
            {
                state=0x0B;
            }
            else if(qd->getDMHStateRight() == DMH_L3)
            {
                state=0x0C;
            }
            else if(qd->getDMHStateRight() == DMH_L4)
            {
                state=0x0D;
            }
            else if(qd->getDMHStateRight() == DMH_L5)
            {
                state=0x0E;
            }
            else if(qd->getDMHStateRight() == DMH_JC)
            {
                state=0x0F;
            }
            else if(qd->getDMHStateRight() == DMH_SP)
            {
                state=0x10;
            }
            else if(qd->getDMHStateRight() == DMH_WM)
            {
                state=0x1F;
            }
            msg[lenth]=state;
            lenth++;
            qdnum++;
        }
    }
    msg[devNumCount]=dcnum;
    msg[devNumCount+1]=dcnum>>8; //道岔个数
    msg[devNumCount+2]=xhdnum;
    msg[devNumCount+3]=xhdnum>>8; //信号机个数
    msg[devNumCount+4]=qdnum;
    msg[devNumCount+5]=qdnum>>8; //区段个数

    //自动闭塞设备状态打包

    return lenth;
}
void OperationalOfCTC(int anType,int dev1,int dev2,int dev3,int dev4,int dev5)
{
    if((anType == 0x01) || anType == 0x02)  //列车进路、调车进路
    {

    }
    else if(anType == 0x03)   //通过进路
    {

    }
    else if(anType == 0x04)  //总取消
    {

    }
    else if(anType == 0x05)  //信号重开
    {

    }
    else if(anType == 0x06)  //道岔总定
    {

    }
    else if(anType == 0x07)  //道岔总反
    {

    }
    else if(anType == 0x08)  //道岔单锁
    {

    }
    else if(anType == 0x09)  //道岔单解
    {

    }
    else if(anType == 0x10)  //封闭操作
    {

    }
    else if(anType == 0x11)  //解封操作
    {

    }
    else if(anType == 0x12)  //引导按钮
    {

    }
    else if(anType == 0x13)  //引导总锁
    {

    }
    else if(anType == 0x14)  //总人解
    {

    }
    else if(anType == 0x15)  //坡道解锁
    {

    }
    else if(anType == 0x18)  //总辅助
    {

    }
    else if(anType == 0x19)  //发车辅助
    {

    }
    else if(anType == 0x20)  //接车辅助
    {

    }
    else if(anType == 0x21)  //允许改方
    {

    }
    else if(anType == 0x23)  //上电解锁
    {

    }
    else if(anType == 0x24)  //引导总锁取消
    {

    }
    else if(anType == 0x25)  //区故解
    {

    }
    else if(anType == 0x36)  //半自动闭塞
    {

    }
    else if(anType == 0x37)  //半自动事故
    {

    }
    else if(anType == 0x38)  //半自动复原
    {

    }
    else if(anType == 0x39)  //点灯
    {

    }
    else if(anType == 0x40)  //灭灯
    {

    }
    else if(anType == 0x41)  //全场封锁
    {

    }
    else if(anType == 0x47)  //尖轨故障
    {

    }
    else if(anType == 0x48)  //心轨故障
    {

    }
    else if(anType == 0x50)  //计轴复零
    {

    }
    else if(anType == 0x51)  //计轴使用
    {

    }
    else if(anType == 0x52)  //计轴停止
    {

    }
    else if(anType == 0x60)  //清除
    {

    }
    else if(anType == 0x63)  //故障通知
    {

    }
}
//功能函数实现
void CStation::setShow_Menu(int type)
{
    if(type == 0x11)   //设置站名显示隐藏
    {
        for(int i=0;i<DevArray.size();i++)
        {
            if(DevArray[i]->getDevType() == Dev_TEXT)
            {
               CText *text;
               text=(CText *)(DevArray[i]);
               if(text->getTextType() == STATIONNAMETEXT)
               {
                   text->setDisplayName(!(text->getDisplayName()));
               }
            }
        }
    }
    else if(type == 0x22)  //设置信号机名称显示隐藏
    {
        for(int i=0;i<DevArray.size();i++)
        {
            if(DevArray[i]->getDevType() == Dev_XH)
            {
                DevArray[i]->setDisplayName(!(DevArray[i]->getDisplayName()));
            }
            if(DevArray[i]->getDevType() == Dev_TG)
            {
                DevArray[i]->setDisplayName(!(DevArray[i]->getDisplayName()));
            }
        }
    }
    else if(type == 0x33)  //设置道岔名称显示隐藏
    {
        for(int i=0;i<DevArray.size();i++)
        {
            if(DevArray[i]->getDevType() == Dev_DC)
            {
                DevArray[i]->setDisplayName(!(DevArray[i]->getDisplayName()));
            }
        }
    }
    else if(type == 0x44)  //设置无区段名称显示隐藏
    {
        for(int i=0;i<DevArray.size();i++)
        {
            if(DevArray[i]->getDevType() == Dev_GD)
            {
                DevArray[i]->setDisplayName(!(DevArray[i]->getDisplayName()));
            }
        }
    }
    else if(type == 0x55)  //设置文本显示隐藏
    {
        for(int i=0;i<DevArray.size();i++)
        {
            if(DevArray[i]->getDevType() == Dev_TEXT)
            {
               CText *text;
               text=(CText *)(DevArray[i]);
               if(text->getTextType() == TEXT)
               {
                   text->setDisplayName(!(text->getDisplayName()));
               }
            }
        }
    }
    else if(type == 0x66)  //设置道岔区段显示隐藏
    {
        for(int i=0;i<DevArray.size();i++)
        {
            if(DevArray[i]->getDevType() == Dev_TEXT)
            {
               CText *text;
               text=(CText *)(DevArray[i]);
               if(text->getTextType() == DCQDNAMETEXT)
               {
                   text->setDisplayName(!(text->getDisplayName()));
               }
            }
        }
    }
    else if(type == 0x77)  //设置道岔位置显示隐藏
    {
        for(int i=0;i<DevArray.size();i++)
        {
            if(DevArray[i]->getDevType() == Dev_DC)
            {
                CGDDC *gddc;
                gddc=(CGDDC *)(DevArray[i]);
                gddc->setDisDCWZ(!(gddc->getDisDCWZ()));
            }
        }
    }
    else if(type == 0x88)  //设置道岔接通光带显示隐藏
    {
        for(int i=0;i<DevArray.size();i++)
        {
            if(DevArray[i]->getDevType() == Dev_DC)
            {
                CGDDC *gddc;
                gddc=(CGDDC *)(DevArray[i]);
                gddc->setDisLight(!(gddc->getDisLight()));
            }
        }
    }
}
void CStation::set_ZCReset()
{
    for(int i=0;i<DevArray.size();i++)
    {
        if(DevArray[i]->getDevType() == Dev_XH)
        {
            CXHD *xhd;
        }
        else if(DevArray[i]->getDevType() == Dev_DC)
        {
            CGDDC *gddc;
            gddc=(CGDDC *)(DevArray[i]);
            //gddc->setIsQDGZ(false);
            //gddc->setIsQDZY(false);
            gddc->setIsFLBL_CQ(false);
            gddc->setIsFLBL_DW(false);
            gddc->setIsFLBL_FW(false);
            if(gddc->getDCWZ()==DCSK)
            {
                gddc->setDCWZ(DCDW);
            }
        }
        else if(DevArray[i]->getDevType() == Dev_GD)
        {
            CGD *gd;
            gd=(CGD *)(DevArray[i]);
            //gd->setIsQDGZ(false);
            //gd->setIsQDZY(false);
            gd->setGDFLBL(false);
        }
    }
}
void CStation::setQDZYFun(CQD *qd,bool flag)
{
    if(qd==NULL)
    {
        return;
    }
    //qd->setIsQDZY(flag);
    for(int i=0;i<qd->getChildNum();i++)
    {
        for(int j=0;j<DevArray.size();j++)
        {
            if(qd->getChild(i) == DevArray[j]->getCode())
            {
                if(DevArray[j]->getDevType() == Dev_DC)
                {
                    CGDDC *gddc;
                    gddc=(CGDDC*)(DevArray[j]);
                    //gddc->setIsQDZY(flag);
                    break;
                }
                else if(DevArray[j]->getDevType() == Dev_GD)
                {
                    CGD *gd;
                    gd=(CGD*)(DevArray[j]);
                    //gd->setIsQDZY(flag);
                    break;
                }
            }
        }
    }
}
void CStation::setQDGZFun(CQD *qd,bool flag)
{
    if(qd==NULL)
    {
        return;
    }
    //qd->setIsQDGZ(flag);
    for(int i=0;i<qd->getChildNum();i++)
    {
        for(int j=0;j<DevArray.size();j++)
        {
            if(qd->getChild(i) == DevArray[j]->getCode())
            {
                if(DevArray[j]->getDevType() == Dev_DC)
                {
                    CGDDC *gddc;
                    gddc=(CGDDC*)(DevArray[j]);
                    //gddc->setIsQDGZ(flag);
                    break;
                }
                else if(DevArray[j]->getDevType() == Dev_GD)
                {
                    CGD *gd;
                    gd=(CGD*)(DevArray[j]);
                    //gd->setIsQDGZ(flag);
                    break;
                }
            }
        }
    }
}
CQD* CStation::getQDFromGDDCNode(int code)
{
    CQD *qd;
    for(int i=0;i<DevArray.size();i++)
    {
        if(DevArray[i]->getDevType() != Dev_QD)
        {
            continue;
        }
        qd=(CQD *)(DevArray[i]);
        if(qd->getChildNum() <= 0)
        {
            continue;
        }
        for(int j=0;j<qd->getChildNum();j++)
        {
            if(qd->getChild(j) == code)
            {
                return qd;
            }
        }
    }
    return NULL;
}
int CStation::getDCNodeFromDCName(QString name)
{
    for(auto ement:DevArray)
    {
        if((ement->getDevType() == Dev_DC) && (ement->getName() == name))
        {
            return ement->getCode();
        }
    }
    return 0xffff;
}
void CStation::stationInit(int dymType)
{
}

void CStation::drawStation(QPainter *painter,long nElapsed, double nDiploid,int JmType)
{
    //setGD();
    for(int i=0;i<DevArray.size();i++)
    {
        if(DevArray[i]->getDevType() == Dev_DC)
        {
            //continue;
        }
        DevArray[i]->Draw(painter,nElapsed,nDiploid,offsetPt,JmType);
    }

    //绘制鼠标移动提示信息
    for(int i=0;i<DevArray.size();i++)
    {
        DevArray[i]->Draw_ToolTip(painter,nDiploid);
    }
}
void CStation::volloverPro()
{
    QPoint pt_Min=QPoint(0,0);
    QPoint pt_Max=QPoint(0,0);
    QPoint pt_Base=QPoint(0,0);
    for(int i=0;i<DevArray.size();i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD *)(DevArray[i]);
            if((gd->getp12().x()<pt_Min.x()) || (pt_Min.x() == 0))
            {
                pt_Min.setX(gd->getp12().x());
            }
            if((gd->getp12().y()<pt_Min.y()) || (pt_Min.y() == 0))
            {
                pt_Min.setY(gd->getp12().y());
            }
            if((gd->getp34().x()>pt_Max.x()) || (pt_Max.x() == 0))
            {
                pt_Max.setX(gd->getp34().x());
            }
            if((gd->getp34().y()<pt_Max.y()) || (pt_Max.y() == 0))
            {
                pt_Max.setY(gd->getp34().y());
            }
        }
    }
    pt_Base=QPoint(((pt_Min.x()+pt_Max.x())/2),((pt_Min.y()+pt_Max.y())/2));
    for(int i=0;i<DevArray.size();i++)
    {
        //翻转算法
        DevArray[i]->setVollover(pt_Base);
    }
}

void CStation::clearDrawGD()
{
    for (int m = 0; m < DevArray.count(); m++)
    {
        if(DevArray[m]->getDevType() != Dev_QD)
        {
            continue;
        }
        CQD *qd=(CQD *)(DevArray[m]);
        if(qd->getChildNum()>1){
            CGDDC *dc;
            for (int i=0;i<qd->getChildNum();i++)
            {
                dc=(CGDDC *)(DevArray[qd->getChild(i)]);
                dc->setDrawGD(false);
            }
        }
    }

}
//成员变量封装函数实现
void CStation::setMainGD()
{
    int i, j, k, l = -1;
    CGDDC *p, *q, *r, *s = NULL;
    for (int m = 0; m < DevArray.count(); m++)
    {
        if(DevArray[m]->getDevType() != Dev_QD)
        {
            continue;
        }
        //此处必须重新置为空，否则会导致下一个区段误判2023.8.23
        i = -1;
        j = -1;
        k = -1;
        l = -1;
        p= NULL;
        q= NULL;
        r= NULL;
        s= NULL;
        CQD *qd=(CQD *)(DevArray[m]);
        if (qd->getChildNum() > 1)
        {
            i = qd->getChild(0);
            j = qd->getChild(1);
            for (int x = 0; x < DevArray.count(); x++)
            {
                if (i == (int)DevArray[x]->getCode())
                {
                    p = (CGDDC*)(DevArray[x]);
                }
                else if (j == (int)DevArray[x]->getCode())
                {
                    q = (CGDDC*)(DevArray[x]);
                }
            }
            if (qd->getChildNum() > 2)
            {
                k = qd->getChild(2);
                for (int x = 0; x < DevArray.count(); x++)
                {
                    if (k == (int)DevArray[x]->getCode())
                    {
                        if(Dev_DC != DevArray[x]->getDevType())
                        {
                            continue;
                        }
                        r = (CGDDC*)(DevArray[x]);
                        if (p->getDWdev() == k && r->getDWdev() == i)                //道岔1，3定位相连
                        {
                            if ((p->getCQdev() == j && q->getCQdev() == i) ||        //道岔1，2岔前相连
                                (r->getCQdev() == j && q->getCQdev() == k))          //道岔2，3岔前相连
                            {
                                s = q; l = j;                             //取出道岔2
                            }
                            q = r; j = k;
                        }
                        else if (q->getDWdev() == k && r->getDWdev() == j)           //道岔2，3定位相连
                        {
                            if ((p->getCQdev() == j && q->getCQdev() == i) ||        //道岔1，2岔前相连
                                (r->getCQdev() == i && p->getCQdev() == k))          //道岔1，3岔前相连
                            {
                                s = p; l = i;                             //取出道岔1
                            }
                            p = r; i = k;
                        }
                        else if (p->getDWdev() == j && q->getDWdev() == i)           //道岔1，2定位相连
                        {
                            if ((q->getCQdev() == k && r->getCQdev() == j) ||        //道岔2，3岔前相连
                                (r->getCQdev() == i && p->getCQdev() == k))          //道岔1，3岔前相连
                            {
                                s = r; l = k;                             //取出道岔3
                            }
                        }
                        break;
                    }

                }
            }
            else if (p->getCQdev() == j && q->getCQdev() == i)               //只有两个道岔，道岔1，2岔前相连
            {
                p->setIsMainGD(1);
                q->setIsMainGD(1);
            }

            if (p->getDWdev() == j && q->getDWdev() == i)                    //道岔1，2定位相连
            {
                if ((p->getDCWZ() == DCFW) && (q->getDCWZ() == DCDW))  //道岔1反位，2定位
                {
                    if (s)
                    {
                        if (q->getCQdev() == l && s->getCQdev() == j)        //道岔2，3岔前相连
                            s->setIsMainGD(0);
                        else
                            s->setIsMainGD(1);
                    }
                    p->setIsMainGD(1);
                    q->setIsMainGD(0);
                    if(r)
                        r->setIsMainGD(0);//2023.10.7
                }
                else if ((q->getDCWZ() == DCFW) && (p->getDCWZ() == DCDW))  //道岔1定位，2反位
                {
                    if (s)
                    {
                        if (p->getCQdev() == l && s->getCQdev() == i)             //道岔2，3岔前相连
                            s->setIsMainGD(0);
                        else
                            s->setIsMainGD(1);
                    }
                    p->setIsMainGD(0);
                    q->setIsMainGD(1);
                    if(r)
                        r->setIsMainGD(1);//2023.10.7
                }
                else if ((q->getDCWZ() == DCDW) && (p->getDCWZ() == DCDW))     //道岔1定位，2定位
                {
                    if (s)
                        s->setIsMainGD(1);
                    p->setIsMainGD(1);
                    q->setIsMainGD(1);
                    if(r)
                        r->setIsMainGD(1);//2023.10.7
                }
                else if ((q->getDCWZ() == DCFW) && (p->getDCWZ() == DCFW))     //道岔1反位，2反位
                {
                    p->setIsMainGD(1);
                    q->setIsMainGD(1);
                }
                else if (s)
                    s->setIsMainGD(1);
                //clearDrawGD();
            }
        }
    }
}

void CStation::setGD()
{
    clearDrawGD();
    setMainGD();
    CGDDC *gddc;
    for (int i = 0; i < DevArray.size(); i++)
    {
        if(DevArray[i]->getDevType() != Dev_QD)
        {
            continue;
        }
        CQD *qd=(CQD *)(DevArray[i]);
        if (qd->getChildNum() > 1)
        {
            for (int k = 0; k < qd->getChildNum(); k++)
            {
                for (int j = 0; j < DevArray.size(); j++)
                {
                    if (qd->getChild(k) == (int)DevArray[j]->getCode())
                    {
                        gddc = (CGDDC*)(DevArray[j]);
                        if (gddc->getIsMainGD() == true)
                        {
                            int nNext = gddc->setDrawGD(true);
                            int m = 1;
                            while (nNext != -1) //直到后继为空
                            {
                                m++;
                                for (int l = 0; l < DevArray.size(); l++)
                                {
                                    if (nNext == (int)DevArray[l]->getCode())
                                    {
                                        if(DevArray[l]->getDevType() == Dev_DC)
                                        {
                                            gddc = (CGDDC *)DevArray[l];
                                            nNext = gddc->setDrawGD(true);
                                            break;
                                        }
                                    }
                                }
                                if (m >= 3)
                                    break;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}
void CStation::setStationName(QString name)
{
    m_strStationName = name;
}
QString CStation::getStationName()
{
    return m_strStationName;
}
void CStation::setStationID(unsigned int id)
{
    m_nStationID=id;
}
unsigned int CStation::getStationID()
{
    return m_nStationID;
}
void CStation::setStationType(unsigned int type)
{
    m_nStationType = type;
}
unsigned int CStation::getStationType()
{
    return m_nStationType;
}
void CStation::setLSProducer(QString producer)
{
    m_strLS_Producer = producer;
}
QString CStation::getLSProducer()
{
    return m_strLS_Producer;
}
void CStation::setIsGT(bool flag)
{
    isGT = flag;
}
bool CStation::getIsGT()
{
    return isGT;
}
void CStation::setIsZCZX(bool flag)
{
    isZCZX=flag;
}
bool CStation::getIsZCZX()
{
    return isZCZX;
}
void CStation::setOffSet(QPoint offset)
{
    offsetPt.setX(offset.x());
    offsetPt.setY(offset.y());
}
QPoint CStation::getOffSet()
{
    return offsetPt;
}
void CStation::setCTCSoftID(int x)
{
    m_nCTCSoftID = x;
}
int CStation::getCTCSoftID()
{
    return m_nCTCSoftID;
}
void CStation::setLSProcessID(int x)
{
    m_nLS_ProcessID=x;
}
int CStation::getLSProcessID()
{
    return m_nLS_ProcessID;
}
void CStation::setLSSoftID(int x)
{
    m_nLS_SoftID=x;
}
int CStation::getLSSoftID()
{
    return m_nLS_SoftID;
}
//void CStation::setLSProcessFlag(bool flag)
//{
//    isLSProcessFlag=flag;
//}
//bool CStation::getLSProcessFlag()
//{
//    return isLSProcessFlag;
//}
void CStation::setXLMNJSoftID(int x)
{
    m_nXLMNJSoftID=x;
}
int CStation::getXLMNJSoftID()
{
    return m_nXLMNJSoftID;
}
void CStation::setLSComState(bool flag)
{
    isLSComState=flag;
}
bool CStation::getLSComState()
{
    return isLSComState;
}
void CStation::setLSComTime(int x)
{
    LSComTime=x;
}
int CStation::getLSComTime()
{
    return LSComTime;
}
