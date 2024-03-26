#include "mystation.h"

#include <QTextCodec>
#include <QDebug>
#include <windows.h>
#include "Log/log.h"
#include <QTimer>

#pragma execution_character_set("utf-8")

MyStation::MyStation()
{
    installEventFilter(this);
    startTimer(1000);
    m_pDataAccess = nullptr;

    isThreadStart = false;
    ticksOfThread = 0;

    //通信端口初始化
    lsAddr = "";
    lsPort = 0;
    m_nTimeLSBreak = commBreakTimes/2;//0
    isLSConnected = false;
    for(int i=0; i<10; i++)
    {
        ctcAddr[i] = "";
        ctcPort[i] = 0;
        m_nTimeCTCBreak[i] = commBreakTimes;
        isCTCConnected[i] = false;
        ctcAddr2[i] = "";
        ctcPort2[i] = 0;
        m_nTimeCTCBreak2[i] = commBreakTimes;
        isCTCConnected2[i] = false;

        zxbAddr[i] = "";
        zxbPort[i] = 0;
        m_nTimeBoardBreak[i] = 0;
        isBoardConnected[i] = false;
        zxbAddr2[i] = "";
        zxbPort2[i] = 0;
        m_nTimeBoardBreak2[i] = 0;
        isBoardConnected2[i] = false;

        jkAddr[i] = "";
        jkPort[i] = 0;
        m_nTimeJKBreak[i] = 0;
        isJKConnected[i] = false;
        jkAddr2[i] = "";
        jkPort2[i] = 0;
        m_nTimeJKBreak2[i] = 0;
        isJKConnected2[i] = false;

        zxtAddr[i] = "";
        zxtPort[i] = 0;
        m_nTimeZXTBreak[i] = 0;
        isZXTConnected[i] = false;
        zxtAddr2[i] = "";
        zxtPort2[i] = 0;
        m_nTimeZXTBreak2[i] = 0;
        isZXTConnected2[i] = false;
    }
    trainningAddr = "";
    trainningPort = 0;
    //车站配置信息初始化
    StaConfigInfo.YXJLBeginNum = 0;
    StaConfigInfo.JCKCount = 0;
    for(int i=0; i<40; i++)
    {
        StaConfigInfo.JFCKInfo[i].nJCKCode = 0;
        StaConfigInfo.JFCKInfo[i].nFCKCode = 0;
    }
    StaConfigInfo.BTJLCount = 0;
    StaConfigInfo.bXHDDSBJStatus = false;
    //StaConfigInfo.HaveCLPZFlag = false;
    StaConfigInfo.bZDBSLightExState = false;
    StaConfigInfo.bQJAllLock = false;
    StaConfigInfo.bChgModeNeedJSJ = false;
    StaConfigInfo.RouteDeleteSeconds = 0;

    ModalSelect.nAgrRequest = false;	//同意转换
    ModalSelect.nPlanCtrl = false; //计划控制
    ModalSelect.nStateSelect = 0;	//0 手工排路 1 按图排路
    ModalSelect.nModeState = 0;		//当前模式-0 中心控制 1 车站控制 2 分散自律
    ModalSelect.nRequestMode = 0;	//请求模式-0 中心控制 1 车站控制 2 分散自律(请求转换的模式)   //1 中心控制 2 车站控制 3 分散自律(请求转换的模式) （0为默认值，处理不便）
    nOldModeState = 0;//上一次的控制模式
    m_bModeChanged = false;
    m_nFCZKMode = false; //非常站控
    RoutePermit = 0;
    AutoSignStage = true;

    m_nDCSXAllLock = false;
    m_nDCXXAllLock = false;
    m_nPDJS180 = 0;
    m_nSRJ180 = 0;//S人解180s
    m_nSRJ30 = 0;//S人解30s
    m_nXRJ180 = 0;//X人解180s
    m_nXRJ30 = 0;//X人解30s

    m_bRouteStateCheckXHD = false;
    bInitGDNodeByConfig = false;

    m_nCountDown = 0;
    m_bBeginCount = false;

    //得赋初值
    this->m_LastTimeOfRouteDo = QDateTime::currentDateTime().addDays(-1);
}
MyStation::~MyStation()
{
    isThreadStart = false;
    //发信结束信号
    emit endWorkSignal();
}
//设置数据访问接口
void MyStation::setDataAccess(DataAccess *_dataAccess)
{
    m_pDataAccess = _dataAccess;
}

//通信更新判断
void MyStation::UpdateConnStatus()
{
    int maxInt = 99999999;
    if(m_nTimeLSBreak<commBreakTimes)//3秒
    {
//            if(!isLSConnected)
//            {
//                QLOG_INFO()<<QString("%1与联锁通信成功！").arg(this->getStationName());
//            }
        //isLSConnected = true;
    }
    else
    {
        //只发送一次
        if(m_nTimeLSBreak==commBreakTimes && isLSConnected)
        {
            this->sendWarningMsgToCTC(1,1,"自律机与联锁A机通信中断");
            QLOG_WARN()<<QString("%1与联锁通信中断！").arg(this->getStationName());
            //this->ClearStationDevStatus();
            this->setDevStateToSafe();
        }
        isLSConnected = false;
    }
    m_nTimeLSBreak++;
    //防溢出处理
    if(m_nTimeLSBreak > maxInt)
    {
        m_nTimeLSBreak=commBreakTimes+1;
    }

    for(int i=0; i<10; i++)//10
    {
        //CTC-udp通道
        if(m_nTimeCTCBreak[i]<commBreakTimes)//3秒
        {
            //isCTCConnected[i] = true;
        }
        else
        {
            //只发送一次
            if(m_nTimeCTCBreak[i]==commBreakTimes && isCTCConnected[i])
            {
                QLOG_WARN()<<QString("%1服务端与CTC通信1中断！").arg(this->getStationName());
                //清除CTC操作权限
                this->ResetRoutePermit(1);
            }
            isCTCConnected[i] = false;
            ctcAddr[i] = "";//IP地址
            ctcPort[i] = 0;//端口
        }
        m_nTimeCTCBreak[i]++;
        //防溢出处理
        if(m_nTimeCTCBreak[i] > maxInt)
        {
            m_nTimeCTCBreak[i]=commBreakTimes+1;
        }
        //qDebug()<<QString("m_nTimeCTCBreak[%1]=%2,Connected=%3").arg(i)
        //        .arg(QString::number(m_nTimeCTCBreak[i])).arg(isCTCConnected[i]);

        //CTC-tcp通道
        if(m_nTimeCTCBreak2[i]<commBreakTimes)//3秒
        {
            //isCTCConnected2[i] = true;
            //qDebug()<<QString("%1服务端与CTC通信2-%2计数器=%3").arg(this->getStationName()).arg(i).arg(m_nTimeCTCBreak2[i]);
        }
        else
        {
            //只发送一次
            if(m_nTimeCTCBreak2[i]==commBreakTimes && isCTCConnected2[i])
            {
                QLOG_WARN()<<QString("%1服务端与CTC通信2中断！").arg(this->getStationName());
            }
            isCTCConnected2[i] = false;
            ctcAddr2[i] = "";//IP地址
            ctcPort2[i] = 0;//端口
        }
        m_nTimeCTCBreak2[i]++;
        //防溢出处理
        if(m_nTimeCTCBreak2[i] > maxInt)
        {
            m_nTimeCTCBreak2[i]=commBreakTimes+1;
        }
        //qDebug()<<QString("m_nTimeCTCBreak[%1]=%2,Connected=%3").arg(i)
        //        .arg(QString::number(m_nTimeCTCBreak[i])).arg(isCTCConnected[i]);

        //占线板-udp通道
        if(m_nTimeBoardBreak[i]<commBreakTimes)//3秒
        {
            //isBoardConnected[i] = true;
        }
        else
        {
            //只发送一次
            if(m_nTimeBoardBreak[i]==commBreakTimes && isBoardConnected[i])
            {
                QLOG_WARN()<<QString("%1服务端与占线板通信中断！").arg(this->getStationName());
                //清除占线板操作权限
                this->ResetRoutePermit(2);
            }
            isBoardConnected[i] = false;
            zxbAddr[i] = "";//IP地址
            zxbPort[i] = 0;//端口
        }
        m_nTimeBoardBreak[i]++;
        //防溢出处理
        if(m_nTimeBoardBreak[i] > maxInt)
        {
            m_nTimeBoardBreak[i]=commBreakTimes+1;
        }

        //JK集控-udp通道 集控
        if(m_nTimeJKBreak[i]<commBreakTimes)//3秒
        {
            //isJKConnected[i] = true;
        }
        else
        {
            //只发送一次
            if(m_nTimeJKBreak[i]==commBreakTimes && isJKConnected[i])
            {
                QLOG_WARN()<<QString("%1服务端与集控通信1中断！").arg(this->getStationName());
                //清除CTC操作权限
                //this->ResetRoutePermit(1);
            }
            isJKConnected[i] = false;
            jkAddr[i] = "";//IP地址
            jkPort[i] = 0;//端口
        }
        m_nTimeJKBreak[i]++;
        //防溢出处理
        if(m_nTimeJKBreak[i] > maxInt)
        {
            m_nTimeJKBreak[i]=commBreakTimes+1;
        }
        //qDebug()<<QString("m_nTimeJKBreak[%1]=%2,Connected=%3").arg(i)
        //        .arg(QString::number(m_nTimeJKBreak[i])).arg(isJKConnected[i]);

        //zxt占线图-udp通道 集控
        if(m_nTimeZXTBreak[i]<commBreakTimes)//3秒
        {
            //isZXTConnected[i] = true;
        }
        else
        {
            //只发送一次
            if(m_nTimeZXTBreak[i]==commBreakTimes && isZXTConnected[i])
            {
                QLOG_WARN()<<QString("%1服务端与占线图通信1中断！").arg(this->getStationName());
            }
            isZXTConnected[i] = false;
            zxtAddr[i] = "";//IP地址
            zxtPort[i] = 0;//端口
        }
        m_nTimeZXTBreak[i]++;
        //防溢出处理
        if(m_nTimeZXTBreak[i] > maxInt)
        {
            m_nTimeZXTBreak[i]=commBreakTimes+1;
        }
        //qDebug()<<QString("m_nTimeJKBreak[%1]=%2,Connected=%3").arg(i)
        //        .arg(QString::number(m_nTimeJKBreak[i])).arg(isJKConnected[i]);
    }
}

//线程关联槽
void MyStation::startWorkSlot()
{
    int maxInt = 99999999;
    ticksOfThread = 0;
    while (isThreadStart)
    {
        //(1)检查区域
        QT_TRY
        {
            //互斥量，作用域结束后自动解锁
            //QMutexLocker locker(&Mutex);

            //throw(EXCEP_ZERO);//(2)必需主动抛出异常

            ticksOfThread++;
            //防溢出处理
            if(ticksOfThread > maxInt)
            {
                ticksOfThread=0;
            }
            //qDebug()<<QString("StationId=%1,ticksOfThread=%2").arg(QString::number(this->getStationID())).arg(QString::number(ticksOfThread));

            //********通信判断 start*******
            UpdateConnStatus();
            //********通信判断 end*******

            //线程按照休眠250ms计算的话，1000ms发送一次
            if(ticksOfThread%4==0)
            {
                //向教师机发送心跳信息
                this->sendHeartBeatToTeacher();

                //检查列车在限速区域的运行(1s发送一次)
                this->CheckTrainInLimitSpeed();
            }

            //命令清除倒计时计算
            this->computeCmdCountDown();

            //更新车次活跃信息及股道车次
            this->UpdateStationCheCiInfo(ticksOfThread);

            //合并车次（CTC车次和联锁车次）
            this->mergeCheci();
            //向CTC发送车次
            this->sendCheciToCTC();
            //自动处理CTC修改的计划
            if(ManSignRouteSynType == 0)
                this->AutoRecvModifiedPlan();
            //进路序列各项逻辑处理
            //自动删除已出清进路序列
            this->AutoDeleleFinishedTrainRoute();
            //自动设置进路序列的执行状态
            this->AutoCheckAndSetRouteOrderState();
            //自动检查和设置计划的完成标志
            this->AutoCheckTrafficExecuteFlag();
            //自动排序进路序列
            this->AutoSortRouteOrder();
            //发送进路序列执行指令给联锁
            this->SendOrderToInterlockSys();
            //检查股道车次及信号状态，并发送发车命令
            this->CheckGdTrainStatus();
            //更新进路预告信息
            this->UpdateRoutePreWndInfo();
            //列车接近进路开放判断
            this->CheckTrainCloseToJZXHD();
            //更新股道防溜
            this->SetAllGDAntiSlip();

            //线程按照休眠250ms计算的话，500ms执行一次
            if(ticksOfThread%2==0)
            {
                //自动触发邻站模拟的进路序列
                AutoTouchLZMNRouteOrder();

                //与联锁建立通信时才向CTC+占线板发送站场状态，即通信卡控授权
                if(this->isLSConnected)
                {
                    //向前端发送新的站场状态
                    this->packStationStatusToCTC();
                }
                //qDebug()<<"---------------"<<ticksOfThread;
            }

            QThread::msleep(250);//100
        }
        //(3)捕获异常(所有异常)
        QT_CATCH(...){
            qDebug()<<QString("%1, thread catch exception!").arg(this->getStationName());
        }
    }

    //发信结束信号
    emit endWorkSignal();
    qDebug()<<QString("Send endWorkSignal");
}
//绘制站场
void MyStation::draw(QPainter *painter, long nElapsed, double nDiploid)
{
    this->drawStation(painter,nElapsed,nDiploid,0x55);
    this->drawMyStation(painter,nElapsed,nDiploid);
}
//绘制CTC站场
void MyStation::drawMyStation(QPainter *painter, long nElapsed, double nDiploid)
{
    for(int i=0;i<m_ArrayTrain.size();i++)
    {
        QPoint offsetPt = QPoint(0,0);
        m_ArrayTrain[i]->Draw(painter,nElapsed,nDiploid,offsetPt);
    }
}
//向教师机发送心跳信息
void MyStation::sendHeartBeatToTeacher()
{
    int nLength = 60;
    QByteArray dataArray;
    dataArray.append(nLength, char(0));//添加nLength个字节并全部置零
    dataArray[9] = 0x23;//分类信息码
    dataArray[10] = CTC_TYPE_CTC2_CASCO;//CTC制式
    //打包数据
    packHeadAndTail(&dataArray, nLength);
    //信号-发送数据
    emit sendDataToTeacherSignal(this, dataArray, nLength);
}
//发送更新数据消息（给所有连接终端-CTC、占线板）
void MyStation::sendUpdateDataMsg(MyStation *pStation, int _type, int currCtcIndex, int currZxbIndex)
{
    //信号-发送数据
    emit sendUpdateDataMsgSignal(pStation, _type, currCtcIndex, currZxbIndex);
}
//读取站场设备数据
bool MyStation::readStationDev(QString fileName, MyStation *pMyStation)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.isFile())
    {
        QLOG_ERROR()<<QString::fromLocal8Bit("File %1 not existed!").arg(fileName);
        return false;
    }

    QFile file(fileName);
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //QString msg = "打开文件失败+"+fileName;
        //QMessageBox::warning(NULL,VERSION,msg);
        //QLOG_ERROR()<<msg;
        QLOG_ERROR()<<QString("Open file failed! %1").arg(fileName);
        return false;
    }
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString str(line);
        QString getString;
        if(str.left(4) == "####")
        {
            break;
        }
        //忽略注释和空格
        else if(str.left(2) == "//" || str.trimmed() == "")
        {
            continue;
        }
        else if(str.left(4) == "##XH")
        {
            CXHD *xhd = new CXHD();
            while(!file.atEnd())
            {
                QByteArray line1 = file.readLine();
                QString m_str(line1);
                if (m_str.left(12)=="xhd.m_nType=")
                {
                    xhd->setType((m_str.mid(12,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(14)=="xhd.m_strName=")
                {
                    xhd->setName(m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-3));
                }
                else if (m_str.left(12)=="xhd.m_nCode=")
                {
                    getString = m_str.mid(12,m_str.indexOf(";")-m_str.indexOf("=")-1);
                    int nCodee = StringToHex(getString);//"0x10"
                    xhd->setCode(nCodee);
                }
                else if (m_str.left(16)=="xhd.Module_Code=")
                {
                    getString = m_str.mid(16,m_str.indexOf(";")-m_str.indexOf("=")-1);
                    int nCodee = StringToHex(getString);//"0x10"
                }
                else if (m_str.left(11)=="xhd.center=")
                {
                    xhd->setCenterPt(StringToPoint(m_str.mid(11,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(15)=="xhd.m_nXHDType=")
                {
                    xhd->setXHDType(m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1));
                }
                else if (m_str.left(15)=="xhd.m_textRect=")
                {
                    xhd->setTextRect(StringToRect(m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(10)=="xhd.m_nSX=")
                {
                    xhd->setSX((m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(15)=="xhd.SignalType=")
                {
                    xhd->setSignalType((m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(17)=="xhd.D_B_C_Signal=")
                {
                }
                else if (m_str.left(17)=="xhd.DC_LC_Signal=")
                {
                }
                else if (m_str.left(13)=="xhd.safeLamp=")
                {
                    xhd->setSafeLamp(m_str.mid(13,m_str.indexOf(";")-m_str.indexOf("=")-1));
                }
                else if (m_str.left(11)=="xhd.isHigh=")
                {
                    xhd->setIsHigh((m_str.mid(11,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(11)=="xhd.isYDSD=")
                {
                    xhd->setIsYDSD((m_str.mid(11,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(4)=="ADD_")
                {
                    break;
                }
            }
            xhd->XHDInit(0x55);
            pMyStation->DevArray.append(xhd);
            pMyStation->XhNum++;
        }
        else if(str.left(4)=="##DC")
        {
            CGDDC *gddc=new CGDDC();
            while(!file.atEnd())
            {
                QByteArray line1 = file.readLine();
                QString m_str(line1);
                if (m_str.left(13)=="gddc.m_nType=")
                {
                    gddc->setType((m_str.mid(13,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(15)=="gddc.m_strName=")
                {
                    gddc->setName(m_str.mid(16,m_str.indexOf(";")-m_str.indexOf("=")-3));
                }
                else if (m_str.left(13)=="gddc.m_nCode=")
                {
                    getString = m_str.mid(13,m_str.indexOf(";")-m_str.indexOf("=")-1);
                    int nCodee = StringToHex(getString);//例"0x10"
                    gddc->setCode(nCodee);
                }
                else if (m_str.left(15)=="gddc.m_nQDCode=")
                {
                    getString = m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1);
                    int nCodee = StringToHex(getString);//例"0x10"
                    gddc->setQDCode(nCodee);
                }
                else if (m_str.left(13)=="gddc.m_nCxjy=")
                {
                    gddc->setCxjy((m_str.mid(13,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(15)=="gddc.m_nDSCode=")
                {
                }
                else if (m_str.left(17)=="gddc.Module_Code=")
                {
                }
                else if (m_str.left(17)=="gddc.m_nQDMKCode=")
                {
                }
                else if (m_str.left(10)=="gddc.m_nZ=")
                {
                    gddc->setZ((m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(11)=="gddc.m_nSX=")
                {
                    gddc->setSX((m_str.mid(11,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(12)=="gddc.m_nJyj=")
                {
                    gddc->setJyj((m_str.mid(12,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(17)=="gddc.oneToMore=")
                {
                }
                else if (m_str.left(16)=="gddc.m_textRect=")
                {
                    gddc->setTextRect(StringToRect(m_str.mid(16,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(8)=="gddc.p1=")
                {
                    gddc->setp1(StringToPoint(m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(8)=="gddc.p2=")
                {
                    gddc->setp2(StringToPoint(m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(8)=="gddc.p3=")
                {
                    gddc->setp3(StringToPoint(m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(8)=="gddc.p4=")
                {
                    gddc->setp4(StringToPoint(m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(8)=="gddc.p5=")
                {
                    gddc->setp5(StringToPoint(m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(8)=="gddc.p6=")
                {
                    gddc->setp6(StringToPoint(m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(9)=="gddc.p12=")
                {
                    gddc->setp12(StringToPoint(m_str.mid(9,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(9)=="gddc.p34=")
                {
                    gddc->setp34(StringToPoint(m_str.mid(9,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(9)=="gddc.p56=")
                {
                    gddc->setp56(StringToPoint(m_str.mid(9,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(12)=="gddc.center=")
                {
                    gddc->setCenterPt(StringToPoint(m_str.mid(12,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(10)=="gddc.pZ12=")
                {
                    gddc->setpz12(StringToPoint(m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(10)=="gddc.pZ34=")
                {
                    gddc->setpz34(StringToPoint(m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(10)=="gddc.pZ56=")
                {
                    gddc->setpz56(StringToPoint(m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(15)=="gddc.m_bMainGD=")
                {
                    gddc->setIsMainGD((m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(10)=="gddc.m_nQ=")
                {
                    gddc->setCQdev((m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(10)=="gddc.m_nD=")
                {
                    gddc->setDWdev((m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(10)=="gddc.m_nF=")
                {
                    gddc->setFWdev((m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(4)=="ADD_")
                {
                    break;
                }
            }
            gddc->GDDCInit(0x55);
            pMyStation->DevArray.append(gddc);
            pMyStation->DcNum++;
        }
        else if(str.left(4)=="##GD")
        {
            CGD *gd=new CGD();
            while(!file.atEnd())
            {
                QByteArray line1 = file.readLine();
                QString m_str(line1);
                if (m_str.left(11)=="gd.m_nType=")
                {
                    gd->setType((m_str.mid(11,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(13)=="gd.m_strName=")
                {
                    gd->setName(m_str.mid(14,m_str.indexOf(";")-m_str.indexOf("=")-3));
                }
                else if (m_str.left(11)=="gd.m_nCode=")
                {
                    getString = m_str.mid(11,m_str.indexOf(";")-m_str.indexOf("=")-1);
                    int nCodee = StringToHex(getString);//例"0x10"
                    gd->setCode(nCodee);
                }
                else if (m_str.left(15)=="gd.Module_Code=")
                {
                }
                else if (m_str.left(11)=="gd.GD_Type=")
                {
                    gd->setGDType(m_str.mid(11,m_str.indexOf(";")-m_str.indexOf("=")-1));
                }
                else if (m_str.left(8)=="gd.m_nZ=")
                {
                    gd->setZ((m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(9)=="gd.m_nSX=")
                {
                    gd->setSX((m_str.mid(9,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(14)=="gd.m_textRect=")
                {
                    gd->setTextRect(StringToRect(m_str.mid(14,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(6)=="gd.p1=")
                {
                    gd->setp1(StringToPoint(m_str.mid(6,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(6)=="gd.p2=")
                {
                    gd->setp2(StringToPoint(m_str.mid(6,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(6)=="gd.p3=")
                {
                    gd->setp3(StringToPoint(m_str.mid(6,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(6)=="gd.p4=")
                {
                    gd->setp4(StringToPoint(m_str.mid(6,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(7)=="gd.p12=")
                {
                    gd->setp12(StringToPoint(m_str.mid(7,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(7)=="gd.p34=")
                {
                    gd->setp34(StringToPoint(m_str.mid(7,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(8)=="gd.pz12=")
                {
                    gd->setpz12(StringToPoint(m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(8)=="gd.pz34=")
                {
                    gd->setpz34(StringToPoint(m_str.mid(8,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(10)=="gd.center=")
                {
                    gd->setCenterPt(StringToPoint(m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(12)=="gd.QJXHName=")
                {
                }
                else if (m_str.left(10)=="gd.QJXHPt=")
                {
                }
                else if (m_str.left(4)=="ADD_")
                {
                    break;
                }
            }
            gd->GDInit(0x55);
            pMyStation->DevArray.append(gd);
            pMyStation->GdNum++;

            //股道接点自动获取 //增加无岔区段，用于组合进路
            if(JJ_QD != gd->getGDType())//if(GD_QD == gd->getGDType())
            {
                if(!bInitGDNodeByConfig)
                {
                    StationGDNode gdNode;
                    gdNode.bIgnoreLeftSXX = false;
                    gdNode.bIgnoreRightSXX = false;
                    gdNode.nCode = gd->getCode();
                    gdNode.strGDName = gd->getName();
                    vectStationGDNode.append(gdNode);
                }
            }
        }
        else if(str.left(8)=="##BuRect")
        {
            CTG *tg=new CTG();
            while(!file.atEnd())
            {
                QByteArray line1 = file.readLine();
                QString m_str(line1);
                if (m_str.left(15)=="BuRect.m_nType=")
                {
                    tg->setType((m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(17)=="BuRect.m_strName=")
                {
                    tg->setName(m_str.mid(18,m_str.indexOf(";")-m_str.indexOf("=")-3));
                }
                else if (m_str.left(15)=="BuRect.m_nCode=")
                {
                    //tg->setCode((m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                    getString = m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1);
                    int nCodee = StringToHex(getString);//例"0x10"
                    tg->setCode(nCodee);
                }
                else if (m_str.left(10)=="BuRect.p1=")
                {
                    tg->setCenterPt(StringToPoint(m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(18)=="BuRect.m_textRect=")
                {
                    tg->setTextRect(StringToRect(m_str.mid(18,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(14)=="BuRect.m_nTZB=")
                {
                }
                else if (m_str.left(4)=="ADD_")
                {
                    break;
                }
            }
            pMyStation->DevArray.append(tg);
        }
        else if(str.left(4)=="##TX")
        {
            CText *text=new CText();
            while(!file.atEnd())
            {
                QByteArray line1 = file.readLine();
                //QString m_str(line1);
                QString m_str = codec->toUnicode(line1);
                if (m_str.left(14)=="txt.m_strName=")
                {
                    text->setName(m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-3));
                }
                else if (m_str.left(13)=="txt.m_DCname=")
                {
                    text->m_DCname=m_str.mid(14,m_str.indexOf(";")-m_str.indexOf(";")-3);
                    text->setGLDCNode(pMyStation->getDCNodeFromDCName(text->m_DCname));
                }
                else if (m_str.left(12)=="txt.m_nSize=")
                {
                    text->setTextSize((m_str.mid(12,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(15)=="txt.m_textRect=")
                {
                    text->setTextRect(StringToRect(m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(10)=="txt.color=")
                {
                    text->setTextColor(m_str.mid(10,m_str.indexOf(";")-m_str.indexOf("=")-1));
                }
                else if (m_str.left(4)=="ADD_")
                {
                    if(text->getTextSize() > 30)
                    {
                        pMyStation->setStationName(text->getName());
                    }
                    break;
                }
            }
            text->TextInit();
            pMyStation->DevArray.append(text);
        }
        else if(str.left(5)=="##JTX")
        {
            CJTX *jtx=new CJTX();
            while(!file.atEnd())
            {
                QByteArray line1 = file.readLine();
                QString m_str(line1);
                if (m_str.left(13)=="jtx.JTX_Type=")
                {
                    jtx->setJTXType((m_str.mid(13,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(15)=="jtx.m_strName=")
                {
                    jtx->setName(m_str.mid(16,m_str.indexOf(";")-m_str.indexOf("=")-3));
                }
                else if (m_str.left(12)=="jtx.m_nType=")
                {
                    jtx->setType((m_str.mid(12,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
                }
                else if (m_str.left(7)=="jtx.p1=")
                {
                    jtx->setp1(StringToPoint(m_str.mid(7,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(7)=="jtx.p2=")
                {
                    jtx->setp2(StringToPoint(m_str.mid(7,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(7)=="jtx.p3=")
                {
                    jtx->setp3(StringToPoint(m_str.mid(7,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(7)=="jtx.p4=")
                {
                    jtx->setp3(StringToPoint(m_str.mid(7,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(11)=="jtx.center=")
                {
                    jtx->setCenterPt(StringToPoint(m_str.mid(11,m_str.indexOf(";")-m_str.indexOf("=")-1)));
                }
                else if (m_str.left(4)=="ADD_")
                {
                    break;
                }
            }
            jtx->JTXInit();
            pMyStation->DevArray.append(jtx);
        }
        else if(str.left(4)=="##QD")
        {
            CQD *qd=new CQD();
            QByteArray line1 = file.readLine();
            QString m_str(line1);
            if (m_str.left(15)=="qd.m_nChildNum=")
            {
                qd->setChildNum((m_str.mid(15,m_str.indexOf(";")-m_str.indexOf("=")-1)).toInt());
            }
            for (int j=0;j<qd->getChildNum();j++)
            {
                QByteArray line2 = file.readLine();
                QString child_str(line2);
                QString subStr=QString("qd.m_nChild[%1]=").arg(j);
                if (child_str.left(15)==subStr)
                {
                    qd->setChild(j,(child_str.mid(15,child_str.indexOf(";")-child_str.indexOf("=")-1)).toInt());
                }
            }
            pMyStation->DevArray.append(qd);
            pMyStation->QdNum++;
        }
    }
    //关闭文件
    file.close();
    return true;
}
//读取站场配置数据
bool MyStation::readStationConfig(QString fileName, MyStation *pMyStation)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.isFile())
    {
        QLOG_ERROR()<<QString::fromLocal8Bit("File %1 not existed!").arg(fileName);
        return false;
    }

    //    //配置文件ini读取
    //    QSettings* settings = new QSettings(fileName, QSettings::IniFormat);
    //    // 指定为中文
    //    settings->setIniCodec("UTF-8");

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString msg = "Open file failed！"+fileName;
        //QMessageBox::warning(NULL,VERSION,msg);
        QLOG_ERROR()<<msg;
        return false;
    }

    QStringList strArr;
    int count = 0;

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        //QString m_str(line);
        QString m_str = codec->toUnicode(line);
        QString getString;
        if(m_str.left(4) == "####")
        {
            break;
        }
        //忽略注释和空格
        else if(m_str.left(2) == "//" || m_str.trimmed() == "")
        {
            continue;
        }
        else if (m_str.left(11) == "ISHAVEPXRJ:")
        {
            getString = m_str.mid(11, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            pMyStation->StaConfigInfo.isHavePXRJ = getString=="1"?true:false;
        }
        else if (m_str.left(14) == "STATIONSXLORR:")
        {
            //站场方向
            getString = m_str.mid(14, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            pMyStation->StaConfigInfo.bStaSXLORR = getString=="1"?true:false;
        }
        else if (m_str.left(15) == "CHGMODENEEDJSJ:")
        {
            //读取配置-控制模式转换
            getString = m_str.mid(15, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            pMyStation->StaConfigInfo.bChgModeNeedJSJ = getString.toInt() == 1 ? true : false;
        }
        else if (m_str.left(14) == "XHJDSDSSTATUS:")
        {
            //读取配置-信号机灯丝报警
            getString = m_str.mid(14, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            pMyStation->StaConfigInfo.bXHDDSBJStatus = getString.toInt() == 1 ? true : false;
        }
//        else if (m_str.left(9) == "CLPZFLAG:")
//        {
//            //场联配置
//            getString = m_str.mid(9, m_str.indexOf(";") - m_str.indexOf(":") - 1);
//            pMyStation->StaConfigInfo.HaveCLPZFlag = getString.toInt() == 1 ? true : false;
//        }
        else if (m_str.left(12) == "ZDBSDZTSHOW:")
        {
            //自动闭塞指示灯扩展状态
            getString = m_str.mid(12, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            pMyStation->StaConfigInfo.bZDBSLightExState = getString.toInt() == 1 ? true : false;
        }
        else if (m_str.left(10) == "QJALLLOCK:")
        {
            //区间封锁单个或全部
            getString = m_str.mid(10, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            pMyStation->StaConfigInfo.bQJAllLock = getString.toInt() == 1 ? true : false;
        }
        else if (m_str.left(5) == "ZDBS:")
        {
            //自动闭塞
            getString = m_str.mid(5, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            AutomaticBlock* pAutomaticBlock = new AutomaticBlock;
            pAutomaticBlock->strJuncXHD = getString.trimmed();
            pMyStation->vectAutomaticBlock.append(pAutomaticBlock);
        }
        else if (m_str.left(6) == "BZDBS:")
        {
            //半自动闭塞
            getString = m_str.mid(6, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            count = StringSplit(getString, "|", strArr);
            if(2 == count)
            {
                SemiAutomaticBlock* pSemiAutomaticBlock = new SemiAutomaticBlock;
                pSemiAutomaticBlock->strJuncXHD = strArr[0].trimmed();
                bool ok;
                pSemiAutomaticBlock->code = strArr[1].trimmed().toInt(&ok,16);//以16进制读入
                pMyStation->vectSemiAutomaticBlock.append(pSemiAutomaticBlock);
            }
        }
        else if (m_str.left(7) == "JIZHOU:")
        {
            //计轴
            getString = m_str.mid(7, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            AxleCounter* pAxleCounter = new AxleCounter;
            pAxleCounter->strJuncXHD = getString.trimmed();
            pMyStation->vectAxleCounter.append(pAxleCounter);
        }
        else if (m_str.left(3) == "CL:")
        {
            //场联
            getString = m_str.mid(3, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            InterfieldConnection* pCL = new InterfieldConnection;
            pCL->strJuncXHD = getString.trimmed();
            pMyStation->vectInterfieldConnection.append(pCL);
        }
        else if (m_str.left(8) == "JCK|FCK:")
        {
            getString = m_str.mid(8, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            QStringList str;
            int c = StringSplit(getString, "|", str);
            if(c==3)
            {
                int j = pMyStation->StaConfigInfo.JCKCount;
                pMyStation->StaConfigInfo.JFCKInfo[j].strJCKName = str[0];
                pMyStation->StaConfigInfo.JFCKInfo[j].strFCKName = str[1];
                QString name = str[2];
                pMyStation->StaConfigInfo.JFCKInfo[j].strDirectSta = name;
                pMyStation->StaConfigInfo.JCKCount++;
            }
            else if(4==c)
            {
                int j = pMyStation->StaConfigInfo.JCKCount;
                pMyStation->StaConfigInfo.JFCKInfo[j].strJCKName = str[0];
                pMyStation->StaConfigInfo.JFCKInfo[j].strFCKName = str[1];
                QString name = str[2];
                pMyStation->StaConfigInfo.JFCKInfo[j].strDirectSta = name;
                pMyStation->StaConfigInfo.JFCKInfo[j].nLZStationId = str[3].toInt();//new
                pMyStation->StaConfigInfo.JCKCount++;
            }
        }
        else if (m_str.left(10) == "ALONEXHAN:")
        {
            getString = m_str.mid(10, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            QStringList str;
            int c = StringSplit(getString, "|", str);
            if(5==c)
            {
                XhBtn* pXhBtn = new XhBtn;
                //pXhBtn->m_pCenter = StringToCPoint(str[0]);
                pXhBtn->m_strName = str[1];
                pXhBtn->m_nCode = str[2].toInt();
                if(str[3] == "DCAN")
                {
                    pXhBtn->m_nANTYPE = DCAN;
                }
                else if(str[3] == "LCAN")
                {
                    pXhBtn->m_nANTYPE = LCAN;
                }
                //pXhBtn->m_bNameUp = str[4].toInt() == 1 ? true : false;
                vectXhBtn.append(pXhBtn);
            }
        }
        else if(m_str.left(13) == "YXJLXHJGROUP:")
        {
            getString = m_str.mid(13, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            //getString = S|{6G,4G@AP1A,XN,X,XY}|{IIG,IG,IIIG,5G@XN,X,XY}
            QString ch = "|";
            QStringList strArr;
            int c = StringSplit(getString, ch, strArr);
            if (2 <= c)
            {
                int j = pMyStation->StaConfigInfo.YXJLBeginNum;
                pMyStation->StaConfigInfo.YXJLGroup[j].strXHDbegin = strArr[0];
                pMyStation->StaConfigInfo.YXJLGroup[j].nGDYXJLCount = c-1;
                //{}共c-1组
                for (int a=1; a<c; a++)
                {
                    //strArr[a] = {6G,4G@AP1A,XN,X,XY}
                    pMyStation->StaConfigInfo.YXJLGroup[j].nGDYXJLCount = a;
                    strArr[a].replace("{","");
                    strArr[a].replace("}","");
                    QString ch1 = "@";
                    QStringList strArrGrp;
                    int cGrp = StringSplit(strArr[a], ch1, strArrGrp);
                    //strArr[0] == 6G,4G  股道列表
                    //strArr[1] == AP1A,XN,X,XY  终端列表
                    if (2 == cGrp)
                    {
                        QString ch2 = ",";
                        QStringList strArrGd;
                        int cGD = StringSplit(strArrGrp[0], ch2, strArrGd);
                        for (int g=0;g<cGD;g++)
                        {
                            pMyStation->StaConfigInfo.YXJLGroup[j].GD_YXJL[a-1].strArrGD.append(strArrGd[g]);
                        }

                        QStringList strArrEnd;
                        int cEnd = StringSplit(strArrGrp[1], ch2, strArrEnd);
                        for (int g=0;g<cEnd;g++)
                        {
                            pMyStation->StaConfigInfo.YXJLGroup[j].GD_YXJL[a-1].strDefaultEnd = strArrEnd[0];
                            pMyStation->StaConfigInfo.YXJLGroup[j].GD_YXJL[a-1].strArrYXEnd.append(strArrEnd[g]);
                        }
                    }
                }
                pMyStation->StaConfigInfo.YXJLBeginNum++;
            }
        }
        else if (m_str.left(5) == "BTJL:")//变通进路
        {
            int j = pMyStation->StaConfigInfo.BTJLCount;
            getString = m_str.mid(5, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            pMyStation->StaConfigInfo.BTJLArray[j] = getString;
            pMyStation->StaConfigInfo.BTJLCount++;
        }
        else if (m_str.left(9) == "ZHONGCHA:")//中岔
        {
            getString = m_str.mid(9, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            StringSplit(getString, ",", pMyStation->StaConfigInfo.centrSwitchList);
        }
        else if(m_str.left(16)=="ROUTEDELETETIME:")//进路序列删除时间秒
        {
            getString=m_str.mid(16,m_str.indexOf(";") - m_str.indexOf(":") - 1);
            pMyStation->StaConfigInfo.RouteDeleteSeconds = getString.toInt();
        }
        else if (m_str.left(5) == "ZHJL:")//组合进路
        {
            getString = m_str.mid(5, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            QStringList strArray;
            int c = StringSplit(getString, "|", strArray);
            //组合进路至少包含1主进路+2条组合进路
            if(c>=3)
            {
                ZHJL zhjl;
                for(int j=0; j<c ;j++)
                {
                    zhjl.vectZHJLChild.append(strArray[j]);
                }
                vectZHJL.append(zhjl);
            }
        }
        else if (m_str.left(5) == "TGJL:")//通过进路
        {
            getString = m_str.mid(5, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            QStringList strArray;
            int c = StringSplit(getString, "|", strArray);
            //TGJL:X|IIIG|S;
            if(c==3)
            {
                TGJL tgjl;
                tgjl.SDXHName = strArray[0];
                tgjl.ZXTGGDName = strArray[1];
                tgjl.ZDXHName = strArray[2];
                vectTGJL.append(tgjl);
            }
        }
        else if (m_str.left(13) == "XIANLUSUOXHD:")//线路所信号机
        {
            getString = m_str.mid(13, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            StringSplit(getString, ",", pMyStation->StaConfigInfo.XianLuSuoXHDArray);
        }
        else if (m_str.left(10) == "YCJSPOINT:")//腰岔解锁
        {
            getString = m_str.mid(10, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            YCJS ycjs;
            ycjs.init();
            ycjs.strName = getString;
            vectYCJS.append(ycjs);
        }
        else if (m_str.left(10) == "GDQRPOINT:")//股道确认空闲配置
        {
            getString = m_str.mid(10, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            GDQR gdqr;
            gdqr.init();
            gdqr.strName = getString;
            vectGDQR.append(gdqr);
        }
        else if(m_str.left(9)=="XXHDZTPZ:")//虚信号灯状态配置，按钮下达命令类型配置
        {
            getString = m_str.mid(9, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            getString.trimmed();
            QStringList strArr;
            int c = StringSplit(getString, "|", strArr);
            if(c==2)
            {
                mXXHDBtnCmdType.BtnCmdType = strArr[0].toInt();
                StringSplit(strArr[1], ",", mXXHDBtnCmdType.XXHDArray);
            }
        }
    }
    file.close();
    return true;
}
//读取联锁表
bool MyStation::readInterlockTable(QString fileName, MyStation *pStation)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.isFile())
    {
        QLOG_ERROR()<<QString::fromLocal8Bit("File %1 not existed!").arg(fileName);
        return false;
    }

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString msg = "Open file failed！"+fileName;
        QLOG_ERROR()<<msg;
        return false;
    }

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        //QString m_str(line);
        QString m_str = codec->toUnicode(line);
        QString getString;
        if(m_str.left(4) == "####")
        {
            break;
        }
        //忽略注释和空格
        else if(m_str.left(2) == "//" || m_str.trimmed() == "")
        {
            continue;
        }
        else
        {
            getString = m_str.mid(0,m_str.indexOf(";"));
            getString.replace(" ", "");
            QStringList strArr;
            QString ch = "|";
            int c = StringSplit(getString, ch, strArr);
            if(c >= 7)
            {
                InterlockRoute *pRut = new InterlockRoute;
                QString ch1 = ",";
                pRut->Number = strArr[0];//序号
                pRut->Type   = strArr[1];//类型
                StringSplit(strArr[2], ch1, pRut->BtnArr);//按钮
                pRut->strXHD = strArr[3];//信号机
                StringSplit(strArr[4], ch1, pRut->DCArr);//道岔
                StringSplit(strArr[5], ch1, pRut->QDArr);//区段
                StringSplit(strArr[6], ch1, pRut->DDXHDArr);//敌对信号
                pStation->vectInterlockRoute.append(pRut);
            }
        }
    }
    //关闭文件
    file.close();
    return true;
}
//读取股道配置
bool MyStation::readGDConfigInfo(QString fileName, MyStation *pStation)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.isFile())
    {
        QLOG_INFO()<<QString::fromLocal8Bit("File %1 not existed!").arg(fileName);
        return false;
    }

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString msg = "Open file failed！"+fileName;
        QLOG_ERROR()<<msg;
        return false;
    }

    bInitGDNodeByConfig = true;
    //情况之前自动初始化的内容
    vectStationGDNode.clear();

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString m_str = codec->toUnicode(line);
        QString getString;
        if(m_str.left(4) == "####")
        {
            break;
        }
        //忽略注释和空格
        else if(m_str.left(2) == "//" || m_str.left(2) == "/*" ||m_str.trimmed() == "")
        {
            continue;
        }
        else if(m_str.left(3) == "##>")
        {
            StationGDNode gdNode;
            gdNode.bIgnoreLeftSXX = false;
            gdNode.bIgnoreRightSXX = false;
            for(int i=0; i<4; i++)
            {
                line = file.readLine();
                m_str = codec->toUnicode(line);
                m_str.replace(" ", "");
                if (m_str.left(6) == "nCode:")
                {
                    getString = m_str.mid(6, m_str.indexOf(";") - m_str.indexOf(":") - 1);
                    gdNode.nCode = getString.toInt();
                }
                else if (m_str.left(10) == "strGDName:")
                {
                    getString = m_str.mid(10, m_str.indexOf(";") - m_str.indexOf(":") - 1);
                    if(getString.indexOf(",")>-1)
                    {
                        QStringList strArray;
                        int c = StringSplit(getString, ",", strArray);
                        if(c>=2)
                        {
                            gdNode.strGDName = strArray[0];
                            gdNode.strGDName1 = strArray[1];
                        }
                    }
                    else
                    {
                        gdNode.strGDName = getString;
                    }
                }
                else if (m_str.left(12) == "strLeftNode:")
                {
                    getString = m_str.mid(12, m_str.indexOf(";") - m_str.indexOf(":") - 1);
                    if(getString.indexOf("#")>-1)
                    {
                        getString.replace("#","");
                        gdNode.bIgnoreLeftSXX = true;
                    }
                    if(getString.indexOf(",")>-1)
                    {
                        QStringList strArray;
                        int c = StringSplit(getString, ",", strArray);
                        if(c>=2)
                        {
                            gdNode.strLeftNode = strArray[0];
                            gdNode.strLeftNode1 = strArray[1];
                        }
                    }
                    else
                    {
                        gdNode.strLeftNode = getString;
                    }
                }
                else if (m_str.left(13) == "strRightNode:")
                {
                    getString = m_str.mid(13, m_str.indexOf(";") - m_str.indexOf(":") - 1);
                    if(getString.indexOf("#")>-1)
                    {
                        getString.replace("#","");
                        gdNode.bIgnoreRightSXX = true;
                    }
                    if(getString.indexOf(",")>-1)
                    {
                        QStringList strArray;
                        int c = StringSplit(getString, ",", strArray);
                        if(c>=2)
                        {
                            gdNode.strRightNode = strArray[0];
                            gdNode.strRightNode1 = strArray[1];
                        }
                    }
                    else
                    {
                        gdNode.strRightNode = getString;
                    }
                }
            }
            vectStationGDNode.append(gdNode);
        }
    }
    //关闭文件
    file.close();
    return true;
}
//读取进路序列信号机判断配置文件
void MyStation::readTempRouteXHD(QString fileName, MyStation *pStation)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.isFile())
    {
        QLOG_ERROR()<<QString::fromLocal8Bit("File %1 not existed!").arg(fileName);
        return;
    }

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString msg = "Open file failed！"+fileName;
        QLOG_ERROR()<<msg;
        return;
    }

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString m_str(line);
        QString getString;
        //bool ok;
        if(m_str.left(4) == "####")
        {
            break;
        }
        //忽略注释和空格
        else if(m_str.left(2) == "//" || m_str.trimmed() == "")
        {
            continue;
        }
        else if(m_str.left(10) == "ROUTE-XHD:")
        {
            //ROUTE-XHD:SLIV,X|SLIV,SIV;
            getString = m_str.mid(10, m_str.indexOf(";") - m_str.indexOf(":") - 1);
            QStringList strArr;
            int c = StringSplit(getString, "|", strArr);
            if(2 == c)
            {
                QStringList strArr1,strArr2;
                int c1 = StringSplit(strArr[0], ",", strArr1);
                int c2 = StringSplit(strArr[1], ",", strArr2);
                MyStation::RouteCheckXhd routeChk;
                for(int i=0; i<c1; i++)
                {
                    if(i==0)
                    {
                        routeChk.strRoute = strArr1[i];
                    }
                    else
                    {
                        routeChk.strRoute = routeChk.strRoute + "-" + strArr1[i];
                    }
                }
                for(int i=0; i<c2; i++)
                {
                    routeChk.vectXHD.push_back(strArr2[i]);
                }
                pStation->m_vectRouteCheckXhd.append(routeChk);
                pStation->m_bRouteStateCheckXHD = true;
            }
        }
    }
    //关闭文件
    file.close();
}
//读取线路公里标配置信息
void MyStation::readXLGLBConfigInfo(QString fileName, MyStation *pStation)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.isFile())
    {
        QLOG_ERROR()<<QString::fromLocal8Bit("File %1 not existed!").arg(fileName);
        return;
    }

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString msg = "Open file failed！"+fileName;
        QLOG_ERROR()<<msg;
        return;
    }

    while(!file.atEnd())
    {
        //读一行
        QByteArray line = file.readLine();
        //QString m_str = QString(line);
        QString m_str = codec->toUnicode(line);
        QString getString;
        if(m_str.left(4) == "####")
        {
            break;
        }
        //忽略注释和空格
        else if(m_str.left(2) == "//" || m_str.trimmed() == "")
        {
            continue;
        }
        else if(m_str.left(8) == "##XIANLU")
        {
            bool bFlag = true;
            XLGLB_T glb;
            while(!file.atEnd() && bFlag)
            {
                //读一行
                line = file.readLine();
                //m_str = QString(line);
                m_str = codec->toUnicode(line);
                if(m_str.left(2) == "//")
                {
                    continue;
                }
                else if (m_str.left(5) == "INFO=")
                {
                    getString = m_str.mid(5, m_str.indexOf(";") - m_str.indexOf("=") - 1);
                    QStringList arrGet;
                    int c = StringSplit(getString, "|", arrGet);
                    if (4 == c)
                    {
                        glb.XLNum = StringToHex(arrGet[0]);
                        glb.XLName = arrGet[1];;
                        glb.strStartGLB = arrGet[2];//K1000+100
                        glb.strFinishGLB = arrGet[3];
                        glb.StartGLB = GLBStrToInt(glb.strStartGLB);
                        glb.FinishGLB = GLBStrToInt(glb.strFinishGLB);
                    }
                }
                else if (m_str.left(3) == "GD=")
                {
                    getString = m_str.mid(3, m_str.indexOf(";") - m_str.indexOf("=") - 1);
                    QStringList arrGet;
                    int c = StringSplit(getString, "|", arrGet);
                    if(2 == c)
                    {
                        CGD *pGD = new CGD;
                        pGD->m_strName = arrGet[0];
                        pGD->strGLB = arrGet[1];
                        pGD->GLB = GLBStrToInt(pGD->strGLB);
                        glb.DevArray.append(pGD);
                    }
                }
                else if (m_str.left(3) == "DC=")
                {
                    getString = m_str.mid(3, m_str.indexOf(";") - m_str.indexOf("=") - 1);
                    QStringList arrGet;
                    int c = StringSplit(getString, "|", arrGet);
                    if(2 == c)
                    {
                        CGDDC *pGDDC = new CGDDC;
                        pGDDC->m_strName = arrGet[0];
                        pGDDC->strGLB = arrGet[1];
                        pGDDC->GLB = GLBStrToInt(pGDDC->strGLB);
                        glb.DevArray.append(pGDDC);
                    }
                }
                else if (m_str.left(5) == "##END")
                {
                    bFlag = false;
                    break;
                }
            }
            this->XLGLBArray.append(glb);
        }
    }
    //关闭文件
    file.close();
}

//初始化数据
void MyStation::InitData()
{
    initBTJLinfo();
    initJFCK();
    initJJQD();
    initGDNode();
    initBSData();
    initRoutePreWnd();
}
//根据设备号获取设备名称
QString MyStation::GetStrNameByCode(int nCode)
{
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            if(nCode == (int)gddc->getCode())
            {
                return gddc->getName();
            }
        }
        else if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            if(nCode == (int)xhd->getCode())
            {
                return xhd->getName();
            }
        }
        else if(ement->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD*)ement;
            if(nCode == (int)gd->getCode())
            {
                return gd->getName();
            }
        }
        else if(ement->getDevType() == Dev_TG)
        {
            CTG *tg=(CTG *)ement;
            if(nCode == (int)tg->getCode())
            {
                QString devName = tg->getName();
                devName.replace("TA","");
                int devIndex = this->GetIndexByStrName(devName);
                if(devIndex>0)
                {
                    CXHD *pXHD=(CXHD*)DevArray[devIndex];
                    return pXHD->getName();
                }
                return "";
            }
        }
    }
    return "";//"未知"
}
//根据设备名称获取设备号
int MyStation::GetCodeByStrName(QString devName)
{
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            if(devName == gddc->getName())
            {
                return gddc->getCode();
            }
        }
        else if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            if(devName == xhd->getName())
            {
                return xhd->getCode();
            }
        }
        else if(ement->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD*)ement;
            if(devName == gd->getName())
            {
                return gd->getCode();
            }
        }
    }
    return -1;
}
//股道在站场数组中的索引（+1的）
int MyStation::GetGDPosInzcArray(int nCode)
{
    for(int i=0; i<DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD*)DevArray[i];
            if(nCode == (int)gd->getCode())
            {
                return i+1;
            }
        }
        else if(DevArray[i]->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)DevArray[i];
            if(nCode == gddc->getCode())
            {
                return i+1;
            }
        }
        else if(DevArray[i]->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)DevArray[i];
            if(nCode == xhd->getCode())
            {
                return i+1;
            }
        }
    }
    return 0;
}

CGDDC* MyStation::GetDCQDByCode(int nCode)
{
    CGDDC* pCGDDC = nullptr;
    for(int i=0; i<DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_DC)
        {
            pCGDDC = (CGDDC*)DevArray[i];
            if(nCode == pCGDDC->getQDCode())
            {
                return pCGDDC;
            }
        }

    }
    return nullptr;
}
//信号机是否属于线路所
bool MyStation::IsXHDinXianLuSuo(QString _strXHD)
{
    for(int i=0; i<StaConfigInfo.XianLuSuoXHDArray.count(); i++)
    {
        if(_strXHD == StaConfigInfo.XianLuSuoXHDArray[i])
        {
            return true;
        }
    }
    return false;
}
//根据名称获取设备索引
int MyStation::GetIndexByStrName(QString devName)
{
    for(int i=0; i<DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)DevArray[i];
            if(devName == gddc->getName())
            {
                return i;
            }
        }
        else if(DevArray[i]->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)DevArray[i];
            if(devName == xhd->getName())
            {
                return i;
            }
        }
        else if(DevArray[i]->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD*)DevArray[i];
            if(devName == gd->getName())
            {
                return i;
            }
        }
    }
    return -1;
}
//设备是否是股道
bool MyStation::DevIsGDByCode(int nCode)
{
    bool rvalue = false;

    for(int i=0; i<DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD)
        {
            CGD *pGD = (CGD*)DevArray[i];
            if((nCode == (int)pGD->getCode()) && (pGD->getGDType() == GD_QD))
            {
                rvalue = true;
                break;
            }
        }
    }

    if(rvalue == false)
    {
        //股道和股道配置的相邻区段判断
        QString strDev = GetStrNameByCode(nCode);
        for(int i=0; i<vectStationGDNode.size();i++)
        {
            if(strDev == vectStationGDNode[i].strGDName)
            {
                rvalue = true;
                break;
            }
            else if(strDev == vectStationGDNode[i].strGDName1 && vectStationGDNode[i].strGDName1 != "")
            {
                rvalue = true;
                break;
            }
        }
    }

    return rvalue;
}

//信号机及其他数据初始化
void MyStation::initJFCK()
{
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_XH)
        {
            CXHD *pXHD=(CXHD *)ement;
            int ct = StaConfigInfo.JCKCount;
            for (int t = 0; t < ct; t++)
            {
                if (pXHD->m_strName == StaConfigInfo.JFCKInfo[t].strJCKName)
                {
                    StaConfigInfo.JFCKInfo[t].nJCKCode = pXHD->m_nCode;
                }
                if (pXHD->m_strName == StaConfigInfo.JFCKInfo[t].strFCKName)
                {
                    StaConfigInfo.JFCKInfo[t].nFCKCode = pXHD->m_nCode;
                }
            }
        }
    }

}
//接近区段数据初始化
void MyStation::initJJQD()
{
#if 1
    int ct = StaConfigInfo.JCKCount;
    for (int t = 0; t < ct; t++)
    {
        //接车口关联的接近区段获取
        int jckXHDindex = GetIndexByStrName(StaConfigInfo.JFCKInfo[t].strJCKName);
        if(jckXHDindex < 0)
            break;
        CXHD *pxhd=(CXHD *)(DevArray[jckXHDindex]);
        //QString str1 = GetXHDjucJJQDName(pxhd);//第1个接近
        QString str1 = GetXHDjucJJQDName(pxhd->p1, pxhd->p2);//第1个接近
        StaConfigInfo.JFCKInfo[t].strJCKjucJJQD = str1;
        if(str1 != "")
        {
            StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(str1);
            QString strNext2 = FindNextJJQD(t, str1);//第2个接近
            if(strNext2 != "")
            {
                StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext2);
                QString strNext3 = FindNextJJQD(t, strNext2);//第3个接近
                if(strNext3 != "")
                {
                    StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext3);
                    QString strNext4 = FindNextJJQD(t, strNext3);//第4个接近
                    if(strNext4 != "")
                    {
                        StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext4);
                        QString strNext5 = FindNextJJQD(t, strNext4);//第5个接近
                        if(strNext5 != "")
                        {
                            StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext5);
                            QString strNext6 = FindNextJJQD(t, strNext5);//第6个接近
                            if(strNext6 != "")
                            {
                                StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext6);
                                QString strNext7 = FindNextJJQD(t, strNext6);//第7个接近
                                if(strNext7 != "")
                                {
                                    StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext7);
                                    QString strNext8 = FindNextJJQD(t, strNext7);//第8个接近
                                    if(strNext8 != "")
                                    {
                                        StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext8);
                                        QString strNext9 = FindNextJJQD(t, strNext8);//第9个接近
                                        if(strNext9 != "")
                                        {
                                            StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext9);
                                            QString strNext10 = FindNextJJQD(t, strNext9);//第10个接近
                                            if(strNext10 != "")
                                            {
                                                StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext10);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        //发车口关联的离去区段获取
        int fckXHDindex = GetIndexByStrName(StaConfigInfo.JFCKInfo[t].strFCKName);
        if (fckXHDindex < 0)
            break;
        CXHD *pxhd2 = (CXHD *)(DevArray[fckXHDindex]);
        //str1 = GetXHDjucJJQDName(pxhd);//第1个离去
        QString str11 = GetXHDjucJJQDName(pxhd2->p1, pxhd2->p2);//第1个接近
        StaConfigInfo.JFCKInfo[t].strFCKjucLQQD = str11;
        if (str11 != "")
        {
            StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(str11);
            QString strNext2 = FindNextJJQD(t, str11);//第2个离去
            if (strNext2 != "")
            {
                StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext2);
                QString strNext3 = FindNextJJQD(t, strNext2);//第3个离去
                if (strNext3 != "")
                {
                    StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext3);
                    QString strNext4 = FindNextJJQD(t, strNext3);//第4个离去
                    if (strNext4 != "")
                    {
                        StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext4);
                        QString strNext5 = FindNextJJQD(t, strNext4);//第5个离去
                        if (strNext5 != "")
                        {
                            StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext5);
                            QString strNext6 = FindNextJJQD(t, strNext5);//第6个离去
                            if (strNext6 != "")
                            {
                                StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext6);
                                QString strNext7 = FindNextJJQD(t, strNext6);//第7个离去
                                if (strNext7 != "")
                                {
                                    StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext7);
                                    QString strNext8 = FindNextJJQD(t, strNext7);//第8个离去
                                    if (strNext8 != "")
                                    {
                                        StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext8);
                                        QString strNext9 = FindNextJJQD(t, strNext8);//第9个离去
                                        if (strNext9 != "")
                                        {
                                            StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext9);
                                            QString strNext10 = FindNextJJQD(t, strNext9);//第10个离去
                                            if (strNext10 != "")
                                            {
                                                StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext10);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif
}
//获取（接车口）信号机关联的接近区段("XHD.h"循环调用修改)
QString MyStation::GetXHDjucJJQDName(QPoint xhdP1, QPoint xhdP2)
{
    //CPoint *pt[]={&pxhd->p1,&pxhd->p2};
    QPoint pt[]={xhdP1,xhdP2};
    for(int i = 0; i < (int)DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD)
        {
            CGD *pGD = (CGD*)DevArray[i];
            if(JJ_QD == (int)pGD->getGDType())
            {
                QRect rectGD;
                QPoint pPtGD[]={pGD->p1.toPoint(),pGD->p2.toPoint(),pGD->p3.toPoint(),pGD->p4.toPoint()};
                for(int k=0;k<2;k++)
                {
                    for(int j=0;j<4;j++)
                    {
                        //rectGD.setRect(pPtGD[j].x()-10,pPtGD[j].y()-10,pPtGD[j].x()+10,pPtGD[j].y()+10);
                        rectGD.setRect(pPtGD[j].x()-5,pPtGD[j].y()-5,10,10);
                        //if(PtInRect(rectGD,*pt[k]))
                        //if(PtInRect(rectGD,pt[k]))
                        if(rectGD.contains(pt[k]))
                        {
                            return pGD->m_strName;
                            break;
                        }
                    }
                }
            }
        }
    }

    return "";
}
//根据第一个接近区段名称查找下一个接近区段
QString MyStation::FindNextJJQD(int index, QString strJJQD)
{
    int indexJJ = GetIndexByStrName(strJJQD);
    if(indexJJ == -1)
    {
        return "";
    }
    CGD *pGD = (CGD*)DevArray[indexJJ];
    QPoint pt[]={pGD->p12.toPoint(),pGD->p34.toPoint()};
    //int m = (int)DevArray.count();
    for(int i = 0; i < (int)DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD)
        {
            CGD *pGDNext = (CGD*)DevArray[i];
            if(JJ_QD == (int)pGDNext->getGDType())
            {
                if(strJJQD == pGDNext->m_strName)//不判断自己
                {
                    continue;
                }
                if(FindJJQDFromJCK(index, pGDNext->m_strName))//如何自己已经在接近区段数组中，则跳过
                {
                    continue;
                }
                QPoint pPtGDNext[]={pGDNext->p12.toPoint(),pGDNext->p34.toPoint()};
                for(int k=0;k<2;k++)
                {
                    for(int j=0;j<2;j++)
                    {
                        QRect rectGDNext;
                        //rectGDNext.setRect(pPtGDNext[j].x()-5,pPtGDNext[j].y()-5,pPtGDNext[j].x()+5,pPtGDNext[j].y()+5);
                        rectGDNext.setRect(pPtGDNext[j].x()-5,pPtGDNext[j].y()-5, 10, 10);
                        //if(PtInRect(rectGDNext,*pt[k]))
                        if(rectGDNext.contains(pt[k]))
                        {
                            return pGDNext->m_strName;
                            break;
                        }
                    }
                }
            }
        }
    }

    return  "";
}
//在接车口接近区段能否找到该接近区段
bool MyStation::FindJJQDFromJCK(int index, QString strJJQD)
{
    for(int i=0;i<StaConfigInfo.JFCKInfo[index].strArrJckJJQD.count();i++)
    {
        if(strJJQD == StaConfigInfo.JFCKInfo[index].strArrJckJJQD[i])
        {
            return true;
        }
    }
    for(int i=0;i<StaConfigInfo.JFCKInfo[index].strArrFckLQQD.count();i++)
    {
        if(strJJQD == StaConfigInfo.JFCKInfo[index].strArrFckLQQD[i])
        {
            return true;
        }
    }

    return false;
}
//闭塞数据初始化
void MyStation::initBSData()
{
    //自动闭塞
    for(int i=0; i<vectAutomaticBlock.size(); i++)
    {
        vectAutomaticBlock[i]->codejuncXHD = this->GetCodeByStrName(vectAutomaticBlock[i]->strJuncXHD);
    }
    //自动闭塞
    for(int i=0; i<vectSemiAutomaticBlock.size(); i++)
    {
        vectSemiAutomaticBlock[i]->codejuncXHD = this->GetCodeByStrName(vectSemiAutomaticBlock[i]->strJuncXHD);
    }
    //计轴
    for(int i=0; i<vectAxleCounter.size(); i++)
    {
        vectAxleCounter[i]->codejuncXHD = this->GetCodeByStrName(vectAxleCounter[i]->strJuncXHD);
    }
    //场联
    for(int i=0; i<vectInterfieldConnection.size(); i++)
    {
        vectInterfieldConnection[i]->codejuncXHD = this->GetCodeByStrName(vectInterfieldConnection[i]->strJuncXHD);
    }
}
//股道接点数据初始化
void MyStation::initGDNode()
{
    //根据配置文件初始化，则退出
    if(bInitGDNodeByConfig)
    {
        return;
    }

    for(int i=0; i<vectStationGDNode.size(); i++)
    {
        for(auto ement:DevArray)
        {
            if(ement->getDevType() == Dev_GD)
            {
                CGD *gd=(CGD*)ement;
                if(vectStationGDNode[i].nCode == (int)gd->getCode())
                {
                    //正常绘制方向p12|--|p34
                    if((gd->getp12().rx() < gd->getp34().rx()) || (gd->getp12().ry() < gd->getp34().ry()))
                    {
                        QPoint point = gd->getp12().toPoint();
                        CXHD *xhdLeft = GetJucXHDbyPoint(point);
                        if(xhdLeft)
                        {
                            vectStationGDNode[i].strLeftNode = xhdLeft->getName();//GetJucXHDbyPoint(point);
                        }
//                        if(vectStationGDNode[i].strLeftNode == "")
//                        {
//                            vectStationGDNode[i].strLeftNode = judgeCentrSwitchbyPoint(point, &vectStationGDNode[i]);
//                        }
                        point = gd->getp34().toPoint();
                        CXHD *xhdRight = GetJucXHDbyPoint(point);
                        if(xhdRight)
                        {
                            vectStationGDNode[i].strRightNode = xhdRight->getName();//GetJucXHDbyPoint(point);
                        }
//                        if(vectStationGDNode[i].strRightNode == "")
//                        {
//                            vectStationGDNode[i].strRightNode = judgeCentrSwitchbyPoint(point, &vectStationGDNode[i]);
//                        }

                        //左边为调车信号机，右边为列车信号机，继续计算左边的节点
                        if(xhdLeft->getXHDType() == DC_XHJ && xhdRight->getXHDType() != DC_XHJ)
                        {
                            point = gd->getp12().toPoint();
                            vectStationGDNode[i].strLeftNode = judgeCentrSwitchbyPoint(point, &vectStationGDNode[i], true);
                        }
                        //左边为列车信号机，右边为调车信号机，继续计算右边的节点
                        else if(xhdLeft->getXHDType() != DC_XHJ && xhdRight->getXHDType() == DC_XHJ)
                        {
                            point = gd->getp34().toPoint();
                            vectStationGDNode[i].strRightNode = judgeCentrSwitchbyPoint(point, &vectStationGDNode[i], false);
                        }

                    }
                    //反向的绘制方向p34|--|p12
                    else
                    {
                        QPoint point = gd->getp34().toPoint();
                        //vectStationGDNode[i].strLeftNode = GetJucXHDbyPoint(point);
                        CXHD *xhdLeft = GetJucXHDbyPoint(point);
                        if(xhdLeft)
                        {
                            vectStationGDNode[i].strLeftNode = xhdLeft->getName();//GetJucXHDbyPoint(point);
                        }
                        point = gd->getp12().toPoint();
                        //vectStationGDNode[i].strRightNode = GetJucXHDbyPoint(point);
                        CXHD *xhdRight = GetJucXHDbyPoint(point);
                        if(xhdRight)
                        {
                            vectStationGDNode[i].strRightNode = xhdRight->getName();//GetJucXHDbyPoint(point);
                        }
                    }
                }
            }
        }
    }
}
//根据坐标点获取关联的信号机
CXHD* MyStation::GetJucXHDbyPoint(QPoint point)
{
    CXHD *xhd = nullptr;
    QRect rect = QRect(point.x()-10, point.y()-10, 20, 20);//10,10
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_XH)
        {
            xhd=(CXHD *)ement;
            if(rect.contains(xhd->center) || rect.contains(xhd->p1) || rect.contains(xhd->p2))
            {
                return xhd;//xhd->getName();
            }
        }
    }
    return xhd;//""
}
//根据坐标点获取关联的道岔是否中岔，并继续向下查找关联信号机名称
QString MyStation::judgeCentrSwitchbyPoint(QPoint point, StationGDNode* gdNode, bool bLeft)
{
    QRect rect = QRect(point.x()-5, point.y()-5, 10, 10);
    //中岔判断
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *dc=(CGDDC *)ement;
            //判断与中岔相连接，原始股道与P12相连
            if(rect.contains(dc->getp12().toPoint()) && isCentrSwitch(dc->getName()))
            {
                //获取另外一端的接点
                CGD *gd = GetJucGDbyPoint(dc->getp34().toPoint());
                if(gd)
                {
                    gdNode->strGDName1 = gd->getName();
                    QPoint pointB = dc->getp34().toPoint();
                    QRect rectB = QRect(pointB.x()-5, pointB.y()-5, 10, 10);
                    //中岔与股道的P12相连，则判断P34连接的信号机
                    if(rectB.contains(gd->getp12().toPoint()))
                    {
                        CXHD *xhd = GetJucXHDbyPoint(gd->getp34().toPoint());
                        if(xhd)
                        {
                            if(bLeft)
                            {
                                gdNode->strLeftNode = xhd->getName();
                            }
                            else
                            {
                                gdNode->strRightNode = xhd->getName();
                            }
                            return xhd->getName();
                        }
                    }
                    //中岔与股道的P34相连，则判断P12连接的信号机
                    else if(rectB.contains(gd->getp34().toPoint()))
                    {
                        CXHD *xhd = GetJucXHDbyPoint(gd->getp12().toPoint());
                        if(xhd)
                        {
                            if(bLeft)
                            {
                                gdNode->strLeftNode = xhd->getName();
                            }
                            else
                            {
                                gdNode->strRightNode = xhd->getName();
                            }
                            return xhd->getName();
                        }
                    }
                }
                //return xhd->getName();
            }
            //判断与中岔相连接，原始股道与P34相连
            else if(rect.contains(dc->getp34().toPoint()) && isCentrSwitch(dc->getName()))
            {
                //获取另外一端的接点
                CGD *gd = GetJucGDbyPoint(dc->getp12().toPoint());
                if(gd)
                {
                    gdNode->strGDName1 = gd->getName();
                    QPoint pointB = dc->getp12().toPoint();
                    QRect rectB = QRect(pointB.x()-5, pointB.y()-5, 10, 10);
                    //中岔与股道的P12相连，则判断P34连接的信号机
                    if(rectB.contains(gd->getp12().toPoint()))
                    {
                        CXHD *xhd = GetJucXHDbyPoint(gd->getp34().toPoint());
                        if(xhd)
                        {
                            if(bLeft)
                            {
                                gdNode->strLeftNode = xhd->getName();
                            }
                            else
                            {
                                gdNode->strRightNode = xhd->getName();
                            }
                            return xhd->getName();
                        }
                    }
                    //中岔与股道的P34相连，则判断P12连接的信号机
                    if(rectB.contains(gd->getp34().toPoint()))
                    {
                        CXHD *xhd = GetJucXHDbyPoint(gd->getp12().toPoint());
                        if(xhd)
                        {
                            if(bLeft)
                            {
                                gdNode->strLeftNode = xhd->getName();
                            }
                            else
                            {
                                gdNode->strRightNode = xhd->getName();
                            }
                            return xhd->getName();
                        }
                    }
                }
            }
        }
    }

    return "";
}
//道岔是否为中岔
bool MyStation::isCentrSwitch(QString dcName)
{
    for(auto elem:StaConfigInfo.centrSwitchList)
    {
        QString elemName = (QString)elem;
        if(dcName == elemName)
        {
            return true;
        }
    }
    return false;
}
//根据坐标点获取关联的股道
CGD* MyStation::GetJucGDbyPoint(QPoint point)
{
    CGD *gd = nullptr;
    QRect rect = QRect(point.x()-5, point.y()-5, 10, 10);
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_GD || ement->getDevType() == Dev_WCQD)
        {
            gd=(CGD *)ement;
            if(rect.contains(gd->getp12().toPoint()) || rect.contains(gd->getp34().toPoint()))
            {
                return gd;
            }
        }
    }
    return gd;
}

//初始化本站变通进路基础信息
void MyStation::initBTJLinfo()
{
    //变通进路初始化
    for (int i = 0; i < StaConfigInfo.BTJLCount; i++)
    {
        QString btjlInfo = StaConfigInfo.BTJLArray[i];
        QStringList strArray;
        QString ch = "|";
        int c = StringSplit(btjlInfo, ch, strArray);
        BTJL btjl;
        for(int j=0; j<c ;j++)
        {
            btjl.vectBTJLChild.append(strArray[j]);
        }
        vectBTJL.append(btjl);
    }
}
//初始化进路预告窗口信息
void MyStation::initRoutePreWnd()
{
    //自动闭塞
    for(int i=0; i<vectAutomaticBlock.size(); i++)
    {
        RoutePreWnd *pRoutePreWnd = new RoutePreWnd;
        pRoutePreWnd->juncXHDName = vectAutomaticBlock[i]->strJuncXHD;
        pRoutePreWnd->juncXHDCode = vectAutomaticBlock[i]->codejuncXHD;
        vectRoutePreWnd.append(pRoutePreWnd);
    }
    //半自动闭塞
    for(int i=0; i<vectSemiAutomaticBlock.size(); i++)
    {
        RoutePreWnd *pRoutePreWnd = new RoutePreWnd;
        pRoutePreWnd->juncXHDName = vectSemiAutomaticBlock[i]->strJuncXHD;
        pRoutePreWnd->juncXHDCode = vectSemiAutomaticBlock[i]->codejuncXHD;
        vectRoutePreWnd.append(pRoutePreWnd);
    }
    //场联
    for(int i=0; i<vectInterfieldConnection.size(); i++)
    {
        RoutePreWnd *pRoutePreWnd = new RoutePreWnd;
        pRoutePreWnd->juncXHDName = vectInterfieldConnection[i]->strJuncXHD;
        pRoutePreWnd->juncXHDCode = vectInterfieldConnection[i]->codejuncXHD;
        vectRoutePreWnd.append(pRoutePreWnd);
    }
}
//重置站场状态
void MyStation::ResetStationDevStatus()
{
    //CTC设置的状态清除
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            gddc->isPowerCutDW = false;
            gddc->isPowerCutFW = false;
            gddc->isPowerCutCQ = false;
            gddc->flblStatusDW = 0;
            gddc->flblStatusFW = 0;
            gddc->flblStatusCQ = 0;
            gddc->speedLimitStatus = 0;
            gddc->m_nCheciLost = false;
            gddc->m_bLCTW = false;
            //gddc->m_nDCSXAllLock=0;
//              gddc->isJGGZ = false;
//              gddc->isXGGZ = false;
        }
        else if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            xhd->m_nFuncLockState = 0;
            xhd->setIsDCANFB(false);
            xhd->setIsLCANFB(false);
            xhd->setIsYDANFB(false);
        }
        else if(ement->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD*)ement;
            gd->isPowerCut = false;
            gd->flblStatus = 0;
            gd->isLock = false;
            gd->speedLimitStatus = 0;
        }
    }
}
//联锁状态解析
void MyStation::updateDevStateOfLS(unsigned char *array)
{
    //联锁站场停电故障
    m_bLSPowerFailure = CheckStationPowerFailureOfLS(array);

    QDateTime timeNow = QDateTime::currentDateTime();
    //qDebug()<<"recv ls status!"<<timeNow.toString(TIME_FORMAT_HMSM);
    //setGD();
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
            updateGDDC_StateOfLS(gddc,state);
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
            updateXHD_StateOfLS(xhd,state1);
            //包含信号机灯丝报警状态
            if(StaConfigInfo.bXHDDSBJStatus)
            {
                int state2=array[count+1];
                if((state2&0x01) == 0x01)//红灯灯丝断丝
                {
                    //pXHD->setXHDState(XHD_DS);
                    if(xhd->XHD_ds_HD==false)
                    {
                        if(xhd->m_nSafeLamp == XHD_HD)
                        {
                            if(!m_bLSPowerFailure)
                            {
                                //报警提示信号机断丝
                                QString strSys = QString("[%1]信号机断丝").arg(xhd->getName());
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                            }
                        }
                    }
                    xhd->XHD_ds_HD=true;
                }
                else
                {
                    xhd->XHD_ds_HD=false;
                }
                if((state2&0x02)==0x02)//绿灯灯丝断丝
                {
                    xhd->XHD_ds_LD=true;
                }
                else
                {
                    xhd->XHD_ds_LD=false;
                }
                if((state2&0x04)==0x04)//黄灯灯丝断丝
                {
                    //pXHD->setXHDState(XHD_DS);
                    xhd->XHD_ds_UD=true;
                }
                else
                {
                    xhd->XHD_ds_UD=false;
                }
                if((state2&0x08)==0x08)//白灯灯丝断丝
                {
                    //pXHD->setXHDState(XHD_DS);
                    xhd->XHD_ds_BD=true;
                }
                else
                {
                    xhd->XHD_ds_BD=false;
                }
                if((state2&0x10)==0x10)//蓝灯灯丝断丝
                {
                    //pXHD->setXHDState(XHD_DS);
                    if(xhd->XHD_ds_AD==false)
                    {
                        if(xhd->m_nSafeLamp == XHD_AD)
                        {
                            if(!m_bLSPowerFailure)
                            {
                                //报警提示信号机断丝
                                QString strSys = QString("[%1]信号机断丝").arg(xhd->getName());
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                            }
                        }
                    }
                    xhd->XHD_ds_AD=true;
                }
                else
                {
                    xhd->XHD_ds_AD=false;
                }
                if((state2&0x20)==0x20)//引导白灯灯丝断丝
                {
                    //pXHD->setXHDState(XHD_DS);
                    xhd->XHD_ds_YBD=true;
                }
                else
                {
                    xhd->XHD_ds_YBD=false;
                }
                count+=2;
            }
            else
            {
                count++;
            }
            num++;
        }
    }
//    //股道状态
//    num=0;
//    for(auto ement:DevArray)
//    {
//        if(num>qdNum)
//        {
//            break;
//        }
//        if(ement->getDevType() == Dev_GD)
//        {
//            CGD *gd=(CGD *)ement;
//            int state1=array[count];
//            count++;
//            updateGD_StateOfLS(gd,state1);
//            num++;
//        }
//    }
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
            updateQD_StateOfLS(qd,state1);
            num++;
        }
    }

    //设置道岔的绘制颜色
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            gddc->setDCColor();
        }
    }
    //道岔区段状态逻辑
    setGD();

    //自动闭塞
    unsigned int devCode = 0;
    int zdbsCount = vectAutomaticBlock.size();
    for(int i=0; i<zdbsCount; i++)
    {
        devCode = array[count]|(array[count+1]<<8);
        //qDebug()<<QString("Recv zdbs[%1] code=%2").arg(i).arg(devCode);
        for(int j=0; j<zdbsCount; j++)
        {
            AutomaticBlock* pAutomaticBlock = vectAutomaticBlock[j];
            if(devCode == pAutomaticBlock->codejuncXHD)
            {
                int state = array[count+2];
                //监督区间灯
                pAutomaticBlock->lightStateJDQJ = (state&0x80) == 0x80 ? 1 : 0;
                pAutomaticBlock->lightStateQJD = pAutomaticBlock->lightStateJDQJ;
                //接发车箭头状态
                pAutomaticBlock->arrowState = (state&0x70)>>4;
                //按钮状态
                //按钮状态-总辅助
                pAutomaticBlock->btnStateZFZ = (state&0x01) == 0x01 ? 1 : 0;
                //辅助灯=总辅助按钮
                pAutomaticBlock->lightStateFZD = pAutomaticBlock->btnStateZFZ;
                //按钮状态-发辅助
                pAutomaticBlock->btnStateFFZ = (state&0x02) == 0x02 ? 1 : 0;
                //按钮状态-接辅助
                pAutomaticBlock->btnStateJFZ = (state&0x04) == 0x04 ? 1 : 0;
                //按钮状态-改方
                pAutomaticBlock->btnStateGF = (state&0x08) == 0x08 ? 1 : 0;

                if(StaConfigInfo.bZDBSLightExState)
                {
                    state = array[count+2+1];
                    //指示灯的状态-区间灯
                    pAutomaticBlock->lightStateQJD = state&0x0F;
                    //指示灯的状态-辅助灯
                    pAutomaticBlock->lightStateFZD = (state&0xF0)>>4;

                    state = array[count+2+2];
                    //指示灯的状态-区轨灯
                    pAutomaticBlock->lightStateQGD = state&0x0F;
                    //指示灯的状态-允许发车灯
                    pAutomaticBlock->lightStateYXFCD = (state&0xF0)>>4;

                    count+=5;
                }
                else
                {
                    count+=3;
                }
                break;
            }
        }
    }

    //半自动闭塞
    int bzdbsCount = vectSemiAutomaticBlock.size();
    for(int i=0; i<bzdbsCount; i++)
    {
        devCode = array[count]|(array[count+1]<<8);
        for(int j=0; j<bzdbsCount; j++)
        {
            SemiAutomaticBlock* pSemiAutomaticBlock = vectSemiAutomaticBlock[j];
            if(devCode == pSemiAutomaticBlock->code)
            {
                int state = array[count+2];
                //接车箭头状态
                pSemiAutomaticBlock->arrowStateReach = state&0x0F;
                //发车箭头状态
                pSemiAutomaticBlock->arrowStateDepart = (state&0xF0)>>4;

                break;
            }
        }
        count+=3;
    }

    //计轴状态
    int jzCount = vectAxleCounter.size();
    for(int i=0; i<jzCount; i++)
    {
        devCode = array[count]|(array[count+1]<<8);
        for(int j=0; j<jzCount; j++)
        {
            if(devCode == vectAxleCounter[j]->codejuncXHD)
            {
                int state = array[count+2];
                //按钮状态-复零
                vectAxleCounter[j]->btnStateFL = (state&0x01) == 0x01 ? 1 : 0;
                //按钮状态-使用
                vectAxleCounter[j]->btnStateSY = (state&0x02) == 0x02 ? 1 : 0;
                //按钮状态-停止
                vectAxleCounter[j]->btnStateTZ = (state&0x04) == 0x04 ? 1 : 0;
                //指示灯状态-计轴报警灯
                vectAxleCounter[j]->lightStateJZBJ = (state&0x08) == 0x08 ? 1 : 0;

                //指示灯的状态-复零灯
                vectAxleCounter[j]->lightStateFL = (state&0x10) == 0x10 ? 1 : 0;
                //指示灯的状态-使用灯
                vectAxleCounter[j]->lightStateSY = (state&0x20) == 0x20 ? 1 : 0;
                //指示灯的状态-停止灯
                vectAxleCounter[j]->lightStateTZ = (state&0x40) == 0x40 ? 1 : 0;
                //指示灯的状态-区轨灯
                vectAxleCounter[j]->lightStateQG = (state&0x80) == 0x80 ? 1 : 0;

                break;
            }
        }
        count+=3;
    }

    //机务段
//    if(StaConfigInfo.HaveCLPZFlag)
//    {
//        count++;
//    }
    //场联+机务段
    int clCount = vectInterfieldConnection.size();
    for(int i=0; i<clCount; i++)
    {
        devCode = array[count]|(array[count+1]<<8);
        for(int j=0; j<clCount; j++)
        {
            InterfieldConnection* pCL = vectInterfieldConnection[j];
            if(devCode == pCL->codejuncXHD)
            {
                //qDebug()<<"pCL="<<pCL->strJuncXHD;
                int state = array[count+2];
                //指示灯的状态-箭头 0111 000
                pCL->arrowState = (state&0x70)>>4;
                //指示灯的状态-接近轨 1000 0000
                pCL->lightStateJGJ = (state&0x80) == 0x80 ? 1 : 0;
                //指示灯的状态-C灯 0000 1111
                pCL->lightStateCFJ = (state&0x0F);
                int state2 = array[count+3];
                //指示灯的状态-A灯 0000 1111
                pCL->lightStateAFJ = (state2&0x0F);
                //指示灯的状态-B灯 1111 0000
                pCL->lightStateBFJ = (state2&0xF0)>>4;

                break;
            }
        }
        count+=4;
    }

    //道岔尖轨心轨故障状态
    count += updateDCJGXGStatus(array, count);

    //信号机倒计时
    if (((array[count]&0xDA) == 0xDA) && ((array[count+1]&0xDA) == 0xDA))
    {
        count +=2;
        count += updateXHDTimeCount(array,count);
    }

    //腰岔解锁+股道确认
    count += updateYXJSGDRQ(array,count);

    //************* 其他状态 *************
    int dataLength=(int)(array[4] | array[5]<<8);
    bool bFCZK = true;
    if(array[dataLength-5] == 0x22)//联锁非常站控模式
    {
        bFCZK = true;
    }
    else//CTC模式
    {
        bFCZK = false;
    }
    if(bFCZK != m_nFCZKMode)//模式切换了
    {
        m_bModeChanged = true;
    }
    m_nFCZKMode = bFCZK;

    //咽喉总锁状态上行(S引导总锁)
    m_nDCSXAllLock = (array[dataLength-6]&0x01)==0x01?true:false;
    //咽喉总锁状态下行(X引导总锁)
    m_nDCXXAllLock = (array[dataLength-6]&0x02)==0x02?true:false;
    //引导总锁状态处理
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            if(m_nDCSXAllLock)
            {
                gddc->m_nDCSXAllLock |= GDDC_S_LOCK;
            }
            else
            {
                gddc->m_nDCSXAllLock = gddc->m_nDCSXAllLock&(~GDDC_S_LOCK);
            }
            if(m_nDCXXAllLock)
            {
                gddc->m_nDCSXAllLock |= GDDC_X_LOCK;
            }
            else
            {
                gddc->m_nDCSXAllLock = gddc->m_nDCSXAllLock&(~GDDC_X_LOCK);
            }
        }
    }

    //允许转回灯  0为点灯，1为灭灯
    if(array[dataLength-7] & 0x01)//灭灯 493
    {
        m_bAllowZH = false;
    }
    else
    {
        m_bAllowZH = true;
    }
    m_nPDJS180 = array[dataLength-8];//坡道解锁倒计时180秒计数
    m_nSRJ180 = array[dataLength-9];//S人解180s
    m_nSRJ30 = array[dataLength-10];//S人解30s
    m_nXRJ180 = array[dataLength-11];//X人解180s
    m_nXRJ30 = array[dataLength-12];//X人解30s
    m_nSRJ60 = array[dataLength-13];//S人解60s
    m_nXRJ60 = array[dataLength-14];//X人解60s

    //上电解锁
    if(array[dataLength-15] & 0x01)
    {
        m_bSDJS = true;
    }
    else
    {
        m_bSDJS = false;
    }
}

void MyStation::updateGDDC_StateOfLS(CGDDC *gddc, int state)
{
    QDateTime time0;
    QDateTime timeNow = QDateTime::currentDateTime();
    //设置状态
    if((state & 0x08) == 0x08)//故障
    {
        gddc->setDCWZ(DCSK);
        gddc->setDCState(4);

        //非联锁停电故障
        if(!m_bLSPowerFailure)
        {
            if(gddc->m_TimeDCSK == time0)
            {
                gddc->m_TimeDCSK = timeNow;
            }
            //四开时间比当前时间早，则为正值
            qint64 seconds = gddc->m_TimeDCSK.secsTo(timeNow);
            //qDebug()<<"seconds="<<seconds;
            if(!gddc->m_bWarning && seconds==15)
            {
                QString msg = QString("CTC:[%1]道岔挤岔").arg(gddc->getName());
                //发送报警提示信息
                this->sendWarningMsgToCTC(1,2,msg);
                //语音播报
                this->SendSpeachText("道岔挤岔");
                //道岔挤岔
                gddc->m_bWarning = true;
            }
        }
    }
    else if((state & 0x01) == 0x01)//反位
    {
        gddc->setDCWZ(DCFW);
        gddc->setDCState(2);
        gddc->m_TimeDCSK = time0;
        gddc->m_bWarning = false;
    }
    else if((state & 0x01) == 0x00)//定位
    {
        gddc->setDCWZ(DCDW);
        gddc->setDCState(1);
        gddc->m_TimeDCSK = time0;
        gddc->m_bWarning = false;
    }
    else
    {
        gddc->setDCWZ(DCSK);
    }
    if((state & 0x02) == 0x02)//锁闭
    {
        gddc->setIsDS(true);
    }
    else
    {
        gddc->setIsDS(false);
    }
    if((state & 0x04) == 0x04)//封锁
    {
        gddc->setIsFS(true);
    }
    else
    {
        gddc->setIsFS(false);
    }

//    if(gddc->getOldState(QDZY) && !gddc->getState(QDZY))
//    {
//        //分路不良的区段， 在进路出清后， 会自动变为“未确认空闲” 的闪烁状态 20180510
////        if( (gddc->isFLBL[0] || gddc->isFLBL[1] || gddc->isFLBL[2]) && gddc->m_nCQFLBLKX)
////        {
////            gddc->m_nCQFLBLKX = false;
////        }
////        if (gddc->flblStatusDW==2 || gddc->flblStatusFW==2 || gddc->flblStatusCQ==2)
////        {
////            gddc->flblStatusDW = 1;
////            gddc->flblStatusFW = 1;
////            gddc->flblStatusCQ = 1;
////        }
//        if(gddc->flblStatusDW == 2)
//        {
//            gddc->flblStatusDW = 1;
//        }
//        if(gddc->flblStatusFW == 2)
//        {
//            gddc->flblStatusFW = 1;
//        }
//        if(gddc->flblStatusCQ == 2)
//        {
//            gddc->flblStatusCQ = 1;
//        }
//        gddc->m_nOldState2 = gddc->m_nOldState;
//        gddc->m_nOldState  = gddc->getState();
//    }
//    if ((gddc->getOldState(QDSB) && gddc->getState(QDKX)))
//    {
//        //分路不良的区段， 在进路出清后， 会自动变为“未确认空闲” 的闪烁状态 20180510
//        //进路办理后又取消进路，则自动变为“未确认空闲” 的闪烁状态 2021.1.22
//        if(gddc->flblStatusDW == 2)
//        {
//            gddc->flblStatusDW = 1;
//        }
//        if(gddc->flblStatusFW == 2)
//        {
//            gddc->flblStatusFW = 1;
//        }
//        if(gddc->flblStatusCQ == 2)
//        {
//            gddc->flblStatusCQ = 1;
//        }
//        gddc->m_nOldState2 = gddc->m_nOldState;
//        gddc->m_nOldState  = gddc->getState();
//    }
}

void MyStation::updateXHD_StateOfLS(CXHD *xhd, int state1)
{
    if((state1 & 0x0f)==0x07)	//开放红灯
    {
        xhd->setXHDState(XHD_HD);
    }
    else if((state1 & 0x0f) == 0x03)//开放绿灯
    {
        xhd->setXHDState(XHD_LD);
    }
    else  if((state1 & 0x0f) == 0x04)//开放黄灯
    {
        xhd->setXHDState(XHD_UD);
    }
    else if((state1 & 0x0f) == 0x01)//开放白灯
    {
        xhd->setXHDState(XHD_BD);
    }
    else if((state1 & 0x0f) == 0x08)//开放蓝灯
    {
        xhd->setXHDState(XHD_AD);
    }
    else if((state1 & 0x0f) == 0x05)//开放绿黄灯
    {
        xhd->setXHDState(XHD_LU);
    }
    else if((state1 & 0x0f) == 0x02)//开放红白灯
    {
        xhd->setXHDState(XHD_YD);
    }
    else if((state1 & 0x0f) == 0x06)//开放双黄灯
    {
        xhd->setXHDState(XHD_UU);
    }
    else if((state1 & 0x0f) == 0x09)//断丝
    {
        xhd->setXHDState(XHD_DS);
    }
    else if((state1 & 0x0f) == 0x0A)//黄闪黄
    {
        xhd->setXHDState(XHD_USU);
    }
    else if((state1 & 0x0f) == 0x0C)//双绿
    {
        xhd->setXHDState(XHD_LL);
    }

    if((state1 & 0x10) == 0x10)//点灯灭灯
    {
        xhd->setIsMD(false);//1是点灯
    }
    else
    {
        xhd->setIsMD(true);
    }
    if((state1 & 0x20) == 0x20)//封闭
    {
        xhd->m_nFuncLockState = 1;
        xhd->setIsDCANFB(true);
        xhd->setIsLCANFB(true);
        xhd->setIsYDANFB(true);
    }
    else
    {
        xhd->m_nFuncLockState = 0;
        xhd->setIsDCANFB(false);
        xhd->setIsLCANFB(false);
        xhd->setIsYDANFB(false);
    }
}

void MyStation::updateGD_StateOfLS(CGD *gd, int state)
{
    if((state & 0x01) == 0x01)//区段占用
    {
        gd->setState(QDZY);
        gd->setQDColor(Qt::red);
    }
    else if((state & 0x02) == 0x02)//区段锁闭
    {
        gd->setState(QDSB);
        gd->setQDColor(Qt::white);
    }
    else//区段空闲
    {
        gd->setState(QDKX);
        gd->setQDColor(SkyBlue);
    }

//    if( gd->getOldState(QDZY) && !gd->getState(QDZY) )
//    {
//        //分路不良的区段， 在进路出清后， 会自动变为“未确认空闲” 的闪烁状态 20180510
//        if( (gd->isGDFLBL==true) && (gd->m_nGDFLBLKX == true))
//        {
//            gd->m_nGDFLBLKX  = false;
//            gd->m_nOldState2 = gd->m_nOldState;
//            gd->m_nOldState  = gd->getState();
//        }
//    }
}

void MyStation::updateQD_StateOfLS(CQD *qd, int state)
{
    if((state & 0x01) == 0x01)//区段占用
    {
        qd->setState(QDZY);
        setQD_Color(qd, Qt::red, state);
    }
    else if((state & 0x02) == 0x02)//区段锁闭
    {
        qd->setState(QDSB);
        setQD_Color(qd, Qt::white, state);
    }
    else//区段空闲
    {
        qd->setState(QDKX);
        setQD_Color(qd, SkyBlue, state);
    }
}
void MyStation::setQD_Color(CQD *qd,QColor color,int state)
{
    if(qd==NULL)
    {
        return;
    }
    for(int i=0;i<qd->getChildNum();i++)
    {
        for(int j=0;j<DevArray.size();j++)
        {
            if(qd->getChild(i) == (int)DevArray[j]->getCode())
            {
                if(DevArray[j]->getDevType() == Dev_DC)
                {
                    CGDDC *gddc;
                    gddc=(CGDDC*)(DevArray[j]);
                    gddc->setQDColor(color);
                    if(qd->getState(QDZY))
                    {
                       gddc->setQDState(QDZY);
                    }
                    else if(qd->getState(QDSB))
                    {
                       gddc->setQDState(QDSB);
                    }
                    else if(qd->getState(QDKX))
                    {
                       gddc->setQDState(QDKX);
                    }
                    if(gddc->getOldQDState(QDZY) && !gddc->getQDState(QDZY))
                    {
                        //分路不良的区段， 在进路出清后， 会自动变为“未确认空闲” 的闪烁状态 20180510
                        if(gddc->flblStatusDW == 2)
                        {
                            gddc->flblStatusDW = 1;
                        }
                        if(gddc->flblStatusFW == 2)
                        {
                            gddc->flblStatusFW = 1;
                        }
                        if(gddc->flblStatusCQ == 2)
                        {
                            gddc->flblStatusCQ = 1;
                        }
                        gddc->m_nOldQDState2 = gddc->m_nOldQDState;
                        gddc->m_nOldQDState  = gddc->getQDState();
                    }
                    if ((gddc->getOldQDState(QDSB) && gddc->getQDState(QDKX)))
                    {
                        //分路不良的区段， 在进路出清后， 会自动变为“未确认空闲” 的闪烁状态 20180510
                        //进路办理后又取消进路，则自动变为“未确认空闲” 的闪烁状态 2021.1.22
                        if(gddc->flblStatusDW == 2)
                        {
                            gddc->flblStatusDW = 1;
                        }
                        if(gddc->flblStatusFW == 2)
                        {
                            gddc->flblStatusFW = 1;
                        }
                        if(gddc->flblStatusCQ == 2)
                        {
                            gddc->flblStatusCQ = 1;
                        }
                        gddc->m_nOldQDState2 = gddc->m_nOldQDState;
                        gddc->m_nOldQDState  = gddc->getQDState();
                    }
                    break;
                }
                else if(DevArray[j]->getDevType() == Dev_GD)
                {
                    CGD *gd;
                    gd=(CGD*)(DevArray[j]);
                    gd->setQDColor(color);
                    if(qd->getState(QDZY))
                    {
                       gd->setState(QDZY);
                    }
                    else if(qd->getState(QDSB))
                    {
                       gd->setState(QDSB);
                    }
                    else if(qd->getState(QDKX))
                    {
                       gd->setState(QDKX);
                    }
                    //闭塞区间发送码
                    if((state&0x70) == 0x10)//L码 //0111 0000
                    {
                        gd->bsqdfmCode = 0x11;
                    }
                    else if((state&0x70) == 0x20)//LU码
                    {
                        gd->bsqdfmCode = 0x0F;
                    }
                    else if((state&0x70) == 0x30)//U码
                    {
                        gd->bsqdfmCode = 0x0C;
                    }
                    else if((state&0x70) == 0x40)//HU码
                    {
                        gd->bsqdfmCode = 0x03;
                    }
                    else //灭灯
                    {
                        gd->bsqdfmCode = 0x00;
                    }
                    //gd->bsqdfmCode = state&0x70;
                    if((state&0x80) == 0x80)
                    {
                        gd->bsqdfmDirection = 2;//向右
                    }
                    else
                    {
                        gd->bsqdfmDirection = 1;//向左
                    }

                    if( gd->getOldState(QDZY) && !gd->getState(QDZY) )
                    {
                        //分路不良的区段， 在进路出清后， 会自动变为“未确认空闲” 的闪烁状态 20180510
                        if( (gd->isGDFLBL==true) && (gd->m_nGDFLBLKX == true))
                        {
                            gd->m_nGDFLBLKX  = false;
                            gd->m_nOldState2 = gd->m_nOldState;
                            gd->m_nOldState  = gd->getState();
                        }
                        if(gd->flblStatus == 2)
                        {
                            gd->flblStatus  = 1;
                            gd->m_nOldState2 = gd->m_nOldState;
                            gd->m_nOldState  = gd->getState();
                        }

                    }
                    if ((gd->getOldState(QDSB) && gd->getState(QDKX)))
                    {
                        //AfxMessageBox(pGD->m_strName);
                        //分路不良的区段， 在进路出清后， 会自动变为“未确认空闲” 的闪烁状态 20180510
                        //进路办理后又取消进路，则自动变为“未确认空闲” 的闪烁状态 2021.1.22
                        if ((gd->isGDFLBL == true) && (gd->m_nGDFLBLKX == true))
                        {
                            gd->m_nGDFLBLKX = false;
                            gd->m_nOldState2 = gd->m_nOldState;
                            gd->m_nOldState = gd->getState();
                        }
                        if(gd->flblStatus == 2)
                        {
                            gd->flblStatus  = 1;
                            gd->m_nOldState2 = gd->m_nOldState;
                            gd->m_nOldState  = gd->getState();
                        }
                    }
                    break;
                }
            }
        }
    }
}

//道岔尖轨心轨状态
int MyStation::updateDCJGXGStatus(unsigned char *array, int count)
{
    int nCode = 0;
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            int state=array[count+nCode];
            //道岔其它状态解析
            if ((state&0x01) == 0x01)//尖轨故障
            {
                gddc->isJGGZ = true;
            }
            else
            {
                gddc->isJGGZ = false;
            }
            if ((state&0x02) == 0x02)//心轨故障
            {
                gddc->isXGGZ = true;
            }
            else
            {
                gddc->isXGGZ = false;
            }
            nCode++;
        }
    }
    return nCode;
}
//更新信号机倒计时
int MyStation::updateXHDTimeCount(unsigned char *array, int count)
{
    int nCode = 0;
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            int timeCount = array[count+nCode]&0xFF;
            if(timeCount>0)
            {
//                qDebug()<<"XHD-timeCount="<<timeCount;
            }
            xhd->setTimeCount(timeCount);
            nCode++;
        }
    }
    return nCode;
}
//更新腰岔解锁+股道确认
int MyStation::updateYXJSGDRQ(unsigned char *array, int count)
{
    int nCode = 0;
    //更新腰岔解锁
    int YCJSCount = array[count+nCode]&0xFF;
    nCode++;
    int nCount = 0;
    for(int i=0; (i<vectYCJS.size()) && (YCJSCount==vectYCJS.size()); i++)
    {
        //是否锁闭
        bool bLocking = (array[count+nCode+nCount]&0x01 == 0x01)?true:false;
        vectYCJS[i].bLocking = bLocking;
        nCount++;
        //倒计时
        int timeCount = array[count+nCode+nCount]&0xFF;
        vectYCJS[i].countdown = timeCount;
        //qDebug()<<QString("vectYCJS[%1].countdown=%2").arg(i).arg(timeCount);
        nCount++;
    }
    nCode +=(YCJSCount*2);

    //更新股道确认
    int GDQRCount = array[count+nCode]&0xFF;
    nCode++;
    nCount = 0;
    for(int i=0; (i<vectGDQR.size()) && (GDQRCount==vectGDQR.size()); i++)
    {
        //倒计时
        int timeCount = array[count+nCode+nCount]&0xFF;
        vectGDQR[i].countdown = timeCount;
        //qDebug()<<QString("vectGDQR[%1].countdown=%2").arg(i).arg(timeCount);
        nCount++;
    }
    nCode+=(GDQRCount*1);

    return nCode;
}
//判断是否联锁状态停电故障
bool MyStation::CheckStationPowerFailureOfLS(unsigned char *array)
{
    //站场停电故障
    //bool bPowerFailure = false;
    int dcNum=(array[10]);
    int xhdNum=(array[11]);
    int qdNum=(array[12]);
    int count=13;
    //道岔故障个数
    int dcFailureCount = 0;
    //区段故障个数
    int qdFailureCount = 0;
    //信号机故障个数
    int XHDFailureCount = 0;
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
            //updateGDDC_StateOfLS(gddc,state);
            //判断状态
            if((state & 0x08) == 0x08)//故障
            {
                dcFailureCount++;
            }
            num++;
        }
    }
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
            //包含信号机灯丝报警状态
            if(StaConfigInfo.bXHDDSBJStatus)
            {
                count+=2;
            }
            else
            {
                count++;
            }
            num++;
        }
    }
    //区段状态
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
            //包含信号机灯丝报警状态
            if(StaConfigInfo.bXHDDSBJStatus)
            {
                count+=2;
            }
            else
            {
                count++;
            }
            num++;
        }
    }
    //区段状态
    int num1=0;
    for(auto ement:DevArray)
    {
        if(num1>qdNum)
        {
            break;
        }
        if(ement->getDevType() == Dev_QD)
        {
            CQD *qd=(CQD *)ement;
            int state1=array[count];
            count++;
            //updateQD_StateOfLS(qd,state1);
            if((state1 & 0x01) == 0x01)//区段占用
            {
                qdFailureCount++;
            }
            num1++;
        }
    }
//    qDebug()<<QString("%1,dcNum=%2,dcFailCount=%3,qdNum=%4,qdFailCount=%5,")
//              .arg(this->getStationName())
//              .arg(dcNum).arg(dcFailureCount).arg(num1).arg(qdFailureCount);

    //所有道岔都故障 && 超过2/3的区段故障占用
    if((dcFailureCount==dcNum) && (qdFailureCount>=(qdNum/3*2)))
    {
        qDebug()<<QString("%1,联锁站场停电故障!").arg(this->getStationName());
        return true;
    }
    else
    {
        return false;
    }
}

//处理功能按钮命令,处理成功则返回true
bool MyStation::dealFuncBtnCmd(unsigned char* StatusArray, int nLength)
{
    if ((int)StatusArray[9] != PLAN_CMD_FUNC || nLength<18)
        return false;
    bool bRtValue = false;
    //功能分类码
    int funcCode = StatusArray[10];
    //操作设备号123
    int devCode1 = (int)(StatusArray[11] | (StatusArray[12]<<8));
    int devCode2 = (int)(StatusArray[13] | (StatusArray[14]<<8));
    int devCode3 = (int)(StatusArray[15] | (StatusArray[16]<<8));
    QString devName1 = GetStrNameByCode(devCode1);
    QString devType1 = getTypeByCode(devCode1);
    //功能码
    switch(funcCode)
    {
    case 0x10://封锁
        {
            if("CGD" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGD *pGD = (CGD*)DevArray[devIndex];
                    pGD->isLock = true;
                    bRtValue = true;
                    setAllQJLock(true, devName1);
                }
            }
        }
        break;
    case 0x11://解封
        {
            if("CGD" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGD *pGD = (CGD*)DevArray[devIndex];
                    pGD->isLock = false;
                    bRtValue = true;
                    setAllQJLock(false, devName1);
                }
            }
        }
        break;
    case 0x16://区段分路不良
        {
            if("CGD" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGD *pGD = (CGD*)DevArray[devIndex];
                    if(pGD->flblStatus>0)
                    {
                        //pGD->setGDFLBL(false);
                        //pGD->m_nGDFLBLKX = false;
                        pGD->flblStatus = 0;
                    }
                    else
                    {
                        //pGD->setGDFLBL(true);
                        pGD->flblStatus = 1;
                    }
                    bRtValue = true;
                    qDebug()<<"分路不良"<<devName1<<devType1<<pGD->flblStatus;
                }
            }
        }
        break;
    case 0x30://岔前分路不良
    case 0x31://定位分路不良
    case 0x32://反位分路不良
        {
            if("CGDDC" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGDDC *pGDDC = (CGDDC*)DevArray[devIndex];
                    if(0x30 == funcCode)//岔前分路不良
                    {
//                        //if(pGDDC->getCQFLBL())
//                        if(pGDDC->flblStatusCQ)
//                        {
//                            //pGDDC->m_nCQFLBLKX = false;
//                            //pGDDC->setCQFLBL(false);
//                            pGDDC->flblStatusCQ = 0;
//                        }
//                        else
//                        {
//                            //pGDDC->setCQFLBL(true);
//                            pGDDC->flblStatusCQ = 1;
//                            //其他位置的分路若空闲则变回分路
//                            if(pGDDC->flblStatusDW)
//                            {
//                                pGDDC->flblStatusDW = 1;
//                            }
//                            if(pGDDC->flblStatusFW)
//                            {
//                                pGDDC->flblStatusFW = 1;
//                            }
//                        }
                        //其他位置是确认空闲
                        if(pGDDC->flblStatusDW==2 || pGDDC->flblStatusFW==2)
                        {
                            if(pGDDC->flblStatusCQ == 0)
                            {
                                pGDDC->flblStatusCQ = 2;//也为确认空闲
                            }
                            else
                            {
                                pGDDC->flblStatusCQ = 0;//取消分路
                            }
                        }
                        else
                        {
                            if(pGDDC->flblStatusCQ>0)
                            {
                                pGDDC->flblStatusCQ = 0;
                            }
                            else
                            {
                                pGDDC->flblStatusCQ = 1;
                            }
                        }
                    }
                    else if(0x31 == funcCode)//定位分路不良
                    {
//                        //if(pGDDC->getDWFLBL())
//                        if(pGDDC->flblStatusDW)
//                        {
//                            //pGDDC->setDWFLBL(false);
//                            //pGDDC->m_nCQFLBLKX = false;
//                            pGDDC->flblStatusDW = 0;
//                        }
//                        else
//                        {
//                            //pGDDC->setDWFLBL(true);
//                            pGDDC->flblStatusDW = 1;
//                            //其他位置的分路若空闲则变回分路
//                            if(pGDDC->flblStatusCQ)
//                            {
//                                pGDDC->flblStatusCQ = 1;
//                            }
//                            if(pGDDC->flblStatusFW)
//                            {
//                                pGDDC->flblStatusFW = 1;
//                            }
//                        }
                        //其他位置是确认空闲
                        if(pGDDC->flblStatusCQ==2 || pGDDC->flblStatusFW==2)
                        {
                            if(pGDDC->flblStatusDW == 0)
                            {
                                pGDDC->flblStatusDW = 2;//也为确认空闲
                            }
                            else
                            {
                                pGDDC->flblStatusDW = 0;//取消分路
                            }
                        }
                        else
                        {
                            if(pGDDC->flblStatusDW>0)
                            {
                                pGDDC->flblStatusDW = 0;
                            }
                            else
                            {
                                pGDDC->flblStatusDW = 1;
                            }
                        }
                    }
                    else if(0x32 == funcCode)//反位分路不良
                    {
//                        //if(pGDDC->getFWFLBL())
//                        if(pGDDC->flblStatusFW)
//                        {
//                            //pGDDC->setFWFLBL(false);
//                            //pGDDC->m_nCQFLBLKX = false;
//                            pGDDC->flblStatusFW = 0;
//                        }
//                        else
//                        {
//                            //pGDDC->setFWFLBL(true);
//                            pGDDC->flblStatusFW = 1;
//                            //其他位置的分路若空闲则变回分路
//                            if(pGDDC->flblStatusCQ)
//                            {
//                                pGDDC->flblStatusCQ = 1;
//                            }
//                            if(pGDDC->flblStatusDW)
//                            {
//                                pGDDC->flblStatusDW = 1;
//                            }
//                        }
                        //其他位置是确认空闲
                        if(pGDDC->flblStatusCQ==2 || pGDDC->flblStatusDW==2)
                        {
                            if(pGDDC->flblStatusFW == 0)
                            {
                                pGDDC->flblStatusFW = 2;//也为确认空闲
                            }
                            else
                            {
                                pGDDC->flblStatusFW = 0;//取消分路
                            }
                        }
                        else
                        {
                            if(pGDDC->flblStatusFW>0)
                            {
                                pGDDC->flblStatusFW = 0;
                            }
                            else
                            {
                                pGDDC->flblStatusFW = 1;
                            }
                        }
                    }
                    bRtValue = true;
                }
            }
        }
        break;
    case 0x43://确认空闲(道岔+股道)
        {
            if("CGD" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGD *pGD = (CGD*)DevArray[devIndex];
                    //pGD->flblStatus = 2;
                    //pGD->m_nGDFLBLKX = true;
                    //if(pGD->flblStatus == 1)
                    //{
                    //    pGD->flblStatus = 2;
                    //}
                    //else if(pGD->flblStatus == 2)
                    //{
                    //    pGD->flblStatus = 1;
                    //}
                    if(pGD->flblStatus == 2)
                    {
                        pGD->flblStatus = 1;
                    }
                    else if(pGD->flblStatus == 1)
                    {
                        pGD->flblStatus = 2;
                    }
                    qDebug()<<"确认空闲"<<devName1<<devType1<<pGD->flblStatus;
                }
            }
            else //if("CGDDC" == devType1)
            {
                //新版本道岔下发道岔设备号
                int devIndex = GetIndexByStrName(devName1);
                //道岔下发的区段号
                //int devIndex = StatusArray[13];
                if(devIndex > -1)
                {
                    CGDDC *pGDDC = (CGDDC*)DevArray[devIndex];
                    ////pGDDC->m_nCQFLBLKX = TRUE;
                    //if (pGDDC->m_nCQFLBLKX == true)
                    //{
                    //    pGDDC->m_nCQFLBLKX = false;
                    //}
                    //else
                    //{
                    //    pGDDC->m_nCQFLBLKX = true;
                    //}
//                    if (pGDDC->flblStatusDW==2 || pGDDC->flblStatusFW==2 || pGDDC->flblStatusCQ==2)
//                    {
//                        pGDDC->flblStatusDW = 1;
//                        pGDDC->flblStatusFW = 1;
//                        pGDDC->flblStatusCQ = 1;
//                    }
//                    else
//                    {
//                        pGDDC->flblStatusDW = 2;
//                        pGDDC->flblStatusFW = 2;
//                        pGDDC->flblStatusCQ = 2;
//                    }
                    //定位
                    if (pGDDC->flblStatusDW==1)
                    {
                        pGDDC->flblStatusDW = 2;
                    }
                    else if (pGDDC->flblStatusDW==2)
                    {
                        pGDDC->flblStatusDW = 1;
                    }
                    //反位
                    if (pGDDC->flblStatusFW==1)
                    {
                        pGDDC->flblStatusFW = 2;
                    }
                    else if (pGDDC->flblStatusFW==2)
                    {
                        pGDDC->flblStatusFW = 1;
                    }
                    //岔前
                    if (pGDDC->flblStatusCQ==1)
                    {
                        pGDDC->flblStatusCQ = 2;
                    }
                    else if (pGDDC->flblStatusCQ==2)
                    {
                        pGDDC->flblStatusCQ = 1;
                    }

                }
            }
            bRtValue = true;
        }
        break;
    case 0x45://区段停电
        {
            if("CGD" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGD *pGD = (CGD*)DevArray[devIndex];
                    pGD->isPowerCut = true;
                    bRtValue = true;
                }
            }
        }
        break;
    case 0x46://区段供电
        {
            if("CGD" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGD *pGD = (CGD*)DevArray[devIndex];
                    pGD->isPowerCut = false;
                    bRtValue = true;
                }
            }
        }
        break;
    case 0x55://0x49://道岔定位停电
        {
            if("CGDDC" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGDDC *pGDDC = (CGDDC*)DevArray[devIndex];
                    pGDDC->isPowerCutDW = true;
                    bRtValue = true;
                }
            }
        }
        break;
    case 0x56://0x50://道岔定位供电
        {
            if("CGDDC" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                {
                    CGDDC *pGDDC = (CGDDC*)DevArray[devIndex];
                    pGDDC->isPowerCutDW = false;
                    bRtValue = true;
                }
            }
        }
        break;
    case 0x57://0x51://道岔反位停电
        {
            if("CGDDC" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGDDC *pGDDC = (CGDDC*)DevArray[devIndex];
                    pGDDC->isPowerCutFW = true;
                    bRtValue = true;
                }
            }
        }
        break;
    case 0x58://0x52://道岔反位供电
        {
            if("CGDDC" == devType1)
            {
                int devIndex = GetIndexByStrName(devName1);
                if(devIndex > -1)
                {
                    CGDDC *pGDDC = (CGDDC*)DevArray[devIndex];
                    pGDDC->isPowerCutFW = false;
                    bRtValue = true;
                }
            }
        }
        break;
    default:
        bRtValue = false;
        break;
    }
    return bRtValue;
}
//根据设备号获取设备类型（道岔、信号、股道）
QString MyStation::getTypeByCode(int nCode)
{
    CGD *pGD;
    CXHD *pXHD;
    CGDDC *pGDDC;
    for(int i = 0; i < (int)DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD)
        {
            pGD = (CGD*)DevArray[i];
            if(nCode == (int)pGD->m_nCode)
            {
                return "CGD";
            }
        }
        else if(DevArray[i]->getDevType() == Dev_XH)
        {
            pXHD = (CXHD*)DevArray[i];
            if(nCode == (int)pXHD->m_nCode)
            {
                return "CXHD";
            }
        }
        else if(DevArray[i]->getDevType() == Dev_DC)
        {
            pGDDC = (CGDDC*)DevArray[i];
            if(nCode == (int)pGDDC->m_nCode)
            {
                return "CGDDC";
            }
        }
    }
    return "";//"未知"
}
//设置所有区间封锁解封
void MyStation::setAllQJLock(bool bLock, QString strQj)
{
    if(!StaConfigInfo.bQJAllLock)
            return;

    int ct = StaConfigInfo.JCKCount;
    for (int t = 0; t < ct; t++)
    {
        for(int k=0; k<StaConfigInfo.JFCKInfo[t].strArrJckJJQD.count(); k++)
        {
            QString jfkName = StaConfigInfo.JFCKInfo[t].strArrJckJJQD.at(k);
            if(strQj == jfkName)
            {
                for(int kk=0; kk<StaConfigInfo.JFCKInfo[t].strArrJckJJQD.count(); kk++)
                {
                    jfkName = StaConfigInfo.JFCKInfo[t].strArrJckJJQD.at(kk);
                    int devIndex = GetIndexByStrName(jfkName);
                    if(devIndex > -1)
                    {
                        CGD* pGD = (CGD*)DevArray[devIndex];
                        pGD->isLock = bLock;
                    }
                }
                return;
            }
        }
        for(int k=0; k<StaConfigInfo.JFCKInfo[t].strArrFckLQQD.count(); k++)
        {
            QString jfkName = StaConfigInfo.JFCKInfo[t].strArrFckLQQD.at(k);
            if(strQj == jfkName)
            {
                for(int kk=0; kk<StaConfigInfo.JFCKInfo[t].strArrFckLQQD.count(); kk++)
                {
                    jfkName = StaConfigInfo.JFCKInfo[t].strArrFckLQQD.at(kk);
                    int devIndex = GetIndexByStrName(jfkName);
                    if(devIndex > -1)
                    {
                        CGD* pGD = (CGD*)DevArray[devIndex];
                        pGD->isLock = bLock;
                    }
                }
                return;
            }
        }
    }
}
//控制模式转换时取消分路不良区段确认空闲状态(站内区段、道岔)
void MyStation::CancleFlblKXFalg()
{
    CGD *pGD;
    CGDDC *pGDDC;
    for(int i = 0; i < (int)DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD)
        {
            pGD = (CGD*)DevArray[i];
            if(pGD->flblStatus == 2 && pGD->getGDType()!=JJ_QD)
            {
                pGD->flblStatus=1;
            }
        }
        else if(DevArray[i]->getDevType() == Dev_DC)
        {
            pGDDC = (CGDDC*)DevArray[i];
            if(pGDDC->flblStatusCQ == 2)
            {
                pGDDC->flblStatusCQ = 1;
            }
            if(pGDDC->flblStatusDW == 2)
            {
                pGDDC->flblStatusDW = 1;
            }
            if(pGDDC->flblStatusFW == 2)
            {
                pGDDC->flblStatusFW = 1;
            }
        }
    }
}
//处理发给联锁的功能按钮命令,处理成功则返回0，否则返回错误码
int MyStation::dealFuncBtnCmdToLS(unsigned char* StatusArray, int nLength, int currCtcIndex)
{
    int funcType = (int)StatusArray[9];
    if (funcType != PLAN_CMD_FUNC || nLength<18)
        return false;
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(StatusArray, nLength);
    //返回值
    int bRtValue = 0;
    //功能分类码
    int funcCode = (int)StatusArray[10];
//    //操作设备号123
//    int devCode1 = (int)(StatusArray[11] | (StatusArray[12]<<8));
//    QString devName1 = GetStrNameByCode(devCode1);
//    QString devType1 = getTypeByCode(devCode1);
    //解析位置
    int nCount = 11;
    //进路按钮数组
    QVector<CmdJLBtn> cmdBtnArray;
    //进路设备名称数组
    QStringList cmdDevNameList;
    //进路按钮数组
    QStringList routeBtnTempArray;
    //提示信息
    QString strMsg;

    //进路办理
    if(0x01==funcCode || 0x02==funcCode || 0x03==funcCode)
    {
        CXHD* pXHDFirst = nullptr;
        //固定8个设备处理
        for(int i=0; i<8; i++)
        {
            CmdJLBtn cmd;
            //设备号
            int devCode = (int)(StatusArray[nCount+(2*i)] | (StatusArray[nCount+(2*i+1)]&MASK05)<<8);
            //按钮类型
            int btnType = (int)((StatusArray[nCount+(2*i+1)]&(~MASK05))>>6);
            //设备号为0时不再增加
            if(devCode == 0)
            {
                break;
            }

            cmd.devCode = devCode;
            cmd.btnType = btnType;
            cmdBtnArray.append(cmd);
            QString devName = this->GetStrNameByCode(devCode);
            if(devName=="")
            {
                break;
            }
            int devIndex = this->GetIndexByStrName(devName);
            if(devIndex == -1)
            {
                break;
            }
            CXHD *pXHD = (CXHD*)this->DevArray[devIndex];
            if(0==i)
            {
                pXHDFirst = pXHD;
            }
            cmdDevNameList.append(pXHD->getName());
            QString btnName;
            btnName = this->GetBtnNameInAloneXHD(devCode, btnType);
            if(btnName == "")
            {
                if (pXHD->getSignalType())
                {
                    btnName = pXHD->getName();
                }
                else if (pXHD->getXHDType() == DC_XHJ)
                {
                    btnName = pXHD->getName() + "A";
                }
                else if (devName.right(2)=="DA")//devName.indexOf("DA") > -1 //若需要按下列车信号机的调车按钮，则变通进路中的列车信号机需要配置“DA”！
                {
                    btnName = pXHD->getName();
                }
                else
                {
                    if(0x00==btnType)
                    {
                        btnName = pXHD->getName() + "LA";
                    }
                    else if(0x01==btnType)
                    {
                        btnName = pXHD->getName() + "DA";
                    }
                    else if(0x02==btnType)
                    {
                        btnName = pXHD->getName() + "TA";
                    }
                    else
                    {
                        btnName = pXHD->getName();
                    }
                }
            }
            routeBtnTempArray.append(btnName);
        }

        //引导保持判断
        //引导保持开放需要发送列车进路标志
        if(0x01==funcCode && pXHDFirst != nullptr && cmdDevNameList.size()==1)
        {
            //红白(引导) && 有倒计时
            if(pXHDFirst->getXHDState() == XHD_YD && pXHDFirst->getTimeCount()>0)
            {
                return 0;//发给联锁
            }
        }

        //进路报警类型
        int JLWarningType = 0;
        QString JLWarningMsg = "";
        //进路是否存在
        int nIndexRoute = -1;
        //列车进路+通过进路
        if(0x01==funcCode || 0x03==funcCode)
        {
            //-->去匹配进路序列计划来判断
            TrainRouteOrder* pTrainRouteOrder = nullptr;
            //车次长度
            int len = 0;
            //车次
            QString inputCheCi = "";
            //车次解析位置（固定位置）
            nCount = 10+2*8+1;//27
            //防溢出
            if(nCount<qRecvArray.size()-4)
            {
                //车次长度
                len = (int)(qRecvArray[nCount]&0xFF);
                nCount++;
                inputCheCi = ByteArrayToUnicode(qRecvArray.mid(nCount,len));
                qDebug()<<"CTC inputCheCi="<<inputCheCi;
            }

            //列车进路
            if(0x01==funcCode)
            {
                nIndexRoute = this->FindRouteIndexInLSB(ROUTE_LC, routeBtnTempArray);
            }
            //通过进路
            else if(0x03==funcCode)
            {
                nIndexRoute = this->FindRouteIndexInLSB(ROUTE_TG, routeBtnTempArray);
            }
            //*******进路联锁关系判断*******
            //进路不存在则报警提示
            if(-1 == nIndexRoute)
            {
                //暂时进行提示
                this->SendMessageBoxMsg(3, "进路不存在，请重新操作！", currCtcIndex);
                return 1;
            }
            //进路存在
            else
            {
                CheckResult* ckResult = new CheckResult;
                QDateTime timeNow = QDateTime::currentDateTime();
                ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
                ckResult->indexRoute = nIndexRoute;

                //列车进路
                if(0x01==funcCode)
                {
                    strMsg = QString("建立列车进路\r\n");
                }
                //通过进路
                else if(0x03==funcCode)
                {
                    strMsg = QString("建立通过进路\r\n");
                }
//                for(int j=0; j<routeBtnTempArray.size(); j++)
//                {
//                    QString strTemp = QString("第%1个按钮：%2 ").arg(j+1).arg(routeBtnTempArray[j]);
//                    strMsg += strTemp;
//                    if(j == routeBtnTempArray.size()-1)
//                    {
//                        strMsg += "\r\n";
//                    }
//                }
                for(int j=0; j<cmdDevNameList.size(); j++)
                {
                    QString strTemp = QString("第%1个按钮：%2 ").arg(j+1).arg(cmdDevNameList[j]);
                    strMsg += strTemp;
                    if(j == cmdDevNameList.size()-1)
                    {
                        strMsg += "\r\n";
                    }
                }

                //进路防溜是否清除判断
                int flType = 0;
                //发车进路-从股道判断（出站信号机）
                if(CZ_XHJ == this->GetXHDType(pXHDFirst->getCode()))
                {
                    QString strGD = this->GetGDNameInGDNodeList(pXHDFirst->getName());
                    //检查股道防溜是否撤除
                    flType = this->CheckGDFL(strGD);//1上行 2下行
                }
                //接车进路-从联锁表判断
                else
                {
                    //进路防溜是否清除判断
                    flType = this->isGDFLClear(nIndexRoute);
                }
                if(flType>0)
                {
                    QString strSys = QString("违反站细规定\r\n");
                    if(1==flType)
                    {
                        strSys += QString("股道上行有防溜设备");
                    }
                    else if(2==flType)
                    {
                        strSys += QString("股道下行有防溜设备");
                    }
                    else
                    {
                        strSys += QString("股道下行有防溜设备，股道上行有防溜设备");
                    }
                    strMsg += strSys;
                    ckResult->bEnforced = false;
                    ckResult->checkMsg = strMsg;
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //发送操作报警信息
                    this->SendRouteCheckResult(ckResult, currCtcIndex);
                    //退出
                    return 1;
                }

                //*******车次信息判断*******
                //-->去匹配进路序列计划来判断
                //TrainRouteOrder* pTrainRouteOrder = nullptr;
                //进路检查通过
                //*******进路序列判断*******
                //输入了车次，则查找并更新相应进路序列的状态
                if(inputCheCi != "")
                {
                    pTrainRouteOrder = FindTrainRouteIndexByCmd(qRecvArray, inputCheCi);
                    if(pTrainRouteOrder != nullptr)
                    {
                        //命令按钮匹配，不匹配不可办理进路序列
                        QStringList routeDevList = pTrainRouteOrder->m_strRouteDescripReal.split(",");
                        bool bMath = false;
                        if(cmdDevNameList.size() == routeDevList.size())
                        {
                            bMath = true;
                            for(int c=0; c<routeDevList.size(); c++)
                            {
                                if(cmdDevNameList[c] != routeDevList[c])
                                {
                                    bMath = false;
                                }
                            }
                        }
                        if(bMath)
                        {
                            //人工办理-客车正线通过进路检查
                            if(pTrainRouteOrder->m_nLHFlg == LCTYPE_KC)
                            {
                                //正线通过进路检查
                                bool tgRouteIsZXTGJL = CheckZXTGJL(pTrainRouteOrder);
                                if(!tgRouteIsZXTGJL)
                                {
                                    CheckResult* ckResult = new CheckResult;
                                    QDateTime timeNow = QDateTime::currentDateTime();
                                    ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
                                    ckResult->route_id = pTrainRouteOrder->route_id;
                                    ckResult->check = JLWARNING_ROUTE_CXTG;
                                    QString strSys = QString("命令与计划冲突\r\n客车%1次在股道%2侧线通过")
                                            .arg(pTrainRouteOrder->m_strTrainNum)
                                            .arg(pTrainRouteOrder->m_strTrack);
                                    strMsg += strSys;
                                    ckResult->checkMsg = strMsg;
                                    ckResult->bEnforced = true;
                                    //可强制执行，增加交互
                                    if(ckResult->bEnforced)
                                    {
                                        ckResult->cmdArray.append(qRecvArray);
                                        UpdateCheckResultArray(ckResult);//交互，则增加
                                    }
                                    //发送操作报警信息
                                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                                    //发送系统消息信息
                                    //this->sendWarningMsgToCTC(1,2,strSys);
                                    bRtValue = ckResult->check;
                                    return bRtValue;
                                }
                            }


                            //【显示优先级顺序：无电>占用>锁闭>封锁>分路不良,当前时刻按照优先级只能显示一种。】
                            //CheckResult* ckResult = CheckPreventConditionAll(pTrainRouteOrder);

                            //************* 联锁条件 *************
                            CheckResult* ckResult = CheckPreventConditionInterlock(pTrainRouteOrder);
                            //无电
                            if(JLWARNING_QDPOWERCUT == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n电力牵引列车%1次无法通过无电进路(%2)")
                                        .arg(pTrainRouteOrder->m_strTrainNum).arg(pTrainRouteOrder->m_strRouteDescrip);
                                strMsg.append(str);
                                ckResult->bEnforced = false;//不可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //人工触发才发送强制执行信息
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//默认发送给第一个终端
                                return bRtValue;
                            }
                            //分路不良
                            if(JLWARNING_FLBL_GD == ckResult->check
                               || JLWARNING_FLBL_DC == ckResult->check
                               || JLWARNING_FLBL_GDKX == ckResult->check
                               )
                            {
                                //系统报警信息
                                QString strSys = QString("人工触发%1次%2进路(%3)联锁逻辑检查失败，进路不能办理，存在分路不良区段")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip);
                                QString strWar = QString("命令与联锁冲突\r\n进路不能办理，存在分路不良区段");
                                strMsg.append(strWar);
                                //强制执行判定
                                if((ckResult->check&JLWARNING_FLBL_DC) && !(ckResult->check&JLWARNING_FLBL_GD))
                                {
                                    ckResult->bEnforced = true;//可强制执行，道岔分路不良
                                }
                                else if(ckResult->check&JLWARNING_FLBL_GDKX)
                                {
                                    ckResult->bEnforced = true;//可强制执行，股道分路不良空闲
                                }
                                else
                                {
                                    ckResult->bEnforced = false;//不可强制执行，股道分路不良
                                }
                                ckResult->checkMsg = strMsg;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                                //发送系统消息信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                bRtValue = ckResult->check;
                                return bRtValue;
                            }

                            //************* 站细条件 *************
                            ckResult = CheckPreventConditionStaDetails(pTrainRouteOrder);
                            //不满足-股道类型
                            if(JLWARNING_ATTR_GDTYPE == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n列车%1次%2类型不满足")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                strMsg.append(str);
                                ckResult->bEnforced = false;//不可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有（兰州西站AB机均弹）
                                return bRtValue;
                            }
                            //不满足-超限条件
                            else if(JLWARNING_ATTR_LEVELCX == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n%1次股道%2超限条件不满足")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                strMsg.append(str);
                                ckResult->bEnforced = true;//可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有（兰州西站AB机均弹）
                                return bRtValue;
                            }
                            //不满足-客运设备股道（站台）
                            else if(JLWARNING_ATTR_PLATFORM1 == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n旅客列车%1次无法接入无客运股道%2")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                strMsg.append(str);
                                ckResult->bEnforced = false;//不可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有
                                return bRtValue;
                            }
                            //不满足-货车不可以接入高站台
                            else if(JLWARNING_ATTR_PLATFORM2 == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n货车%1次不能通过高站台股道%2")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                strMsg.append(str);
                                ckResult->bEnforced = true;//可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有（兰州西站AB机均弹）
                                return bRtValue;
                            }
                            //不满足-上水
                            else if(JLWARNING_ATTR_FLOWSS == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n%1次股道%2无上水设备")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                strMsg.append(str);
                                ckResult->bEnforced = true;//可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有
                                return bRtValue;
                            }
                            //不满足-吸污
                            else if(JLWARNING_ATTR_FLOWXW == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n%1次股道%2无吸污设备")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                strMsg.append(str);
                                ckResult->bEnforced = true;//可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有
                                return bRtValue;
                            }
                            //不满足-出入口超限
                            else if(JLWARNING_ENEX_LEVELCX == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n%1次%2方向不允许超限车(出入口%3)")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
                                strMsg.append(str);
                                ckResult->bEnforced = true;//可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有
                                return bRtValue;
                            }
                            //不满足-客货类型错误
                            else if(JLWARNING_ENEX_KHTYPE == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n%1次%2方向客货类型错误(出入口%3)")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
                                strMsg.append(str);
                                ckResult->bEnforced = true;//可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有
                                return bRtValue;
                            }
                            //不满足-军运
                            else if(JLWARNING_ATTR_ARMY == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n军用列车%1次无法进路非军用股道%2")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                strMsg.append(str);
                                ckResult->bEnforced = true;//可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有
                                return bRtValue;
                            }
                            //不满足-列车固定径路信息
                            else if(JLWARNING_ENEX_UNSAME == ckResult->check)
                            {
                                QString str = QString("命令与计划冲突\r\n%1次%2方向与固定路径不一致")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车");
                                strMsg.append(str);
                                ckResult->bEnforced = true;//可强制执行
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有
                                return bRtValue;
                            }

                            //************* 时序条件 *************
                            ckResult = this->CheckPreventConditionSequence(pTrainRouteOrder);
                            //不满足-时序车次冲突
                            if(JLWARNING_SEQU_CCCT == ckResult->check
                               || JLWARNING_SEQU_CROSS == ckResult->check)
                            {
                                strMsg.append(ckResult->checkMsg);
                                ckResult->checkMsg = strMsg;
                                bRtValue = ckResult->check;
                                //可强制执行，增加交互
                                if(ckResult->bEnforced)
                                {
                                    ckResult->cmdArray.append(qRecvArray);
                                    UpdateCheckResultArray(ckResult);//交互，则增加
                                }
                                //发送操作报警信息
                                this->SendRouteCheckResult(ckResult, -1);//发所有（兰州西站AB机均弹）
                                return bRtValue;
                            }
                        }
                        //进路不匹配（但输入了这个序列的车次）
                        else
                        {
                            CheckResult* ckResult = new CheckResult;
                            QDateTime timeNow = QDateTime::currentDateTime();
                            ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
                            ckResult->route_id = pTrainRouteOrder->route_id;
                            //联锁表索引
                            int idxLsb = GetTrainRouteOrderLSBRouteIndex(pTrainRouteOrder);
                            ckResult->indexRoute = idxLsb;
                            //提示信息
                            QString str = QString("命令与计划冲突\r\n%1次%2方向与计划不一致")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车");
                            strMsg.append(str);
                            ckResult->bEnforced = false;//不可强制执行
                            ckResult->checkMsg = strMsg;
                            ckResult->check = 1;
                            bRtValue = ckResult->check;
                            //发送操作报警信息
                            this->SendRouteCheckResult(ckResult, -1);//发所有（兰州西站AB机均弹）
                            return bRtValue;
                        }
                    }
                }
                //没有输入车次
                else
                {
//                    for(int i=0; i<this->m_ArrayRouteOrder.size(); i++)
//                    {
//                        TrainRouteOrder* pTrainRouteOrder1 = this->m_ArrayRouteOrder[i];
//                        //等待状态
//                        if(pTrainRouteOrder1->m_nOldRouteState==0)
//                        {
//                            //命令按钮匹配，不匹配不可办理进路序列
//                            QStringList routeDevList = pTrainRouteOrder1->m_strRouteDescripReal.split(",");
//                            bool bMath = false;
//                            if(cmdDevNameList.size() == routeDevList.size())
//                            {
//                                bMath = true;
//                                for(int c=0; c<routeDevList.size(); c++)
//                                {
//                                    if(cmdDevNameList[c] != routeDevList[c])
//                                    {
//                                        bMath = false;
//                                    }
//                                }
//                            }
//                            if(bMath)
//                            {
//                                QString strWar = QString("命令与计划冲突\r\n与%1次的计划冲突")
//                                        .arg(pTrainRouteOrder1->m_strTrainNum);
//                                strMsg.append(strWar);
//                                //可强制执行
//                                ckResult->bEnforced = true;
//                                ckResult->cmdArray.append(qRecvArray);
//                                ckResult->checkMsg = strMsg;
//                                //可强制执行，增加交互
//                                UpdateCheckResultArray(ckResult);//交互，则增加
//                                //发送操作报警信息
//                                this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
//                                //退出
//                                return 1;
//                            }
//                        }
//                    }
                }

                //进路区段占用判断
                bool bRouteZY = IsQDZYInLSB(nIndexRoute,JLWarningType,JLWarningMsg);
                if(bRouteZY)
                {
                    QString strSys = QString("命令与联锁冲突\r\n进路不能办理，有区段占用");
                    strMsg += strSys;
                    ckResult->bEnforced = false;
                    ckResult->checkMsg = strMsg;
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //发送操作报警信息
                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                    //退出
                    return 1;
                }

                //进路区段锁闭判断
                bool bRouteSB = IsQDHaveStateInLSB(nIndexRoute, QDSB, JLWarningType, JLWarningMsg);
                if(bRouteSB)
                {
                    QString strSys = QString("命令与联锁冲突\r\n进路不能办理，有区段锁闭");
                    strMsg += strSys;
                    ckResult->bEnforced = false;
                    ckResult->checkMsg = strMsg;
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //发送操作报警信息
                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                    //退出
                    return 1;
                }

                //进路道岔四开检查
                bool bRouteSK = IsQDDCSKInLSB(nIndexRoute,JLWarningType,JLWarningMsg);
                //进路空闲判断
                if(bRouteSK)
                {
                    QString strSys = QString("命令与联锁冲突\r\n进路不能办理，道岔四开");
                    strMsg += strSys;
                    ckResult->bEnforced = false;
                    ckResult->checkMsg = strMsg;
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //发送操作报警信息
                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                    //退出
                    return 1;
                }

                //封锁检查
                bool bRouteFS = IsQDFSInLSB(nIndexRoute,JLWarningType,JLWarningMsg);
                if(bRouteFS)
                {
                    QString strSys = QString("命令与联锁冲突\r\n进路不能办理，进路设备封锁%1")
                            .arg(JLWarningMsg);
                    strMsg += strSys;
                    ckResult->bEnforced = false;
                    ckResult->checkMsg = strMsg;
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //发送操作报警信息
                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                    //退出
                    return 1;
                }

                //分路不良检查
                //进路分路不良并确认空闲判断
                //bool bRouteFLBLKX = CheckRouteCanCmd(2, _pTrainRouteOrder->m_byArrayUDPJLOrderDate, _pTrainRouteOrder->tempRouteBtnArray);
                bool bRouteFLBL = isQDHaveFLBLInLSB(nIndexRoute,JLWarningType,JLWarningMsg);
                if(bRouteFLBL)
                {
                    QString strSys = QString("命令与联锁冲突\r\n进路不能办理，存在分路不良区段");
                    strMsg.append(strSys);
                    if((JLWarningType&JLWARNING_FLBL_DC) && !(JLWarningType&JLWARNING_FLBL_GD))
                    {
                        ckResult->bEnforced = true;//可强制执行，道岔分路不良
                        ckResult->cmdArray.append(qRecvArray);
                    }
                    else if(JLWarningType&JLWARNING_FLBL_GDKX)
                    {
                        ckResult->bEnforced = true;//可强制执行，股道分路不良空闲
                        ckResult->cmdArray.append(qRecvArray);
                    }
                    else
                    {
                        ckResult->bEnforced = false;//不可强制执行，股道分路不良
                    }
                    ckResult->checkMsg = strMsg;
                    ckResult->check = JLWarningType;

                    bRtValue = ckResult->check;
                    //可强制执行，增加交互
                    if(ckResult->bEnforced)
                    {
                        UpdateCheckResultArray(ckResult);//交互，则增加
                    }
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //防错办报警信息
                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                    //退出
                    return bRtValue;
                }

                //进路检查通过
                ckResult->check = 0;
                ckResult->checkMsg = "";

                //输入了车次，相应的进路序列状态更新
                if(inputCheCi != "" && pTrainRouteOrder!=nullptr)
                {
                    //执行进路
                    //进路表单状态刷新
                    pTrainRouteOrder->SetState(1);
                    //记录最近一次办理时间
                    this->m_LastTimeOfRouteDo = QDateTime::currentDateTime();
                    //记录进路实际触发时间
                    pTrainRouteOrder->m_timRealTouching = this->m_LastTimeOfRouteDo;
                    pTrainRouteOrder->m_nTouchingCount = 1;
                    //更新数据库
                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                    //发送同步消息
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                    //进路命令后续代码发送
                    //return bRtValue;
                }
            }

            //人工排路时生成进路序列
            if(MakeRouteOrderWhenClick && pTrainRouteOrder==nullptr)
            {
                //生成新的进路序列
                MakeNewRouteOrder(funcCode, nIndexRoute, cmdDevNameList, inputCheCi);
            }
        }

        //调车进路
        if(0x02==funcCode)
        {
            //调车占用时间解析位置（固定位置）
            nCount = 10+2*8+1;//27
            //调车占用时间-分钟
            int takenMinutes = (int)(qRecvArray[nCount]&0xFF);
            nCount++;
            //0x01电力机车、0x02内燃机车
            bool bElectric = ((int)(qRecvArray[nCount]&0xFF)==0x01)?true:false;
            nCount++;

            nIndexRoute = this->FindRouteIndexInLSB(ROUTE_DC, routeBtnTempArray);

            //*******进路联锁关系判断*******//
            //进路不存在则报警提示
            if(-1 == nIndexRoute)
            {
                //暂时进行提示
                this->SendMessageBoxMsg(3, "进路不存在，请重新操作！", currCtcIndex);
                return 1;
            }
            //进路存在
            else
            {
                //调车进路
                strMsg = QString("建立调车进路\r\n");
//                for(int j=0; j<routeBtnTempArray.size(); j++)
//                {
//                    QString strTemp = QString("第%1个按钮：%2 ").arg(j+1).arg(routeBtnTempArray[j]);
//                    strMsg += strTemp;
//                    if(j == routeBtnTempArray.size()-1)
//                    {
//                        strMsg += "\r\n";
//                    }
//                }
                for(int j=0; j<cmdDevNameList.size(); j++)
                {
                    QString strTemp = QString("第%1个按钮：%2 ").arg(j+1).arg(cmdDevNameList[j]);
                    strMsg += strTemp;
                    if(j == cmdDevNameList.size()-1)
                    {
                        strMsg += "\r\n";
                    }
                }

                CheckResult* ckResult = new CheckResult;
                QDateTime timeNow = QDateTime::currentDateTime();
                ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
                ckResult->indexRoute = nIndexRoute;

                //电力机车，判断接触网
                if(bElectric)
                {
                    //进路接触网供电判断
                    bool bRoutePowerCut = isQDHavePowerCutInLSB(nIndexRoute,JLWarningType,JLWarningMsg);
                    if(bRoutePowerCut)
                    {
                        //QString strSys = QString("命令与联锁冲突\r\n进路不能办理，电力牵引列车无法通过无电进路");
                        QString strSys = QString("违反站细规定\r\n电力牵引列车进入无电区");
                        strMsg += strSys;
                        ckResult->bEnforced = false;
                        ckResult->checkMsg = strMsg;
                        //发送系统消息信息
                        this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                        //发送操作报警信息
                        this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                        //退出
                        return 1;
                    }
                }

                //进路道岔四开检查
                bool bRouteSK = IsQDDCSKInLSB(nIndexRoute,JLWarningType,JLWarningMsg);
                //进路空闲判断
                if(bRouteSK)
                {
                    QString strSys = QString("命令与联锁冲突\r\n进路不能办理，道岔四开");
                    strMsg += strSys;
                    ckResult->bEnforced = false;
                    ckResult->checkMsg = strMsg;
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //发送操作报警信息
                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                    //退出
                    return 1;
                }

                //封锁检查
                bool bRouteFS = IsQDFSInLSB(nIndexRoute,JLWarningType,JLWarningMsg);
                if(bRouteFS)
                {
                    QString strSys = QString("命令与联锁冲突\r\n进路不能办理，进路设备封锁%1")
                            .arg(JLWarningMsg);
                    strMsg += strSys;
                    ckResult->bEnforced = false;
                    ckResult->checkMsg = strMsg;
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //发送操作报警信息
                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                    //退出
                    return 1;
                }

                //分路不良检查
                //进路分路不良并确认空闲判断
                bool bRouteFLBL = isQDHaveFLBLInLSB(nIndexRoute,JLWarningType,JLWarningMsg);
                if(bRouteFLBL)
                {
                    QString strSys = QString("命令与计划冲突\r\n进路不能办理，存在分路不良区段");
                    strMsg += strSys;
                    ckResult->bEnforced = true;
                    ckResult->checkMsg = strMsg;
                    //可强制执行，增加交互
                    if(ckResult->bEnforced)
                    {
                        ckResult->cmdArray.append(qRecvArray);
                        UpdateCheckResultArray(ckResult);//交互，则增加
                    }
                    //发送系统消息信息
                    this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                    //发送操作报警信息
                    this->SendRouteCheckResult(ckResult, -1);//currCtcIndex
                    //退出
                    return 1;
                }
                //调车进路与进路序列冲突判断
                if((this->ModalSelect.nStateSelect == 1)&&(this->ModalSelect.nPlanCtrl==1))//在按图排路模式下,并且计划控制灯亮起的情况下,才能做出该冲突判断
                {
                    QString m_TrainNum = "";
                    bool bDCJLAndJLXL = CheckDCRouteSameQDwithJLXL(routeBtnTempArray[0],routeBtnTempArray[routeBtnTempArray.size()-1],takenMinutes,m_TrainNum);
                    if(bDCJLAndJLXL)
                    {
                        QString strSys = QString("命令与计划冲突\r\n与%1次的计划冲突")
                                .arg(m_TrainNum);
                        strMsg += strSys;
                        ckResult->bEnforced = true;
                        ckResult->checkMsg = strMsg;
                        //可强制执行，增加交互
                        if(ckResult->bEnforced)
                        {
                            ckResult->cmdArray.append(qRecvArray);
                            UpdateCheckResultArray(ckResult);//交互，则增加
                        }
                        //发送系统消息信息
                        //this->sendWarningMsgToCTC(1,2,strSys.replace("\r\n",","));
                        //发送操作报警信息
                        this->SendRouteCheckResult(ckResult, -1);
                        //退出
                        return 1;
                    }
                }
            }
        }
    }

    //功能码
    switch(funcCode)
    {
    case 0x01://列车进路
        {
            this->sendWarningMsgToCTC(3,2,"办理列车进路指令下达");
        }
        break;
    case 0x02://调车进路
        {
            this->sendWarningMsgToCTC(3,2,"办理调车进路指令下达");
        }
        break;
    case 0x03://通过进路
        {
            this->sendWarningMsgToCTC(3,2,"办理通过进路指令下达");
        }
        break;
    case 0x04://进路取消
        {
            this->sendWarningMsgToCTC(3,2,"取消进路指令下达");
        }
        break;
    default:
        break;
    }

    return bRtValue;
}
////发送站场状态给CTC、占线板终端
//void MyStation::sendStationStatusToCTC()
//{
//    //QByteArray sendArray = packStationStatusToCTC();
//}

//打包站场状态给CTC终端，返回数据包
//int MyStation::packStationStatusToCTC(QByteArray* pDataArray)
QByteArray MyStation::packStationStatusToCTC()
{
    //CGD *pGD;
    //CXHD *pXHD;
    //CGDDC *pGDDC;
    int nCount=0;
    QByteArray tempArray;
    //添加4096个字节并全部置零
    tempArray.append(1024*3, char(0));

    //标志位
    nCount = 9;
    tempArray[nCount++] = 0x11;
    tempArray[nCount++] = this->DcNum;
    tempArray[nCount++] = this->XhNum;
    tempArray[nCount++] = this->QdNum;

    //道岔设备状态
    for(int i = 0; i < (int)DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_DC)
        {
            CGDDC *pGDDC = (CGDDC*)DevArray[i];
            //第0字节
            int state=0x00;
            //占用状态
            if(pGDDC->getDCColor() == Qt::red)//gdColor
            {
                state = 0x01;
            }
            else //if(pGDDC->gdColor == SkyBlue)
            {
                state = 0x00;
            }
            //锁闭状态
            if(pGDDC->getDCColor() == Qt::white)//gdColor
            {
                state |= 0x02;
            }
            //单锁状态
            if(pGDDC->getIsDS() == true)
            {
                state |= 0x04;
            }
            //单封状态
            if(pGDDC->getIsFS() == true)
            {
                state |= 0x08;
            }
            //获取道岔位置
            if(pGDDC->getDCWZ() == DCDW)
            {
                state |= (0x01<<4);
            }
            else if(pGDDC->getDCWZ() == DCFW)
            {
                state |= (0x02<<4);
            }
            else //if(pGDDC->getDCWZ() == DCSK)
            {
                state |= (0x03<<4);
            }
            tempArray[nCount] = state;

            //第1字节
            //尖轨心轨故障
            state=0x00;
            if(pGDDC->isJGGZ)
            {
                state |= 0x01;
            }
            if(pGDDC->isXGGZ)
            {
                state |= 0x02;
            }
            //分路不良
            state |= (pGDDC->flblStatusDW<<2);
            state |= (pGDDC->flblStatusFW<<4);
            state |= (pGDDC->flblStatusCQ<<6);
            tempArray[nCount+1] = state;

            //第2字节
            state=0x00;
            if(!pGDDC->isPowerCutDW)
            {
                state |= 0x01;
            }
            if(!pGDDC->isPowerCutFW)
            {
                state |= 0x02;
            }
            if(!pGDDC->isPowerCutCQ)
            {
                state |= 0x04;
            }
            tempArray[nCount+2] = state;

            //第3字节
            state=0x00;
            state |= pGDDC->speedLimitStatus;
            tempArray[nCount+3] = state;

            nCount+=4;
            pGDDC = nullptr;
        }
    }

    //信号机设备状态
    for(int i = 0; i < (int)DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_XH)
        {
            CXHD *pXHD = (CXHD*)DevArray[i];
            //第0字节状态
            int state=0x00;
            if(pXHD->getXHDState() == XHD_BD)
            {
                state = 0x01;//0001白灯
            }
            else if(pXHD->getXHDState() == XHD_YD)
            {
                state = 0x02;//0010红白(引导)
            }
            else if(pXHD->getXHDState() == XHD_LD)
            {
                state = 0x03;//0011绿灯
            }
            else if(pXHD->getXHDState() == XHD_UD)
            {
                state = 0x04;//0100黄灯
            }
            else if(pXHD->getXHDState() == XHD_LU)
            {
                state = 0x05;//0101绿黄
            }
            else if(pXHD->getXHDState() == XHD_UU)
            {
                state = 0x06;//0110双黄
            }
            else if(pXHD->getXHDState() == XHD_HD)
            {
                state = 0x07;//0111红灯
            }
            else if(pXHD->getXHDState() == XHD_AD)
            {
                state = 0x08;//1000蓝灯
            }
            else if(pXHD->getXHDState() == XHD_DS)
            {
                state = 0x09;//1001信号机断丝
            }
            else if(pXHD->getXHDState() == XHD_USU)
            {
                state = 0x0A;//1010黄闪黄
            }
            else if(pXHD->getXHDState() == XHD_BS)
            {
                state = 0x0B;//1011白闪（驼峰溜放）
            }
            else if(pXHD->getXHDState() == XHD_LL)
            {
                state = 0x0C;//1100双绿（开往次要线路）
            }
            if(pXHD->getIsMD() == false)
            {
                state |= 0x10;//点灯
            }
            if(pXHD->getIsLCANFB() == true)
            {
                state |= 0x20;//列车按钮
            }
            if(pXHD->getIsDCANFB() == true)
            {
                state |= 0x40;//调车按钮
            }
            if(pXHD->getIsYDANFB() == true)
            {
                state |= 0x80;//引导按钮
            }
            tempArray[nCount] = state;

            //第1字节状态
            state=0x00;
            if(pXHD->XHD_ds_HD)
            {
                state |= 0x01;
            }
            if(pXHD->XHD_ds_LD)
            {
                state |= 0x02;
            }
            if(pXHD->XHD_ds_UD)
            {
                state |= 0x04;
            }
            if(pXHD->XHD_ds_BD)
            {
                state |= 0x08;
            }
            if(pXHD->XHD_ds_AD)
            {
                state |= 0x10;
            }
            if(pXHD->XHD_ds_YBD)
            {
                state |= 0x20;
            }
            tempArray[nCount+1] = state;

            //第2字节状态
            state = 0x00;
            if(pXHD->getTimeCount() > 0 && pXHD->getTimeCount()<0xFF)
            {
                state = pXHD->getTimeCount();
            }
            tempArray[nCount+2] = state;

            //第3字节状态
            state=0x00;
            if(pXHD->isLCANFlash)
            {
                state |= 0x01;
            }
            if(pXHD->isDCANFlash)
            {
                state |= 0x02;
            }
            if(pXHD->isBTANFlash)
            {
                state |= 0x04;
            }
            tempArray[nCount+3] = state;

            //结束
            nCount+=4;
            pXHD = nullptr;
        }
    }

    //股道/无岔区段设备状态
    for(int i = 0; i < (int)DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD || DevArray[i]->getDevType() == Dev_WCQD)
        {
            CGD *pGD = (CGD*)DevArray[i];

            //第0字节
            int state=0x00;
            //占用状态
            if(pGD->getState(QDZY))
            {
                state = 0x01;
            }
            else //if(pGD->getState(QDKX))
            {
                state = 0x00;
            }
            //锁闭状态
            if(pGD->getState(QDSB))
            {
                state |= 0x02;
            }
            //封锁状态
            if(pGD->isLock)
            {
                state |= 0x04;
            }
            //有电无电
            if(!pGD->isPowerCut)
            {
                state |= 0x10;
            }
            //分路不良
            state |= (pGD->flblStatus<<5);
            tempArray[nCount] = state;

            //第1字节
            //显示方向（区间低频码）
            state=0x00;
            state |= pGD->bsqdfmDirection;
            //显示方向（区间低频码）
            state |= (pGD->bsqdfmCode<<2);
            tempArray[nCount+1] = state;

            //第2字节
            state=0x00;
            state |= pGD->speedLimitStatus;
            tempArray[nCount+2] = state;

            nCount+=3;
            pGD = nullptr;
        }
    }

    //自动闭塞箭头方向
    for(int i=0; i<vectAutomaticBlock.size(); i++)
    {
        AutomaticBlock *pAutomaticBlock = vectAutomaticBlock[i];
        int state=0x00;
        //第0-1字节
        tempArray[nCount] = pAutomaticBlock->codejuncXHD;
        tempArray[nCount+1] = pAutomaticBlock->codejuncXHD>>8;

        //第2字节
        state=0x00;
        state |= (pAutomaticBlock->btnStateZFZ == 1 ? 0x01 : 0x00);
        state |= (pAutomaticBlock->btnStateFFZ == 1 ? 0x02 : 0x00);
        state |= (pAutomaticBlock->btnStateJFZ == 1 ? 0x04 : 0x00);
        state |= (pAutomaticBlock->btnStateGF == 1 ? 0x08 : 0x00);
        state |= (pAutomaticBlock->arrowState<<4);
        state |= (pAutomaticBlock->lightStateJDQJ == 1 ? 0x80 : 0x00);
        tempArray[nCount+2] = state;

        //第3字节
        state=0x00;
        state |= (pAutomaticBlock->lightStateQJD);
        state |= (pAutomaticBlock->lightStateFZD<<4);
        tempArray[nCount+3] = state;

        //第4字节
        state=0x00;
        state |= (pAutomaticBlock->lightStateQGD);
        state |= (pAutomaticBlock->lightStateYXFCD<<4);
        tempArray[nCount+4] = state;

        //第5字节
        state=0x00;
        state |= (pAutomaticBlock->sectionLogicCheckState&0x03);
        tempArray[nCount+5] = state;


        pAutomaticBlock = nullptr;
        nCount+=6;
    }

    //半自动闭塞箭头方向
    for(int i=0; i<vectSemiAutomaticBlock.size(); i++)
    {
        SemiAutomaticBlock *pSemiAutomaticBlock = vectSemiAutomaticBlock[i];
        int state=0x00;
        //第0-1字节
        tempArray[nCount] = pSemiAutomaticBlock->code;
        tempArray[nCount+1] = pSemiAutomaticBlock->code>>8;

        //第2字节
        state=0x00;
        state |= (pSemiAutomaticBlock->arrowStateReach&0x0F);
        state |= ((pSemiAutomaticBlock->arrowStateDepart&0x0F)<<4);
        tempArray[nCount+2] = state;

        //第3字节
        state=0x00;
        //预留
        tempArray[nCount+3] = state;

        nCount+=4;
        pSemiAutomaticBlock = nullptr;
    }

    //计轴状态
    for(int i=0; i<vectAxleCounter.size(); i++)
    {
        AxleCounter *pAxleCounter = vectAxleCounter[i];
        int state=0x00;
        //第0-1字节
        tempArray[nCount] = pAxleCounter->codejuncXHD;
        tempArray[nCount+1] = pAxleCounter->codejuncXHD>>8;

        //第2字节
        state=0x00;
        state |= (pAxleCounter->btnStateFL == 1 ? 0x01 : 0x00);
        state |= (pAxleCounter->btnStateSY == 1 ? 0x02 : 0x00);
        state |= (pAxleCounter->btnStateTZ == 1 ? 0x04 : 0x00);
        state |= (pAxleCounter->lightStateJZBJ == 1 ? 0x08 : 0x00);
        state |= (pAxleCounter->lightStateFL == 1 ? 0x10 : 0x00);
        state |= (pAxleCounter->lightStateSY == 1 ? 0x20 : 0x00);
        state |= (pAxleCounter->lightStateTZ == 1 ? 0x40 : 0x00);
        state |= (pAxleCounter->lightStateQG == 1 ? 0x80 : 0x00);
        tempArray[nCount+2] = state;

        //第3字节
        state=0x00;
        state |= (pAxleCounter->lightStateQJ == 1 ? 0x01 : 0x00);
        tempArray[nCount+3] = state;

        nCount+=4;
        //pAxleCounter = nullptr;
    }

//    //机务段状态
//    if(StaConfigInfo.HaveCLPZFlag)
//    {
//        nCount++;
//    }
    //场联+机务段
    for(int i=0; i<vectInterfieldConnection.size(); i++)
    {
        InterfieldConnection *pCL = vectInterfieldConnection[i];
        int state=0x00;
        //第0-1字节
        tempArray[nCount] = pCL->codejuncXHD;
        tempArray[nCount+1] = pCL->codejuncXHD>>8;

        //第2字节
        state=0x00;
        state |= (pCL->lightStateCFJ&0x0F);
        state |= ((pCL->arrowState&0x07)<<4);
        state |= (pCL->lightStateJGJ == 1 ? 0x80 : 0x00);
        tempArray[nCount+2] = state;

        //第3字节
        state=0x00;
        state |= (pCL->lightStateAFJ&0x0F);
        state |= ((pCL->lightStateBFJ&0x0F)<<4);
        tempArray[nCount+3] = state;

        nCount+=4;
    }

    //预留2个空白
    nCount+=2;

    //CTC控制模式状态
    if(m_nFCZKMode)
    {
        //非常站控模式
        tempArray[nCount++] = 0x22;
    }
    else
    {
        //分散自律模式
        tempArray[nCount++] = ModalSelect.nModeState;
    }
    //计划控制模式状态
    int state0 = 0x00;
    state0 |= (ModalSelect.nStateSelect==1?0x01:0x00);//第1位排路模式
    state0 |= (ModalSelect.nPlanCtrl==true?0x02:0x00);//第2位计划控制
    state0 |= (AutoSignStage==true?0x04:0x00);//第3位阶段计划自动签收
    tempArray[nCount++] = state0;

    //咽喉总锁状态
    int state1 = 0x00;
    state1 |= (m_nDCSXAllLock?0x01:0x00);
    state1 |= (m_nDCXXAllLock?0x02:0x00);
    tempArray[nCount++] = state1;
    //允许转回灯状态
    tempArray[nCount++] = m_bAllowZH?0x01:0x00;
    //坡道解锁倒计时180秒计数
    tempArray[nCount++] = m_nPDJS180;
    //S人解180s
    tempArray[nCount++] = m_nSRJ180;
    //S人解30s
    tempArray[nCount++] = m_nSRJ30;
    //X人解180s
    tempArray[nCount++] = m_nXRJ180;
    //X人解30s
    tempArray[nCount++] = m_nXRJ30;
    //上电解锁状态
    tempArray[nCount++] = m_bSDJS?0x01:0x00;

    //通信状态
    int state2 = 0x00;
    state2 |= (isLSConnected?0x01:0x00);//第0位：与联锁通信状态
    state2 |= (0x01<<1);//第1位：与中心机(教师机)通信状态
    tempArray[nCount++] = state2;

    //进路权限
    tempArray[nCount++] = RoutePermit;

    //预留2个空白
    nCount+=2;

    //登录用户个数(最多2个，第1个为当前用户)
    int userCount = this->vectLoginUser.size();
    int sendCount = userCount<=2 ? userCount : 2;
    //qDebug()<<"send user count="<<sendCount;
    tempArray[nCount++] = sendCount;
    for(int i=(sendCount-1); i>=0; i--)
    {
        QString loginName = this->vectLoginUser[i]->loginName;
        int     loginStat = this->vectLoginUser[i]->loginStatus;
        int nameLen = loginName.toLocal8Bit().length();
        tempArray[nCount++] = nameLen;
        for(int j=0; j<nameLen; j++)
        {
            tempArray[nCount++] = loginName.toLocal8Bit().at(j);
        }
        tempArray[nCount++] = loginStat;
    }

    //预留2个空白
    nCount+=2;

    //预告信息标志码
    tempArray[nCount++] = 0xAA;
    int preCount = vectRoutePreWnd.size();
    //接发车口（进路预告窗）数目
    tempArray[nCount++] = preCount;
    for(int i=0; i<preCount; i++)
    {
        RoutePreWnd *pRoutePreWnd = vectRoutePreWnd[i];
//        //进路窗关联信号机设备号
//        memcpy(&tempArray.data()[nCount], &pRoutePreWnd->juncXHDCode, 2);
//        nCount += 2;
//        //进路序列id
//        for (int j=0; j<pRoutePreWnd->vectRouteInfo.size(); j++) {
//            RoutePreWnd::RouteInfo routeInfo = pRoutePreWnd->vectRouteInfo[j];
//            //进路序列id
//            memcpy(&tempArray.data()[nCount+2*(j+1)], &routeInfo.route_id, 2);
//        }
//        nCount += (2+2*3);
        //新协议
        QByteArray dataArrayRpw = packRoutePreWndToCTC(pRoutePreWnd);
        tempArray = tempArray.replace(nCount, dataArrayRpw.length(), dataArrayRpw);
        nCount+=dataArrayRpw.length();
    }

    //打包腰岔解锁
    int YCJSCount = vectYCJS.size();
    //数目
    tempArray[nCount++] = YCJSCount;
    for(int i=0; i<YCJSCount; i++)
    {
        //第1字节状态
        int state=0x00;
        if(vectYCJS[i].bLocking)
        {
            state = 0x01;
        }
        tempArray[nCount+0] = state;
        //第2字节状态
        state = 0x00;
        state = vectYCJS[i].countdown;
        tempArray[nCount+1] = state;
        nCount+=2;
    }
    //打包股道确认
    int GDQRCount = vectGDQR.size();
    //数目
    tempArray[nCount++] = GDQRCount;
    for(int i=0; i<GDQRCount; i++)
    {
        //第1字节状态
        int state=0x00;
        state = vectGDQR[i].countdown;
        tempArray[nCount+0] = state;
        nCount+=1;
    }

    //帧尾,预留2个空白
    nCount+=(4+2);

    //打包数据
    packHeadAndTail(&tempArray, nCount);
    ////tempArray.resize(nCount);
    //qDebug()<<"tempArray="<<ByteArrayToString(tempArray);

    //copy数据
    QByteArray dataArray;
    //此处memcpy不能用，会导致报错
    //memcpy(&dataArray, tempArray.data(), nCount);
    dataArray = tempArray.left(nCount);
    //qDebug()<<"StationID="<<this->getStationID()<<"status dataArray="<<ByteArrayToString(dataArray);

    //信号-发送数据给CTC
    emit sendDataToCTCSignal(this, dataArray, nCount);
    //信号-发送数据给占线板
    emit sendDataToBoardSignal(this, dataArray, nCount);
    //信号-发送数据给集控台
    emit sendDataToJKSignal(this, dataArray, nCount);
    //信号-发送数据给占线图
    emit sendDataToZXTSignal(this, dataArray, nCount);

    return dataArray;
}
//打包进路预告窗的信息
QByteArray MyStation::packRoutePreWndToCTC(RoutePreWnd *_pRoutePreWnd)
{
    QByteArray dataArray;
    int nCount = 0;
    //关联信号机设备号
    nCount+=2;
    dataArray.append(2, char(0));
    memcpy(dataArray.data()+(nCount-2),&_pRoutePreWnd->juncXHDCode,2);
    //进路序列信息
    int routeCount = _pRoutePreWnd->vectRouteInfo.size();
    //进路序列个数
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = routeCount;
    //遍历赋值
    for(int i=0; i<routeCount; i++)
    {
        RoutePreWnd::RouteInfo routeInfo = _pRoutePreWnd->vectRouteInfo[i];
        //进路序列id
        nCount+=2;
        dataArray.append(2, char(0));
        memcpy(dataArray.data()+(nCount-2),&routeInfo.route_id,2);

        //车次
        QByteArray byteArray = routeInfo.CheCi.toLocal8Bit();
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

        //股道名
        byteArray = routeInfo.GDName.toLocal8Bit();
        ccLen = byteArray.count();
        //股道名长度
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = ccLen;
        //股道名内容
        nCount+=ccLen;
        dataArray.append(ccLen, char(0));
        for(int u=0; u<ccLen; u++)
        {
            dataArray[nCount-ccLen+u] = byteArray[u];
        }

        //客货类型
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = routeInfo.m_nKHType;

        //进路接发车类型
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = routeInfo.routeType;

        //进路状态
        nCount++;
        dataArray.append(1, char(0));
        dataArray[nCount-1] = routeInfo.routeState;
    }

    //区间逻辑检查状态
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = _pRoutePreWnd->statusQJLJJC;

//    //其余补0
//    nCount += (3-routeCount);
//    dataArray.append(3-routeCount, char(0));

    return dataArray;
}
//制作数据帧头帧尾和关键信息
void MyStation::packHeadAndTail(QByteArray *pDataArray, int length)
{
    //帧头4+分类码6+帧尾4
    if(length < 14)
        return;
    for(int i=0; i<4; i++)
    {
        //帧头
        pDataArray->data()[i] = 0xEF;
        //帧尾
        pDataArray->data()[length-i-1] = 0xFE;
    }
    //帧长度
    memcpy(&pDataArray->data()[4], &length, 2);
    //车站id
    pDataArray->data()[6] = this->getStationID();
    //岗位码
    //pDataArray->data()[7] = 0;
    //数据来源标记
    pDataArray->data()[8] = DATATYPE_SERVER;
}
//接收CTC数据-进路按钮按下（终端闪烁）
void MyStation::recvCTCData_RouteBtnClick(unsigned char *array, int count)
{
    CXHD *pXHD = nullptr;
    int devCode = (int)(array[10] | (array[11]<<8));
    int btnType = (int)(array[12]);
    int xhbtnType = 0;
    if(0x01 == btnType)
    {
        xhbtnType = LCAN;
    }
    else if(0x02 == btnType)
    {
        xhbtnType = DCAN;
    }
    //先判断是否是独立的按钮
    QString devName = GetStrNameByCodeInXhBtnArray(devCode, xhbtnType);
    if(devName != "")
    {
        routeBtnTempArray.append(devName);
    }
    else
    {
        //再判断是否是常规的信号机按钮
        devName = GetStrNameByCode(devCode);
        if(devName == "")
        {
            return;
        }

        QString devType = getTypeByCode(devCode);
        if("CXHD" == devType)
        {
            int devIndex = GetIndexByStrName(devName);
            if(devIndex > -1)
            {
                pXHD = (CXHD*)DevArray[devIndex];
            }
        }
        switch (btnType) {
        //列车按钮
        case 0x01:
            //虚信号
            if(pXHD && pXHD->getSignalType())
            {
                routeBtnTempArray.append(devName);
            }
            //列按
            else
            {
                devName = devName + "LA";
                routeBtnTempArray.append(devName);
            }
            break;
        //调车按钮
        case 0x02:
            //虚信号
            if(pXHD && pXHD->getSignalType())
            {
                routeBtnTempArray.append(devName);
            }
            //调按
            if(pXHD && pXHD->getXHDType() == DC_XHJ)
            {
                devName = devName + "A";
                routeBtnTempArray.append(devName);
            }
            //列信调按
            else
            {
                devName = devName + "DA";
                routeBtnTempArray.append(devName);
            }
            break;
        //通过按钮
        case 0x03:
            //列信通按
            if(pXHD && pXHD->getXHDType() == JZ_XHJ)
            {
                devName = devName + "TA";
                routeBtnTempArray.append(devName);
            }
            break;
        //变通按钮
        case 0x04:
            routeBtnTempArray.append(devName);
            break;
        default:
            return;
        }
    }
    //开始计数
    this->setCmdCountDown();
    //计算下一个闪烁的按钮
    this->setNextXHDBtnFlash(routeBtnTempArray);
}
//终端闪烁功能-清除所有按钮闪烁
void MyStation::clearXHDBtnFlash()
{
    int anCount = vectXhBtn.size();
    for (int i = 0; i<anCount; i++)
    {
        XhBtn *pXHAN = (XhBtn*)vectXhBtn[i];
        pXHAN->m_nBtnIsDown = false;
        pXHAN->m_nBtnFlash = false;
    }
    //站场信号机按钮,清除闪烁
    for(int i=0; i<DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)DevArray[i];
            xhd->isLCANFlash=false;
            xhd->isDCANFlash=false;
            xhd->isBTANFlash=false;
        }
    }
}
//终端闪烁功能-闪烁下一个可按下信号机的按钮
void MyStation::setNextXHDBtnFlash(QStringList btnArray)
{
    //清除上一次的所有闪烁
    //独立的列车调车按钮
    clearXHDBtnFlash();

    //CXHD *pXHD;
    //匹配联锁表进路，并设置闪烁
    int sizeDown = btnArray.size();
    int routeCount = vectInterlockRoute.size();
    for (int i = 0; i < routeCount; i++)
    {
        InterlockRoute *pRut = vectInterlockRoute[i];
        int sizeRut = pRut->BtnArr.size();
        if (sizeDown <= sizeRut)
        {
            bool bMath = false;
            for (int j = 0; j < sizeDown; j++)
            {
                if (btnArray.at(j) == pRut->BtnArr.at(j))
                {
                    bMath = true;
                }
                else
                {
                    bMath = false;
                    break;
                }
            }
            //进路匹配
            if (bMath)
            {
                //联锁表进路还有下一个按钮
                if (sizeDown < sizeRut)
                {
                    QStringList qdArray;
                    qdArray.append(pRut->QDArr);
                    //进路是否空闲
                    if (!isQDKXInLSB(qdArray))
                    {
                        continue;
                    }

                    //下一个信号机按钮
                    QString btnName = pRut->BtnArr.at(sizeDown);
                    QString nextBtnName = btnName;
                    QString xhdName = btnName;
                    //判断独立的按钮
                    bool bNextFlashed = setAloneXHDBtnFlash(btnName);
                    //非独立的按钮
                    if (!bNextFlashed)
                    {
                        //变更按钮BA判断，变更按钮名称不变，非变更按钮则需要去除按钮类型
                        //if (-1 == xhdName.Find(_T("BA")))   //有的按钮为B2A则无法判断
                        //{
                        //	xhdName = GetLSBRouteBtnXhdName(xhdName);
                        //}
                        int xhdIndx = GetIndexByStrName(xhdName);
                        if(xhdIndx > -1)
                        {
                            CXHD* pXHDNext = (CXHD *)(DevArray[xhdIndx]);
                            //虚信号(变通按钮或者终端按钮)
                            if(pXHDNext->getSignalType())
                            {
                                xhdName = pXHDNext->getName();
                            }
                            else
                            {
                                xhdName = getLSBRouteBtnXhdName(xhdName);
                            }
                        }
                        else
                        {
                            xhdName = getLSBRouteBtnXhdName(xhdName);
                        }
                        xhdIndx = GetIndexByStrName(xhdName);
                        int xhdIndxFirst = GetIndexByStrName(pRut->strXHD);
                        if (xhdIndx > -1 && xhdIndxFirst > -1)
                        {
                            CXHD* pXHDFirst = (CXHD *)(DevArray[xhdIndxFirst]);
                            CXHD* pXHDNext = (CXHD *)(DevArray[xhdIndx]);
                            //调车进路
                            if (pXHDFirst->getXHDType() == DC_XHJ)
                            {
                                pXHDNext->isDCANFlash = true;//4;//调车按钮可按下
                            }
                            //列车进路、通过进路
                            else
                            {
                                QString strFisrtBtn = btnArray.at(0);
                                //始端列车按钮按下的是调车按钮，后续按钮为调车或虚拟信号;可调车的列车信号机做变按
                                //if (-1 < strFisrtBtn.indexOf("DA")
                                //    || pXHDNext->getXHDType() == DC_XHJ
                                //    || pXHDNext->getSignalType()
                                //    || (-1 < nextBtnName.indexOf("DA"))
                                //   )
                                if ((strFisrtBtn.right(2)=="DA")
                                    || (pXHDNext->getXHDType() == DC_XHJ)
                                    || (nextBtnName.right(2)=="DA")
                                   )
                                {
                                    pXHDNext->isDCANFlash = true;//4;//调车按钮可按下
                                }
                                else
                                {
                                    pXHDNext->isLCANFlash = true;//4;//列车按钮可按下
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}
//设置独立的信号机按钮闪烁
bool MyStation::setAloneXHDBtnFlash(QString _strName)
{
    //独立的列车调车按钮
    int anCount = vectXhBtn.size();
    for (int i = 0; i<anCount; i++)
    {
        XhBtn *pXHAN = (XhBtn*)vectXhBtn[i];
        if (_strName == pXHAN->m_strName)
        {
            //若已经加封不进行操作
            if (!pXHAN->m_nFuncLockState)
            {
                //pXHAN->m_nBtnIsDown = true;
                pXHAN->m_nBtnFlash = true;
                QString xhdName = this->GetStrNameByCode(pXHAN->m_nCode);
                if(xhdName != "")
                {
                    int devIndex = this->GetIndexByStrName(xhdName);
                    if(devIndex > -1)
                    {
                        CXHD* pXHD= (CXHD *)(DevArray[devIndex]);
                        if(LCAN == pXHAN->m_nANTYPE)
                        {
                            //列车按钮
                            pXHD->isLCANFlash = true;
                        }
                        else
                        {
                            //调车按钮
                            pXHD->isDCANFlash = true;
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}
//获取联锁表进路按钮的信号机名称
QString MyStation::getLSBRouteBtnXhdName(QString rtBtnName)
{
    QString xhdName = rtBtnName;
    if(rtBtnName.right(2) == "LA")
    {
        xhdName = rtBtnName.left(rtBtnName.length() - 2);
    }
    else if(rtBtnName.right(2) == "TA")
    {
        xhdName = rtBtnName.left(rtBtnName.length() - 2);
    }
    else if(rtBtnName.right(3) == "DZA")
    {
        xhdName = rtBtnName.left(rtBtnName.length() - 3);
    }
    else if(rtBtnName.right(2) == "ZA")
    {
        xhdName = rtBtnName.left(rtBtnName.length() - 2);
    }
    else if(rtBtnName.right(2) == "DA")
    {
        xhdName = rtBtnName.left(rtBtnName.length() - 2);
    }
    else if(rtBtnName.right(1) == "A")
    {
        xhdName = rtBtnName.left(rtBtnName.length() - 1);
    }

    return xhdName;
}
//根据设备号获取设备名称独立信号按钮数组
QString MyStation::GetStrNameByCodeInXhBtnArray(int nCode, int btnType)
{
    for (int i = 0; i<vectXhBtn.size(); i++)
    {
        XhBtn *pXHAN = (XhBtn*)vectXhBtn[i];
        if(nCode==pXHAN->m_nCode && btnType==pXHAN->m_nANTYPE)
        {
            return pXHAN->m_strName;
        }
    }
    return "";
}
//进路表中的区段是否空闲
bool MyStation::isQDKXInLSB(QStringList pQDArray)
{
    //int c = pQDArray.size();
    for(int i = 0; i < pQDArray.size(); i++)
    {
        QString qdname = (QString)pQDArray[i];
        int nArraySize =  DevArray.size();//获取站场大小
        CGD *pGD;
        CGDDC *pGDDC;

        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            int pos = dcname.indexOf("-");
            if( pos > 0 )
            {
                QStringList strArr;
                StringSplit(dcname, "-", strArr);
                dcname = strArr[0];
            }
            //查找设备
            for(int z=0; z<nArraySize; z++)
            {
                //if (DevArray[z]->GetRuntimeClass()->m_lpszClassName == "CGDDC")
                if(DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname == pGDDC->m_strName)
                    {
                        if( !pGDDC->getQDState(QDKX) || pGDDC->getQDState(DCSK) || pGDDC->getIsFS())
                        {
                            if(!pGDDC->getState(QDKX))
                            {
                                //m_nJLWariningType |= JLWARNING_QDZY;
                            }
                            if(pGDDC->getIsFS())
                            {
                                //m_nJLWariningType |= JLWARNING_FS_DC;
                            }
                            return false;
                        }
                        ////道岔四开 或 封锁，不可办理
                        //if( pGDDC->getState(DCSK) || pGDDC->m_nFS)
                        //	return FALSE;
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z<nArraySize; z++)
            {
                if(DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        if( !pGD->getState(QDKX) || pGD->isLock)//不空闲或者区段封锁
                        {
                            if( !pGD->getState(QDKX))
                            {
                                //m_nJLWariningType |= JLWARNING_QDZY;
                            }
                            if(pGD->isLock)
                            {
                                //m_nJLWariningType |= JLWARNING_FS_GD;
                            }
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}
//设置命令倒计时（界面操作读秒提示，15s读秒结束后清除操作）
void MyStation::setCmdCountDown()
{
    m_bBeginCount = true;
    //命令按下时的记录时间
    m_timeCmdRecord = QDateTime::currentDateTime();
    qDebug()<<"time="<<m_timeCmdRecord.toString()<<"setCmdCountDown m_bBeginCount="<<m_bBeginCount;
}
//清除命令倒计时读秒
void MyStation::clearCmdCountDown()
{
    routeBtnTempArray.clear();
    m_nCountDown = 0;
    m_bBeginCount = false;
    clearXHDBtnFlash();
    //qDebug()<<"time="<<QDateTime::currentDateTime().toString()<<"clearCmdCountDown!";
}
//定时自动计算读秒
void MyStation::computeCmdCountDown()
{
    if (m_bBeginCount)
    {
        QDateTime timeNow = QDateTime::currentDateTime();
        //倒计时15秒结束
        //m_nCountDown = timeNow.secsTo(m_timeCmdRecord); //负数
        m_nCountDown = m_timeCmdRecord.secsTo(timeNow); //正数
        if (m_nCountDown >= 15)
        {
            clearCmd();
        }
    }
}
//发送清除命令给CTC
void MyStation::sendClearCmdToCTC()
{
    //发送清除命令给CTC
    QByteArray dataArray;
    int nCount = 20;
    //添加nCount个字节并全部置零
    dataArray.append(nCount, char(0));
    //标志位
    dataArray[9] = FUNCTYPE_CMDCLEAR;
    //打包数据
    packHeadAndTail(&dataArray, nCount);
    //qDebug()<<"dataArray="<<ByteArrayToString(dataArray);
    //信号-发送数据给CTC
    emit sendDataToCTCSignal(this, dataArray, nCount);
}
//进行清除命令操作
void MyStation::clearCmd()
{
    clearCmdCountDown();
    //发送清除命令给CTC
    sendClearCmdToCTC();
}

//更新收到的阶段计划信息
StagePlan *MyStation::updateStagePlan(unsigned char *recvArray, int len)
{
    //    //数据帧
    //    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, len);
    //    qDebug()<<"CTC-STAGEPLAN-updateStagePlan="<<ByteArrayToString(qRecvArray);

        StagePlan *stagePlan = new StagePlan();
        stagePlan->station_id = this->getStationID();
        stagePlan->m_timJHRcv = QDateTime::currentDateTime();
        //新协议
        int nCount = 10;
        //接受阶段计划协议制定
    //    unsigned char nPlanNumber[5];
    //    memset(nPlanNumber,0,sizeof(nPlanNumber));
    //    memcpy(nPlanNumber, &recvArray[nCount], 4);
    //    QString number = UnsignedCharArrayToString(nPlanNumber);
    //    stagePlan->m_nPlanNumber = number.toInt();
        //stagePlan->m_nPlanNumber = (int)(recvArray[nCount] | (recvArray[nCount+1]<<8)| (recvArray[nCount+2]<<16)| (recvArray[nCount+3]<<32));
        stagePlan->m_nPlanNumber = RecvArrayToInt(&recvArray[nCount], 4);
        nCount += 4;

        //到达车次长度
        int lenDDCC = recvArray[nCount];
        nCount += 1;
        //到达车次
        QString strDDCC;
        unsigned char strRecvDDCC[20];
        memset(strRecvDDCC,0,sizeof(strRecvDDCC));
        memcpy(strRecvDDCC, &recvArray[nCount], lenDDCC);
        strDDCC = UnsignedCharArrayToString(strRecvDDCC);
        stagePlan->m_strReachTrainNum = strDDCC;
        stagePlan->m_strReachTrainNumOld = stagePlan->m_strReachTrainNum;
        nCount += lenDDCC;

        //出发车次长度
        int lenCFCC = recvArray[nCount];
        nCount += 1;
        //出发车次
        QString strCFCC;
        unsigned char strRecvCFCC[20];
        memset(strRecvCFCC,0,sizeof(strRecvCFCC));
        memcpy(strRecvCFCC, &recvArray[nCount], lenCFCC);
        strCFCC = UnsignedCharArrayToString(strRecvCFCC);
        stagePlan->m_strDepartTrainNum = strCFCC;
        stagePlan->m_strDepartTrainNumOld = stagePlan->m_strDepartTrainNum;
        nCount += lenCFCC;

        //类型：添加还是删除
        stagePlan->m_btStagePlanKind = recvArray[nCount];
        nCount += 1;

        //接车股道
        //stagePlan->m_nRecvTrainTrack = recvArray[nCount] | recvArray[nCount+1]<<8;
        stagePlan->m_nRecvTrainTrack = RecvArrayToInt(&recvArray[nCount], 2);
        stagePlan->m_strRecvTrainTrack = GetStrNameByCode(stagePlan->m_nRecvTrainTrack);
        nCount += 2;

        //到达时间
        {
            int year  = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
            int mouth = (int)recvArray[nCount+2];
            int day   = (int)recvArray[nCount+3];
            int hour  = (int)recvArray[nCount+4];
            int mini  = (int)recvArray[nCount+5];
            int second = 0;//(int)recvArray[nCount+6];
            //"2019-03-31 12:24:36";
            //QString strDateTime= QString("%1-%2-%3 %4:%5:%6").arg(year).arg(mouth).arg(day).arg(hour).arg(mini).arg(second);
            QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
            //qDebug()<<"到达时间="<<strDateTime;
            QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
            stagePlan->m_timProvReachStation = dateTime;
        }
        nCount += 7;

        //发车股道
        //stagePlan->m_nDepartTrainTrack = recvArray[nCount] | recvArray[nCount+1]<<8;
        stagePlan->m_nDepartTrainTrack = RecvArrayToInt(&recvArray[nCount], 2);
        stagePlan->m_strDepartTrainTrack = GetStrNameByCode(stagePlan->m_nDepartTrainTrack);
        nCount += 2;

        //出发时间
        {
            int year = (recvArray[nCount] | recvArray[nCount+1]<<8);
            int mouth = recvArray[nCount+2];
            int day = recvArray[nCount+3];
            int hour = recvArray[nCount+4];
            int mini = recvArray[nCount+5];
            int second = 0;//recvArray[nCount+6];
            //"2019-03-31 12:24:36";
            //QString strDateTime= QString("%1-%2-%3 %4:%5:%6").arg(year).arg(mouth).arg(day).arg(hour).arg(mini).arg(second);
            QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
            //qDebug()<<"出发时间="<<strDateTime;
            QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
            stagePlan->m_timProvDepaTrain = dateTime;
        }
        nCount += 7;

        //始发终到标志
        stagePlan->m_btBeginOrEndFlg = recvArray[nCount];
        nCount += 1;

        //接车口/进站口
        //stagePlan->m_nCodeReachStaEquip = recvArray[nCount] | recvArray[nCount+1]<<8;
        stagePlan->m_nCodeReachStaEquip = RecvArrayToInt(&recvArray[nCount], 2);
        stagePlan->m_strXHD_JZk = GetStrNameByCode(stagePlan->m_nCodeReachStaEquip);
        nCount += 2;

        //发车口/出站口
        //stagePlan->m_nCodeDepartStaEquip = recvArray[nCount] | recvArray[nCount+1]<<8;
        stagePlan->m_nCodeDepartStaEquip = RecvArrayToInt(&recvArray[nCount], 2);
        stagePlan->m_strXHD_CZk = GetStrNameByCode(stagePlan->m_nCodeDepartStaEquip);
        nCount += 2;

        //电力机车  //recvArray[nCount] == 0x11
        stagePlan->m_bElectric = (recvArray[nCount] != 0x00) ? true : false;
        nCount += 1;

        //超限
        stagePlan->m_nLevelCX = recvArray[nCount];
        nCount += 1;

        //列车客货类型
        if(recvArray[nCount] == 0x01)
        {
            stagePlan->m_nLHFlg = LCTYPE_KC;
        }
        else
        {
            stagePlan->m_nLHFlg = LCTYPE_HC;
        }
        nCount += 1;

        //列车类型序号（管内动车组、通勤列车等）
        stagePlan->m_nIndexLCLX = recvArray[nCount];
        stagePlan->m_strLCLX = GetTrainType(stagePlan->m_nIndexLCLX);
        nCount += 1;

        //运行类型序号（动车组、快速旅客列车等）
        stagePlan->m_nIndexYXLX = recvArray[nCount];
        stagePlan->m_strYXLX = GetTrainRunType(stagePlan->m_nIndexYXLX);
        nCount += 1;
        qDebug() << "recive" << stagePlan->m_nIndexLCLX << stagePlan->m_strLCLX << stagePlan->m_nIndexYXLX << stagePlan->m_strYXLX;
        //终到
        if(stagePlan->m_btBeginOrEndFlg == JFC_TYPE_ZD)
        {
            stagePlan->m_strDepartTrainNum = "";

        }
        //始发
        else if(stagePlan->m_btBeginOrEndFlg == JFC_TYPE_SF)
        {
            stagePlan->m_strReachTrainNum = "";

        }

        //调度台
        stagePlan->m_strDispatchDesk = "调度台";
        //线路所
        bool bXianLuSuo = false;
        bXianLuSuo = IsXHDinXianLuSuo(GetStrNameByCode(stagePlan->m_nCodeReachStaEquip));
        if(!bXianLuSuo)
        {
            bXianLuSuo = IsXHDinXianLuSuo(GetStrNameByCode(stagePlan->m_nCodeDepartStaEquip));
        }
        stagePlan->bXianLuSuo = bXianLuSuo;
        if(stagePlan->bXianLuSuo)
        {
            //线路所进路默认全部是通过
            stagePlan->m_btBeginOrEndFlg = JFC_TYPE_TG;
            stagePlan->m_strRecvTrainTrack = stagePlan->m_strXHD_JZk + "-" + stagePlan->m_strXHD_CZk;
            stagePlan->m_strDepartTrainTrack = stagePlan->m_strRecvTrainTrack;
        }

        //信号-发送数据回执【教师机】
        QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, len);
        emit sendDataToTeacherSignal(this, qRecvArray, len);

        return stagePlan;
}
//更新收到的调度命令信息
DispatchOrderStation *MyStation::updateDisorderSta(unsigned char *recvArray, int len)
{
    //调度命令
    DispatchOrderStation* disOrder = new DispatchOrderStation();
    //接受命令号
    unsigned char strNumberRecv[11];
    memset(strNumberRecv,0,sizeof(strNumberRecv));
    memcpy(strNumberRecv, &recvArray[10], 10);
    disOrder->strNumber = UnsignedCharArrayToString(strNumberRecv);
    disOrder->uNumber = disOrder->strNumber.toInt();
    //命令时间
    QDateTime tmCurt = QDateTime::currentDateTime();
    disOrder->timOrder = tmCurt;
    //调度员
    unsigned char strCommander[21];
    memset(strCommander,0,sizeof(strCommander));
    memcpy(strCommander, &recvArray[27], 20);
    disOrder->strDisName = UnsignedCharArrayToString(strCommander);
    //调度中心
    disOrder->strDisCenter = CTCCENTER;//"CTC调度中心";
//        const char *str = "CTC调度中心";
//        disOrder.strDisCenter = str;
//        disOrder.strDisCenter = QStringLiteral("CTC调度中心");
//        disOrder.strDisCenter = QString::fromUtf8(str);
    //命令类型
    unsigned char strCmdType[41];
    memset(strCmdType,0,sizeof(strCmdType));
    memcpy(strCmdType, &recvArray[47], 40);
    disOrder->strType = UnsignedCharArrayToString(strCmdType);
    if(disOrder->strType.length() == 0)
    {
        disOrder->strType = "调度命令";
    }
    //命令内容
    //新协议
    //命令长度
    int lenghCmd = recvArray[87] | recvArray[88]<<8;
    //命令内容
    unsigned char arrContentRecv[1024];
    memset(arrContentRecv,0,sizeof(arrContentRecv));
    if(lenghCmd >= sizeof(arrContentRecv))
    {
        lenghCmd = sizeof(arrContentRecv) - 1;
    }
    memcpy(arrContentRecv, &recvArray[89], lenghCmd);
    //命令内容
    QString recvCont = UnsignedCharArrayToString(arrContentRecv);
    //命令内容
    disOrder->strContent = recvCont;
    //受令单位
    disOrder->listRecvPlace.append(this->getStationName());
    //车站id
    disOrder->station_id = this->getStationID();
//        //增加一条
//        m_ArrayDisOrderRecv.append(disOrder);

    //信号-发送数据回执【教师机】
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, len);
    emit sendDataToTeacherSignal(this, qRecvArray, len);

    return disOrder;
}

//根据索引索取列车类型（管内动车组、通勤列车等几十种）
QString MyStation::GetTrainType(int index)
{
    int count = v_TrainNumType.count();
    if(index >=0 && index<count)
    {
        return v_TrainNumType[index]->strTypeName;
    }
    return "";
}
//根据索引获取客货类型
QString MyStation::GetLHFlg(int index)
{
    int count = v_TrainNumType.count();
    if(index >=0 && index<count)
    {
        return v_TrainNumType[index]->strType;
    }
    return "客车";
}
//根据名称获取列车类型索引
int MyStation::GetTrainTypeIndex(QString _strType)
{
    for(int i=0; i<v_TrainNumType.count(); i++)
    {
        if(_strType == v_TrainNumType[i]->strTypeName)
        {
            return i;
        }
    }
    return -1;
}
//根据索引索取列车运行类型（动车组、快速旅客列车等几种）
QString MyStation::GetTrainRunType(int index)
{
    int count = v_TrainRunType.count();
    if(index >=0 && index<count)
    {
        return v_TrainRunType[index];
    }
    return "";
}
//根据名称获取列车运行类型索引
int MyStation::GetTrainRunTypeIndex(QString _strRunType)
{
    for(int i=0; i<v_TrainRunType.count(); i++)
    {
        if(_strRunType == v_TrainRunType[i])
        {
            return i;
        }
    }
    return -1;
}


//根据索引索取超限级别
QString MyStation::GetChaoXianLevel(int index)
{
    QString strLevel = "";
    if(index == 0)
    {
        strLevel = CHAOXIAN_0;
    }
    else if(index == 1)
    {
        strLevel = CHAOXIAN_1;
    }
    else if(index == 2)
    {
        strLevel = CHAOXIAN_2;
    }
    else if(index == 3)
    {
        strLevel = CHAOXIAN_3;
    }
    else if(index == 4)
    {
        strLevel = CHAOXIAN_4;
    }
    return strLevel;
}
//根据名称获取超限级别索引
int MyStation::GetChaoXianLevelIndex(QString _strChaoXian)
{
    int id = -1;
    if(_strChaoXian == CHAOXIAN_0)
    {
        id = 0;
    }
    else if(_strChaoXian == CHAOXIAN_1)
    {
        id = 1;
    }
    else if(_strChaoXian == CHAOXIAN_2)
    {
        id = 2;
    }
    else if(_strChaoXian == CHAOXIAN_3)
    {
        id = 3;
    }
    else if(_strChaoXian == CHAOXIAN_4)
    {
        id = 4;
    }
    return id;
}
//获取车次在行车日志数组中的索引值
int MyStation::GetIndexInTrafficArray(QString strTrainNum)
{
    for(int i=0; i<m_ArrayTrafficLog.count(); i++)
    {
        TrafficLog* pTrafficLog = m_ArrayTrafficLog[i];
        if ((pTrafficLog->m_strReachTrainNum == strTrainNum && pTrafficLog->m_strReachTrainNum != "")
            || (pTrafficLog->m_strDepartTrainNum == strTrainNum && pTrafficLog->m_strDepartTrainNum != ""))
        {
            return i;
        }
    }
    return -1;
}
// 根据车次查找行车日志，找不到则返回nullprt
TrafficLog *MyStation::GetTrafficLogByCheCi(QString strTrainNum)
{
    for(int i=0; i<m_ArrayTrafficLog.count(); i++)
    {
        TrafficLog* pTrafficLog = m_ArrayTrafficLog[i];
        if ((pTrafficLog->m_strReachTrainNum == strTrainNum && pTrafficLog->m_strReachTrainNum != "")
            || (pTrafficLog->m_strDepartTrainNum == strTrainNum && pTrafficLog->m_strDepartTrainNum != ""))
        {
            return pTrafficLog;
        }
    }
    return nullptr;
}
// 获取进路序列的索引
int MyStation::GetTrainRouteIndex(QString _strCheci, int _JFCType)
{
    for(int i = 0; i < m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = m_ArrayRouteOrder[i];
        if( _JFCType == pTrainRouteOrder->m_btRecvOrDepart
           && _strCheci == pTrainRouteOrder->m_strTrainNum)
        {
            return i;
        }
    }

    return -1;
}
// 获取进路序列的索引,不是组合进路的主序列(即查找子序列)
int MyStation::GetTrainRouteIndexNotZHJL(QString _strCheci, int _JFCType)
{
    for(int i = 0; i < m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = m_ArrayRouteOrder[i];
        if( _JFCType == pTrainRouteOrder->m_btRecvOrDepart
           && _strCheci == pTrainRouteOrder->m_strTrainNum
           && 0 == pTrainRouteOrder->m_bZHJL)
        {
            return i;
        }
    }

    return -1;
}
// 获取进路序列的索引,置顶的等待自触的组合进路的子序列
int MyStation::GetPopTrainRouteIndexOfZHJL(QString _strCheci, int _JFCType)
{
    for(int i = 0; i < m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = m_ArrayRouteOrder[i];
        if( _JFCType == pTrainRouteOrder->m_btRecvOrDepart
           && _strCheci == pTrainRouteOrder->m_strTrainNum
           && 0 == pTrainRouteOrder->m_nOldRouteState
           && 0 == pTrainRouteOrder->m_bZHJL
           && 0 < pTrainRouteOrder->father_id)
        {
            return i;
        }
    }

    return -1;
}
//获取接发车口对应的方向站名， nCode 接车、发车设备号
QString MyStation::GetJFCKDirectByCode(int nCode)
{
    for (int i = 0; i < StaConfigInfo.JCKCount; i++)
    {
        if (nCode == StaConfigInfo.JFCKInfo[i].nJCKCode)
        {
            return StaConfigInfo.JFCKInfo[i].strDirectSta;
        }
        if (nCode == StaConfigInfo.JFCKInfo[i].nFCKCode)
        {
            return StaConfigInfo.JFCKInfo[i].strDirectSta;
        }
    }
    return "未知";
}
// nCode接车、发车设备号, nFlg出发1 到达0 //阶段计划传来该设备号是否是上行
bool MyStation::GetSXByCode(int nCode, int nFlg)
{
    bool bvalue = false;
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            if(nCode == (int)xhd->getCode())
            {
                if(xhd->m_nSX == 1)	//如果是上行口
                {
                    if(nFlg == 0)
                        bvalue = true;
                    else
                        bvalue = false;
                }
                else //if(pXHD->m_nSX == 0) //如果下行口
                {
                    if(nFlg == 0)
                        bvalue = false;
                    else
                        bvalue = true;
                }
                break;
            }
        }
        else if(ement->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD*)ement;
            if(nCode == (int)gd->getCode())
            {
                return gd->m_nSX;
            }
        }
        else if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            if(((-2==nFlg) && (nCode == (int)gddc->getQDCode()))
                || (nCode == (int)gddc->getCode()))
            {
                return gddc->m_nSX;
            }
        }
    }
    return bvalue;
}
//根据阶段计划更新进路序列
void MyStation::UpdatTrainRouteOrderByStagePlan(TrainRouteOrder *_pTrainRouteOrder, StagePlan *_StagePlan, int _JFCType, bool _bConfirmed)
{
    _pTrainRouteOrder->station_id = this->getStationID();
    if(_JFCType == ROUTE_JC)
    {
        _pTrainRouteOrder->m_nPlanNumber = _StagePlan->m_nPlanNumber;
        if(_bConfirmed)
        {
            _pTrainRouteOrder->m_timPlanned = _StagePlan->m_timProvReachStation;
            _pTrainRouteOrder->m_timBegin = _StagePlan->m_timProvReachStation;
        }
        else
        {
            _pTrainRouteOrder->bXianLuSuo = _StagePlan->bXianLuSuo;
            _pTrainRouteOrder->m_btRecvOrDepart = ROUTE_JC;//0x00;
            _pTrainRouteOrder->m_btBeginOrEndFlg = _StagePlan->m_btBeginOrEndFlg;//所属阶段计划标志
            _pTrainRouteOrder->m_strTrainNum = _StagePlan->m_strReachTrainNum;
            _pTrainRouteOrder->m_nTrackCode = _StagePlan->m_nRecvTrainTrack;
            _pTrainRouteOrder->m_strTrack = GetStrNameByCode(_StagePlan->m_nRecvTrainTrack);
            _pTrainRouteOrder->m_nGDPos = GetGDPosInzcArray(_StagePlan->m_nRecvTrainTrack);
            _pTrainRouteOrder->m_nCodeReachStaEquip = _StagePlan->m_nCodeReachStaEquip;
            _pTrainRouteOrder->m_strXHD_JZk = GetStrNameByCode(_StagePlan->m_nCodeReachStaEquip);
            //获取进路方向
            _pTrainRouteOrder->m_strDirection = GetDirectByCode(_StagePlan->m_nCodeReachStaEquip,0);
            _pTrainRouteOrder->m_timPlanned = _StagePlan->m_timProvReachStation;
            //_pTrainRouteOrder->m_timBegin = _pTrainRouteOrder->m_timPlanned.addSecs(0-(AutoTouchReachRouteLeadMinutes*60));//_StagePlan->m_timProvReachStation;
            _pTrainRouteOrder->m_timBegin = _pTrainRouteOrder->m_timPlanned;
            _pTrainRouteOrder->m_bElectric = _StagePlan->m_bElectric;
            _pTrainRouteOrder->m_nLevelCX = _StagePlan->m_nLevelCX;
            _pTrainRouteOrder->m_nLHFlg = _StagePlan->m_nLHFlg;
            //获取进路描述
            int nCode = GetCodeByRecvEquipGD(_StagePlan->m_nCodeReachStaEquip,
                _StagePlan->m_nRecvTrainTrack,0);
            _pTrainRouteOrder->m_strXHDBegin = GetStrNameByCode(_StagePlan->m_nCodeReachStaEquip);
            _pTrainRouteOrder->m_strXHDEnd   = GetStrNameByCode(nCode);
            _pTrainRouteOrder->m_strRouteDescrip =  _pTrainRouteOrder->m_strXHDBegin + "-" + _pTrainRouteOrder->m_strXHDEnd;
            _pTrainRouteOrder->m_strRouteDescripReal =  _pTrainRouteOrder->m_strXHDBegin + "," + _pTrainRouteOrder->m_strXHDEnd;
            //线路所无股道信息，将接车口和发车口组成进路描述
            if(_pTrainRouteOrder->bXianLuSuo)
            {
                _pTrainRouteOrder->m_nCodeDepartStaEquip = _StagePlan->m_nCodeDepartStaEquip;
                _pTrainRouteOrder->m_nTrackCode = 0xFFFF;
                _pTrainRouteOrder->m_strTrack = GetStrNameByCode(_StagePlan->m_nCodeReachStaEquip) + "-" + GetStrNameByCode(_StagePlan->m_nCodeDepartStaEquip);
                _pTrainRouteOrder->m_nGDPos = -1;
                _pTrainRouteOrder->m_strDirection = "通过";
                _pTrainRouteOrder->m_strXHDBegin = GetStrNameByCode(_StagePlan->m_nCodeReachStaEquip);
                _pTrainRouteOrder->m_strXHDEnd   = GetStrNameByCode(_StagePlan->m_nCodeDepartStaEquip);
                _pTrainRouteOrder->m_strRouteDescrip =  _pTrainRouteOrder->m_strXHDBegin + "-" + _pTrainRouteOrder->m_strXHDEnd;
                _pTrainRouteOrder->m_strRouteDescripReal =  _pTrainRouteOrder->m_strXHDBegin + "," + _pTrainRouteOrder->m_strXHDEnd;
            }
            //判断延续进路
            InitRouteYXJL(_pTrainRouteOrder);
            //初始化变通进路
            InitRouteBtjl(_pTrainRouteOrder);
            //初始化组合进路
            InitRouteZhjl(_pTrainRouteOrder);
        }
    }
    else if(_JFCType == ROUTE_FC)
    {
        _pTrainRouteOrder->m_nPlanNumber = _StagePlan->m_nPlanNumber;
        if(_bConfirmed)
        {
            _pTrainRouteOrder->m_timPlanned = _StagePlan->m_timProvDepaTrain;
            _pTrainRouteOrder->m_timBegin = _StagePlan->m_timProvDepaTrain;
        }
        else
        {
            _pTrainRouteOrder->bXianLuSuo = _StagePlan->bXianLuSuo;
            _pTrainRouteOrder->m_btRecvOrDepart = ROUTE_FC;//0x01;
            _pTrainRouteOrder->m_btBeginOrEndFlg = _StagePlan->m_btBeginOrEndFlg;//所属阶段计划标志
            _pTrainRouteOrder->m_strTrainNum = _StagePlan->m_strDepartTrainNum;
            //获取股道号
            _pTrainRouteOrder->m_nTrackCode = _StagePlan->m_nDepartTrainTrack;
            _pTrainRouteOrder->m_strTrack = GetStrNameByCode(_StagePlan->m_nDepartTrainTrack);
            _pTrainRouteOrder->m_nGDPos = GetGDPosInzcArray(_StagePlan->m_nDepartTrainTrack);
            _pTrainRouteOrder->m_nCodeDepartStaEquip = _StagePlan->m_nCodeDepartStaEquip;
            _pTrainRouteOrder->m_strXHD_CZk = GetStrNameByCode(_StagePlan->m_nCodeDepartStaEquip);
            //获取进路方向
            _pTrainRouteOrder->m_strDirection = GetDirectByCode(_StagePlan->m_nCodeDepartStaEquip,1);
            _pTrainRouteOrder->m_timPlanned = _StagePlan->m_timProvDepaTrain;
            //_pTrainRouteOrder->m_timBegin = _StagePlan->m_timProvDepaTrain.addSecs(0-(AutoTouchDepartRouteLeadMinutes*60));//_StagePlan->m_timProvDepaTrain;
            _pTrainRouteOrder->m_timBegin = _pTrainRouteOrder->m_timPlanned;
            ////通过进路的时间按照接车的提前量设置
            //if(_pTrainRouteOrder->m_btBeginOrEndFlg == JFC_TYPE_TG)
            //{
            //    _pTrainRouteOrder->m_timBegin = _pTrainRouteOrder->m_timPlanned.addSecs(0-(AutoTouchReachRouteLeadMinutes*60));
            //}
            _pTrainRouteOrder->m_bElectric = _StagePlan->m_bElectric;
            _pTrainRouteOrder->m_nLevelCX = _StagePlan->m_nLevelCX;
            _pTrainRouteOrder->m_nLHFlg = _StagePlan->m_nLHFlg;
            //获取进路描述
            int nCode = GetCodeByRecvEquipGD(_StagePlan->m_nCodeDepartStaEquip,
                _StagePlan->m_nDepartTrainTrack,1);
            _pTrainRouteOrder->m_strXHDBegin = GetStrNameByCode(nCode);
            _pTrainRouteOrder->m_strXHDEnd   = GetStrNameByCode(_StagePlan->m_nCodeDepartStaEquip);
            _pTrainRouteOrder->m_strRouteDescrip =  _pTrainRouteOrder->m_strXHDBegin + "-" + _pTrainRouteOrder->m_strXHDEnd;
            _pTrainRouteOrder->m_strRouteDescripReal =  _pTrainRouteOrder->m_strXHDBegin + "," + _pTrainRouteOrder->m_strXHDEnd;
            //初始化变通进路
            InitRouteBtjl(_pTrainRouteOrder);
            //初始化组合进路
            InitRouteZhjl(_pTrainRouteOrder);
        }
    }

    //没确认的可以进行更新自触
    if(!_bConfirmed)
    {
        //检查进路条件
        CheckTrainRouteOrder(_pTrainRouteOrder);
    }
}

//获取方向 nFlg,出发1 接收0
QString MyStation::GetDirectByCode(int nCode, int nFlg)
{
    QString strName = GetJFCKDirectByCode(nCode);
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_XH)
        {
            CXHD *pXHD=(CXHD *)ement;
            if(nCode == (int)pXHD->getCode())
            {
                if(StaConfigInfo.bStaSXLORR == 1)
                {
                    if (pXHD->m_nSX == 1)	//如果是上行口
                    {
                        if (nFlg == 0)
                            return "<-" + strName;//return L"<-C站";
                        else
                            return "->" + strName;//return L"->C站";
                    }
                    else if (pXHD->m_nSX == 0) //如果下行口
                    {
                        if (nFlg == 0)
                            return strName + "->";//return L"A站->";
                        else
                            return strName+ "<-";//return L"A站<-";
                    }
                }
                else
                {
                    if (pXHD->m_nSX == 1)	//如果是上行口
                    {
                        if (nFlg == 0)
                            return strName + "->";//return L"A站->";
                        else
                            return strName + "<-";//return L"A站<-";
                    }
                    else if (pXHD->m_nSX == 0) //如果下行口
                    {
                        if (nFlg == 0)
                            return "<-" + strName;//return L"<-C站";
                        else
                            return "->" + strName;//return L"->C站";
                    }
                }
            }
        }
    }
    return "未知方向";
}
//获取设备号
int MyStation::GetCodeByRecvEquipGD(int nCodeReachStaEquip, int nTrackCode, int jfType)
{
    int nSXFlg = 0;
    QString leftNodeName = "";
    QString leftNodeName1 = "";
    QString RightNodeName = "";
    QString RightNodeName1 = "";
    bool bIgnoreLeftSXX = false;//忽略左侧信号机的上下行
    bool bIgnoreRightSXX = false;//忽略右侧信号机的上下行

    //寻找股道左右信号灯位置
    for(int i=0; i<vectStationGDNode.size(); i++)
    {
        if(vectStationGDNode[i].nCode == nTrackCode)
        {
            leftNodeName = vectStationGDNode[i].strLeftNode;
            leftNodeName1 = vectStationGDNode[i].strLeftNode1;
            RightNodeName = vectStationGDNode[i].strRightNode;
            RightNodeName1 = vectStationGDNode[i].strRightNode1;
            bIgnoreLeftSXX = vectStationGDNode[i].bIgnoreLeftSXX;
            bIgnoreRightSXX = vectStationGDNode[i].bIgnoreRightSXX;
            break;
        }
    }

    //获取nCodeReachStaEquip 对应设备的上下行
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_XH)
        {
            CXHD *pXHD=(CXHD *)ement;
            if(nCodeReachStaEquip == (int)pXHD->getCode())
            {
                nSXFlg = pXHD->m_nSX + 1;	//获取
                break;
            }

        }
    }

    //获取左节点
    for(auto ement:DevArray)
    {
        if(ement->getDevType() == Dev_XH)
        {
             CXHD *pXHD=(CXHD *)ement;
            //如果是左节点
            if(pXHD->m_strName == leftNodeName)
            {
                if(nSXFlg == 0)
                {
                    qDebug()<<"wu fa huo wu shang xia xing !";
                }
                else if(bIgnoreLeftSXX)
                {
                    return pXHD->m_nCode;
                }
                else if(nSXFlg == (int)(pXHD->m_nSX + 1))	//判断左节点的上下行与输入设备号的上下行一致
                {
                    //if(pXHD->m_nCode==nCodeReachStaEquip)
                    //{
                    //	for(int j = 0; j < DevArray.count(); j++)
                    //	{
                    //		if ( "CXHD" == DevArray[j]->GetRuntimeClass()->m_lpszClassName)
                    //		{
                    //			pXHD_TMP = (CXHD *)DevArray[j];
                    //			if(pXHD_TMP->m_strName == RightNodeName)
                    //			{
                    //				return pXHD_TMP->m_nCode;
                    //			}
                    //		}
                    //	}
                    //}
                    //else
                    if(jfType==0)//接车
                    {
                        return pXHD->m_nCode;
                    }
                    else if(jfType==1)//发车
                    {
                        if(leftNodeName1 != "")
                        {
                            int xhdIndex1 = this->GetIndexByStrName(leftNodeName1);
                            if(xhdIndex1>-1)
                            {
                                CXHD *pXHD1=(CXHD*)(DevArray[xhdIndex1]);
                                return pXHD1->m_nCode;
                            }
                            else
                            {
                                return pXHD->m_nCode;//防护
                            }
                        }
                        else
                        {
                            return pXHD->m_nCode;
                        }
                    }

                }

            }
            //如果是右节点
            else if(pXHD->m_strName == RightNodeName)
            {
                if(nSXFlg == 0)
                {
                    //AfxMessageBox(_T("无法获取上下行信息！！"));]
                }
                else if(bIgnoreRightSXX)
                {
                    return pXHD->m_nCode;
                }
                else if(nSXFlg == (int)(pXHD->m_nSX + 1))	//判断右节点的上下行与输入设备号的上下行一致
                {
                    //if(pXHD->m_nCode==nCodeReachStaEquip)
                    //{
                    //	for(int j = 0; j < DevArray.count(); j++)
                    //	{
                    //		if ( "CXHD" == DevArray[j]->GetRuntimeClass()->m_lpszClassName)
                    //		{
                    //			pXHD_TMP = (CXHD *)DevArray[j];
                    //			if(pXHD_TMP->m_strName == leftNodeName)
                    //			{
                    //				return pXHD_TMP->m_nCode;
                    //			}
                    //		}
                    //	}
                    //}
                    //else
                    if(jfType==0)//接车
                    {
                        return pXHD->m_nCode;
                    }
                    else if(jfType==1)//发车
                    {
                        if(RightNodeName1 != "")
                        {
                            int xhdIndex1 = this->GetIndexByStrName(RightNodeName1);
                            if(xhdIndex1>-1)
                            {
                                CXHD *pXHD1=(CXHD*)(DevArray[xhdIndex1]);
                                return pXHD1->m_nCode;
                            }
                            else
                            {
                                return pXHD->m_nCode;//防护
                            }
                        }
                        else
                        {
                            return pXHD->m_nCode;
                        }
                    }
                }
            }
        }
    }

    return -1;
}
//重置或初始化进路序列的延续进路
bool MyStation::InitRouteYXJL(TrainRouteOrder *_pTrainRouteOrder)
{
    for(int y = 0; y < StaConfigInfo.YXJLBeginNum; y++)
    {
        if(StaConfigInfo.YXJLGroup[y].strXHDbegin == _pTrainRouteOrder->m_strXHDBegin)
        {
            _pTrainRouteOrder->m_bYXJL = true;//是延续进路
            bool bYxjlOK = false;
            for (int gp=0; gp<StaConfigInfo.YXJLGroup[y].nGDYXJLCount; gp++)
            {
                for(int gd=0; gd<StaConfigInfo.YXJLGroup[y].GD_YXJL[gp].strArrGD.count(); gd++)
                {
                    if (_pTrainRouteOrder->m_strTrack == StaConfigInfo.YXJLGroup[y].GD_YXJL[gp].strArrGD[gd])
                    {
                        if (0<StaConfigInfo.YXJLGroup[y].GD_YXJL[gp].strArrYXEnd.count())
                        {
                            //默认的延续终端
                            _pTrainRouteOrder->m_strXHDYXEnd = StaConfigInfo.YXJLGroup[y].GD_YXJL[gp].strArrYXEnd[0];
                            _pTrainRouteOrder->m_strRouteDescrip =
                                _pTrainRouteOrder->m_strRouteDescrip + "-" + _pTrainRouteOrder->m_strXHDYXEnd;
                            _pTrainRouteOrder->m_strRouteDescripReal =
                                _pTrainRouteOrder->m_strRouteDescripReal + "," + _pTrainRouteOrder->m_strXHDYXEnd;
                            bYxjlOK = true;
                            break;
                        }
                    }
                }
            }
            if(true == bYxjlOK)
            {
                break;
            }
        }
    }
    return true;
}

//重置或初始化变通进路
void MyStation::InitRouteBtjl(TrainRouteOrder* pTrainRouteOrder)
{
    if (!pTrainRouteOrder)
    {
        return;
    }

    pTrainRouteOrder->strArrayBTJL.clear();
    pTrainRouteOrder->strArrayBTJL.append(pTrainRouteOrder->m_strRouteDescripReal);//m_strRouteDescrip

    bool bMatch = false;
    int count = vectBTJL.size();
    for (int t = 0; t<count; t++)
    {
        int countChild = vectBTJL[t].vectBTJLChild.size();
        for (int tt = 0; tt<countChild; tt++)
        {
            QString jlinfo = vectBTJL[t].vectBTJLChild[tt];
            QString tempJL = jlinfo;
            tempJL.replace(",", "-");
            if (pTrainRouteOrder->m_strRouteDescrip == tempJL)
            {
                bMatch = true;
            }
        }
        if (bMatch)
        {
            for (int tt = 0; tt<countChild; tt++)
            {
                QString jlinfo = vectBTJL[t].vectBTJLChild[tt];
                QString tempJL = jlinfo;
                tempJL.replace(",", "-");
                if (pTrainRouteOrder->m_strRouteDescrip == tempJL)
                {
                    //基本进路不再重复增加
                    continue;
                }
                pTrainRouteOrder->strArrayBTJL.append(jlinfo);
            }
            break;
        }
    }

    if (pTrainRouteOrder->strArrayBTJL.count() > 1)
    {
        pTrainRouteOrder->m_bBTJL = TRUE;
    }
}
//初始化组合进路
void MyStation::InitRouteZhjl(TrainRouteOrder *pTrainRouteOrder)
{
    if (!pTrainRouteOrder)
    {
        return;
    }
    //清空
    this->m_ArrayRouteOrderSonTemp.clear();
    pTrainRouteOrder->m_bZHJL = FALSE;

    bool bMatch = false;
    int count = vectZHJL.size();
    for (int t = 0; t<count; t++)
    {
        int countChild = vectZHJL[t].vectZHJLChild.size();
        if (countChild>1)
        {
            //第一个为总进路描述
            QString jlinfo = vectZHJL[t].vectZHJLChild[0];
            QString tempJL = jlinfo;
            tempJL.replace(",", "-");
            if (pTrainRouteOrder->m_strRouteDescrip == tempJL)
            {
                bMatch = true;
            }
        }
        if (bMatch)
        {
            for (int tt = 1; tt<countChild; tt++)
            {
                //复制操作
                TrainRouteOrder* pRouteOrderSon = new TrainRouteOrder;
                pTrainRouteOrder->MyCopy(pRouteOrderSon);
                //进路始终端处理 //如X1,SL4
                QString jlinfo = vectZHJL[t].vectZHJLChild[tt];
                QStringList strArr;
                int c = StringSplit(jlinfo, ",", strArr);
                //QString tempJL = jlinfo;
                //tempJL.replace(",", "-");
                if(c>=2)
                {
                    pRouteOrderSon->m_strXHDBegin = strArr[0];
                    pRouteOrderSon->m_strXHDEnd   = strArr[strArr.length()-1];
                    pRouteOrderSon->m_strRouteDescrip =  pRouteOrderSon->m_strXHDBegin + "-" + pRouteOrderSon->m_strXHDEnd;
                    pRouteOrderSon->m_strRouteDescripReal =  pRouteOrderSon->m_strXHDBegin + "," + pRouteOrderSon->m_strXHDEnd;
                    //区分接车、发车
                    if(pRouteOrderSon->m_btRecvOrDepart == ROUTE_JC)
                    {
                        //修改自己的内容
                        //pRouteOrderSon->m_strTrack = GetGDNameInGDNodeList(pRouteOrderSon->m_strXHDEnd);
                        //pRouteOrderSon->m_nTrackCode = this->GetCodeByStrName(pRouteOrderSon->m_strTrack);
                        //pRouteOrderSon->m_nGDPos = GetGDPosInzcArray(pRouteOrderSon->m_nTrackCode);
                        pRouteOrderSon->m_strXHD_JZk = pRouteOrderSon->m_strXHDBegin;
                        pRouteOrderSon->m_nCodeReachStaEquip = this->GetCodeByStrName(pRouteOrderSon->m_strXHDBegin);
                    }
                    else
                    {
                        //修改自己的内容
                        //pRouteOrderSon->m_strTrack = GetGDNameInGDNodeList(pRouteOrderSon->m_strXHDBegin);
                        //pRouteOrderSon->m_nTrackCode = this->GetCodeByStrName(pRouteOrderSon->m_strTrack);
                        //pRouteOrderSon->m_nGDPos = GetGDPosInzcArray(pRouteOrderSon->m_nTrackCode);
                        pRouteOrderSon->m_strXHD_CZk = pRouteOrderSon->m_strXHDEnd;
                        pRouteOrderSon->m_nCodeDepartStaEquip = this->GetCodeByStrName(pRouteOrderSon->m_strXHDEnd);
                    }
                    //判断延续进路
                    InitRouteYXJL(pRouteOrderSon);
                    //初始化变通进路
                    InitRouteBtjl(pRouteOrderSon);
                    //增加临时变量
                    this->m_ArrayRouteOrderSonTemp.append(pRouteOrderSon);
                }
            }
            pTrainRouteOrder->m_bZHJL = TRUE;
            break;
        }
    }
}

//发送系统报警信息给CTC(级别123(4防错办)，类型1系统2行车，描述)
void MyStation::sendWarningMsgToCTC(int nLevel, int nType, QString strMsg)
{
    QDateTime timeNoww = QDateTime::currentDateTime();
    //倒序检查
    for(int i=(vectMsgRecord.size()-1); i>=0; i--)
    {
        MsgRecord msgRecord = vectMsgRecord[i];
        //找到本条记录
        if(nLevel==msgRecord.nLevel && nType==msgRecord.nType && strMsg==msgRecord.strMsg)
        {
            qint64 secondsVSLast = msgRecord.timeSend.secsTo(timeNoww);
            if(secondsVSLast<30)
            {
                return;//30秒内发送过，间隔30秒后再执行
            }
            break;
        }
    }
    //增加一条-消息记录
    MsgRecord msgRecord;
    msgRecord.nLevel = nLevel;
    msgRecord.nType  = nType;
    msgRecord.strMsg = strMsg;
    msgRecord.timeSend = timeNoww;
    vectMsgRecord.append(msgRecord);

    QByteArray dataArray;
    int nCount = 10;
    //添加nCount个字节并全部置零
    dataArray.append(nCount, char(0));
    //标志位
    dataArray[nCount-1] = FUNCTYPE_SYSMSG;
    //子分类码-系统消息框内容
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = 0x01;

    //级别
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = nLevel;
    //类型
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = nType;
    //时间
    nCount+=7;
    dataArray.append(7, char(0));
    QDateTime timeNow = QDateTime::currentDateTime();
    dataArray[nCount-7] = timeNow.date().year();
    dataArray[nCount-6] = timeNow.date().year()>>8;
    dataArray[nCount-5] = timeNow.date().month();
    dataArray[nCount-4] = timeNow.date().day();
    dataArray[nCount-3] = timeNow.time().hour();
    dataArray[nCount-2] = timeNow.time().minute();
    dataArray[nCount-1] = timeNow.time().second();

    //描述，增加站名
    QString sendMsg = QString(this->getStationName()+" ").append(strMsg);
    QByteArray byteArray = sendMsg.toLocal8Bit();//toLatin1();
    int msgLen = byteArray.count();
    //描述长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = msgLen;
    //描述内容
    nCount+=msgLen;
    dataArray.append(msgLen, char(0));
    for(int u=0; u<msgLen; u++)
    {
        dataArray[nCount-msgLen+u] = byteArray[u];
    }

    //来源长度
    byteArray = this->getStationName().toLocal8Bit();//toLatin1();
    int srcLen = byteArray.count();
    //来源长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = srcLen;
    //来源内容
    nCount+=srcLen;
    dataArray.append(srcLen, char(0));
    for(int u=0; u<srcLen; u++)
    {
        dataArray[nCount-srcLen+u] = byteArray[u];
    }

    //帧尾4+空白2
    nCount+=6;
    dataArray.append(6, char(0));
    //打包数据
    packHeadAndTail(&dataArray, nCount);
    //qDebug()<<"msg ="<<sendMsg;
    //qDebug()<<"msg dataArray="<<ByteArrayToString(dataArray);
    //信号-发送数据给CTC
    emit sendDataToCTCSignal(this, dataArray, nCount);
    //信号-发送数据给占线板
    emit sendDataToBoardSignal(this, dataArray, nCount);
    //信号-发送数据给集控台
    emit sendDataToJKSignal(this, dataArray, nCount);
    //信号-发送数据给占线图
    emit sendDataToZXTSignal(this, dataArray, nCount);
}

//接收联锁数据-列车位置信息
void MyStation::recvLSData_TrainPos(unsigned char *recvArray, int recvlen)
{
    Train* pTrainTemp = new Train;
    int nCount=10;
    //车次号长度
    int nNamelength=recvArray[nCount];
    nCount += 1;
    //车次号
    QString strTrainNum="";
    unsigned char strCheCiTemp[21];
    memset(strCheCiTemp,0,sizeof(strCheCiTemp));
    memcpy(strCheCiTemp, &recvArray[11], nNamelength);
    strTrainNum = UnsignedCharArrayToString(strCheCiTemp);
    pTrainTemp->m_strCheCi = strTrainNum;
    nCount += nNamelength;

    //所在股道信号设备号//显示位置
    //int nCode = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
    pTrainTemp->m_nPosCode = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
    nCount += 2;
    //实时位置
    //int nCodeReal = nCode;
    //int nCodeReal = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
    pTrainTemp->m_nPosCodeReal = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
    nCount += 2;

    //左行0x11  右行0x22 如不能提供该信息则添0x00
    bool bRight;
    if(0x22 == (int)recvArray[nCount])
    {
        bRight = true;//右行
        //checi.bRight = TRUE;//右行
    }
    else
    {
        bRight = false;//左行
        //checi.bRight = FALSE;//左行
    }
    pTrainTemp->m_bRight = bRight;
    nCount += 1;
    //qDebug()<<strTrainNum<<" bRight="<<bRight;

    //列车是否丢失 0x01 丢失故障 0x00 正常
    bool bLost;
    if((int)recvArray[nCount] == 0x01)
    {
        bLost = true;//车次丢失
        //checi.bLost = TRUE;//车次丢失
    }
    else
    {
        bLost = false;//正常
        //checi.bLost = FALSE;//正常
    }
    pTrainTemp->m_bLost = bLost;
    nCount += 1;

    //车次类型 0x02列车 0x03调车
    int trainType = recvArray[nCount];
    //checi.trainType = StatusArray[nCount];
    pTrainTemp->m_nType = trainType;
    nCount += 1;

    //列车类型 客车、货车
    int LCType;
    if((int)recvArray[nCount] == 0x01)
    {
        LCType = LCTYPE_KC;
        //checi.LCType = LCTYPE_KC;
    }
    else
    {
        LCType = LCTYPE_HC;
        //checi.LCType = LCTYPE_HC;
    }
    pTrainTemp->m_nKHType = LCType;
    nCount += 1;

    //列车是否正在运行
    bool bRunning;
    if((int)recvArray[nCount] == 0x01)
    {
        bRunning = true;
        //checi.bRunning = TRUE;
    }
    else
    {
        bRunning = false;
        //checi.bRunning = FALSE;
    }
    pTrainTemp->m_bRunning = bRunning;
    nCount += 1;

    //电力机车
    bool bElectric;
    if((int)recvArray[nCount] == 0x11)
    {
        bElectric = true;
        //checi.bElectric = TRUE;
    }
    else
    {
        bElectric = false;
        //checi.bElectric = FALSE;
    }
    pTrainTemp->m_bElectric = bElectric;
    nCount += 1;

    //超限
    int nLevelCX = (int)recvArray[nCount];
    //checi.nLevelCX = StatusArray[nCount];
    pTrainTemp->m_nLevelCX = nLevelCX;
    nCount += 1;

    //编组长度
    int nTrainLengh = (int)recvArray[nCount];
    //checi.nTrainLengh = StatusArray[nCount];
    pTrainTemp->m_nLengh = nTrainLengh;
    nCount += 1;

    //速度（预设）
    int nSpeed = (int)recvArray[nCount];
    //checi.nSpeed = StatusArray[nCount];
    pTrainTemp->m_nSpeed = nSpeed;
    nCount += 1;

    //列车类型序号（管内动车组、通勤列车等）
    int nIndexLCLX = (int)recvArray[nCount];
    pTrainTemp->m_nIndexLCLX = nIndexLCLX;
    nCount += 1;

    this->SetCheCiInfo(pTrainTemp);
    delete pTrainTemp;
}
//设置车次信息
void MyStation::SetCheCiInfo(Train* pTrainTemp)
{
    Train *pTrain;
    int count = m_ArrayTrain.size();
    int i = 0;
    for(i=0; i<count; i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        if(pTrain->m_strCheCi == pTrainTemp->m_strCheCi)//修改车次属性
        {
            pTrain->m_nPosCode     = pTrainTemp->m_nPosCode;
            pTrain->m_nPosCodeReal = pTrainTemp->m_nPosCodeReal;
            pTrain->m_bRight       = pTrainTemp->m_bRight;
            pTrain->m_bLost        = pTrainTemp->m_bLost;
            pTrain->m_nType        = pTrainTemp->m_nType;
            pTrain->m_nKHType      = pTrainTemp->m_nKHType;
            pTrain->m_bRunning     = pTrainTemp->m_bRunning;
            pTrain->m_bElectric    = pTrainTemp->m_bElectric;
            pTrain->m_nLevelCX     = pTrainTemp->m_nLevelCX;
            pTrain->m_nLengh       = pTrainTemp->m_nLengh;
            pTrain->m_nSpeed       = pTrainTemp->m_nSpeed;
            pTrain->m_nIndexLCLX   = pTrainTemp->m_nIndexLCLX;
            pTrain->m_bActive = true;
            break;
        }
    }
    if(i>=count)//新增车次
    {
        pTrain = new Train;
        pTrain->m_strCheCi     = pTrainTemp->m_strCheCi;
        pTrain->m_nPosCode     = pTrainTemp->m_nPosCode;
        pTrain->m_nPosCodeReal = pTrainTemp->m_nPosCodeReal;
        pTrain->m_bRight       = pTrainTemp->m_bRight;
        pTrain->m_bLost        = pTrainTemp->m_bLost;
        pTrain->m_nType        = pTrainTemp->m_nType;
        pTrain->m_nKHType      = pTrainTemp->m_nKHType;
        pTrain->m_bRunning     = pTrainTemp->m_bRunning;
        pTrain->m_bElectric    = pTrainTemp->m_bElectric;
        pTrain->m_nLevelCX     = pTrainTemp->m_nLevelCX;
        pTrain->m_nLengh       = pTrainTemp->m_nLengh;
        pTrain->m_nSpeed       = pTrainTemp->m_nSpeed;
        pTrain->m_nIndexLCLX   = pTrainTemp->m_nIndexLCLX;
        pTrain->m_bActive  = true;
        m_ArrayTrain.append(pTrain);
    }
}
//设置区段车次
void MyStation::SetQDCheCi()
{
    //int ccCount = m_ArrayTrain.count();
    //int nArraySize =  DevArray.count();//获取站场大小
    CGD *pGD;
    CGDDC *pGDDC;
    Train *pTrain;

    for(int i=0; i< DevArray.count(); i++)
    {
        if(DevArray[i]->getDevType() == Dev_GD)
        {
            bool bCheCi = false;//有无车次
            pGD=(CGD *)(DevArray[i]);
            for(int j=0; j<m_ArrayTrain.size(); j++)
            {
                pTrain = (Train *)m_ArrayTrain[j];
                if(pTrain->m_nPosCode == (int)pGD->m_nCode)
                {
                    bCheCi = true;
                    //qDebug()<<"pTrain->m_bRight="<<pTrain->m_bRight;
                    pGD->m_strCheCiNum = pTrain->m_strCheCi;
                    pGD->m_nCheciLost  = pTrain->m_bLost;
                    pGD->m_bElectric   = pTrain->m_bElectric;
                    pGD->m_nSXCheCi    = pTrain->m_bRight;
                    pGD->m_nKHType     = pTrain->m_nKHType;
                    pGD->m_nSpeed     = pTrain->m_nSpeed;
                    pTrain->pCenter = pGD->getCenterPt().toPoint();
                    //if(pGD->getGDType() == GD_QD && !pTrain->m_bRunning)
                    //仅在股道上判断停稳标志
                    if(this->DevIsGDByCode(pGD->getCode()) && !pTrain->m_bRunning)
                    {
                        pTrain->m_bStop = true;
                    }
                    pGD->m_bLCTW       = pTrain->m_bStop;
                    break;
                }
            }
            if(!bCheCi)//无车次
            {
                pGD->m_strCheCiNum = "";
                //pGD->m_nCheciLost  = FALSE;
                //pGD->m_bElectric   = FALSE;
                //pGD->m_nSXCheCi    = 0；
            }
        }
        //增加向道岔区段设置车次.lwm.2021.10.30
        if(DevArray[i]->getDevType() == Dev_DC)
        {
            bool bCheCi = false;//有无车次
            pGDDC=(CGDDC *)(DevArray[i]);
            for(int j=0; j<m_ArrayTrain.size(); j++)
            {
                pTrain = (Train *)m_ArrayTrain[j];
                if(pTrain->m_nPosCode == (int)pGDDC->getQDCode())
                {
                    pGDDC->m_strCheCiNum = pTrain->m_strCheCi;
                    pGDDC->m_nCheciLost  = pTrain->m_bLost;
                    pGDDC->m_bElectric   = pTrain->m_bElectric;
                    pGDDC->m_nSXCheCi    = pTrain->m_bRight;
                    pGDDC->m_nKHType     = pTrain->m_nKHType;
                    pGDDC->m_nSpeed     = pTrain->m_nSpeed;
                    bCheCi = true;
                    pTrain->pCenter = pGDDC->getCenterPt().toPoint();
                    pTrain->m_bStop = false;
                    pGDDC->m_bLCTW       = pTrain->m_bStop;
                    break;
                }
            }
            if(!bCheCi)//无车次
            {
                pGDDC->m_strCheCiNum = "";
                //pGD->m_nCheciLost  = FALSE;
                //pGD->m_bElectric   = FALSE;
                //pGD->m_nSXCheCi    = 0；
            }
        }
    }
}
//更新车次活跃信息
void MyStation::UpdateCheCiInfo()
{
    Train *pTrain;
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        pTrain->m_bActive = false;
    }
}
//删除没有的车次
void MyStation::DeleteCheCi()
{
    Train *pTrain;
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        if(!pTrain->m_bActive)
        {
            m_ArrayTrain.removeAt(i);
            DeleteCheCi();
            break;
        }
    }
}
//修改车次电力属性
void MyStation::ChangeCheCiInfo(QString strCheci, bool bElectric)
{
    Train *pTrain;
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        if(strCheci == pTrain->m_strCheCi)
        {
            pTrain->m_bElectric = bElectric;
            break;
        }
    }
}
//删除某个车次
void MyStation::DeleteOneCheCi(QString strCheci)
{
    Train *pTrain;
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        if(pTrain->m_strCheCi == strCheci)
        {
            m_ArrayTrain.removeAt(i);
            break;
        }
    }
}
//修改车次号
void MyStation::ChangeCheCiNum(QString strOldCheci, QString strNewCheci)
{
    Train *pTrain;
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        if(strOldCheci == pTrain->m_strCheCi)
        {
            pTrain->m_strCheCi = strNewCheci;
            break;
        }
    }
}
//设置车次停稳
void MyStation::SetCheCiStop(QString strCheci, bool bStop)
{
    Train *pTrain;
    bool bCheCiFalg=true;
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        if(strCheci == pTrain->m_strCheCi)
        {
            pTrain->m_bStop = bStop;
            bCheCiFalg=false;
            break;
        }
    }
    if(bCheCiFalg)
    {
        for(int i=0; i<m_ArrayTrainManType.size(); i++)
        {
            pTrain = (Train *)m_ArrayTrainManType[i];
            if(strCheci == pTrain->m_strCheCi)
            {
                pTrain->m_bStop = bStop;
                break;
            }
        }
    }
}

//更新列车早晚点时间
bool MyStation::updateTrainOvertime(QString _strCheci, int _overtime)
{
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        Train *pTrain = (Train *)m_ArrayTrain[i];
        if(_strCheci == pTrain->m_strCheCi)
        {
            pTrain->m_nOvertime = _overtime;//早晚点
            return true;
        }
    }
    return false;
}
//根据计划更新列车早晚点时间
void MyStation::updateTrainOvertimeByPlan(QString _strCheci, QDateTime _time)
{
    QDateTime timeNow = _time;
    Train *pTrain;
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        for(int j=0; j<m_ArrayTrafficLog.size() && _strCheci==pTrain->m_strCheCi; j++)
        {
            TrafficLog *pTrafficLog = m_ArrayTrafficLog[j];
            if(pTrain->m_strCheCi == pTrafficLog->m_strTrainNum)
            {
                //qDebug()<<pTrafficLog->m_strTrainNum<<"m_timProvReachStation.year="<<pTrafficLog->m_timProvReachStation.date().year();
                //qDebug()<<pTrafficLog->m_strTrainNum<<"m_timRealReachStation.year="<<pTrafficLog->m_timRealReachStation.date().year();
                //qDebug()<<pTrafficLog->m_strTrainNum<<"m_timProvDepaTrain.year="<<pTrafficLog->m_timProvDepaTrain.date().year();
                //qDebug()<<pTrafficLog->m_strTrainNum<<"m_timRealDepaTrain.year="<<pTrafficLog->m_timRealDepaTrain.date().year()<<"\r\n";
                //（阶段计划类型）始发（终到）标志位--0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
                //始发车
                if(JFC_TYPE_SF == pTrafficLog->m_btBeginOrEndFlg)
                {
                    //发车报点-还没报点
                    if((1970 == pTrafficLog->m_timRealDepaTrain.date().year())
                        || (0 == pTrafficLog->m_timRealDepaTrain.date().year()))
                    {
                        //当前时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(timeNow)/60;
                        if(minis<=0)
                        {
                            pTrain->m_nOvertime = 0;//早点
                        }
                        else
                        {
                            pTrain->m_nOvertime = minis;//晚点
                        }
                    }
                    //发车报点-已经报点
                    else
                    {
                        //实际时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(pTrafficLog->m_timRealDepaTrain)/60;
                        pTrain->m_nOvertime = minis;
                    }
                }
                //终到车
                else if(JFC_TYPE_ZD == pTrafficLog->m_btBeginOrEndFlg)
                {
                    //接车报点-还没报点
                    if((1970 == pTrafficLog->m_timRealReachStation.date().year())
                        || (0 == pTrafficLog->m_timRealReachStation.date().year()))
                    {
                        //当前时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvReachStation.secsTo(timeNow)/60;
                        if(minis<=0)
                        {
                            pTrain->m_nOvertime = 0;//早点
                        }
                        else
                        {
                            pTrain->m_nOvertime = minis;//晚点
                        }
                    }
                    //接车报点-已经报点
                    else
                    {
                        //实际时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvReachStation.secsTo(pTrafficLog->m_timRealReachStation)/60;
                        pTrain->m_nOvertime = minis;
                    }
                }
                //接发+通过列车计划
                else
                {
                    //接车报点-还没报点
                    if((1970 == pTrafficLog->m_timRealReachStation.date().year())
                        || (0 == pTrafficLog->m_timRealReachStation.date().year()))
                    {
                        //当前时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvReachStation.secsTo(timeNow)/60;
                        if(minis<=0)
                        {
                            pTrain->m_nOvertime = 0;//早点，不显示
                        }
                        else
                        {
                            pTrain->m_nOvertime = minis;//晚点，显示
                        }
                    }
                    //发车报点-还没报点
                    else if((1970 == pTrafficLog->m_timRealDepaTrain.date().year())
                        || (0 == pTrafficLog->m_timRealDepaTrain.date().year()))
                    {
                        //当前时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(timeNow)/60;
                        //如果晚点了，则显示
                        if(minis<=0)
                        {
                            //pTrain->m_nOvertime = 0;//早点，不显示
                            //早点，显示
                            minis = pTrafficLog->m_timProvReachStation.secsTo(pTrafficLog->m_timRealReachStation)/60;
                            pTrain->m_nOvertime = minis;
                        }
                        else
                        {
                            pTrain->m_nOvertime = minis;//晚点，显示
                        }
//                        //按照接车报点来计算
//                        //实际时间比计划时间早，则为负值，负值表示早点
//                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(pTrafficLog->m_timRealDepaTrain)/60;
//                        pTrain->m_nOvertime = minis;
                    }
                    else
                    {
                        //发车报点-已经报点
                        //实际时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(pTrafficLog->m_timRealDepaTrain)/60;
                        pTrain->m_nOvertime = minis;
                    }
                }
                break;
            }
        }
    }
}
//根据计划更新列车早晚点时间
void MyStation::updateTrainOvertimeByPlan()
{
    QDateTime timeNow = QDateTime::currentDateTime();
    Train *pTrain;
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)m_ArrayTrain[i];
        for(int j=0; j<m_ArrayTrafficLog.size(); j++)
        {
            TrafficLog *pTrafficLog = m_ArrayTrafficLog[j];
            if(pTrain->m_strCheCi == pTrafficLog->m_strTrainNum)
            {
                //qDebug()<<pTrafficLog->m_strTrainNum<<"m_timProvReachStation.year="<<pTrafficLog->m_timProvReachStation.date().year();
                //qDebug()<<pTrafficLog->m_strTrainNum<<"m_timRealReachStation.year="<<pTrafficLog->m_timRealReachStation.date().year();
                //qDebug()<<pTrafficLog->m_strTrainNum<<"m_timProvDepaTrain.year="<<pTrafficLog->m_timProvDepaTrain.date().year();
                //qDebug()<<pTrafficLog->m_strTrainNum<<"m_timRealDepaTrain.year="<<pTrafficLog->m_timRealDepaTrain.date().year()<<"\r\n";
                //（阶段计划类型）始发（终到）标志位--0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
                //始发车
                if(JFC_TYPE_SF == pTrafficLog->m_btBeginOrEndFlg)
                {
                    //发车报点-还没报点
                    if((1970 == pTrafficLog->m_timRealDepaTrain.date().year())
                        || (0 == pTrafficLog->m_timRealDepaTrain.date().year()))
                    {
                        //当前时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(timeNow)/60;
                        if(minis<=0)
                        {
                            pTrain->m_nOvertime = 0;//早点
                        }
                        else
                        {
                            pTrain->m_nOvertime = minis;//晚点
                        }
                    }
                    //发车报点-已经报点
                    else
                    {
                        //实际时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(pTrafficLog->m_timRealDepaTrain)/60;
                        pTrain->m_nOvertime = minis;
                    }
                }
                //终到车
                else if(JFC_TYPE_ZD == pTrafficLog->m_btBeginOrEndFlg)
                {
                    //接车报点-还没报点
                    if((1970 == pTrafficLog->m_timRealReachStation.date().year())
                        || (0 == pTrafficLog->m_timRealReachStation.date().year()))
                    {
                        //当前时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvReachStation.secsTo(timeNow)/60;
                        if(minis<=0)
                        {
                            pTrain->m_nOvertime = 0;//早点
                        }
                        else
                        {
                            pTrain->m_nOvertime = minis;//晚点
                        }
                    }
                    //接车报点-已经报点
                    else
                    {
                        //实际时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvReachStation.secsTo(pTrafficLog->m_timRealReachStation)/60;
                        pTrain->m_nOvertime = minis;
                    }
                }
                //接发+通过列车计划
                else
                {
                    //接车报点-还没报点
                    if((1970 == pTrafficLog->m_timRealReachStation.date().year())
                        || (0 == pTrafficLog->m_timRealReachStation.date().year()))
                    {
                        //当前时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvReachStation.secsTo(timeNow)/60;
                        if(minis<=0)
                        {
                            pTrain->m_nOvertime = 0;//早点，不显示
                        }
                        else
                        {
                            pTrain->m_nOvertime = minis;//晚点，显示
                        }
                    }
                    //发车报点-还没报点
                    else if((1970 == pTrafficLog->m_timRealDepaTrain.date().year())
                        || (0 == pTrafficLog->m_timRealDepaTrain.date().year()))
                    {
                        //当前时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(timeNow)/60;
                        //如果晚点了，则显示
                        if(minis<=0)
                        {
                            //pTrain->m_nOvertime = 0;//早点，不显示
                            //早点，显示
                            minis = pTrafficLog->m_timProvReachStation.secsTo(pTrafficLog->m_timRealReachStation)/60;
                            pTrain->m_nOvertime = minis;
                        }
                        else
                        {
                            pTrain->m_nOvertime = minis;//晚点，显示
                        }
//                        //按照接车报点来计算
//                        //实际时间比计划时间早，则为负值，负值表示早点
//                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(pTrafficLog->m_timRealDepaTrain)/60;
//                        pTrain->m_nOvertime = minis;
                    }
                    else
                    {
                        //发车报点-已经报点
                        //实际时间比计划时间早，则为负值，负值表示早点
                        qint64 minis = pTrafficLog->m_timProvDepaTrain.secsTo(pTrafficLog->m_timRealDepaTrain)/60;
                        pTrain->m_nOvertime = minis;
                    }
                }
                break;
            }
        }
    }
}
//更新激活车次信息
void MyStation::UpdateStationCheCiInfo(int nElaps)
{
    //qDebug()<<"StationId="<<this->getStationID()<<"ArrayTrain.size="<<m_ArrayTrain.size();
    if (nElaps % 2 == 0)
    {
        UpdateCheCiInfo();//更新车次信息
    }
    else
    {
        DeleteCheCi();//删除没有的车次
    }
    //updateTrainOvertimeByPlan();//根据计划更新列车早晚点时间
    SetQDCheCi();//设置车次
}
//接收联锁数据-列车报点
void MyStation::recvLSData_ReportTime(unsigned char *recvArray, int recvlen)
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    int nCount=0;
    //车次号
    int nNamelength = recvArray[10];
    QString strTrainNum="";
    unsigned char strCheCiTemp[21];
    memset(strCheCiTemp,0,sizeof(strCheCiTemp));
    memcpy(strCheCiTemp, &recvArray[11], nNamelength);
    strTrainNum = UnsignedCharArrayToString(strCheCiTemp);
    QString str = strTrainNum;
    nCount = 11+nNamelength;

    int rePortType = recvArray[nCount];//记录行车日志报点信息类型
    nCount += 1;

    nCount += 1;  //方向此处未做解析

    //所在股道信号设备号及股道名
    int nCode = recvArray[nCount] | recvArray[nCount+1]<<8;
    bool bDevIsGD  = this->DevIsGDByCode(nCode);
    if(!bDevIsGD)//不是股道的不报点
    {
        return;
    }
    QString strGDName = this->GetStrNameByCode(nCode);
    nCount += 2;

    //时间
    QDateTime tmCurt;
    int year = (recvArray[nCount] | recvArray[nCount+1]<<8);
    int mouth = recvArray[nCount+2];
    int day = recvArray[nCount+3];
    int hour = recvArray[nCount+4];
    int mini = recvArray[nCount+5];
    int second = 0;//recvArray[nCount+6];//以分钟为计时
    QString strTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
    tmCurt = QDateTime::fromString(strTime, "yyyy-MM-dd hh:mm:ss");
    nCount += 7;

    int nSize = this->m_ArrayTrafficLog.count();
    int j = 0;
    if(nSize != 0)
    {
        //行车日志不为空
        for(j = 0; j < (int)this->m_ArrayTrafficLog.count(); j++)
        {
            TrafficLog *pTrafficLog = (TrafficLog *)this->m_ArrayTrafficLog[j];
            if( pTrafficLog->m_strDepartTrainNum == str
               || pTrafficLog->m_strReachTrainNum == str)
            {
                if(rePortType == 0x11)//发车报点
                {
                    pTrafficLog->m_strDepartTrainNum = str;
                    ////pTrafficLog->m_strReachTrainNum = str;
                    //pTrafficLog->m_strDepartTrainTrack = strGDName;//不更新股道
                    pTrafficLog->m_timRealDepaTrain = tmCurt;
                    this->SetCheCiStop(str, false);
                    SetTrafficLogProc(pTrafficLog);
                    m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                    //给邻站报点//出发、通过(报给下一个站)
                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_CZk);//出站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, strTrainNum, rePortType, tmCurt);
                    }

                    break;
                }
                else if(rePortType == 0x22)//停车报点
                {
                    //非始发车报点
                    if(pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_SF)
                    {
                        //pTrafficLog->m_strDepartTrainNum = str;
                        pTrafficLog->m_strReachTrainNum = str;
                        //pTrafficLog->m_strRecvTrainTrack = strGDName;//不更新股道
                        pTrafficLog->m_timRealReachStation = tmCurt;

                        this->SetCheCiStop(str, true);
                        SetTrafficLogProc(pTrafficLog);
                        m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                        //发送同步消息
                        //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                        this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                        //给邻站报点//到达、通过(报给上一个站)
                        int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_JZk);//进站口
                        if(lzId > 0)
                        {
                            emit UpdateLZReportTimeSignal(lzId, strTrainNum, rePortType, tmCurt);
                        }

                    }
                    break;
                }
                else if(rePortType == 0x33)//通过报点
                {
                    pTrafficLog->m_strDepartTrainNum = str;
                    pTrafficLog->m_strReachTrainNum = str;
                    //pTrafficLog->m_strDepartTrainTrack = strGDName;//不更新股道
                    //pTrafficLog->m_strRecvTrainTrack = strGDName;//不更新股道
                    pTrafficLog->m_timRealDepaTrain = tmCurt;
                    pTrafficLog->m_timRealReachStation = tmCurt;
                    SetTrafficLogProc(pTrafficLog);
                    m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                    //给邻站报点//出发、通过(报给下一个站)
                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_CZk);//出站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, strTrainNum, rePortType, tmCurt);
                    }
                    //给邻站报点//到达、通过(报给上一个站)
                    lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_JZk);//进站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, strTrainNum, rePortType, tmCurt);
                    }

                    break;
                }
            }
        }
    }
    //行车日志为空或者原列表不存在该车次 2017.7.25
    if(nSize == 0 || j >= nSize)
    {
        TrafficLog *pTrafficLog = new TrafficLog;
        pTrafficLog->station_id = this->getStationID();
        QString cctemp = str.right(1);
        bool bccSX = false;//上行车次
        if((cctemp.length() == 1) && (cctemp>="0") && (cctemp<="9"))
        {
            int ncc = cctemp.toInt();
            if(ncc%2==0)
            {
                bccSX = true;
            }
        }
        if(bccSX)//设置默认的接发车口 NEW,20180918
        {
            if(this->StaConfigInfo.JCKCount>0)
            {
                //pTrafficLog->m_nCodeReachStaEquip = pStation->StaConfigInfo.S_nCode;
                //pTrafficLog->m_nCodeDepartStaEquip =pStation->StaConfigInfo.XF_nCode;
                pTrafficLog->m_nCodeReachStaEquip = this->StaConfigInfo.JFCKInfo[0].nJCKCode;
                pTrafficLog->m_nCodeDepartStaEquip =this->StaConfigInfo.JFCKInfo[0].nFCKCode;
            }
        }
        else
        {
            if(this->StaConfigInfo.JCKCount>0)
            {
                //pTrafficLog->m_nCodeReachStaEquip = pStation->StaConfigInfo.X_nCode;
                //pTrafficLog->m_nCodeDepartStaEquip = pStation->StaConfigInfo.SF_nCode;
                pTrafficLog->m_nCodeReachStaEquip = this->StaConfigInfo.JFCKInfo[0].nJCKCode;
                pTrafficLog->m_nCodeDepartStaEquip =this->StaConfigInfo.JFCKInfo[0].nFCKCode;
            }
        }
        //0xBB始发 0xCC终到 0xDD通过
        if(rePortType == 0x11)//发车报点
        {
            pTrafficLog->m_btBeginOrEndFlg = 0xBB;
            //pTrafficLog->m_strTypeFlag = _T("始发");
            pTrafficLog->m_bDepartTrainNumSX = bccSX;
            pTrafficLog->m_strDepartTrainNum = str;
            pTrafficLog->m_strDepartTrainTrack = strGDName;
            pTrafficLog->m_timRealDepaTrain = tmCurt;
            this->m_ArrayTrafficLog.append(pTrafficLog);
            SetTrafficLogProc(pTrafficLog);
            this->SetCheCiStop(str, false);
        }
        else if(rePortType == 0x22)//停车报点
        {
            pTrafficLog->m_btBeginOrEndFlg = 0xCC;
            //pTrafficLog->m_strTypeFlag = _T("终到");
            pTrafficLog->m_bReachTrainNumSX = bccSX;
            pTrafficLog->m_strReachTrainNum = str;
            pTrafficLog->m_strRecvTrainTrack = strGDName;
            pTrafficLog->m_timRealReachStation = tmCurt;
            this->m_ArrayTrafficLog.append(pTrafficLog);
            SetTrafficLogProc(pTrafficLog);
            this->SetCheCiStop(str, true);
        }
        else if(rePortType == 0x33)//通过报点
        {
            pTrafficLog->m_btBeginOrEndFlg = 0xDD;
            pTrafficLog->m_strTypeFlag = "通过";
            pTrafficLog->m_bReachTrainNumSX = bccSX;
            pTrafficLog->m_bDepartTrainNumSX = bccSX;
            pTrafficLog->m_strDepartTrainNum = str;
            pTrafficLog->m_timRealDepaTrain = tmCurt;
            pTrafficLog->m_strDepartTrainTrack = strGDName;
            pTrafficLog->m_strReachTrainNum = str;
            pTrafficLog->m_strRecvTrainTrack = strGDName;
            pTrafficLog->m_timRealReachStation = tmCurt;
            SetTrafficLogProc(pTrafficLog);
            this->m_ArrayTrafficLog.append(pTrafficLog);
        }
        m_pDataAccess->InsetTrafficLog(pTrafficLog);
        //发送同步消息
        //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
        this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_ADD,1,1);
    }

    //更新进路序列时间
    UpdateRouteBeginTime(rePortType,str,tmCurt);

}
//接收联锁数据-调度命令
void MyStation::recvLSData_DDML(unsigned char *recvArray, int recvlen)
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    DispatchOrderStation* pDisOrderRecv = new DispatchOrderStation;
    pDisOrderRecv->station_id = this->getStationID();
    //命令号
    int num = (int)(recvArray[10] | recvArray[11]<<8);
    pDisOrderRecv->uNumber = num;
    pDisOrderRecv->strNumber = QString(num);
    //调度员
    pDisOrderRecv->strDisName = "DEFAULT";
    //调度中心
    pDisOrderRecv->strDisCenter = "CTC调度中心";
    //到达时间
    pDisOrderRecv->timOrder = QDateTime::currentDateTime();
    //命令内容
    unsigned char arrContentRecv[484];
    memset(arrContentRecv,0,sizeof(arrContentRecv));
    memcpy(arrContentRecv, &recvArray[12], 484);
    pDisOrderRecv->strContent = UnsignedCharArrayToString(arrContentRecv);
    //受令单位
    pDisOrderRecv->listRecvPlace.append(this->getStationName());
    //数据库访问
    pDisOrderRecv->order_id = m_pDataAccess->InsertDisOrderRecv(pDisOrderRecv);
    this->m_ArrayDisOrderRecv.append(pDisOrderRecv);
    //发送同步消息
    sendUpdateDataMsg(this, UPDATETYPE_DDML);
    //处理和发送1个数据
    this->sendOneDisOrderToSoft(DATATYPE_ALL,pDisOrderRecv,SYNC_FLAG_ADD,1,1);
    //报警信息
    this->sendWarningMsgToCTC(3,2,"收到调度命令");
    //语音播报
    this->SendSpeachText("调度命令下达请签收");
}
//接收联锁数据-阶段计划
void MyStation::recvLSData_JDJH(unsigned char *recvArray, int recvlen)
{
    StagePlan *stagePlan = new StagePlan;
    //始发终到标志
    stagePlan->m_btBeginOrEndFlg = (int)recvArray[40];

    if(stagePlan->m_btBeginOrEndFlg == 0xAA	//正常
        ||stagePlan->m_btBeginOrEndFlg == 0xCC	//终到
        ||stagePlan->m_btBeginOrEndFlg == 0xDD) //通过
    {
        //接受车号
        unsigned char strReachTrainNum[7] = {0,0,0,0,0,0,0};//6
        memcpy(strReachTrainNum, &recvArray[10], 6);
        stagePlan->m_strReachTrainNum = UnsignedCharArrayToString(strReachTrainNum);
        //接车口
        stagePlan->m_nCodeReachStaEquip = RecvArrayToInt(&recvArray[41], 2);
        //接车股道
        stagePlan->m_nRecvTrainTrack = RecvArrayToInt(&recvArray[22], 2);
        //到达时间
        {
            int year = (recvArray[24] | recvArray[25]<<8);
            int mouth = recvArray[26];
            int day = recvArray[27];
            int hour = recvArray[28];
            int mini = recvArray[29];
            int second = 0;//recvArray[30];
            //"2019-03-31 12:24:36";
            //QString strDateTime= QString("%1-%2-%3 %4:%5:%6").arg(year).arg(mouth).arg(day).arg(hour).arg(mini).arg(second);
            QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
            QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
            stagePlan->m_timProvReachStation = dateTime;
        }
    }

    if(stagePlan->m_btBeginOrEndFlg == 0xAA	//正常
        ||stagePlan->m_btBeginOrEndFlg == 0xBB	//始发
        ||stagePlan->m_btBeginOrEndFlg == 0xDD) //通过
    {
        //出发股道
        stagePlan->m_nDepartTrainTrack = RecvArrayToInt(&recvArray[31], 2);
        //出发时间
        {
            int year = (recvArray[33] | recvArray[34]<<8);
            int mouth = recvArray[35];
            int day = recvArray[36];
            int hour = recvArray[37];
            int mini = recvArray[38];
            int second = 0;//recvArray[39];
            //"2019-03-31 12:24:36";
            //QString strDateTime= QString("%1-%2-%3 %4:%5:%6").arg(year).arg(mouth).arg(day).arg(hour).arg(mini).arg(second);
            QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
            QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
            stagePlan->m_timProvDepaTrain = dateTime;
        }
        //出发车号
        unsigned char strDepartTrainNum[7] = {0,0,0,0,0,0,0};//6
        memcpy(strDepartTrainNum, &recvArray[16], 6);
        stagePlan->m_strDepartTrainNum = UnsignedCharArrayToString(strDepartTrainNum);;
        //发车口
        stagePlan->m_nCodeDepartStaEquip = RecvArrayToInt(&recvArray[43], 2);
    }
    //增加信息
    stagePlan->m_btStagePlanKind = 0x11;
    this->m_ArrayStagePlan.append(stagePlan);
    //自动签收
    this->SignStagePlan(true);
}
//接收联锁数据-邻站预告
void MyStation::recvLSData_LZYG(unsigned char *recvArray, int recvlen)
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    TrafficLog* pTrafficLog = new TrafficLog();
    //接受车号
    unsigned char strReachTrainNum[7] = {0,0,0,0,0,0,0};//6
    memcpy(strReachTrainNum, &recvArray[10], 6);
    pTrafficLog->m_strReachTrainNum = UnsignedCharArrayToString(strReachTrainNum);
    pTrafficLog->m_strDepartTrainNum = pTrafficLog->m_strReachTrainNum; //发车车次号
    //获取该车在在行车日志表中的行号
    int nIndex = this->GetIndexInTrafficArray(pTrafficLog->m_strReachTrainNum);
    //表中不存在该车次则添加
    if(nIndex < 0)
    {
        this->m_ArrayTrafficLog.append(pTrafficLog);
        nIndex = this->GetIndexInTrafficArray(pTrafficLog->m_strReachTrainNum);
        m_pDataAccess->InsetTrafficLog(pTrafficLog);//CTC是否也获取了本信息？？ 是否重复添加了？？
    }
    //预告相关显示
    TrafficLog *pTrafficLogExist = this->m_ArrayTrafficLog[nIndex];
    pTrafficLogExist->m_timAgrFromAdjtStaDepaTrain = QDateTime::currentDateTime();
    m_pDataAccess->UpdateTrafficLog(pTrafficLogExist);
    //发送同步数据消息
    //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLogExist,SYNC_FLAG_UPDATE,1,1);
}
//接收联锁数据-进路办理回执信息
void MyStation::recvLSData_RouteReturn(unsigned char *recvArray, int recvlen)
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    int count = 10;//协议起始位置
    bool bSucc = false;//办理成功
    if((int)recvArray[count+0] == 0x01)
    {
        bSucc = true;
    }
    else
    {
        bSucc = false;
    }
    int nANCount = (int)recvArray[count+1];//按钮个数
    int nDevCode[5] = {0};//信号机设备号
    QStringList arrAN;
    for(int i=0; i<nANCount && i<5; i++)
    {
        //memcpy(&nDevCode[i], &RecvArray[2+i*2], 2);
        nDevCode[i] = RecvArrayToInt(&recvArray[count+2+i*2], 2);
        arrAN.append(this->GetStrNameByCode(nDevCode[i]));
    }
    qDebug()<<"收到联锁进路办理回执："<<arrAN.join(",");

    bool bFCJL = false;//发车进路
    int ct = this->StaConfigInfo.JCKCount;
    for (int t = 0; t < ct; t++)
    {
        //终端按钮为发车口
        if (nDevCode[nANCount-1] == this->StaConfigInfo.JFCKInfo[t].nFCKCode //正常
            || nDevCode[nANCount-1] == this->StaConfigInfo.JFCKInfo[t].nJCKCode)//改方
        {
            bFCJL = true;
            break;
        }
    }
    //if(!bFCJL)//不是发车进路，则不做提示（发车进路与计划不一致报警）
    //	return;

//    //根据第一个按钮查找GD,再找车次
    QString startXhdName = this->GetStrNameByCode(nDevCode[0]);
    QString endXhdName = this->GetStrNameByCode(nDevCode[nANCount-1]);

    //灯丝断丝报警
    if(this->StaConfigInfo.bXHDDSBJStatus)
    {
        JudgXHDDSstate(startXhdName);
    }

    int gdCode = -1;
    QString strCheci = "";
    for(int i=0; bFCJL && i<this->vectStationGDNode.size(); i++)
    {
        if(startXhdName == this->vectStationGDNode[i].strLeftNode
            || startXhdName == this->vectStationGDNode[i].strRightNode)
        {
            gdCode = this->vectStationGDNode[i].nCode;
            break;
        }
    }
    CGD *pGD = NULL;
    if(gdCode > 0 && gdCode!=0xFFFF)
    {
        int gdPos = this->GetGDPosInzcArray(gdCode);
        if(gdPos > 0)
        {
            pGD = (CGD*)this->DevArray[gdPos-1];
            if(pGD)
            {
                strCheci = pGD->m_strCheCiNum;
            }
        }
    }

    bool bMatch = true;//进路按钮匹配进路序列计划
    QString strDirect = "";
    TrainRouteOrder* pTrainRouteOrder;
    //int nRouteOrderSize = pStation->m_ArrayRouteOrderGrid.count();
    //1.发车进路防错办
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if(pTrainRouteOrder && (3 < pTrainRouteOrder->m_nOldRouteState))//进路已结束或失效，则跳过 //2
        {
            continue;
        }
        if( bFCJL && pTrainRouteOrder
            && 0x01 == pTrainRouteOrder->m_btRecvOrDepart //发车进路
            && strCheci == pTrainRouteOrder->m_strTrainNum)//车次一致
        {
            pTrainRouteOrder->SetState(1);//列车进路序列状态更新
            if( startXhdName == pTrainRouteOrder->m_strXHDBegin//始端按钮
                && endXhdName != pTrainRouteOrder->m_strXHDEnd )//终端按钮
            {
                bMatch = false;
                strDirect = pTrainRouteOrder->m_strDirection;
                strDirect.replace("->","");
                strDirect.replace("<-","");
                break;
            }
        }

    }

    //2.列车进路序列状态更新
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //进路已结束或失效、或未触发，则跳过
        if(pTrainRouteOrder && (2 < pTrainRouteOrder->m_nOldRouteState || 0 == pTrainRouteOrder->m_nOldRouteState))
        {
            continue;
        }
        ////发车进路，且有车次则匹配车次
        //if(bFCJL && (strCheci.GetLength() > 0))
        //{
        //	if(pTrainRouteOrder
        //		&& startXhdName == pTrainRouteOrder->m_strXHDBegin//始端按钮
        //		&& endXhdName == pTrainRouteOrder->m_strXHDEnd //终端按钮
        //		&& strCheci == pTrainRouteOrder->m_strTrainNum)
        //	{
        //		pTrainRouteOrder->SetState(1);
        //		if(bSucc)
        //			pTrainRouteOrder->m_strRouteState = _T("触发完成");
        //		else
        //			pTrainRouteOrder->m_strRouteState = _T("触发失败");
        //		break;
        //	}
        //}
        ////无车次时先到先得
        //else
        {
            if(pTrainRouteOrder
                && startXhdName == pTrainRouteOrder->m_strXHDBegin //始端按钮
                && endXhdName == pTrainRouteOrder->m_strXHDEnd //终端按钮
                && 1 == pTrainRouteOrder->m_nOldRouteState //进路触发
                )
            {
                //pTrainRouteOrder->SetState(2);
                if(bSucc)
                {
                    pTrainRouteOrder->SetState(2);
                    pTrainRouteOrder->m_strRouteState = "触发完成";
                    pTrainRouteOrder->m_bSuccessed = 1;
                    UpdateTrafficlogJLSuccessed(bFCJL, pTrainRouteOrder->m_strTrainNum, true);
                }
                else
                {
                    pTrainRouteOrder->SetState(1);//还要再次尝试触发
                    //pTrainRouteOrder->m_strRouteState = "触发失败";
                    pTrainRouteOrder->m_bSuccessed = 2;
                    UpdateTrafficlogJLSuccessed(bFCJL, pTrainRouteOrder->m_strTrainNum, false);
                }
                //更新数据库
                m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                break;
            }
        }
    }
}
// 签收阶段计划
void MyStation::SignStagePlan(bool bSign, bool bTrain)
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    int autoSign = this->AutoSignStage;
    int count = this->m_ArrayStagePlan.count();
    StagePlan *pStagePlan;
    bool bUpdate = false;
    for(int i = 0; i < count && (autoSign>0 || bSign); i++)
    {
        pStagePlan = this->m_ArrayStagePlan[i];
        //未签收
        if(pStagePlan->m_nStateSignPlan == 0)
        {
            //通知阶段计划表签收状态已发生改变，行车日志表开始更新
            this->StagePlanDataToTrafficLog(pStagePlan);
            //StagePlanDataToTrafficLog(pStation, pStagePlan);
            //更新阶段计划数组类签收状态
            pStagePlan->m_nStateSignPlan = 1;

            //按图排路，才会生成进路序列
            if(this->ModalSelect.nStateSelect == 1)
            {
                //0-人工签收后自动同步进路信息
                if(ManSignRouteSynType == 0 && !bTrain)
                {
                    //通知阶段计划表签收状态已发生改变，列车进路表开始更新
                    this->StagePlanToRouteOrder(pStagePlan);
                    //StagePlanToRouteOrder(pStation, pStagePlan);
                    //更新阶段计划数组类签收状态
                    pStagePlan->m_nStateSignPlan = 2;
                }
            }

            //数据库访问
            m_pDataAccess->UpdateStagePlanDetail(pStagePlan);
            bUpdate = true;
            //人工签收
            if(autoSign<=0 && bSign)
            {
                //处理和发送1个数据
                this->sendOneStagePlanToSoft(DATATYPE_ALL,pStagePlan,SYNC_FLAG_UPDATE,1,1);
            }
            else
            {
                //处理和发送1个数据
                this->sendOneStagePlanToSoft(DATATYPE_ALL,pStagePlan,SYNC_FLAG_ADD,1,1);
            }
        }
    }
    if(bUpdate)
    {
        //发送同步数据消息
        //sendUpdateDataMsg(this, UPDATETYPE_ALL);
        //已使用同步通道发送单条数据
    }
}

//接收车次操作命令，联锁车次时返回true，否则返回false
bool MyStation::recvCheciCmd(unsigned char *recvArray, int recvlen)
{
    int nCount=10;
    //车次长度
    int lenCC = recvArray[11];
    //车次
    QString strCC;
    unsigned char strRecvDDCC[20];
    memset(strRecvDDCC,0,sizeof(strRecvDDCC));
    memcpy(strRecvDDCC, &recvArray[12], lenCC);
    strCC = UnsignedCharArrayToString(strRecvDDCC);

    //添加车次
    if(0x01 == (int)recvArray[10])
    {
        Train* pTrain = new Train;
        pTrain->m_strCheCi = strCC;
        nCount = 11+1;
        nCount += lenCC;
        //所在股道信号设备号//显示位置
        pTrain->m_nPosCode = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
        pTrain->m_nPosCodeReal = pTrain->m_nPosCode;
        nCount += 2;
        //车次类型 0x02列车 0x03调车
        pTrain->m_nType = recvArray[nCount];
        nCount += 1;
        //方向
        if(0x5A == (int)recvArray[nCount])
        {
            pTrain->m_bRight = true;
        }
        else
        {
            pTrain->m_bRight = false;
        }
        nCount += 1;
        //车速，实际值
        pTrain->m_nSpeed = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
        nCount += 2;
        //电力机车
        if(0x11 == (int)recvArray[nCount])
        {
            pTrain->m_bElectric = true;
        }
        else
        {
            pTrain->m_bElectric = false;
        }
        nCount += 1;
        //超限等级
        pTrain->m_nLevelCX = (int)recvArray[nCount];
        nCount += 1;
        //辆数（编组长度）
        pTrain->m_nLengh = (int)recvArray[nCount];
        nCount += 1;
        //列车类型 客车、货车
        if((int)recvArray[nCount] == 0x01)
        {
            pTrain->m_nKHType = LCTYPE_KC;
        }
        else
        {
            pTrain->m_nKHType = LCTYPE_HC;
        }
        nCount += 1;
        //根据输入车次判断列车的客货类型
        if(IsCheciKC(pTrain->m_strCheCi))
        {
            pTrain->m_nKHType = LCTYPE_KC;
        }
        else
        {
            pTrain->m_nKHType = LCTYPE_HC;
        }

        //是否模拟行车（是则向联锁加车）
        if((int)recvArray[nCount] == 0x01)
        {
            pTrain->m_bManType = false;//非人工标记车次
        }
        else
        {
            pTrain->m_bManType = true;//人工标记车次
        }
        nCount += 1;

        if(!pTrain->m_bManType)
        {
            return true;//
        }
        else
        {
            m_ArrayTrainManType.append(pTrain);
            //逻辑会定时发送人工车次信息给CTC
            return false;
        }
    }
    //删除车次
    else if(0x02 == (int)recvArray[10])
    {
        //从人工标注的车次中查找
        for(int i=0; i<m_ArrayTrainManType.size(); i++)
        {
            if(strCC == m_ArrayTrainManType[i]->m_strCheCi)
            {
                m_ArrayTrainManType.removeAt(i);
                return false;
            }
        }
        if(DispatchCenterTrainModify)
        {
            return true;
        }
    }
    //修改车次
    else if(0x03 == (int)recvArray[10])
    {
        nCount = 11+1;
        nCount += lenCC;
        //所在股道信号设备号
        int posCode = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
        nCount += 2;
        //新车次号长度
        int lenCCNew = (int)(recvArray[nCount]);
        nCount += 1;
        //新车次号
        QString strCCNew;
        unsigned char strRecvCC[20];
        memset(strRecvCC,0,sizeof(strRecvCC));
        memcpy(strRecvCC, &recvArray[nCount], lenCCNew);
        strCCNew = UnsignedCharArrayToString(strRecvCC);

        //从人工标注的车次中查找
        for(int i=0; i<m_ArrayTrainManType.size(); i++)
        {
            if(strCC == m_ArrayTrainManType[i]->m_strCheCi)
            {
                m_ArrayTrainManType[i]->m_strCheCi = strCCNew;
                //根据输入车次判断列车的客货类型
                if(IsCheciKC(m_ArrayTrainManType[i]->m_strCheCi))
                {
                    m_ArrayTrainManType[i]->m_nKHType = LCTYPE_KC;
                }
                else
                {
                   m_ArrayTrainManType[i]->m_nKHType = LCTYPE_HC;
                }
                return false;
            }
        }
        if(DispatchCenterTrainModify)
        {
            return true;
        }
    }
    //修改车次属性
    else if(0x04 == (int)recvArray[10])
    {
        nCount = 11+1;
        nCount += lenCC;
        //所在股道信号设备号
        int posCode = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
        nCount += 2;
        //速度
        int speed = (int)(recvArray[nCount] | recvArray[nCount+1]<<8);
        nCount += 2;
        //电力机车
        bool bElectric = (int)(recvArray[nCount])==0x11?true:false;
       // qDebug()<<"属性"<<(int)(recvArray[nCount]);
        //从人工标注的车次中查找
        for(int i=0; i<m_ArrayTrainManType.size(); i++)
        {
            if(strCC == m_ArrayTrainManType[i]->m_strCheCi)
            {
                m_ArrayTrainManType[i]->m_nSpeed = speed;
                m_ArrayTrainManType[i]->m_bElectric = bElectric;
               // qDebug()<<"属性"<<bElectric;
                return false;
            }
        }
       // qDebug()<<"属性";
        if(DispatchCenterTrainModify)
        {
            return true;
        }
    }
    //车次发车
    else if(0x05 == (int)recvArray[10])
    {
        //从人工标注的车次中查找
        for(int i=0; i<m_ArrayTrainManType.size(); i++)
        {
            if(strCC == m_ArrayTrainManType[i]->m_strCheCi)
            {
                m_ArrayTrainManType[i]->m_bStop = false;
                return false;
            }
        }
        return true;//发联锁命令
    }
    //车次停车
    else if(0x06 == (int)recvArray[10])
    {
        this->SetCheCiStop(strCC, true);
        return true;//发联锁命令
    }

    return false;
}
//合并车次（CTC车次和联锁车次）
void MyStation::mergeCheci()
{
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        Train *pTrain = m_ArrayTrain[i];
        for(int j=0; j<m_ArrayTrainManType.size(); j++)
        {
            Train *pTrainMan = m_ArrayTrainManType[j];
            //实际车次行驶到标注车次后顶掉标注车次（即合并车次）
            if(pTrain->m_strCheCi==pTrainMan->m_strCheCi
               && pTrain->m_nPosCodeReal==pTrainMan->m_nPosCode)
            {
                m_ArrayTrainManType.removeAt(j);
                break;
            }
        }
    }
}
//打包和向CTC发送车次
void MyStation::packAndSendCheci(Train *pTrain)
{
    QByteArray dataArray;
    int nCount = 10;
    //添加nCount个字节并全部置零
    dataArray.append(nCount, char(0));
    //标志位-车次信息
    dataArray[nCount-1] = 0x66;
    //车次号
    QByteArray byteArray = pTrain->m_strCheCi.toLocal8Bit();
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
    //所在区段设备号
    nCount+=2;
    dataArray.append(2, char(0));
    dataArray[nCount-2] = pTrain->m_nPosCode&0xFF;
    //dataArray[nCount-1] = (pTrain->m_nPosCode<<8)&0xFF;
     dataArray[nCount-1] = (pTrain->m_nPosCode>>8)&0xFF;
    //所在实时区段设备号
    nCount+=2;
    dataArray.append(2, char(0));
    dataArray[nCount-2] = pTrain->m_nPosCodeReal&0xFF;
    //dataArray[nCount-1] = (pTrain->m_nPosCodeReal<<8)&0xFF;
    dataArray[nCount-1] = (pTrain->m_nPosCodeReal>>8)&0xFF;
    //方向
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_bRight?0x22:0x11;//无方向
    if(pTrain->m_bManType)
    {
        dataArray[nCount-1] = 0x00;//无方向
    }
    //列车是否丢失
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_bLost?0x01:0x00;
    //车次类型
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_nType;
    //列车类型（客车/货车）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_nKHType==LCTYPE_KC?0x01:0x00;
    //运行状态
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_bRunning?0x01:0x00;//0x00;//停车状态
    //电力机车
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_bElectric?0x11:0x00;
    //超限
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_nLevelCX;
    //编组长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_nLengh;
    //速度（预设）
    nCount+=2;
    dataArray.append(2, char(0));
    dataArray[nCount-2] = pTrain->m_nSpeed&0xFF;
    dataArray[nCount-1] = (pTrain->m_nSpeed<<8)&0xFF;
    //列车类型（管内动车组、通勤列车等）
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_nIndexLCLX;
    //列车停稳
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_bStop?0x01:0x00;
    //人工标记车次
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = pTrain->m_bManType?0x01:0x00;
    //实际车次，包含早晚点
    if(!pTrain->m_bManType)
    {
        nCount+=2;
        dataArray.append(2, char(0));
        int overtime = pTrain->m_nOvertime;
        if(overtime < 0)
        {
            overtime = 0 - overtime;
        }
        dataArray[nCount-2] = overtime&0xFF;
        dataArray[nCount-1] = ((overtime>>8)&0x7F) | (pTrain->m_nOvertime<0?0x80:0x00);//0111 1111 | 1000 000
        //qDebug()<<"send pTrain->m_nOvertime="<<pTrain->m_nOvertime;
    }

    //帧尾4+空白4
    nCount+=8;
    dataArray.append(8, char(0));
    //打包数据
    packHeadAndTail(&dataArray, nCount);
    //qDebug()<<"Checi dataArray="<<ByteArrayToString(dataArray);
    //信号-发送数据给CTC
    emit sendDataToCTCSignal(this, dataArray, nCount);
    //信号-发送数据给占线板
    emit sendDataToBoardSignal(this, dataArray, nCount);
    //信号-发送数据给集控台
    emit sendDataToJKSignal(this, dataArray, nCount);
    //信号-发送数据给占线图
    emit sendDataToZXTSignal(this, dataArray, nCount);
}
//向CTC发送人工标注的车次
void MyStation::sendCheciToCTC()
{
    //实际的车次
    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        Train *pTrain = m_ArrayTrain[i];
        packAndSendCheci(pTrain);
    }
    //人工标注的车次
    for(int i=0; i<m_ArrayTrainManType.size(); i++)
    {
        Train *pTrain = m_ArrayTrainManType[i];
        packAndSendCheci(pTrain);
    }
}
// 解释签收的阶段计划内容转化到进路指令类数组
void MyStation::StagePlanToRouteOrder(StagePlan *pStagePlan)
{
    TrainRouteOrder* pRecvTrainRouteOrder = new TrainRouteOrder;
    TrainRouteOrder* pDepartTrainRouteOrder = new TrainRouteOrder;
//    //非常站控模式下，阶段计划不自动触发
//    if(this->m_nFCZKMode)
//    {
//        pRecvTrainRouteOrder->m_nAutoTouch = false;
//        pDepartTrainRouteOrder->m_nAutoTouch = false;
//    }
//    else
//    {
//        //中心和车站调车模式下为自动触发
//        if(this->ModalSelect.nModeState != 1)
//        {
//            pRecvTrainRouteOrder->m_nAutoTouch = true;
//            pDepartTrainRouteOrder->m_nAutoTouch = true;
//        }
//        //车站控制模式下为人工触发，
//        else if(this->ModalSelect.nModeState == 1)
//        {
//            pRecvTrainRouteOrder->m_nAutoTouch = false;
//            pDepartTrainRouteOrder->m_nAutoTouch = false;
//        }
//    }

    //当签收阶段计划后到列车进路指令的解析
    switch(pStagePlan->m_btStagePlanKind)
    {
    case 0x11:
        {
            bool routeWaite = true;
            //接车进路判断
            if(pStagePlan->m_btBeginOrEndFlg != JFC_TYPE_SF)
            {
                int routIndex = this->GetTrainRouteIndex(pStagePlan->m_strReachTrainNum, ROUTE_JC);
                if(-1 < routIndex)
                {
                    TrainRouteOrder* pTrainRouteExist = (TrainRouteOrder*)this->m_ArrayRouteOrder[routIndex];
                    //只更新等待状态的进路序列
                    if(pTrainRouteExist->m_nOldRouteState == 0)
                    {
                        routeWaite = true;
                        //中心模式和调车模式下为自动触发未确认状态，则需要更新；已确认的只更新时间
                        this->UpdatTrainRouteOrderByStagePlan(pTrainRouteExist, pStagePlan, ROUTE_JC, pTrainRouteExist->m_bConfirmed);
                        //更新数据库
                        m_pDataAccess->UpdateRouteOrder(pTrainRouteExist);
                        //m_pDataAccess->SelectAllRouteOrder(this);
                        //处理和发送1个数据
                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteExist,SYNC_FLAG_UPDATE,1,1);
                        //组合进路判断并更新
                        if(pTrainRouteExist->m_bZHJL)
                        {
                            this->UpdateRouteOrderOfZHJL(pTrainRouteExist, false);
                        }
                        //重新读取数据库（组合进路时将读取子序列）
                        m_pDataAccess->SelectAllRouteOrder(this);
                    }
                    else
                    {
                        routeWaite = false;
                    }
                }
                else
                {
                    //更新内容并增加
                    this->UpdatTrainRouteOrderByStagePlan(pRecvTrainRouteOrder, pStagePlan, ROUTE_JC);
                    //插入数据库
                    m_pDataAccess->InsetRouteOrder(pRecvTrainRouteOrder);
                    //处理和发送1个数据
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRecvTrainRouteOrder,SYNC_FLAG_ADD,1,1);
                    //组合进路判断并更新
                    if(pRecvTrainRouteOrder->m_bZHJL)
                    {
                        this->UpdateRouteOrderOfZHJL(pRecvTrainRouteOrder, false);
//                        for(int r=0; r<this->m_ArrayRouteOrderSonTemp.size(); r++)
//                        {
//                            TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrderSonTemp[r];
//                            //设置父id
//                            pRouteOrderSon->father_id = pRecvTrainRouteOrder->route_id;
//                            pRouteOrderSon->m_nAutoTouch = pRecvTrainRouteOrder->m_nAutoTouch;
//                            //插入数据库
//                            m_pDataAccess->InsetRouteOrder(pRouteOrderSon);
//                            //处理和发送1个数据
//                            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrderSon,SYNC_FLAG_ADD,1,1);
//                        }
//                        this->m_ArrayRouteOrderSonTemp.clear();
                    }
                    //重新读取数据库（组合进路时将读取子序列）
                    m_pDataAccess->SelectAllRouteOrder(this);
                }
            }
            else
            {
                //既有车次计划变更为始发，则查找并删除接车进路
                int routIndex = this->GetTrainRouteIndex(pStagePlan->m_strDepartTrainNum, ROUTE_JC);
                if(-1 < routIndex)
                {
                    TrainRouteOrder* pTrainRouteOrder = this->m_ArrayRouteOrder[routIndex];
                    //删除数据库
                    m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
                    //m_pDataAccess->SelectAllRouteOrder(this);
                    //处理和发送1个数据
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRecvTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
                    //this->m_ArrayRouteOrder.removeAt(routIndex);
                    //组合进路判断并更新
                    if(pTrainRouteOrder->m_bZHJL)
                    {
                        //删除之前的子进路
                        this->UpdateRouteOrderOfZHJL(pTrainRouteOrder, true);
//                        for(int r=0; r<this->m_ArrayRouteOrder.size(); r++)
//                        {
//                            TrainRouteOrder* pRouteOrder1 = this->m_ArrayRouteOrder[r];
//                            if(pTrainRouteOrder->route_id == pRouteOrder1->father_id)
//                            {
//                                //处理和发送1个数据
//                                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrder1,SYNC_FLAG_DELETE,1,1);
//                                //删除数据库
//                                m_pDataAccess->DeleteRouteOrder(pRouteOrder1);
//                            }
//                        }
                    }
                    //重新读取数据库（组合进路时将读取子序列）
                    m_pDataAccess->SelectAllRouteOrder(this);
                }
            }

            if(pStagePlan->bXianLuSuo)
            {
                //线路所只增加一条接车进路
                break;
            }

            //发车进路判断
            if(pStagePlan->m_btBeginOrEndFlg != JFC_TYPE_ZD)
            {
                int routIndex = this->GetTrainRouteIndex(pStagePlan->m_strDepartTrainNum, ROUTE_FC);
                if(-1 < routIndex)
                {
                    TrainRouteOrder* pTrainRouteExist = (TrainRouteOrder*)this->m_ArrayRouteOrder[routIndex];
                    //只更新等待状态的进路序列   且 接车等待状态
                    if(pTrainRouteExist->m_nOldRouteState == 0 && routeWaite)
                    {
                        //更新内容；已确认的只更新时间
                        this->UpdatTrainRouteOrderByStagePlan(pTrainRouteExist, pStagePlan, ROUTE_FC, pTrainRouteExist->m_bConfirmed);
                        //检查发车进路相应的接车进路是否是延续进路
                        CheckJCRouteIsYXJL(pTrainRouteExist);
                        //更新数据库
                        m_pDataAccess->UpdateRouteOrder(pTrainRouteExist);
                        //m_pDataAccess->SelectAllRouteOrder(this);
                        //处理和发送1个数据
                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteExist,SYNC_FLAG_UPDATE,1,1);
                        //组合进路判断并更新
                        if(pTrainRouteExist->m_bZHJL)
                        {
                            this->UpdateRouteOrderOfZHJL(pTrainRouteExist, false);
                        }
                        //重新读取数据库（组合进路时将读取子序列）
                        m_pDataAccess->SelectAllRouteOrder(this);
                    }
                }
                else
                {
                    //更新内容并增加
                    this->UpdatTrainRouteOrderByStagePlan(pDepartTrainRouteOrder, pStagePlan, ROUTE_FC);
                    //检查发车进路相应的接车进路是否是延续进路
                    CheckJCRouteIsYXJL(pDepartTrainRouteOrder);
                    //接车进路检查不通过，则发车进路也不能自动设置自触
                    if(pRecvTrainRouteOrder->checkResultInterlock>0 && pStagePlan->m_btBeginOrEndFlg != JFC_TYPE_SF)
                    {
                        pDepartTrainRouteOrder->m_nAutoTouch = false;
                    }
                    //插入数据库
                    m_pDataAccess->InsetRouteOrder(pDepartTrainRouteOrder);
                    //处理和发送1个数据
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pDepartTrainRouteOrder,SYNC_FLAG_ADD,1,1);
                    //组合进路判断并更新
                    if(pDepartTrainRouteOrder->m_bZHJL)
                    {
                        this->UpdateRouteOrderOfZHJL(pDepartTrainRouteOrder, false);
//                        for(int r=0; r<this->m_ArrayRouteOrderSonTemp.size(); r++)
//                        {
//                            TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrderSonTemp[r];
//                            //设置父id
//                            pRouteOrderSon->father_id = pDepartTrainRouteOrder->route_id;
//                            pRouteOrderSon->m_nAutoTouch = pDepartTrainRouteOrder->m_nAutoTouch;
//                            //插入数据库
//                            m_pDataAccess->InsetRouteOrder(pRouteOrderSon);
//                            //处理和发送1个数据
//                            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrderSon,SYNC_FLAG_ADD,1,1);
//                        }
//                        this->m_ArrayRouteOrderSonTemp.clear();
                    }
                    m_pDataAccess->SelectAllRouteOrder(this);
                }
            }
            else
            {
                //既有车次计划变更为终到，则查找并删除发车进路
                int routIndex = this->GetTrainRouteIndex(pStagePlan->m_strReachTrainNum, ROUTE_FC);
                if(-1 < routIndex)
                {
                    TrainRouteOrder* pTrainRouteOrder = this->m_ArrayRouteOrder[routIndex];
                    //删除数据库
                    m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
                    //m_pDataAccess->SelectAllRouteOrder(this);
                    //处理和发送1个数据
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRecvTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
                    //this->m_ArrayRouteOrder.removeAt(routIndex);
                    //组合进路判断并更新
                    if(pTrainRouteOrder->m_bZHJL)
                    {
                        //删除之前的子进路
                        this->UpdateRouteOrderOfZHJL(pTrainRouteOrder, true);
//                        for(int r=0; r<this->m_ArrayRouteOrder.size(); r++)
//                        {
//                            TrainRouteOrder* pRouteOrder1 = this->m_ArrayRouteOrder[r];
//                            if(pTrainRouteOrder->route_id == pRouteOrder1->father_id)
//                            {
//                                //处理和发送1个数据
//                                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrder1,SYNC_FLAG_DELETE,1,1);
//                                //删除数据库
//                                m_pDataAccess->DeleteRouteOrder(pRouteOrder1);
//                            }
//                        }
                    }
                    //重新读取数据库（组合进路时将读取子序列）
                    m_pDataAccess->SelectAllRouteOrder(this);
                }
            }
        }
        break;
    case 0x22:
        //AfxMessageBox(_T("删除！"));
        //在行车日志解析时删除
        break;
    default:
        break;
    }
//    qDebug() << "StagePlanToRouteOrder" << m_ArrayRouteOrder.size();
//    for(int i = 0; i < m_ArrayRouteOrder.size(); i++)
//    {
//        qDebug() << m_ArrayRouteOrder[i]->m_btRecvOrDepart << m_ArrayRouteOrder[i]->m_btBeginOrEndFlg;
    //    }
}
// 更新或删除组合进路
void MyStation::UpdateRouteOrderOfZHJL(TrainRouteOrder *pTrainRouteOrderFather, bool bDelete)
{
    //删除之前的子进路
    for(int r=0; r<this->m_ArrayRouteOrder.size(); r++)
    {
        qDebug()<<"UpdateRouteOrderOfZHJL() this->m_ArrayRouteOrder.size()="<<this->m_ArrayRouteOrder.size();
        TrainRouteOrder* pRouteOrder1 = this->m_ArrayRouteOrder[r];
        if(pTrainRouteOrderFather->route_id == pRouteOrder1->father_id)
        {
            //处理和发送1个数据
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrder1,SYNC_FLAG_DELETE,1,1);
            //删除数据库
            m_pDataAccess->DeleteRouteOrder(pRouteOrder1);
            //移除
            this->m_ArrayRouteOrder.removeOne(pRouteOrder1);
            r--;
        }
    }
    //新增更新的进路
    for(int r=0; r<this->m_ArrayRouteOrderSonTemp.size() && !bDelete && pTrainRouteOrderFather->m_bZHJL; r++)
    {
        TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrderSonTemp[r];
        //设置父id
        pRouteOrderSon->father_id = pTrainRouteOrderFather->route_id;
        pRouteOrderSon->m_nAutoTouch = pTrainRouteOrderFather->m_nAutoTouch;
        //插入数据库
        m_pDataAccess->InsetRouteOrder(pRouteOrderSon);
        //处理和发送1个数据
        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrderSon,SYNC_FLAG_ADD,1,1);
        //新增
        this->m_ArrayRouteOrder.append(pRouteOrderSon);
    }
    this->m_ArrayRouteOrderSonTemp.clear();
}
//阶段计划同步解析至行车日志
void MyStation::StagePlanDataToTrafficLog(StagePlan *pStagePlan)
{
    if(pStagePlan->m_nStateSignPlan > 0)
    {
        return;//已签收，则退出
    }
    switch(pStagePlan->m_btStagePlanKind)
    {
    case 0x11:
        {
            bool routeWaite = true;
            int logIndex = -1;
            bool bConfirmed = false;
            if((pStagePlan->m_strReachTrainNum != ""))
            {
                logIndex = this->GetIndexInTrafficArray(pStagePlan->m_strReachTrainNum);
            }
            if(logIndex <= -1)
            {
                logIndex = this->GetIndexInTrafficArray(pStagePlan->m_strDepartTrainNum);
            }

            //判断进路是否已确认
            int routIndex = this->GetTrainRouteIndex(pStagePlan->m_strReachTrainNum, ROUTE_JC);
            if(-1 < routIndex)
            {
                TrainRouteOrder* pTrainRouteExist = (TrainRouteOrder*)this->m_ArrayRouteOrder[routIndex];
                bConfirmed = pTrainRouteExist->m_bConfirmed;
                if(pTrainRouteExist->m_nOldRouteState == 0)
                {
                    routeWaite = true;
                }
                else
                {
                    routeWaite = false;
                }
            }
            //接车进路没确认则判断发车进路是否已确认
            if(!bConfirmed)
            {
                routIndex = this->GetTrainRouteIndex(pStagePlan->m_strDepartTrainNum, ROUTE_FC);
                if(-1 < routIndex)
                {
                    TrainRouteOrder* pTrainRouteExist = (TrainRouteOrder*)this->m_ArrayRouteOrder[routIndex];
                    bConfirmed = pTrainRouteExist->m_bConfirmed;
                    if(routeWaite)
                    {
                        if(pTrainRouteExist->m_nOldRouteState == 0)
                        {
                            routeWaite = true;
                        }
                        else
                        {
                            routeWaite = false;
                        }
                    }
                }
            }

            if(-1 < logIndex)
            {
                //进路等待状态的才更新
                if(routeWaite)
                {
                    TrafficLog *pTrafficLogExist = (TrafficLog *)this->m_ArrayTrafficLog[logIndex];
                    //仅更新内容
                    UpdatTrafficLogByStagePlan(pTrafficLogExist, pStagePlan, bConfirmed);
                }
            }
            else
            {
                TrafficLog* pTrafficLog = new TrafficLog();
                //更新内容并增加
                UpdatTrafficLogByStagePlan(pTrafficLog, pStagePlan);
                this->m_ArrayTrafficLog.append(pTrafficLog);
            }
        }
        break;
    case 0x22:
        {
            //AfxMessageBox(_T("删除！"));
            int logIndex = -1;
            if((pStagePlan->m_strReachTrainNum != ""))
            {
                logIndex = this->GetIndexInTrafficArray(pStagePlan->m_strReachTrainNum);
            }
            if(logIndex <= -1)
            {
                logIndex = this->GetIndexInTrafficArray(pStagePlan->m_strDepartTrainNum);
            }
            if(logIndex > -1)
            {
                TrafficLog* pTrafficLog = (TrafficLog *)this->m_ArrayTrafficLog[logIndex];
                DeleteTrainRouteByTrafficlog(pTrafficLog);
                //计划没执行，则可以删除
                if(pTrafficLog->m_timRealReachStation.date().year() == 0 && pTrafficLog->m_timRealDepaTrain.date().year() == 0 )
                {
                    m_pDataAccess->DeleteTrafficLog(pTrafficLog);
                    //处理和发送1个数据
                    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_DELETE,1,1);
                    this->m_ArrayTrafficLog.removeAt(logIndex);
                }
            }
        }
        break;
    case 0x33:
        break;
    default:
        break;
    }
}

//根据阶段计划信息更新行车日志对象
void MyStation::UpdatTrafficLogByStagePlan(TrafficLog *_pTrafficLog, StagePlan *_StagePlan, bool _bConfirmed)
{
    _pTrafficLog->station_id = this->getStationID();
    _pTrafficLog->m_strStaName = this->getStationName();
    _pTrafficLog->m_nPlanNumber = _StagePlan->m_nPlanNumber;
    if(_bConfirmed)
    {
        _pTrafficLog->m_timProvReachStation = _StagePlan->m_timProvReachStation;
        _pTrafficLog->m_timProvDepaTrain = _StagePlan->m_timProvDepaTrain;

        m_pDataAccess->UpdateTrafficLog(_pTrafficLog);
        //处理和发送1个数据
        this->sendOneTrafficLogToSoft(DATATYPE_ALL,_pTrafficLog,SYNC_FLAG_UPDATE,1,1);
    }
    else
    {
        //修改计划，找到已存在的该计划
        if(_StagePlan->m_bModified)
        {
            int logIndex = -1;
            if((_StagePlan->m_strReachTrainNum != ""))
            {
                logIndex = this->GetIndexInTrafficArray(_StagePlan->m_strReachTrainNum);
            }
            if(logIndex <= -1)
            {
                logIndex = this->GetIndexInTrafficArray(_StagePlan->m_strDepartTrainNum);
            }
            if(logIndex > -1)
            {
                TrafficLog *pTrafficLogExist = (TrafficLog *)this->m_ArrayTrafficLog[logIndex];
                //图定时间
                _pTrafficLog->m_timChartReachStation = pTrafficLogExist->m_timChartReachStation;
                _pTrafficLog->m_timChartDepaTrain = pTrafficLogExist->m_timChartDepaTrain;
            }
            else
            {
                //图定时间
                _pTrafficLog->m_timChartReachStation = _StagePlan->m_timProvReachStation;
                _pTrafficLog->m_timChartDepaTrain = _StagePlan->m_timProvDepaTrain;
            }
            _StagePlan->m_bModified = false;
        }
        //新计划
        else
        {
            //图定时间
            _pTrafficLog->m_timChartReachStation = _StagePlan->m_timProvReachStation;
            _pTrafficLog->m_timChartDepaTrain = _StagePlan->m_timProvDepaTrain;
        }

        _pTrafficLog->bXianLuSuo = _StagePlan->bXianLuSuo;
        _pTrafficLog->m_btBeginOrEndFlg = _StagePlan->m_btBeginOrEndFlg;
        //添加到达信息
        if(_StagePlan->m_btBeginOrEndFlg != JFC_TYPE_SF)
        {
            _pTrafficLog->m_nCodeReachStaEquip = _StagePlan->m_nCodeReachStaEquip;//new
            _pTrafficLog->m_strXHD_JZk = this->GetStrNameByCode(_pTrafficLog->m_nCodeReachStaEquip);
            _pTrafficLog->m_strReachTrainNum = _StagePlan->m_strReachTrainNum;
            _pTrafficLog->m_bReachTrainNumSX = this->GetSXByCode(_StagePlan->m_nCodeReachStaEquip, 0);//NEW
            _pTrafficLog->m_strRecvTrainTrack = this->GetStrNameByCode(_StagePlan->m_nRecvTrainTrack);
            if(_pTrafficLog->bXianLuSuo)
            {
                _pTrafficLog->m_strRecvTrainTrack = this->GetStrNameByCode(_StagePlan->m_nCodeReachStaEquip) + "-" + this->GetStrNameByCode(_StagePlan->m_nCodeDepartStaEquip);
            }
            //if(_StagePlan->m_btBeginOrEndFlg == JFC_TYPE_SF)//始发（终到）标志位  0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
            //{
            //	_pTrafficLog->m_strRecvTrainTrack = _T("");
            //}
            _pTrafficLog->m_timProvReachStation = _StagePlan->m_timProvReachStation;
            _pTrafficLog->m_strFromAdjtStation = this->GetJFCKDirectByCode(_StagePlan->m_nCodeReachStaEquip);
        }
        //添加出发信息
        if(_StagePlan->m_btBeginOrEndFlg != JFC_TYPE_ZD)
        {
            _pTrafficLog->m_nCodeDepartStaEquip = _StagePlan->m_nCodeDepartStaEquip;
            _pTrafficLog->m_strXHD_CZk = this->GetStrNameByCode(_pTrafficLog->m_nCodeDepartStaEquip);
            _pTrafficLog->m_strDepartTrainNum = _StagePlan->m_strDepartTrainNum;
            _pTrafficLog->m_bDepartTrainNumSX = this->GetSXByCode(_StagePlan->m_nCodeDepartStaEquip, 1);//NEW
            _pTrafficLog->m_strDepartTrainTrack = this->GetStrNameByCode(_StagePlan->m_nDepartTrainTrack);
            if(_pTrafficLog->bXianLuSuo)
            {
                _pTrafficLog->m_strDepartTrainTrack = this->GetStrNameByCode(_StagePlan->m_nCodeReachStaEquip) + "-" + this->GetStrNameByCode(_StagePlan->m_nCodeDepartStaEquip);
            }
            //if(_StagePlan->m_btBeginOrEndFlg == JFC_TYPE_ZD)//始发（终到）标志位  0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
            //{
            //	_pTrafficLog->m_strDepartTrainTrack = _T("");
            //}
            _pTrafficLog->m_timProvDepaTrain = _StagePlan->m_timProvDepaTrain;
            _pTrafficLog->m_strToAdjtStation = this->GetJFCKDirectByCode(_StagePlan->m_nCodeDepartStaEquip);
        }

        //其他信息
        if(_pTrafficLog->m_btBeginOrEndFlg == JFC_TYPE_SF)//0xBB
        {
            _pTrafficLog->m_strTypeFlag = "始发";
        }
        else if(_pTrafficLog->m_btBeginOrEndFlg == JFC_TYPE_ZD)//0xCC
        {
            _pTrafficLog->m_strTypeFlag = "终到";
        }
        else if(_pTrafficLog->m_btBeginOrEndFlg == JFC_TYPE_TG)//0xDD
        {
            _pTrafficLog->m_strTypeFlag = "通过";
        }
        if(_pTrafficLog->bXianLuSuo)
        {
            _pTrafficLog->m_strTypeFlag = "通过";
        }
        _pTrafficLog->m_strTrainNum = _pTrafficLog->m_strReachTrainNum != "" ? _pTrafficLog->m_strReachTrainNum : _pTrafficLog->m_strDepartTrainNum;//new
        _pTrafficLog->m_strTrainTrack = _pTrafficLog->m_strRecvTrainTrack != "" ? _pTrafficLog->m_strRecvTrainTrack : _pTrafficLog->m_strDepartTrainTrack;//new
        _pTrafficLog->m_bElectric = _StagePlan->m_bElectric;
        _pTrafficLog->m_nLevelCX = _StagePlan->m_nLevelCX;
        _pTrafficLog->m_nLHFlg = _StagePlan->m_nLHFlg;
        _pTrafficLog->m_nIndexLCLX = _StagePlan->m_nIndexLCLX;
        _pTrafficLog->m_nIndexYXLX = _StagePlan->m_nIndexYXLX;
        _pTrafficLog->m_strLCLX = this->GetTrainType(_StagePlan->m_nIndexLCLX);//列车类型
        _pTrafficLog->m_strYXLX = this->GetTrainRunType(_StagePlan->m_nIndexYXLX);//运行类型
        _pTrafficLog->m_strDDCX = this->GetChaoXianLevel(_StagePlan->m_nLevelCX);
        _pTrafficLog->m_strCFCX = _pTrafficLog->m_strDDCX;
        //_pTrafficLog->m_strLCLX = pDoc->GetTrainType(_pTrafficLog->m_nIndexLCLX);//列车类型
        //_pTrafficLog->m_strYXLX = pDoc->GetTrainRunType(_pTrafficLog->m_nIndexYXLX);//运行类型

        _pTrafficLog->m_strReachTrainNumOld = _pTrafficLog->m_strReachTrainNum;
        _pTrafficLog->m_strDepartTrainNumOld = _pTrafficLog->m_strDepartTrainNum;

        SetTrafficLogProc(_pTrafficLog);


        //数据库暂时没合并
        //_pTrafficLog->CheckPlan();

        //插入数据库
        if(m_pDataAccess)
        {
            //既有计划判断
            bool TrainNumFlag = false;
            TrafficLog* pTrafficLog;
            for(int n=0;n<this->m_ArrayTrafficLog.count();n++)
            {
                pTrafficLog = (TrafficLog *)this->m_ArrayTrafficLog[n];
                if((((_pTrafficLog->m_btBeginOrEndFlg == 0xAA)||(_pTrafficLog->m_btBeginOrEndFlg == 0xDD))&&((pTrafficLog->m_strReachTrainNum == _pTrafficLog->m_strReachTrainNum)
                    ||(pTrafficLog->m_strReachTrainNum == _pTrafficLog->m_strDepartTrainNum)
                    ||(pTrafficLog->m_strDepartTrainNum == _pTrafficLog->m_strReachTrainNum)
                    ||(pTrafficLog->m_strDepartTrainNum == _pTrafficLog->m_strDepartTrainNum)))
                    ||((_pTrafficLog->m_btBeginOrEndFlg == 0xBB)&&(pTrafficLog->m_strDepartTrainNum == _pTrafficLog->m_strDepartTrainNum))
                    ||((_pTrafficLog->m_btBeginOrEndFlg == 0xCC)&&(pTrafficLog->m_strReachTrainNum == _pTrafficLog->m_strReachTrainNum)))
                {
                    TrainNumFlag = true;
                }
            }
            //既有计划更新
            if(TrainNumFlag)
            {
                m_pDataAccess->UpdateTrafficLog(_pTrafficLog);
                //处理和发送1个数据
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,_pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
            //新增计划
            else
            {
                m_pDataAccess->InsetTrafficLog(_pTrafficLog);
                //处理和发送1个数据
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,_pTrafficLog,SYNC_FLAG_ADD,1,1);
            }
        }
    }
}
//根据行车日志删除进路序列
void MyStation::DeleteTrainRouteByTrafficlog(TrafficLog *pTrafficLog)
{
    int nRouteOrderSize = this->m_ArrayRouteOrder.count();
    for (int i = 0; i < nRouteOrderSize; i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if (pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC
            && pTrainRouteOrder->m_strTrainNum == pTrafficLog->m_strReachTrainNum)
        {
            //处理和发送1个数据
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
            m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
            this->m_ArrayRouteOrder.removeAt(i);
            nRouteOrderSize--;
            i--;
        }
        else if (pTrainRouteOrder->m_btRecvOrDepart == ROUTE_FC
            && pTrainRouteOrder->m_strTrainNum == pTrafficLog->m_strDepartTrainNum)
        {
            //处理和发送1个数据
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
            m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
            this->m_ArrayRouteOrder.removeAt(i);
            nRouteOrderSize--;
            i--;
        }
    }

    //删除信息发送给联锁
    this->MakeAndSendPlanUDP(this, pTrafficLog, 0x01);
}

//自动设置车站进路序列权限
void MyStation::AutoSetRoutePermission()
{
    //非常站控
    if(this->m_nFCZKMode)
    {
        this->m_bHaveRoutePermissions = false;
    }
    //分散自律 && 车站控制
    else if(this->ModalSelect.nModeState == 1)
    {
        //保持原有状态
        //m_bHaveRoutePermissions = TRUE;
    }
    else
    {
        //没有权限修改-不可修改
        this->m_bHaveRoutePermissions = false;
    }
}
//更新关联列车进路序列为确认状态
TrainRouteOrder* MyStation::UpdateRouteConfirmed(QString _strCheci, int _JFCType)
{
    TrainRouteOrder* pTrainRouteOrder;
    for(int i=0; i<m_ArrayRouteOrder.count(); i++)
    {
        pTrainRouteOrder = (TrainRouteOrder*)m_ArrayRouteOrder[i];
        //根据接车找发车、根据发车找接车
        if(pTrainRouteOrder->m_strTrainNum == _strCheci && pTrainRouteOrder->m_btRecvOrDepart != _JFCType)
        {
            pTrainRouteOrder->m_bConfirmed = true;
            return pTrainRouteOrder;
            //break;
        }
    }
    return nullptr;
}

//接收CTC数据-列车进路序列
void MyStation::recvCTCData_TrainRouteOrder(unsigned char *recvArray, int recvlen, int currCtcIndex, int currZxbIndex)
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    //权限修改

    if(0x05 == recvArray[10])//进路序列修改申请
    {
        //自动设置车站进路序列权限
        this->AutoSetRoutePermission();
        //0xAA修改申请
        if(0xAA == recvArray[11])
        {
            this->m_bHaveRoutePermissions = true;
            //更新数据库
            if(DATATYPE_CTC == recvArray[8]
               || DATATYPE_JK == recvArray[8])//CTC-STPC
            {
                if(this->RoutePermit == 0)
                {
                    this->RoutePermit = 1;
                    m_pDataAccess->UpdateStationInfo(this);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_JLQX, currCtcIndex, -1);
                    //站场状态实时发送
                }
            }
            else if(DATATYPE_BOARD == recvArray[8])//占线板
            {
                if(this->RoutePermit == 0)
                {
                    this->RoutePermit = 2;
                    m_pDataAccess->UpdateStationInfo(this);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_JLQX, -1, currZxbIndex);
                    //站场状态实时发送
                }
            }
        }
        else if(0xBB == recvArray[11])//0xBB只读申请
        {
            this->m_bHaveRoutePermissions = false;
            this->RoutePermit = 0;
            //更新数据库
            m_pDataAccess->UpdateStationInfo(this);
            //发送同步消息
            //sendUpdateDataMsg(this, UPDATETYPE_JLQX, -1, -1);
            //站场状态实时发送
        }
        return;
    }
    //其余的针对进路序列的命令
    else
    {
        int idRoute = (int)(recvArray[11] | (recvArray[12]<<8));
        for(int i=0; i< this->m_ArrayRouteOrder.count(); i++)
        {
            TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder* )this->m_ArrayRouteOrder[i];
            if(idRoute == pTrainRouteOrder->route_id)
            {
                if(0x01 == (int)recvArray[10])//人工触发
                {
                    if(0xAA == (int)recvArray[13])//触发
                    {
                        //进路不存在则报警提示
                        if(-1 == pTrainRouteOrder->m_nIndexRoute)
                        {
                            //暂时进行提示
                            this->SendMessageBoxMsg(3, "进路不存在，请重新操作！", currCtcIndex);
                            return;
                        }
                        else
                        {
                            pTrainRouteOrder->m_nManTouch = true;
                            qDebug()<<QString("%1次%2进路(%3)设置人工触发.")
                                      .arg(pTrainRouteOrder->m_strTrainNum)
                                      .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                      .arg(pTrainRouteOrder->m_strRouteDescrip);
                        }
                    }
                    else if(0xBB == (int)recvArray[13])//0xBB取消进路
                    {
//                        pTrainRouteOrder->m_nAutoTouch = false;
//                        pTrainRouteOrder->m_nManTouch = false;
                        pTrainRouteOrder->SetState(0);
                        pTrainRouteOrder->m_strRouteState = "人工取消";
                        pTrainRouteOrder->m_bSuccessed = 0;
                        pTrainRouteOrder->m_bOnlyLocomotive = false;
                        //数据包-总取消
                        unsigned char byArrayUDPJLOrderDate[40] = {0xEF, 0xEF, 0xEF, 0xEF,	//帧头
                            0x28,	/*帧长低位*/
                            0x00,	/*帧长高位*/
                            0x00,	//目标地址码
                            0x00,	//本机地址码
                            0x00,	//车站标志
                            0x88,	//信息分类码 （CTC车务终端----->联锁仿真机）
                            0x00,	//功能按钮类型
                            0x00,	//设备号
                            0x00,	//设备号
                            0x00,	//设备号
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0xFE, 0xFE, 0xFE, 0xFE};	//帧尾
                        //UINT nLength = byArrayUDPJLOrderDate[4];
                        if (pTrainRouteOrder->m_btRecvOrDepart==0)//接车
                        {
                            byArrayUDPJLOrderDate[10] = 0x04;
                            int devcodeS = this->GetCodeByStrName(pTrainRouteOrder->m_strXHDBegin);
                            byArrayUDPJLOrderDate[11] = (BYTE)(devcodeS);
                            byArrayUDPJLOrderDate[12] = (BYTE)(devcodeS>>8);
                            //数据帧转换
                            QByteArray qRecvArray = UnsignedCharToQByteArray(byArrayUDPJLOrderDate, 40);
                            //sendDataToLS(this, qRecvArray, nLength);
                            //信号-发送数据给联锁
                            emit sendDataToLSSignal(this, qRecvArray, 40);

                            m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                            //发送同步消息
                            //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);

                            for(int j=0; j< this->m_ArrayTrafficLog.count(); j++)
                            {
                                TrafficLog* pTrafficLog = (TrafficLog* )this->m_ArrayTrafficLog[j];
                                if((pTrainRouteOrder->m_strTrainNum == pTrafficLog->m_strTrainNum)&&(pTrainRouteOrder->m_strTrack == pTrafficLog->m_strTrainTrack))
                                {
                                    //更新数据库
                                    pTrafficLog->m_btJCJLStatus=0;
                                    SetTrafficLogProc(pTrafficLog);
                                    m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                                    //发送同步消息
                                    //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                                    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                                    break;//return
                                }
                            }
                        }
                        else if (pTrainRouteOrder->m_btRecvOrDepart==1)//发车
                        {
                            byArrayUDPJLOrderDate[10] = 0x04;
                            int devcodeS = this->GetCodeByStrName(pTrainRouteOrder->m_strXHDBegin);
                            byArrayUDPJLOrderDate[11] = (BYTE)(devcodeS);
                            byArrayUDPJLOrderDate[12] = (BYTE)(devcodeS>>8);
                            //数据帧
                            QByteArray qRecvArray = UnsignedCharToQByteArray(byArrayUDPJLOrderDate, 40);
                            //sendDataToLS(this, qRecvArray, nLength);
                            //信号-发送数据给联锁
                            emit sendDataToLSSignal(this, qRecvArray, 40);
                            m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                            //发送同步消息
                            //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);

                            for(int i=0; i < this->m_ArrayTrafficLog.count(); i++)
                            {
                                TrafficLog* pTrafficLog = (TrafficLog* )this->m_ArrayTrafficLog[i];
                                if((pTrainRouteOrder->m_strTrainNum == pTrafficLog->m_strTrainNum)&&(pTrainRouteOrder->m_strTrack == pTrafficLog->m_strTrainTrack))
                                {
                                    //更新数据库
                                    pTrafficLog->m_btFCJLStatus=0;
                                    SetTrafficLogProc(pTrafficLog);
                                    m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                                    //发送同步消息
                                    //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                                    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                                    break;
                                }
                            }
                        }

                    }
                }
                else if(0x02 == (int)recvArray[10])//设置触发方式
                {
                    if(0xAA == (int)recvArray[13])//自动
                    {
                        //系统报警信息
                        QString strSys ="";

                        //允许时间范围内的进路序列才可以勾选自触，超过一定时间范围的进路序列点击自触，自触框不会打勾。
                        QDateTime timeNow = QDateTime::currentDateTime();
                        qint64 seconds = timeNow.secsTo(pTrainRouteOrder->m_timBegin);
                        if(seconds > (AutoTouchMinutes*60))
                        {
                            strSys = QString("车站值班员更改%1次%2进路(%3)，自动触发，更改失败，进路计划时间不在当前时间范围内，无法自动触发")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(pTrainRouteOrder->m_strRouteDescrip);
                            //系统报警信息
                            this->sendWarningMsgToCTC(3,2,strSys);
                            return;
                        }

                        //设置自触需要先办理预告
                        if(SetAutoTouchNeedNotice && !CheckTrafficLogIsNoticed(pTrainRouteOrder))
                        {
                            strSys = QString("系统规定，设置自触需要先办理预告");//自定义通知，需调研具体通知
                            //系统报警信息
                            this->sendWarningMsgToCTC(3,2,strSys);
                            return;
                        }

                        //客车正线通过进路检查
                        if(pTrainRouteOrder->m_nLHFlg == LCTYPE_KC)
                        {
                            bool tgRouteIsZXTGJL = CheckZXTGJL(pTrainRouteOrder);
                            if(!tgRouteIsZXTGJL)
                            {
                                strSys = QString("车站值班员更改%1次%2进路(%3)，自动触发，更改失败，客车%4次在股道%5侧线通过")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip)
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                //系统报警信息
                                this->sendWarningMsgToCTC(3,2,strSys);
                                return;
                            }
                        }

                        //检查站细，满足才可设置自触，否则给出提示
                        CheckResult* ckResult1 = this->CheckPreventConditionStaDetails(pTrainRouteOrder);
                        //不满足
                        if(ckResult1!=nullptr && ckResult1->check!=0)
                        {
                            strSys = ckResult1->checkMsg;
                            //不满足-股道类型
                            if(JLWARNING_ATTR_GDTYPE == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("列车%1次%2类型不满足，自动选路时设为人工触发")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);

                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                            //不满足-超限条件
                            else if(JLWARNING_ATTR_LEVELCX == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("更改%1次%2进路(%3),自动触发，更改失败，%4次股道%5超限条件不满足")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip)
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                            //不满足-客运设备股道（站台）
                            else if(JLWARNING_ATTR_PLATFORM1 == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("更改%1次%2进路(%3),自动触发，更改失败，旅客列车%4次无法接入无客运设备%5")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip)
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                            //不满足-货车不可以接入高站台
                            else if(JLWARNING_ATTR_PLATFORM2 == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("更改%1次%2进路(%3),自动触发，更改失败，货车%4次不能通过高站台股道%5")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip)
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                            //不满足-上水
                            else if(JLWARNING_ATTR_FLOWSS == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("更改%1次%2进路(%3),自动触发，更改失败，%4次股道%5无上水设备")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip)
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                            //不满足-吸污
                            else if(JLWARNING_ATTR_FLOWXW == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("更改%1次%2进路(%3),自动触发，更改失败，%4次股道%5无吸污设备")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip)
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }

                            //不满足-出入口超限
                            else if(JLWARNING_ENEX_LEVELCX == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("自动触发，更改失败，%1次%2方向不允许超限车(出入口%3)")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                            //不满足-客货类型错误
                            else if(JLWARNING_ENEX_KHTYPE == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("自动触发，更改失败，%1次%2方向客货类型错误(出入口%3)")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                            //不满足-军用
                            else if(JLWARNING_ATTR_ARMY == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("自动触发，更改失败，军用列车%1次无法进入非军用股道%2")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strTrack);
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                            //不满足-列车固定径路信息
                            else if(JLWARNING_ENEX_UNSAME == ckResult1->check)
                            {
                                //修改报警信息
                                strSys = QString("自动触发，更改失败，列车%1次%2方向与固定径路不一致")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车");
                                //系统报警信息
                                this->sendWarningMsgToCTC(1,2,strSys);
                                return;
                            }
                        }

                        //联锁条件
                        CheckResult* ckResult3 = this->CheckPreventConditionInterlock(pTrainRouteOrder);
                        if(ckResult3->check!=0)
                        {
                            QString strMsg = ckResult3->checkMsg;
                            if((ckResult3->check&JLWARNING_QDPOWERCUT)>0)
                            {
                                strMsg = QString("电力牵引列车%1无法通过无电进路(%2),不能自动办理")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_strRouteDescrip);
                            }
                            else if((ckResult3->check&JLWARNING_FS_DC)>0
                                    || (ckResult3->check&JLWARNING_FS_GD)>0
                                    )
                            {
                                strMsg = QString("%1次%2进路(%3)，不能自动办理，进路设备封锁%4")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip)
                                        .arg(ckResult3->checkMsg);
                            }
                            else if((ckResult3->check&JLWARNING_FLBL_GD)>0
                                    || (ckResult3->check&JLWARNING_FLBL_DC)>0
                                    )
                            {
                                strMsg = QString("%1次%2进路(%3)，不能自动办理，进路分路不良未确认空闲")
                                        .arg(pTrainRouteOrder->m_strTrainNum)
                                        .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                        .arg(pTrainRouteOrder->m_strRouteDescrip);
                            }
                            //系统报警信息
                            this->sendWarningMsgToCTC(1,2,strMsg);
                            return;
                        }

                        //设置自触
                        pTrainRouteOrder->m_nAutoTouch = true;
                    }
                    else if(0xBB == (int)recvArray[13])//人工
                    {
                        //正在触发、触发成功的进路不否可取消自触
                        if(!TouchingRouteCancelAutoFlag
                           && (pTrainRouteOrder->m_nOldRouteState==1 || pTrainRouteOrder->m_nOldRouteState==2
                               || pTrainRouteOrder->m_nOldRouteState==3 || pTrainRouteOrder->m_nOldRouteState==4))
                        {
                            return;
                        }
                        else
                        {
                            //取消自触
                            pTrainRouteOrder->m_nAutoTouch = false;
                            pTrainRouteOrder->m_bRunNow = false;
                        }
                    }
                    //确认进路
                    pTrainRouteOrder->m_bConfirmed = true;
                    //pStation->UpdateRouteConfirmed(pTrainRouteOrder->m_strTrainNum, pTrainRouteOrder->m_btRecvOrDepart);
                    //更新数据库
                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                }
                else if(0x03 == (int)recvArray[10])//修改股道
                {
                    if(pTrainRouteOrder->m_nAutoTouch)
                    {
                        return;//已标记“自触”的进路不可修改
                    }
                    if(pTrainRouteOrder->m_nOldRouteState != 0)
                    {
                        return;//仅可修改“等待”状态的进路
                    }
                    int gdCode = (int)(recvArray[13] | (recvArray[14]<<8));
                    QString oldTrainNum=pTrainRouteOrder->m_strTrainNum;
                    QString oldGDName=pTrainRouteOrder->m_strTrack;
                    //int oldGDCode=pTrainRouteOrder->m_nTrackCode;
                    pTrainRouteOrder->m_strTrack = this->GetStrNameByCode(gdCode);
                    pTrainRouteOrder->m_nTrackCode = gdCode;
                    pTrainRouteOrder->m_nGDPos = this->GetGDPosInzcArray(gdCode);
                    //lwm,20171214
                    if(pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC)//到达
                    {
                        int nCode = this->GetCodeByRecvEquipGD(pTrainRouteOrder->m_nCodeReachStaEquip, pTrainRouteOrder->m_nTrackCode,0);
                        pTrainRouteOrder->m_strXHDEnd = this->GetStrNameByCode(nCode);
                        pTrainRouteOrder->m_strRouteDescrip =  pTrainRouteOrder->m_strXHDBegin + "-" + pTrainRouteOrder->m_strXHDEnd;
                        pTrainRouteOrder->m_strRouteDescripReal =  pTrainRouteOrder->m_strXHDBegin + "," + pTrainRouteOrder->m_strXHDEnd;
                        //延续进路重新计算
                        this->InitRouteYXJL(pTrainRouteOrder);
                    }
                    else//出发
                    {
                        int nCode = this->GetCodeByRecvEquipGD(pTrainRouteOrder->m_nCodeDepartStaEquip, pTrainRouteOrder->m_nTrackCode,1);
                        pTrainRouteOrder->m_strXHDBegin = this->GetStrNameByCode(nCode);
                        pTrainRouteOrder->m_strRouteDescrip =  pTrainRouteOrder->m_strXHDBegin + "-" + pTrainRouteOrder->m_strXHDEnd;
                        pTrainRouteOrder->m_strRouteDescripReal =  pTrainRouteOrder->m_strXHDBegin + "," + pTrainRouteOrder->m_strXHDEnd;
                    }
                    //重新初始化变通进路
                    this->InitRouteBtjl(pTrainRouteOrder);
                    //确认进路
                    pTrainRouteOrder->m_bConfirmed = true;
//                    //更新数据库
//                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
//                    //发送同步消息
//                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                    //组合进路判断
//                    if(pTrainRouteOrder->m_bZHJL)
//                    {
//                        //删除子序列
//                        int routeSize = this->m_ArrayRouteOrder.size();
//                        for (int r = 0; r < routeSize; r++)
//                        {
//                            TrainRouteOrder* pTrainRouteOrder1 = this->m_ArrayRouteOrder[r];
//                            //不删除自己
//                            if(pTrainRouteOrder->route_id == pTrainRouteOrder1->route_id)
//                            {
//                                continue;
//                            }
//                            //删除子序列
//                            if (pTrainRouteOrder->m_strTrainNum == pTrainRouteOrder1->m_strTrainNum
//                               || pTrainRouteOrder->route_id == pTrainRouteOrder1->father_id)
//                            {
//                                //删除数据库
//                                m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder1);
//                                //处理和发送1个数据
//                                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder1,SYNC_FLAG_DELETE,1,1);
//                                //m_pDataAccess->SelectAllRouteOrder(this);
//                                this->m_ArrayRouteOrder.removeAt(r);
//                                routeSize = this->m_ArrayRouteOrder.count();
//                                r--;
//                            }
//                        }
//                    }
                    //初始化组合进路
                    this->InitRouteZhjl(pTrainRouteOrder);
                    this->UpdateRouteOrderOfZHJL(pTrainRouteOrder, false);
                    //更新数据库
                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                    //发送同步消息
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
//                    //组合进路判断
//                    if(pTrainRouteOrder->m_bZHJL)
//                    {
//                        //重新生成子序列
//                        for(int r=0; r<this->m_ArrayRouteOrderSonTemp.size(); r++)
//                        {
//                            TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrderSonTemp[r];
//                            //设置父id
//                            pRouteOrderSon->father_id = pTrainRouteOrder->route_id;
//                            pRouteOrderSon->m_nAutoTouch = pTrainRouteOrder->m_nAutoTouch;
//                            //插入数据库
//                            m_pDataAccess->InsetRouteOrder(pRouteOrderSon);
//                            //处理和发送1个数据
//                            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrderSon,SYNC_FLAG_ADD,1,1);
//                            //增加一条
//                            this->m_ArrayRouteOrder.append(pRouteOrderSon);
//                        }
//                        this->m_ArrayRouteOrderSonTemp.clear();
//                        //必须退出大循环，总数已变化，否则数组循环溢出
//                        break;
//                    }

                    //同步修改兄弟进路
                    //接车的发车，发车的接车
                    TrainRouteOrder *pTrainRouteOrderBrother = this->UpdateRouteConfirmed(pTrainRouteOrder->m_strTrainNum, pTrainRouteOrder->m_btRecvOrDepart);
                    //UpdateTrafficLogTrack(pStation, pTrainRouteOrder->m_strTrainNum, pTrainRouteOrder->m_strTrack, pTrainRouteOrder->m_btRecvOrDepart);
                    if(pTrainRouteOrderBrother!=nullptr)
                    {
                        pTrainRouteOrderBrother->m_strTrack = this->GetStrNameByCode(gdCode);
                        pTrainRouteOrderBrother->m_nTrackCode = gdCode;
                        pTrainRouteOrderBrother->m_nGDPos = this->GetGDPosInzcArray(gdCode);
                        if(pTrainRouteOrderBrother->m_btRecvOrDepart == ROUTE_JC)//到达
                        {
                            int nCode = this->GetCodeByRecvEquipGD(pTrainRouteOrderBrother->m_nCodeReachStaEquip, pTrainRouteOrderBrother->m_nTrackCode,0);
                            pTrainRouteOrderBrother->m_strXHDEnd = this->GetStrNameByCode(nCode);
                            pTrainRouteOrderBrother->m_strRouteDescrip =  pTrainRouteOrderBrother->m_strXHDBegin + "-" + pTrainRouteOrderBrother->m_strXHDEnd;
                            pTrainRouteOrderBrother->m_strRouteDescripReal =  pTrainRouteOrderBrother->m_strXHDBegin + "," + pTrainRouteOrderBrother->m_strXHDEnd;
                            //延续进路重新计算
                            this->InitRouteYXJL(pTrainRouteOrderBrother);
                        }
                        else//出发
                        {
                            int nCode = this->GetCodeByRecvEquipGD(pTrainRouteOrderBrother->m_nCodeDepartStaEquip, pTrainRouteOrderBrother->m_nTrackCode,1);
                            pTrainRouteOrderBrother->m_strXHDBegin = this->GetStrNameByCode(nCode);
                            pTrainRouteOrderBrother->m_strRouteDescrip =  pTrainRouteOrderBrother->m_strXHDBegin + "-" + pTrainRouteOrderBrother->m_strXHDEnd;
                            pTrainRouteOrderBrother->m_strRouteDescripReal =  pTrainRouteOrderBrother->m_strXHDBegin + "," + pTrainRouteOrderBrother->m_strXHDEnd;
                        }
                        //重新初始化变通进路
                        this->InitRouteBtjl(pTrainRouteOrderBrother);
                        pTrainRouteOrderBrother->m_bConfirmed = true;
//                        //更新数据库
//                        m_pDataAccess->UpdateRouteOrder(pTrainRouteOrderBrother);
//                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrderBrother,SYNC_FLAG_UPDATE,1,1);
                        //组合进路判断
//                        if(pTrainRouteOrderBrother->m_bZHJL)
//                        {
//                            //删除子序列
//                            int routeSize = this->m_ArrayRouteOrder.size();
//                            for (int r = 0; r < routeSize; r++)
//                            {
//                                TrainRouteOrder* pTrainRouteOrder1 = this->m_ArrayRouteOrder[r];
//                                //不删除自己
//                                if(pTrainRouteOrderBrother->route_id == pTrainRouteOrder1->route_id)
//                                {
//                                    continue;
//                                }
//                                //删除子序列
//                                if (pTrainRouteOrderBrother->m_strTrainNum == pTrainRouteOrder1->m_strTrainNum
//                                   || pTrainRouteOrderBrother->route_id == pTrainRouteOrder1->father_id)
//                                {
//                                    //删除数据库
//                                    m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder1);
//                                    //处理和发送1个数据
//                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder1,SYNC_FLAG_DELETE,1,1);
//                                    //m_pDataAccess->SelectAllRouteOrder(this);
//                                    this->m_ArrayRouteOrder.removeAt(r);
//                                    routeSize = this->m_ArrayRouteOrder.count();
//                                    r--;
//                                }
//                            }
//                        }
                        //初始化组合进路
                        this->InitRouteZhjl(pTrainRouteOrderBrother);
                        this->UpdateRouteOrderOfZHJL(pTrainRouteOrderBrother, false);
                        //更新数据库
                        m_pDataAccess->UpdateRouteOrder(pTrainRouteOrderBrother);
                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrderBrother,SYNC_FLAG_UPDATE,1,1);
//                        //组合进路判断
//                        if(pTrainRouteOrderBrother->m_bZHJL)
//                        {
//                            //重新生成子序列
//                            for(int r=0; r<this->m_ArrayRouteOrderSonTemp.size(); r++)
//                            {
//                                TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrderSonTemp[r];
//                                //设置父id
//                                pRouteOrderSon->father_id = pTrainRouteOrderBrother->route_id;
//                                pRouteOrderSon->m_nAutoTouch = pTrainRouteOrderBrother->m_nAutoTouch;
//                                //插入数据库
//                                m_pDataAccess->InsetRouteOrder(pRouteOrderSon);
//                                //处理和发送1个数据
//                                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrderSon,SYNC_FLAG_ADD,1,1);
//                                //增加一条
//                                this->m_ArrayRouteOrder.append(pRouteOrderSon);
//                            }
//                            this->m_ArrayRouteOrderSonTemp.clear();

//                            //必须退出大循环，总数已变化，否则数组循环溢出
//                            break;
//                        }
                    }
//                    ////①接发车类型：修改接车股道，发车股道不变。修改发车股道，接车股道不变。
//                    ////②通过类型：修改接车股道、发车股道同时变化。修改发车股道，接车股道不变。
//                    //新的逻辑：阶段计划下发为接发车股道一致时，修改接车股道，发车股道同步修改；修改发车股道，接车股道不变。
//                    if((pTrainRouteOrder->m_btBeginOrEndFlg == JFC_TYPE_JF || pTrainRouteOrder->m_btBeginOrEndFlg == JFC_TYPE_TG)
//                       && pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC)
//                    {
//                        for(int index=0;index<this->m_ArrayRouteOrder.count();index++)
//                        {
//                            TrainRouteOrder *pTrainRouteOrder1 = (TrainRouteOrder *)this->m_ArrayRouteOrder[index];
//                            if((pTrainRouteOrder1->m_btRecvOrDepart == ROUTE_FC)
//                                && (oldTrainNum==pTrainRouteOrder1->m_strTrainNum) && (oldGDName==pTrainRouteOrder1->m_strTrack))
//                            {
//                                pTrainRouteOrder1->m_strTrack = this->GetStrNameByCode(gdCode);
//                                pTrainRouteOrder1->m_nTrackCode = gdCode;
//                                int nCode = this->GetCodeByRecvEquipGD(pTrainRouteOrder1->m_nCodeDepartStaEquip, pTrainRouteOrder1->m_nTrackCode);
//                                pTrainRouteOrder1->m_strXHDBegin = this->GetStrNameByCode(nCode);
//                                pTrainRouteOrder1->m_strRouteDescrip =  pTrainRouteOrder1->m_strXHDBegin + "-" + pTrainRouteOrder1->m_strXHDEnd;
//                                pTrainRouteOrder1->m_strRouteDescripReal =  pTrainRouteOrder1->m_strXHDBegin + "," + pTrainRouteOrder1->m_strXHDEnd;
//                                pTrainRouteOrder1->m_nGDPos = this->GetGDPosInzcArray(gdCode);
//                                //重新初始化变通进路
//                                this->InitRouteBtjl(pTrainRouteOrder1);
//                                pTrainRouteOrder1->m_bConfirmed = true;
//                                this->UpdateRouteConfirmed(pTrainRouteOrder1->m_strTrainNum, pTrainRouteOrder1->m_btRecvOrDepart);
//                                //UpdateTrafficLogTrack(pTrainRouteOrder1->m_strTrainNum, pTrainRouteOrder1->m_strTrack, pTrainRouteOrder1->m_btRecvOrDepart);
//                                //更新数据库
//                                m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder1);
//                                this->sendOneTrainRouteOrderToSoft(pTrainRouteOrder1,SYNC_FLAG_UPDATE,1,1);
//                                break;
//                            }
//                        }
//                    }
                    //行车日志同步修改接发车股道
                    UpdateTrafficLogTrack(pTrainRouteOrder->m_strTrainNum, pTrainRouteOrder->m_strTrack);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                    //m_pDataAccess->SelectAllRouteOrder(this);
                    //重新读取数据库（组合进路时将读取子序列）
                    //m_pDataAccess->SelectAllRouteOrder(this);
                    return;
                }
                else if(0x04 == (int)recvArray[10])//修改进路描述
                {
                    if(pTrainRouteOrder->m_nAutoTouch)
                    {
                        return;//已标记“自触”的进路不可修改
                    }
                    int length = (int)recvArray[13];
                    //命令内容
                    unsigned char arrRecv[100];
                    memset(arrRecv,0,sizeof(arrRecv));
                    memcpy(arrRecv, &recvArray[14], length);
                    pTrainRouteOrder->m_strRouteDescripReal =  UnsignedCharArrayToString(arrRecv);
                    pTrainRouteOrder->m_strRouteDescrip =  pTrainRouteOrder->m_strRouteDescripReal;
                    pTrainRouteOrder->m_strRouteDescrip.replace(",","-");
                    this->UpdateRouteConfirmed(pTrainRouteOrder->m_strTrainNum, pTrainRouteOrder->m_btRecvOrDepart);
                    //更新数据库
                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                }
                else if(0x06 == (int)recvArray[10])//删除
                {
                    if(0xAA == (int)recvArray[13])//删除
                    {
                        //Mutex.lock();
                        //更新数据库--进路序列下发时更新
                        m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);//idRoute
                        //m_pDataAccess->SelectAllRouteOrder(this);
                        //发送同步消息
                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
                        this->m_ArrayRouteOrder.removeAt(i);
                        //Mutex.unlock();
                        break;
                    }
                }
                else if(0x08 == (int)recvArray[10])//更新进路序列状态
                {
                    int nState = recvArray[13];
                    pTrainRouteOrder->SetState(nState);
                    //更新数据库
                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                    //发送同步消息
                    //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                }
                else if(0x09 == (int)recvArray[10])//单机开行
                {
                    //单击开行-设置
                    if(0xAA == (int)recvArray[13])
                    {
                        pTrainRouteOrder->m_bOnlyLocomotive = true;
                    }
                    //单击开行-取消
                    else if(0xBB == (int)recvArray[13])
                    {
                        pTrainRouteOrder->m_bOnlyLocomotive = false;
                    }
                }

                //return;
            }
        }
    }
}
//更新关联行车日志的股道信息,当_JFCType=-1时都更新接发车进路
void MyStation::UpdateTrafficLogTrack(QString _strCheci, QString _strNewGd, int _JFCType)
{
    TrafficLog* pTrafficLog;
    if(-1 == _JFCType)
    {
        for(int t = 0; t < this->m_ArrayTrafficLog.count(); t++)
        {
            pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[t];
            if(pTrafficLog->m_strReachTrainNum == _strCheci || pTrafficLog->m_strDepartTrainNum == _strCheci)
            {
                pTrafficLog->m_strRecvTrainTrack = _strNewGd;
                pTrafficLog->m_strDepartTrainTrack = _strNewGd;
                pTrafficLog->m_bModify = true;
                //更新数据库
                m_pDataAccess->UpdateTrafficLogTrack(pTrafficLog);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                break;
            }
        }
    }
    else
    {
        for(int t = 0; t < this->m_ArrayTrafficLog.count(); t++)
        {
            pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[t];
            if(pTrafficLog->m_strReachTrainNum == _strCheci && _JFCType == ROUTE_JC)
            {
                pTrafficLog->m_strRecvTrainTrack = _strNewGd;
                pTrafficLog->m_bModify = true;
                //更新数据库
                m_pDataAccess->UpdateTrafficLogTrack(pTrafficLog);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
            else if(pTrafficLog->m_strDepartTrainNum == _strCheci && _JFCType == ROUTE_FC)
            {
                pTrafficLog->m_strDepartTrainTrack = _strNewGd;
                pTrafficLog->m_bModify = true;
                //更新数据库
                m_pDataAccess->UpdateTrafficLogTrack(pTrafficLog);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
    }

    SendModifiedPlanToLS(this);
}
//发送修改后过的计划给LS
bool MyStation::SendModifiedPlanToLS(MyStation *pStation)
{
    bool bValue = false;
    TrafficLog *pTrafficLog;
    for(int i = 0; i < pStation->m_ArrayTrafficLog.count(); i++)
    {
        pTrafficLog = (TrafficLog*)pStation->m_ArrayTrafficLog[i];
        if(true == pTrafficLog->m_bModify)
        {
            //封装了方法 2021.8.19.lwm
            MakeAndSendPlanUDP(pStation, pTrafficLog, 0x00);
            pTrafficLog->m_bModify = false;
            bValue = true;
        }
    }

    return bValue;
}
//组织和发送计划的信息（00修改或01删除）
void MyStation::MakeAndSendPlanUDP(MyStation *pStation, TrafficLog *pTrafficLog, int _cmdType)
{
    int nLength = 0;
    unsigned int nCount=0;
    unsigned int nameLen=0;
    QString strCheCi;
    unsigned char byArrayUDPJLOrderDate[500];
    memset(byArrayUDPJLOrderDate,0,500);
    nCount = 9;

    byArrayUDPJLOrderDate[nCount]  = 0x61;//[9]信息分类码：车次计划修改
    nCount += 1;
    byArrayUDPJLOrderDate[nCount] = _cmdType;//[10]0x00; 0x00修改 0x01删除//预留
    nCount += 1;
    //车次计划类型
    byArrayUDPJLOrderDate[nCount] = pTrafficLog->m_btBeginOrEndFlg;//[11]
    nCount += 1;

    //原到达车次
    strCheCi = pTrafficLog->m_strReachTrainNumOld;
    QByteArray byteArray = strCheCi.toLatin1();
    nameLen = byteArray.count();
    byArrayUDPJLOrderDate[nCount] = nameLen;
    nCount += 1;
    for(int u=0; u<byteArray.count(); u++)
    {
        byArrayUDPJLOrderDate[nCount+u] = byteArray[u];
    }
    nCount += nameLen;
    //新到达车次号
    strCheCi = pTrafficLog->m_strReachTrainNum;
    byteArray = strCheCi.toLatin1();
    nameLen = byteArray.count();
    byArrayUDPJLOrderDate[nCount] = nameLen;
    nCount += 1;
    for(int u=0; u<byteArray.count(); u++)
    {
        byArrayUDPJLOrderDate[nCount+u] = byteArray[u];
    }
    nCount += nameLen;

    //接车口
    memcpy(&byArrayUDPJLOrderDate[nCount], &pTrafficLog->m_nCodeReachStaEquip, 2);
    nCount += 2;
    //接车股道
    int jcGdCode = pStation->GetCodeByStrName(pTrafficLog->m_strRecvTrainTrack);
    memcpy(&byArrayUDPJLOrderDate[nCount], &jcGdCode, 2);
    nCount += 2;

    //计划到达时间 7
    int Year1 = pTrafficLog->m_timProvReachStation.date().year();
    int Month1 = pTrafficLog->m_timProvReachStation.date().month();
    int Day1 = pTrafficLog->m_timProvReachStation.date().day();
    int Hour1 = pTrafficLog->m_timProvReachStation.time().hour();
    int Minu1 = pTrafficLog->m_timProvReachStation.time().minute();
    int Secd1 = pTrafficLog->m_timProvReachStation.time().second();
    memcpy(&byArrayUDPJLOrderDate[nCount], &Year1, 2);
    memcpy(&byArrayUDPJLOrderDate[nCount+2], &Month1, 1);
    memcpy(&byArrayUDPJLOrderDate[nCount+3], &Day1, 1);
    memcpy(&byArrayUDPJLOrderDate[nCount+4], &Hour1, 1);
    memcpy(&byArrayUDPJLOrderDate[nCount+5], &Minu1, 1);
    memcpy(&byArrayUDPJLOrderDate[nCount+6], &Secd1, 1);
    nCount += 7;

    //原出发车次
    strCheCi = pTrafficLog->m_strDepartTrainNumOld;
    byteArray = strCheCi.toLatin1();
    nameLen = byteArray.count();
    byArrayUDPJLOrderDate[nCount] = nameLen;
    nCount += 1;
    for(int u=0; u<byteArray.count(); u++)
    {
        byArrayUDPJLOrderDate[nCount+u] = byteArray[u];
    }
    nCount += nameLen;
    //新出发车次号
    strCheCi = pTrafficLog->m_strDepartTrainNum;
    byteArray = strCheCi.toLatin1();
    nameLen = byteArray.count();
    byArrayUDPJLOrderDate[nCount] = nameLen;
    nCount += 1;
    for(int u=0; u<byteArray.count(); u++)
    {
        byArrayUDPJLOrderDate[nCount+u] = byteArray[u];
    }
    nCount += nameLen;

    //发车口
    memcpy(&byArrayUDPJLOrderDate[nCount], &pTrafficLog->m_nCodeDepartStaEquip, 2);
    nCount += 2;
    //发车股道
    int fcGdCode = pStation->GetCodeByStrName(pTrafficLog->m_strDepartTrainTrack);
    memcpy(&byArrayUDPJLOrderDate[nCount], &fcGdCode, 2);
    nCount += 2;

    //计划发车时间 7
    int Year2 = pTrafficLog->m_timProvDepaTrain.date().year();
    int Month2 = pTrafficLog->m_timProvDepaTrain.date().month();
    int Day2 = pTrafficLog->m_timProvDepaTrain.date().day();
    int Hour2 = pTrafficLog->m_timProvDepaTrain.time().hour();
    int Minu2 = pTrafficLog->m_timProvDepaTrain.time().minute();
    int Secd2 = pTrafficLog->m_timProvDepaTrain.time().second();
    memcpy(&byArrayUDPJLOrderDate[nCount], &Year2, 2);
    memcpy(&byArrayUDPJLOrderDate[nCount+2], &Month2, 1);
    memcpy(&byArrayUDPJLOrderDate[nCount+3], &Day2, 1);
    memcpy(&byArrayUDPJLOrderDate[nCount+4], &Hour2, 1);
    memcpy(&byArrayUDPJLOrderDate[nCount+5], &Minu2, 1);
    memcpy(&byArrayUDPJLOrderDate[nCount+6], &Secd2, 1);
    nCount += 7;

    //电力.new
    byArrayUDPJLOrderDate[nCount]  = pTrafficLog->m_bElectric ? 0x11 : 0x00;
    nCount += 1;
    //超限等级.new
    byArrayUDPJLOrderDate[nCount]  = pTrafficLog->m_nLevelCX;
    nCount += 1;
    //列车类型序号.new
    byArrayUDPJLOrderDate[nCount]  = pTrafficLog->m_nIndexLCLX;
    nCount += 1;
    //运行类型序号.new
    byArrayUDPJLOrderDate[nCount]  = pTrafficLog->m_nIndexYXLX;
    nCount += 1;

    nLength = nCount + 4;
    memcpy(&byArrayUDPJLOrderDate[4], &nLength, 2);//帧长度
    for(int i = 0; i < 4; i++)
    {
        byArrayUDPJLOrderDate[i] = 0xEF;
        byArrayUDPJLOrderDate[nLength - i -1] = 0xFE;
    }
    byArrayUDPJLOrderDate[8]=DATATYPE_SERVER;//DATATYPE_CTRL;//集控类型

    //数据帧-发送
    QByteArray qSendArray = UnsignedCharToQByteArray(byArrayUDPJLOrderDate, nLength);
    //sendDataToLS(pStation, qRecvArray, nLength);
    //信号-发送数据给联锁
    emit sendDataToLSSignal(pStation, qSendArray, nLength);
    //SendUDPDataToCTC(pStation, qRecvArray, nLength);
}
//处理进路取消命令
void MyStation::DealRouteCancleCmd(unsigned char *recvArray, int nLength)
{
    //总取消 || 总人解
    if(0x04 == (int)recvArray[10] || 0x14 == (int)recvArray[10])
    {
        int xhdCode = (int)(recvArray[11] | (recvArray[12]<<8));
        QXJL_TrainRoute(xhdCode);
        //发送同步消息
        //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
        //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
    }
}
//处理取消进路
void MyStation::QXJL_TrainRoute(int XHDBeginCode)
{
    for(int i = 0; i < (int)this->DevArray.count(); i++)
    {
        if(this->DevArray[i]->getDevType() == Dev_XH)
        {
             CXHD *pXHD = (CXHD *)this->DevArray[i];
            if((int)pXHD->m_nCode == XHDBeginCode)
            {
                for(int i=0; i<this->m_ArrayRouteOrder.count(); i++)
                {
                    TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder* )this->m_ArrayRouteOrder[i];
                    //TrafficLog *pTrafficLog = (TrafficLog *)(m_ArrayTrafficLog[m_nLBtnDawnPos-m_nRowStart]);
                    //if((pXHD->m_strName == pTrainRouteOrder->m_strXHDBegin)&&(pTrainRouteOrder->m_strRouteState == "触发完成"))
                    if((pXHD->m_strName == pTrainRouteOrder->m_strXHDBegin)
                        &&(pTrainRouteOrder->m_nOldRouteState==1||pTrainRouteOrder->m_nOldRouteState==2)
                      )
                    {
                        //设置自触标记办理的进路，人工取消后，自触标记会自动清除.lwm.2024.1.31
                        pTrainRouteOrder->m_nAutoTouch = false;
                        //pTrainRouteOrder->m_nManTouch = false;
                        pTrainRouteOrder->SetState(0);
                        pTrainRouteOrder->m_bSuccessed = 0;
                        pTrainRouteOrder->m_strRouteState = "人工取消";
                        pTrainRouteOrder->m_bOnlyLocomotive = false;
                        m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);

                        for(int i=0; i<this->m_ArrayTrafficLog.count(); i++)
                        {
                            TrafficLog* pTrafficLog = (TrafficLog* )this->m_ArrayTrafficLog[i];
                            if((pTrainRouteOrder->m_strTrainNum == pTrafficLog->m_strTrainNum)&&(pTrainRouteOrder->m_strTrack == pTrafficLog->m_strTrainTrack))
                            {
                                //更新数据库
                                pTrafficLog->m_btJCJLStatus=0;
                                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                                //return;
                            }
                        }
                    }
                }
            }
        }
    }
}
//接收CTC数据-修改计划
void MyStation::recvCTCData_ChangePlan(unsigned char *StatusArray, int recvlen)
{
    QByteArray arr;
    qDebug() << "recive plan" << QByteArray::fromRawData((const char*)StatusArray, recvlen).toHex();
    if((int)StatusArray[0] != 0x00)
    {
        //信号员发给值班员的数据处理
        //值班员发给信号员的数据处理
     }
#pragma region//修改后的阶段计划 0x61
    if ((int)StatusArray[9] == PLAN_CMD_CHG)
    {
        ////双机版时，信号员收到值班员CTC转发的修改计划，再转发给本机占线板
        //if(CTCDEVTYPE_XHY == pFrame->GlobalInfo.nCTCDevType)
        //{
        //	//数据转发给本机的3.0占线板-教师机通道
        //	SendUDPData(&m_sockServerToBoard2, &m_addrSendClientToBoard2, StatusArray, nLength);
        //}
        if ((int)StatusArray[10] == 0x01)
        {
            //实际删除
            //数据帧
            QByteArray qRecvArray = UnsignedCharToQByteArray(StatusArray, recvlen);
            //信号-发送数据
            emit sendDataToLSSignal(this, qRecvArray, recvlen);
        }
        //新协议
        int nLength = 0;
        unsigned int nCount=11;
        unsigned int nameLen=0;
        QString strCheCi="";

        StagePlan *stagePlan = new StagePlan();
        if ((int)StatusArray[10] == 0x01)
        {
            stagePlan->m_nDeleteReal = 1;//删除计划（物理）
        }
        //始发终到标志
        stagePlan->m_btBeginOrEndFlg = (int)StatusArray[nCount];//[11]
        nCount += 1;

        //原到达车次号
        strCheCi="";
        nameLen = (int)StatusArray[nCount];
        nCount += 1;
        unsigned char strRecvDDCC[20];
        memset(strRecvDDCC,0,sizeof(strRecvDDCC));
        memcpy(strRecvDDCC, &StatusArray[nCount], nameLen);
        strCheCi = UnsignedCharArrayToString(strRecvDDCC);
        stagePlan->m_strReachTrainNumOld = strCheCi;
        nCount += nameLen;

        //新到达车次号
        strCheCi="";
        nameLen = (int)StatusArray[nCount];
        nCount += 1;
        unsigned char strRecvNEWCC[20];
        memset(strRecvNEWCC,0,sizeof(strRecvNEWCC));
        memcpy(strRecvNEWCC, &StatusArray[nCount], nameLen);
        strCheCi = UnsignedCharArrayToString(strRecvNEWCC);
        stagePlan->m_strReachTrainNum = strCheCi;
        nCount += nameLen;

        //接车口
        stagePlan->m_nCodeReachStaEquip =(int)(StatusArray[nCount] | (StatusArray[nCount+1]<<8));
        stagePlan->m_strXHD_JZk = this->GetStrNameByCode(stagePlan->m_nCodeReachStaEquip);
        nCount += 2;

        //接车股道
        stagePlan->m_nRecvTrainTrack = (int)(StatusArray[nCount] | (StatusArray[nCount+1]<<8));
        nCount += 2;

        //计划到达时间
        {
            int year = (int)(StatusArray[nCount] | StatusArray[nCount+1]<<8);
            int mouth = (int)StatusArray[nCount+2];
            int day = (int)StatusArray[nCount+3];
            int hour = (int)StatusArray[nCount+4];
            int mini = (int)StatusArray[nCount+5];
            int second = (int)StatusArray[nCount+6];
            QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
            QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
            stagePlan->m_timProvReachStation = dateTime;
        }
        nCount += 7;

        //原出发车次号
        strCheCi="";
        nameLen = (int)StatusArray[nCount];
        nCount += 1;
        unsigned char strRecvCFCC[20];
        memset(strRecvCFCC,0,sizeof(strRecvCFCC));
        memcpy(strRecvCFCC, &StatusArray[nCount], nameLen);
        strCheCi = UnsignedCharArrayToString(strRecvCFCC);
        stagePlan->m_strDepartTrainNumOld = strCheCi;
        nCount += nameLen;

        //新出发车次号
        strCheCi="";
        nameLen = (int)StatusArray[nCount];
        nCount += 1;
        unsigned char strRecvNEWCF[20];
        memset(strRecvNEWCF,0,sizeof(strRecvNEWCF));
        memcpy(strRecvNEWCF, &StatusArray[nCount], nameLen);
        strCheCi = UnsignedCharArrayToString(strRecvNEWCF);
        stagePlan->m_strDepartTrainNum = strCheCi;
        nCount += nameLen;

        //发车口
        stagePlan->m_nCodeDepartStaEquip = (int)(StatusArray[nCount] | (StatusArray[nCount+1]<<8));
        stagePlan->m_strXHD_CZk = this->GetStrNameByCode(stagePlan->m_nCodeDepartStaEquip);
        nCount += 2;

        //出发股道
        stagePlan->m_nDepartTrainTrack = (int)(StatusArray[nCount] | (StatusArray[nCount+1]<<8));
        nCount += 2;
        //出发时间
        {
            int year = (StatusArray[nCount] | StatusArray[nCount+1]<<8);
            int mouth = StatusArray[nCount+2];
            int day = StatusArray[nCount+3];
            int hour = StatusArray[nCount+4];
            int mini = StatusArray[nCount+5];
            int second = StatusArray[nCount+6];
            QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
            QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
            stagePlan->m_timProvDepaTrain = dateTime;
        }
        nCount += 7;

        //电力机车
        stagePlan->m_bElectric = (int)StatusArray[nCount] == 0x11 ? TRUE : FALSE;
        nCount += 1;

        //超限
        stagePlan->m_nLevelCX = (int)StatusArray[nCount];
        nCount += 1;

        //列车类型序号（管内动车组、通勤列车等）
        stagePlan->m_nIndexLCLX = (int)StatusArray[nCount];
        stagePlan->m_strLCLX = GetTrainType(stagePlan->m_nIndexLCLX);
        nCount += 1;

        //运行类型序号（动车组、快速旅客列车等）
        stagePlan->m_nIndexYXLX = (int)StatusArray[nCount];
        stagePlan->m_strYXLX = GetTrainRunType(stagePlan->m_nIndexYXLX);
        nCount += 1;

        qDebug() << "recive" << stagePlan->m_nIndexLCLX << stagePlan->m_strLCLX << stagePlan->m_nIndexYXLX << stagePlan->m_strYXLX;
        QString strLHFlg = GetLHFlg(stagePlan->m_nIndexLCLX);
        if(strLHFlg == "客车")
            stagePlan->m_nLHFlg = LCTYPE_KC;
        else stagePlan->m_nLHFlg = LCTYPE_HC;

        //计划删除标志[74]
        stagePlan->m_bDeleteFlag = (int)StatusArray[nCount] == 0x11 ? TRUE : FALSE;
        nCount += 1;
        //办理客运
        stagePlan->m_bBLKY = (int)StatusArray[nCount] == 0x01 ? TRUE : FALSE;
        nCount += 1;
        //重点
        stagePlan->m_bImportant = (int)StatusArray[nCount] == 0x01 ? TRUE : FALSE;
        nCount += 1;
        //军运
        stagePlan->m_bArmy = (int)StatusArray[nCount] == 0x01 ? TRUE : FALSE;
        nCount += 1;
        //允许股道与固定径路不同
        stagePlan->m_bAllowGDNotMatch = (int)StatusArray[nCount] == 0x01 ? TRUE : FALSE;
        nCount += 1;
        //允许出入口与固定进路不同
        stagePlan->m_bAllowCRKNotMatch = (int)StatusArray[nCount] == 0x01 ? TRUE : FALSE;
        nCount += 1;

        stagePlan->m_btLJStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btJALStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btJPStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btLWStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btJCStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btHJStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btCJStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btSSStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btZGStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btHCStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btZXStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btXWStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btDKStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btCHStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btZWStatus = (int)StatusArray[nCount];
        nCount += 1;
        stagePlan->m_btZKStatus = (int)StatusArray[nCount];
        nCount += 1;
        this->m_ArrayStagePlanChg.append(stagePlan);
    }
#pragma endregion//占线板修改后的阶段计划
}
//自动处理接收到的CTC/占线板修改后的计划
void MyStation::AutoRecvModifiedPlan()
{
    if(this->m_ArrayStagePlanChg.count() > 0)
    {
        //互斥锁，作用域结束自动解锁
        QMutexLocker locker(&Mutex);
        for (int i = 0; i < this->m_ArrayStagePlanChg.count(); i++)
        {
            StagePlan *pStagePlanChg = (StagePlan *)this->m_ArrayStagePlanChg[i];
            //更新计划进路信息
            ModifyTrainRouteOrder(pStagePlanChg);
            //更新计划日志信息
            ModifyTrafficLog(pStagePlanChg);
        }
        //清空
        this->m_ArrayStagePlanChg.clear();
    }
}
//更新修改的计划进路信息
void MyStation::ModifyTrainRouteOrder(StagePlan *pStagePlanChg)
{
    TrainRouteOrder* pTrainRouteOrder;
    m_pDataAccess->SelectAllRouteOrder(this);
    int nRouteOrderSize = this->m_ArrayRouteOrder.size();
    for (int i = 0; i < nRouteOrderSize; i++)
    {
        pTrainRouteOrder = this->m_ArrayRouteOrder[i];
        if (pTrainRouteOrder->m_strTrainNum == pStagePlanChg->m_strReachTrainNumOld
            || pTrainRouteOrder->m_strTrainNum == pStagePlanChg->m_strDepartTrainNumOld)
        {
            if(pTrainRouteOrder->m_nOldRouteState != 0)
            {
                return;//仅可修改“等待”状态的进路
            }
            //删除数据库
            m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
            //处理和发送1个数据
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
            m_pDataAccess->SelectAllRouteOrder(this);
            nRouteOrderSize = this->m_ArrayRouteOrder.count();
            i--;
        }
    }
    m_pDataAccess->SelectAllRouteOrder(this);
    //非删除计划时增加
    if(pStagePlanChg->m_nDeleteReal==0)
    {
        AddNewRouteOrder(pStagePlanChg);
    }
}
//void MyStation::ModifyTrainRouteOrder2(StagePlan *pStagePlanChg)
//{
//    TrainRouteOrder* pTrainRouteOrder;
//    bool bNew = true;
//    m_pDataAccess->SelectAllRouteOrder(this);
//    int nRouteOrderSize = this->m_ArrayRouteOrder.count();

//    for (int i = 0; i < nRouteOrderSize; i++)
//    {
//        pTrainRouteOrder = this->m_ArrayRouteOrder[i];
//        qDebug() << "ModifyTrainRouteOrder"<< nRouteOrderSize << pTrainRouteOrder->route_id << pTrainRouteOrder->m_btRecvOrDepart << pTrainRouteOrder->m_btBeginOrEndFlg;
//        //接车进路 && 车次和行车日志接车车次一致
//        if ((ROUTE_JC == pTrainRouteOrder->m_btRecvOrDepart) && (pTrainRouteOrder->m_strTrainNum == pStagePlanChg->m_strReachTrainNumOld))
//        {
////            if(pStagePlanChg->m_bDeleteFlag)
////            {
////                this->sendOneTrainRouteOrderToSoft(pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
////                m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
////                this->m_ArrayRouteOrder.removeAt(i);
////                nRouteOrderSize = this->m_ArrayRouteOrder.count();
////                i--;
////                continue;
////            }

//            //既有车次计划变更为始发，则查找并删除接车进路
//            if(pStagePlanChg->m_btBeginOrEndFlg == JFC_TYPE_SF)
//            {
//                //删除数据库
//                m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
//                //处理和发送1个数据
//                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);

//                this->m_ArrayRouteOrder.clear();
//                m_pDataAccess->SelectAllRouteOrder(this);
//                nRouteOrderSize = this->m_ArrayRouteOrder.count();
//                i--;
//                continue;
//            }

//            pTrainRouteOrder->m_btBeginOrEndFlg = pStagePlanChg->m_btBeginOrEndFlg;//所属阶段计划标志
//            pTrainRouteOrder->m_strTrainNum = pStagePlanChg->m_strReachTrainNum;
//            pTrainRouteOrder->m_nTrackCode = pStagePlanChg->m_nRecvTrainTrack;
//            pTrainRouteOrder->m_strTrack = this->GetStrNameByCode(pStagePlanChg->m_nRecvTrainTrack);
//            pTrainRouteOrder->m_nGDPos = this->GetGDPosInzcArray(pStagePlanChg->m_nRecvTrainTrack);
//            pTrainRouteOrder->m_nCodeReachStaEquip = pStagePlanChg->m_nCodeReachStaEquip;
//            pTrainRouteOrder->m_strXHD_JZk = GetStrNameByCode(pStagePlanChg->m_nCodeReachStaEquip);
//            pTrainRouteOrder->m_strDirection = this->GetDirectByCode(pStagePlanChg->m_nCodeReachStaEquip, 0);//获取进路方向
//            pTrainRouteOrder->m_timBegin = pStagePlanChg->m_timProvReachStation;
//            pTrainRouteOrder->m_timPlanned = pStagePlanChg->m_timProvReachStation;
//            pTrainRouteOrder->m_bElectric = pStagePlanChg->m_bElectric;
//            pTrainRouteOrder->m_nLevelCX = pStagePlanChg->m_nLevelCX;
//            pTrainRouteOrder->m_nLHFlg = pStagePlanChg->m_nLHFlg;
//            //获取进路描述
//            int nCode = this->GetCodeByRecvEquipGD(pStagePlanChg->m_nCodeReachStaEquip,
//                pStagePlanChg->m_nRecvTrainTrack);
//            pTrainRouteOrder->m_strXHDBegin = this->GetStrNameByCode(pStagePlanChg->m_nCodeReachStaEquip);
//            pTrainRouteOrder->m_strXHDEnd = this->GetStrNameByCode(nCode);
//            pTrainRouteOrder->m_strRouteDescrip = pTrainRouteOrder->m_strXHDBegin + "-" + pTrainRouteOrder->m_strXHDEnd;
//            pTrainRouteOrder->m_strRouteDescripReal = pTrainRouteOrder->m_strXHDBegin + "," + pTrainRouteOrder->m_strXHDEnd;
//            pTrainRouteOrder->m_bDeleteFlag = pStagePlanChg->m_bDeleteFlag;
//            //判断延续进路
//            this->InitRouteYXJL(pTrainRouteOrder);
//            bNew = false;
//        }
//        //发车进路 && 车次和行车日志发车车次一致
//        if ((ROUTE_FC == pTrainRouteOrder->m_btRecvOrDepart) && (pTrainRouteOrder->m_strTrainNum == pStagePlanChg->m_strDepartTrainNumOld))
//        {
////            if(pStagePlanChg->m_bDeleteFlag)
////            {
////                this->sendOneTrainRouteOrderToSoft(pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
////                m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
////                this->m_ArrayRouteOrder.removeAt(i);
////                nRouteOrderSize = this->m_ArrayRouteOrder.count();
////                i--;
////                continue;
////            }

//            //既有车次计划变更为终到，则查找并删除发车进路
//            qDebug() << "DeleteRouteOrder" << pStagePlanChg->plan_id << pStagePlanChg->m_btBeginOrEndFlg;
//            if(pStagePlanChg->m_btBeginOrEndFlg == JFC_TYPE_ZD)
//            {
//                //删除数据库

//                m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
//                //处理和发送1个数据
//                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
//                //this->m_ArrayRouteOrder.removeAt(i);
//                this->m_ArrayRouteOrder.clear();
//                m_pDataAccess->SelectAllRouteOrder(this);
//                nRouteOrderSize = this->m_ArrayRouteOrder.count();
//                i--;

//                continue;
//            }

//            pTrainRouteOrder->m_btBeginOrEndFlg = pStagePlanChg->m_btBeginOrEndFlg;//所属阶段计划标志
//            pTrainRouteOrder->m_strTrainNum = pStagePlanChg->m_strDepartTrainNum;
//            pTrainRouteOrder->m_bElectric = pStagePlanChg->m_bElectric;
//            pTrainRouteOrder->m_nLevelCX = pStagePlanChg->m_nLevelCX;
//            //获取股道号
//            pTrainRouteOrder->m_nTrackCode = pStagePlanChg->m_nDepartTrainTrack;
//            pTrainRouteOrder->m_strTrack = this->GetStrNameByCode(pStagePlanChg->m_nDepartTrainTrack);
//            pTrainRouteOrder->m_nGDPos = this->GetGDPosInzcArray(pStagePlanChg->m_nDepartTrainTrack);
//            pTrainRouteOrder->m_nCodeDepartStaEquip = pStagePlanChg->m_nCodeDepartStaEquip;
//            pTrainRouteOrder->m_strXHD_CZk = GetStrNameByCode(pStagePlanChg->m_nCodeDepartStaEquip);
//            //获取进路方向
//            pTrainRouteOrder->m_strDirection = this->GetDirectByCode(pStagePlanChg->m_nCodeDepartStaEquip, 1);
//            pTrainRouteOrder->m_timBegin = pStagePlanChg->m_timProvDepaTrain;
//            pTrainRouteOrder->m_timPlanned = pStagePlanChg->m_timProvDepaTrain;
//            //获取进路描述
//            int nCode = this->GetCodeByRecvEquipGD(pStagePlanChg->m_nCodeDepartStaEquip,
//                pStagePlanChg->m_nDepartTrainTrack);
//            pTrainRouteOrder->m_strXHDBegin = this->GetStrNameByCode(nCode);
//            pTrainRouteOrder->m_strXHDEnd = this->GetStrNameByCode(pStagePlanChg->m_nCodeDepartStaEquip);
//            pTrainRouteOrder->m_strRouteDescrip = pTrainRouteOrder->m_strXHDBegin + "-" + pTrainRouteOrder->m_strXHDEnd;
//            pTrainRouteOrder->m_strRouteDescripReal = pTrainRouteOrder->m_strXHDBegin + "," + pTrainRouteOrder->m_strXHDEnd;
//            pTrainRouteOrder->m_bDeleteFlag = pStagePlanChg->m_bDeleteFlag;
//            pTrainRouteOrder->m_nLHFlg = pStagePlanChg->m_nLHFlg;

//            bNew = false;
//        }
//        //初始化变通进路
//        this->InitRouteBtjl(pTrainRouteOrder);
//        if(!bNew)
//        {
//            m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
//            m_pDataAccess->SelectAllRouteOrder(this);
//            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
//        }
//    }
//    if(bNew)
//    {
//        AddNewRouteOrder(pStagePlanChg);
//    }
//}

//更新修改的计划日志信息
void MyStation::ModifyTrafficLog(StagePlan *pStagePlanChg)
{
    bool bNew = true;
    TrafficLog* pTrafficLog;
    for (int i = 0; i < this->m_ArrayTrafficLog.count(); i++)
    {
        pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[i];
        if ((pTrafficLog->m_strReachTrainNum == pStagePlanChg->m_strReachTrainNumOld && pStagePlanChg->m_strReachTrainNumOld != "")
            || (pTrafficLog->m_strDepartTrainNum == pStagePlanChg->m_strDepartTrainNumOld && pStagePlanChg->m_strDepartTrainNumOld != ""))
        {
            //实际删除计划
            if(pStagePlanChg->m_nDeleteReal==1)
            {
                m_pDataAccess->DeleteTrafficLog(pTrafficLog);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_DELETE,1,1);
                qDebug() << "send TrafficLog delete" << pTrafficLog->m_strReachTrainNum << pTrafficLog->m_strDepartTrainNum;
                this->m_ArrayTrafficLog.removeAt(i);
                return;
            }
            if(TRUE == pStagePlanChg->m_bDeleteFlag)
            {
                pTrafficLog->m_nExecuteFlag = 1;
                pTrafficLog->m_bDeleteFlag = pStagePlanChg->m_bDeleteFlag;
            }
            pTrafficLog->m_btBeginOrEndFlg = pStagePlanChg->m_btBeginOrEndFlg;
            pTrafficLog->m_nLHFlg = pStagePlanChg->m_nLHFlg;
            pTrafficLog->m_nIndexLCLX = pStagePlanChg->m_nIndexLCLX;
            pTrafficLog->m_nIndexYXLX = pStagePlanChg->m_nIndexYXLX;
            pTrafficLog->m_strLCLX = pStagePlanChg->m_strLCLX;
            pTrafficLog->m_strYXLX = pStagePlanChg->m_strYXLX;
            //添加到达列车指令/
            pTrafficLog->m_strReachTrainNum = pStagePlanChg->m_strReachTrainNum;
            pTrafficLog->m_strReachTrainNumOld = pStagePlanChg->m_strReachTrainNumOld;
            pTrafficLog->m_strXHD_JZk = pStagePlanChg->m_strXHD_JZk;
            pTrafficLog->m_nCodeReachStaEquip = pStagePlanChg->m_nCodeReachStaEquip;//进站口信号设备号
            pTrafficLog->m_bReachTrainNumSX = this->GetSXByCode(pTrafficLog->m_nCodeReachStaEquip, 0);
            pTrafficLog->m_strFromAdjtStation = this->GetJFCKDirectByCode(pStagePlanChg->m_nCodeReachStaEquip);
            pTrafficLog->m_strRecvTrainTrack = this->GetStrNameByCode(pStagePlanChg->m_nRecvTrainTrack);
            if (pStagePlanChg->m_btBeginOrEndFlg == 0xBB)//始发（终到）标志位  0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
            {
                pTrafficLog->m_strRecvTrainTrack = "";
            }
            pTrafficLog->m_timProvReachStation = pStagePlanChg->m_timProvReachStation;


            //添加出发列车指令/
            pTrafficLog->m_strDepartTrainNum = pStagePlanChg->m_strDepartTrainNum;
            pTrafficLog->m_strDepartTrainNumOld = pStagePlanChg->m_strDepartTrainNumOld;
            pTrafficLog->m_strXHD_CZk = pStagePlanChg->m_strXHD_CZk;
            pTrafficLog->m_nCodeDepartStaEquip = pStagePlanChg->m_nCodeDepartStaEquip;//出站口信号设备号
            pTrafficLog->m_bDepartTrainNumSX = this->GetSXByCode(pTrafficLog->m_nCodeDepartStaEquip, 1);
            pTrafficLog->m_strToAdjtStation = this->GetJFCKDirectByCode(pStagePlanChg->m_nCodeDepartStaEquip);
            pTrafficLog->m_strDepartTrainTrack = this->GetStrNameByCode(pStagePlanChg->m_nDepartTrainTrack);
            if (pStagePlanChg->m_btBeginOrEndFlg == 0xCC)//始发（终到）标志位  0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
            {
                pTrafficLog->m_strDepartTrainTrack = "";
            }
            pTrafficLog->m_timProvDepaTrain = pStagePlanChg->m_timProvDepaTrain;


            if (pTrafficLog->m_btBeginOrEndFlg == 0xBB)
            {
                pTrafficLog->m_strTypeFlag = "始发";
            }
            else if (pTrafficLog->m_btBeginOrEndFlg == 0xCC)
            {
                pTrafficLog->m_strTypeFlag = "终到";
            }
            else if (pTrafficLog->m_btBeginOrEndFlg == 0xDD)
            {
                pTrafficLog->m_strTypeFlag = "通过";
            }
            pTrafficLog->m_bElectric = pStagePlanChg->m_bElectric;
            pTrafficLog->m_bAllowGDNotMatch = pStagePlanChg->m_bAllowGDNotMatch;
            pTrafficLog->m_bAllowCRKNotMatch = pStagePlanChg->m_bAllowCRKNotMatch;
            pTrafficLog->m_bBLKY = pStagePlanChg->m_bBLKY;//办理客运
            pTrafficLog->m_bArmy = pStagePlanChg->m_bArmy;
            pTrafficLog->m_bImportant = pStagePlanChg->m_bImportant;
            pTrafficLog->m_bDeleteFlag = pStagePlanChg->m_bDeleteFlag;
            //new 20180823
            //pTrafficLog->m_strTrainNum = pTrafficLog->m_strReachTrainNum != _T("") ? pTrafficLog->m_strReachTrainNum : pTrafficLog->m_strDepartTrainNum;//new
            //pTrafficLog->m_strTrainTrack = pTrafficLog->m_strRecvTrainTrack != _T("") ? pTrafficLog->m_strRecvTrainTrack : pTrafficLog->m_strDepartTrainTrack;//new
            //pTrafficLog->m_strFromWhere = GetJFCKDirectByCode(pStagePlanChg->m_nCodeReachStaEquip);// _T("");
            //if (pTrafficLog->m_btBeginOrEndFlg == 0xBB)//"始发"
            //{
            //	pTrafficLog->m_strFromWhere = pFrame->StaConfigInfo.strStaName;
            //}
            //pTrafficLog->m_strToWhere = GetJFCKDirectByCode(pStagePlanChg->m_nCodeDepartStaEquip);// _T("");
            //if (pTrafficLog->m_btBeginOrEndFlg == 0xCC)//"终到"
            //{
            //	pTrafficLog->m_strToWhere = pFrame->StaConfigInfo.strStaName;
            //}
            pTrafficLog->m_nLevelCX = pStagePlanChg->m_nLevelCX;
            pTrafficLog->m_strDDCX = GetChaoXianLevel(pTrafficLog->m_nLevelCX);
            pTrafficLog->m_strCFCX = GetChaoXianLevel(pTrafficLog->m_nLevelCX);
            pTrafficLog->m_btLJStatus = pStagePlanChg->m_btLJStatus;
            pTrafficLog->m_btJALStatus =pStagePlanChg->m_btJALStatus;
            pTrafficLog->m_btJPStatus = pStagePlanChg->m_btJPStatus;
            pTrafficLog->m_btLWStatus = pStagePlanChg->m_btLWStatus;
            pTrafficLog->m_btJCStatus = pStagePlanChg->m_btJCStatus;
            pTrafficLog->m_btHJStatus = pStagePlanChg->m_btHJStatus;
            pTrafficLog->m_btCJStatus = pStagePlanChg->m_btCJStatus;
            pTrafficLog->m_btSSStatus = pStagePlanChg->m_btSSStatus;
            pTrafficLog->m_btZGStatus = pStagePlanChg->m_btZGStatus;
            pTrafficLog->m_btHCStatus = pStagePlanChg->m_btHCStatus;
            pTrafficLog->m_btZXStatus = pStagePlanChg->m_btZXStatus;
            pTrafficLog->m_btXWStatus = pStagePlanChg->m_btXWStatus;
            pTrafficLog->m_btDKStatus = pStagePlanChg->m_btDKStatus;
            pTrafficLog->m_btCHStatus = pStagePlanChg->m_btCHStatus;
            pTrafficLog->m_btZWStatus = pStagePlanChg->m_btZWStatus;
            pTrafficLog->m_btZKStatus = pStagePlanChg->m_btZKStatus;

            bNew = FALSE;
            if(!bNew)
            {
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //m_pDataAccess->SelectAllTrafficLog(this);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                qDebug() << "send TrafficLog updata" << pTrafficLog->m_nIndexLCLX << pTrafficLog->m_nIndexYXLX;
            }
            break;
        }
    }
    if(bNew)
    {
        AddNewTrafficLog(pStagePlanChg);
    }
}
//根据新的计划增加相应的进路信息
void MyStation::AddNewRouteOrder(StagePlan *pStagePlanChg)
{
    TrainRouteOrder* pRecvTrainRouteOrder = new TrainRouteOrder;
    TrainRouteOrder* pDepartTrainRouteOrder = new TrainRouteOrder;
//    if (this->m_nFCZKMode)//非常站控模式下，阶段计划不自动触发
//    {
//        pRecvTrainRouteOrder->m_nAutoTouch = false;
//        pDepartTrainRouteOrder->m_nAutoTouch = false;
//    }
//    else
//    {
//        ////中心和车站调车模式下为自动触发
//        //if (StationModalSelect.nModeState != 1)
//        //{
//        //	pRecvTrainRouteOrder->m_nAutoTouch = TRUE;
//        //	pDepartTrainRouteOrder->m_nAutoTouch = TRUE;
//        //}
//        ////车站控制模式下为人工触发，
//        //else if (StationModalSelect.nModeState == 1)
//        {
//            pRecvTrainRouteOrder->m_nAutoTouch = false;
//            pDepartTrainRouteOrder->m_nAutoTouch = false;
//        }
//    }

    //if ((((this->GetIndexInTrafficArray(pStagePlanChg->m_strReachTrainNum) == -1) && (pStagePlanChg->m_strReachTrainNum != ""))
    //    || (pStagePlanChg->m_strReachTrainNum == ""))
    //    &&
    //    (((this->GetIndexInTrafficArray(pStagePlanChg->m_strDepartTrainNum) == -1) && (pStagePlanChg->m_strDepartTrainNum != ""))
    //    || (pStagePlanChg->m_strDepartTrainNum == ""))
    //    )
    {
        if (pStagePlanChg->m_strReachTrainNum != "")
        {
            pRecvTrainRouteOrder->station_id = getStationID();
            pRecvTrainRouteOrder->m_btRecvOrDepart = ROUTE_JC;// 0x00;
            pRecvTrainRouteOrder->m_btBeginOrEndFlg = pStagePlanChg->m_btBeginOrEndFlg;//所属阶段计划标志
            pRecvTrainRouteOrder->m_strTrainNum = pStagePlanChg->m_strReachTrainNum;
            pRecvTrainRouteOrder->m_strTrainNumOld = pStagePlanChg->m_strReachTrainNumOld;
            pRecvTrainRouteOrder->m_strTrack = this->GetStrNameByCode(pStagePlanChg->m_nRecvTrainTrack);
            pRecvTrainRouteOrder->m_nTrackCode = pStagePlanChg->m_nRecvTrainTrack;
            pRecvTrainRouteOrder->m_nGDPos = this->GetGDPosInzcArray(pRecvTrainRouteOrder->m_nTrackCode);
            pRecvTrainRouteOrder->m_nCodeReachStaEquip = pStagePlanChg->m_nCodeReachStaEquip;
            pRecvTrainRouteOrder->m_strXHD_JZk = pStagePlanChg->m_strXHD_JZk;
            pRecvTrainRouteOrder->m_nLHFlg = pStagePlanChg->m_nLHFlg;
            pRecvTrainRouteOrder->m_bElectric = pStagePlanChg->m_bElectric;
            pRecvTrainRouteOrder->m_bDeleteFlag = pStagePlanChg->m_bDeleteFlag;
            pRecvTrainRouteOrder->m_nLevelCX = pStagePlanChg->m_nLevelCX;
            //获取进路方向
            pRecvTrainRouteOrder->m_strDirection = this->GetDirectByCode(pRecvTrainRouteOrder->m_nCodeReachStaEquip, 0);
            pRecvTrainRouteOrder->m_timBegin = pStagePlanChg->m_timProvReachStation;
            pRecvTrainRouteOrder->m_timPlanned = pStagePlanChg->m_timProvReachStation;
            //获取进路描述
            int nCode = this->GetCodeByRecvEquipGD(pRecvTrainRouteOrder->m_nCodeReachStaEquip, pRecvTrainRouteOrder->m_nTrackCode,0);
            pRecvTrainRouteOrder->m_strXHDBegin = this->GetStrNameByCode(pRecvTrainRouteOrder->m_nCodeReachStaEquip);
            pRecvTrainRouteOrder->m_strXHDEnd = this->GetStrNameByCode(nCode);
            //pRecvTrainRouteOrder->m_strRouteDescrip =  GetStrNameByCode(pDoc->m_StagePlanMemery->m_nCodeReachStaEquip) + _T("-") + GetStrNameByCode(nCode);
            pRecvTrainRouteOrder->m_strRouteDescrip = pRecvTrainRouteOrder->m_strXHDBegin + "-" + pRecvTrainRouteOrder->m_strXHDEnd;
            pRecvTrainRouteOrder->m_strRouteDescripReal = pRecvTrainRouteOrder->m_strXHDBegin + "," + pRecvTrainRouteOrder->m_strXHDEnd;
            //判断延续进路
            this->InitRouteYXJL(pRecvTrainRouteOrder);
            //初始化变通进路
            this->InitRouteBtjl(pRecvTrainRouteOrder);
            this->m_ArrayRouteOrder.append(pRecvTrainRouteOrder);
            //检查进路条件
            CheckTrainRouteOrder(pRecvTrainRouteOrder);
            //初始化组合进路
            this->InitRouteZhjl(pRecvTrainRouteOrder);
            //操作数据库
            m_pDataAccess->InsetRouteOrder(pRecvTrainRouteOrder);
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRecvTrainRouteOrder,SYNC_FLAG_ADD,1,1);
            //组合进路判断
            if(pRecvTrainRouteOrder->m_bZHJL)
            {
                for(int r=0; r<this->m_ArrayRouteOrderSonTemp.size(); r++)
                {
                    TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrderSonTemp[r];
                    //设置父id
                    pRouteOrderSon->father_id = pRecvTrainRouteOrder->route_id;
                    pRouteOrderSon->m_nAutoTouch = pRecvTrainRouteOrder->m_nAutoTouch;
                    //插入数据库
                    m_pDataAccess->InsetRouteOrder(pRouteOrderSon);
                    //处理和发送1个数据
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrderSon,SYNC_FLAG_ADD,1,1);
                }
                this->m_ArrayRouteOrderSonTemp.clear();
            }
            m_pDataAccess->SelectAllRouteOrder(this);
        }

        //添加出发列车指令
        if (pStagePlanChg->m_strDepartTrainNum != "")
        {
            pDepartTrainRouteOrder->station_id = getStationID();
            pDepartTrainRouteOrder->m_btRecvOrDepart = ROUTE_FC;// 0x01;
            pDepartTrainRouteOrder->m_btBeginOrEndFlg = pStagePlanChg->m_btBeginOrEndFlg;//所属阶段计划标志
            pDepartTrainRouteOrder->m_strTrainNum = pStagePlanChg->m_strDepartTrainNum;
            pDepartTrainRouteOrder->m_strTrainNumOld = pStagePlanChg->m_strDepartTrainNumOld;
            pDepartTrainRouteOrder->m_strTrack = this->GetStrNameByCode(pStagePlanChg->m_nDepartTrainTrack);
            pDepartTrainRouteOrder->m_nTrackCode = pStagePlanChg->m_nDepartTrainTrack;
            pDepartTrainRouteOrder->m_nGDPos = this->GetGDPosInzcArray(pDepartTrainRouteOrder->m_nTrackCode);
            pDepartTrainRouteOrder->m_nCodeDepartStaEquip = pStagePlanChg->m_nCodeDepartStaEquip;
            pDepartTrainRouteOrder->m_strXHD_CZk = pStagePlanChg->m_strXHD_CZk;
            pDepartTrainRouteOrder->m_nLHFlg = pStagePlanChg->m_nLHFlg;
            pDepartTrainRouteOrder->m_bElectric = pStagePlanChg->m_bElectric;
            pDepartTrainRouteOrder->m_bDeleteFlag = pStagePlanChg->m_bDeleteFlag;
            pDepartTrainRouteOrder->m_nLevelCX = pStagePlanChg->m_nLevelCX;
            //获取进路方向
            pDepartTrainRouteOrder->m_strDirection = this->GetDirectByCode(pDepartTrainRouteOrder->m_nCodeDepartStaEquip, 1);
            pDepartTrainRouteOrder->m_timBegin = pStagePlanChg->m_timProvDepaTrain;
            pDepartTrainRouteOrder->m_timPlanned = pStagePlanChg->m_timProvDepaTrain;
            //获取进路描述
            int nCode = this->GetCodeByRecvEquipGD(pDepartTrainRouteOrder->m_nCodeDepartStaEquip, pDepartTrainRouteOrder->m_nTrackCode,1);
            pDepartTrainRouteOrder->m_strXHDBegin = this->GetStrNameByCode(nCode);
            pDepartTrainRouteOrder->m_strXHDEnd = this->GetStrNameByCode(pDepartTrainRouteOrder->m_nCodeDepartStaEquip);
            //pDepartTrainRouteOrder->m_strRouteDescrip =  GetStrNameByCode(nCode) + _T("-") + GetStrNameByCode(pDoc->m_StagePlanMemery->m_nCodeDepartStaEquip);
            pDepartTrainRouteOrder->m_strRouteDescrip = pDepartTrainRouteOrder->m_strXHDBegin + "-" + pDepartTrainRouteOrder->m_strXHDEnd;
            pDepartTrainRouteOrder->m_strRouteDescripReal = pDepartTrainRouteOrder->m_strXHDBegin + "," + pDepartTrainRouteOrder->m_strXHDEnd;
            //初始化变通进路
            this->InitRouteBtjl(pDepartTrainRouteOrder);
            this->m_ArrayRouteOrder.append(pDepartTrainRouteOrder);
            //检查进路条件
            CheckTrainRouteOrder(pDepartTrainRouteOrder);
            //初始化组合进路
            this->InitRouteZhjl(pDepartTrainRouteOrder);
            //操作数据库
            m_pDataAccess->InsetRouteOrder(pDepartTrainRouteOrder);
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pDepartTrainRouteOrder,SYNC_FLAG_ADD,1,1);
            //组合进路判断
            if(pDepartTrainRouteOrder->m_bZHJL)
            {
                for(int r=0; r<this->m_ArrayRouteOrderSonTemp.size(); r++)
                {
                    TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrderSonTemp[r];
                    //设置父id
                    pRouteOrderSon->father_id = pDepartTrainRouteOrder->route_id;
                    pRouteOrderSon->m_nAutoTouch = pDepartTrainRouteOrder->m_nAutoTouch;
                    //插入数据库
                    m_pDataAccess->InsetRouteOrder(pRouteOrderSon);
                    //处理和发送1个数据
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRouteOrderSon,SYNC_FLAG_ADD,1,1);
                }
                this->m_ArrayRouteOrderSonTemp.clear();
            }
            m_pDataAccess->SelectAllRouteOrder(this);
        }
    }
}
//根据新的计划增加相应的行车日志
void MyStation::AddNewTrafficLog(StagePlan *pStagePlanChg)
{
    TrafficLog* pTrafficLog = new TrafficLog;
    //添加到达列车指令
    pTrafficLog->station_id = getStationID();
    pTrafficLog->m_nCodeReachStaEquip = pStagePlanChg->m_nCodeReachStaEquip;//new
    pTrafficLog->m_btBeginOrEndFlg = pStagePlanChg->m_btBeginOrEndFlg;
    pTrafficLog->m_strReachTrainNum = pStagePlanChg->m_strReachTrainNum;
    pTrafficLog->m_strReachTrainNumOld = pStagePlanChg->m_strReachTrainNumOld;
    pTrafficLog->m_strFromAdjtStation = this->GetJFCKDirectByCode(pStagePlanChg->m_nCodeReachStaEquip);
    pTrafficLog->m_strXHD_JZk = pStagePlanChg->m_strXHD_JZk;
    pTrafficLog->m_bReachTrainNumSX = this->GetSXByCode(pStagePlanChg->m_nCodeReachStaEquip, 0);//NEW
    pTrafficLog->m_strRecvTrainTrack = this->GetStrNameByCode(pStagePlanChg->m_nRecvTrainTrack);
    pTrafficLog->m_nLHFlg = pStagePlanChg->m_nLHFlg;
    pTrafficLog->m_bElectric = pStagePlanChg->m_bElectric;
    pTrafficLog->m_nLevelCX = pStagePlanChg->m_nLevelCX;
    pTrafficLog->m_strDDCX = GetChaoXianLevel(pTrafficLog->m_nLevelCX);
    pTrafficLog->m_strCFCX = GetChaoXianLevel(pTrafficLog->m_nLevelCX);
    if(pStagePlanChg->m_btBeginOrEndFlg == 0xBB)//始发（终到）标志位  0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
    {
        pTrafficLog->m_strRecvTrainTrack = "";
    }
    pTrafficLog->m_timProvReachStation = pStagePlanChg->m_timProvReachStation;
    pTrafficLog->m_nIndexLCLX = pStagePlanChg->m_nIndexLCLX;
    pTrafficLog->m_nIndexYXLX = pStagePlanChg->m_nIndexYXLX;
    pTrafficLog->m_strLCLX = pStagePlanChg->m_strLCLX;
    pTrafficLog->m_strYXLX = pStagePlanChg->m_strYXLX;

    //添加出发列车指令/
    pTrafficLog->m_strDepartTrainNum = pStagePlanChg->m_strDepartTrainNum;
    pTrafficLog->m_strReachTrainNumOld = pStagePlanChg->m_strReachTrainNumOld;
    pTrafficLog->m_strXHD_CZk = pStagePlanChg->m_strXHD_CZk;
    pTrafficLog->m_nCodeDepartStaEquip = pStagePlanChg->m_nCodeDepartStaEquip;
    pTrafficLog->m_bDepartTrainNumSX = this->GetSXByCode(pStagePlanChg->m_nCodeDepartStaEquip, 1);//NEW
    pTrafficLog->m_strToAdjtStation = this->GetJFCKDirectByCode(pStagePlanChg->m_nCodeDepartStaEquip);
    pTrafficLog->m_strDepartTrainTrack = this->GetStrNameByCode(pStagePlanChg->m_nDepartTrainTrack);
    if(pStagePlanChg->m_btBeginOrEndFlg == 0xCC)//始发（终到）标志位  0xAA正常(接发) 0xBB始发 0xCC终到 0xDD通过
    {
        pTrafficLog->m_strDepartTrainTrack = "";
    }
    pTrafficLog->m_timProvDepaTrain = pStagePlanChg->m_timProvDepaTrain;

    if(pTrafficLog->m_btBeginOrEndFlg == 0xBB)
    {
        pTrafficLog->m_strTypeFlag = "始发";
    }
    else if(pTrafficLog->m_btBeginOrEndFlg == 0xCC)
    {
        pTrafficLog->m_strTypeFlag = "终到";
    }
    else if(pTrafficLog->m_btBeginOrEndFlg == 0xDD)
    {
        pTrafficLog->m_strTypeFlag = "通过";
    }
    pTrafficLog->m_bBLKY = pStagePlanChg->m_bBLKY;
    pTrafficLog->m_bAllowGDNotMatch = pStagePlanChg->m_bAllowGDNotMatch;
    pTrafficLog->m_bAllowCRKNotMatch = pStagePlanChg->m_bAllowCRKNotMatch;
    pTrafficLog->m_bArmy = pStagePlanChg->m_bArmy;
    pTrafficLog->m_bImportant = pStagePlanChg->m_bImportant;
    pTrafficLog->m_bDeleteFlag = pStagePlanChg->m_bDeleteFlag;
    pTrafficLog->m_btLJStatus = pStagePlanChg->m_btLJStatus;
    pTrafficLog->m_btJALStatus =pStagePlanChg->m_btJALStatus;
    pTrafficLog->m_btJPStatus = pStagePlanChg->m_btJPStatus;
    pTrafficLog->m_btLWStatus = pStagePlanChg->m_btLWStatus;
    pTrafficLog->m_btJCStatus = pStagePlanChg->m_btJCStatus;
    pTrafficLog->m_btHJStatus = pStagePlanChg->m_btHJStatus;
    pTrafficLog->m_btCJStatus = pStagePlanChg->m_btCJStatus;
    pTrafficLog->m_btSSStatus = pStagePlanChg->m_btSSStatus;
    pTrafficLog->m_btZGStatus = pStagePlanChg->m_btZGStatus;
    pTrafficLog->m_btHCStatus = pStagePlanChg->m_btHCStatus;
    pTrafficLog->m_btZXStatus = pStagePlanChg->m_btZXStatus;
    pTrafficLog->m_btXWStatus = pStagePlanChg->m_btXWStatus;
    pTrafficLog->m_btDKStatus = pStagePlanChg->m_btDKStatus;
    pTrafficLog->m_btCHStatus = pStagePlanChg->m_btCHStatus;
    pTrafficLog->m_btZWStatus = pStagePlanChg->m_btZWStatus;
    pTrafficLog->m_btZKStatus = pStagePlanChg->m_btZKStatus;

    this->m_ArrayTrafficLog.append(pTrafficLog);
    //数据库操作
    //m_pDataAccess->UpdateTrafficLog(pTrafficLog);
    m_pDataAccess->InsetTrafficLog(pTrafficLog);
    m_pDataAccess->SelectAllTrafficLog(this);
    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_ADD,1,1);
    qDebug() << "send TrafficLog new" << pTrafficLog->m_nIndexLCLX << pTrafficLog->m_nIndexYXLX;
}
//自动检查和设置计划的完成标志
void MyStation::AutoCheckTrafficExecuteFlag()
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    for(int t = 0; t < this->m_ArrayTrafficLog.count(); t++)
    {
        TrafficLog* pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[t];
        if(pTrafficLog->m_nExecuteFlag == 0)
        {
            //已经报点的计划更新状态为执行，其余为未执行。
            //始发车，发车已报点  进路已出清
            if((pTrafficLog->m_btBeginOrEndFlg == JFC_TYPE_SF)
                && (1970 != pTrafficLog->m_timRealDepaTrain.date().year())
                && (0    != pTrafficLog->m_timRealDepaTrain.date().year())
                && this->RouteIsClear(ROUTE_FC, pTrafficLog->m_strDepartTrainNum))
            {
                pTrafficLog->m_nExecuteFlag = 1;
            }
            //终到车，到达已报点 进路已出清
            else if((pTrafficLog->m_btBeginOrEndFlg == JFC_TYPE_ZD)
                && (1970 != pTrafficLog->m_timRealReachStation.date().year())
                && (0    != pTrafficLog->m_timRealReachStation.date().year())
                && this->RouteIsClear(ROUTE_JC, pTrafficLog->m_strReachTrainNum))
            {
                pTrafficLog->m_nExecuteFlag = 1;
            }
            //接发和通过，都已报点 进路已出清
            else if((1970 != pTrafficLog->m_timRealReachStation.date().year())
                    && (0 != pTrafficLog->m_timRealReachStation.date().year())
                    && (1970 != pTrafficLog->m_timRealDepaTrain.date().year())
                    && (0    != pTrafficLog->m_timRealDepaTrain.date().year())
                && this->RouteIsClear(ROUTE_FC, pTrafficLog->m_strDepartTrainNum)
                && this->RouteIsClear(ROUTE_JC, pTrafficLog->m_strReachTrainNum))
            {
                pTrafficLog->m_nExecuteFlag = 1;
            }
            if(pTrafficLog->m_nExecuteFlag == 1)
            {
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
    }
}
//自动删除已出清的进路序列
void MyStation::AutoDeleleFinishedTrainRoute()
{
    //互斥锁
    QMutexLocker locker(&Mutex);

    QDateTime timeNow = QDateTime::currentDateTime();
    int size = this->m_ArrayRouteOrder.count();
    for(int i=0; i<size; i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //进路已经出清
        if(pTrainRouteOrder->m_nOldRouteState == 4)
        {
            //LONGLONG seconds = (timeNow - pTrainRouteOrder->m_timClean).GetTotalSeconds();
            qint64 seconds = pTrainRouteOrder->m_timClean.secsTo(timeNow);
            if(seconds > this->StaConfigInfo.RouteDeleteSeconds)
            {
                //发送同步消息
                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
                //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                //更新数据库
                m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
                this->m_ArrayRouteOrder.removeAt(i);
                size--;
            }
        }
        //组合进路的子序列全部删除时，主序列也删除
        if(pTrainRouteOrder->m_bZHJL)
        {
             if(nullptr == FindSonRouteOrderByFatherId(pTrainRouteOrder->route_id))
             {
                 //发送同步消息
                 this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
                 //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                 //更新数据库
                 m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
                 this->m_ArrayRouteOrder.removeAt(i);
                 return;
             }
        }
    }
}

//进路是否出清
bool MyStation::RouteIsClear(int _btJFC, QString strcheci)
{
    for(int i=0; i<this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder* )this->m_ArrayRouteOrder[i];
        if(pTrainRouteOrder->m_btRecvOrDepart == _btJFC && pTrainRouteOrder->m_strTrainNum == strcheci)
        {
            if(pTrainRouteOrder->m_nOldRouteState == 4)
            {
                return true;
            }
            return false;
        }
    }
    return true;
}


//更新进路序列的开始时间(type-0x11发车报点,0x22-停车报点,0x33通过报点)
void MyStation::UpdateRouteBeginTime(int type, QString strCheCi, QDateTime time)
{
    bool bUpdate = false;
    for(int i=0; i<this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = this->m_ArrayRouteOrder[i];
        //发车报点-对应发车进路
        if(type==0x11 && pTrainRouteOrder->m_strTrainNum == strCheCi
                && pTrainRouteOrder->m_btRecvOrDepart == ROUTE_FC)
        {
            pTrainRouteOrder->m_bReportTime = true;
            pTrainRouteOrder->m_timBegin = time;
            //开始时间比计划时间早，则为负值，负值表示早点
            qint64 minis = pTrainRouteOrder->m_timPlanned.secsTo(pTrainRouteOrder->m_timBegin)/60;
            pTrainRouteOrder->m_nOvertime = minis;
            qDebug()<<"m_timBegin="<<pTrainRouteOrder->m_timBegin.toString();
            qDebug()<<"m_timPlanned="<<pTrainRouteOrder->m_timPlanned.toString();
            updateTrainOvertime(strCheCi, pTrainRouteOrder->m_nOvertime);
            bUpdate = true;
            m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
            //发送同步消息
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
            break;
        }
        //停车报点-对应接车进路
        else if(type==0x22 && pTrainRouteOrder->m_strTrainNum == strCheCi
                && pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC)
        {
            pTrainRouteOrder->m_bReportTime = true;
            pTrainRouteOrder->m_timBegin = time;
            //开始时间比计划时间早，则为负值，负值表示早点
            qint64 minis = pTrainRouteOrder->m_timPlanned.secsTo(pTrainRouteOrder->m_timBegin)/60;
            pTrainRouteOrder->m_nOvertime = minis;
            qDebug()<<"m_timBegin="<<pTrainRouteOrder->m_timBegin.toString();
            qDebug()<<"m_timPlanned="<<pTrainRouteOrder->m_timPlanned.toString();
            updateTrainOvertime(strCheCi, pTrainRouteOrder->m_nOvertime);
            bUpdate = true;
            m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
            //发送同步消息
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
            break;
        }
        //通过报点-对应接车、发车进路
        else if(type==0x33 && pTrainRouteOrder->m_strTrainNum == strCheCi)
        {
            pTrainRouteOrder->m_bReportTime = true;
            pTrainRouteOrder->m_timBegin = time;
            //开始时间比计划时间早，则为负值，负值表示早点
            qint64 minis = pTrainRouteOrder->m_timPlanned.secsTo(pTrainRouteOrder->m_timBegin)/60;
            pTrainRouteOrder->m_nOvertime = minis;
            qDebug()<<"m_timBegin="<<pTrainRouteOrder->m_timBegin.toString();
            qDebug()<<"m_timPlanned="<<pTrainRouteOrder->m_timPlanned.toString();
            updateTrainOvertime(strCheCi, pTrainRouteOrder->m_nOvertime);
            bUpdate = true;
            m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
            //发送同步消息
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
        }
    }
    //更新列车早晚点
    if(!bUpdate)
    {
        //没有匹配进路序列时间，则根据行车日志时间更新列车早晚点
        updateTrainOvertimeByPlan(strCheCi, time);
    }
}
//设置进路序列的自触状态
void MyStation::SetRouteAutoTouchState(bool bAutoTouch)
{
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //等待状态的进路序列
        if(0 == pTrainRouteOrder->m_nOldRouteState)
        {
            //上一次的自触状态
            bool bOldAutoTouchState = pTrainRouteOrder->m_nAutoTouch;
            //设置自触
            if(bAutoTouch)
            {
                //检查站细，满足才可设置自触，否则不行
                CheckResult* ckResult1 = this->CheckPreventConditionStaDetails(pTrainRouteOrder);
                if(ckResult1->check==0)
                {
                    pTrainRouteOrder->m_nAutoTouch = true;
                }
                else
                {
                    pTrainRouteOrder->m_nAutoTouch = false;
                }
            }
            //取消自触
            else
            {
                pTrainRouteOrder->m_nAutoTouch = false;
            }
            //上一次和本次的状态不一致，则更新
            if(bOldAutoTouchState != pTrainRouteOrder->m_nAutoTouch)
            {
                m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                //发送同步消息
                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
            }
        }
    }
}
//进路表中的区段是否占用/进路表中是否有区段占用
bool MyStation::IsQDZYInLSB(int _IndexRoute, int &warn, QString &msg)
{
    if(_IndexRoute < 0 || _IndexRoute>=vectInterlockRoute.count())
    {
        return false;
    }

    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_IndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);
//    for(int i=0; i<pRut->QDArr.count(); i++)
//    {
//        pQDArray.append(pRut->QDArr[i]);
//    }

    int c = pQDArray.count();
    for(int i = 0; i < c; i++)
    {
        QString qdname = pQDArray[i];
        CGD *pGD;
        CGDDC *pGDDC;

        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            QString dcname2 = "";
            int pos = dcname.indexOf("-");
            if( pos > 0 )
            {
                QStringList strArr;
                QString ch = "-";
                StringSplit(dcname, ch, strArr);
                dcname = strArr[0];
                dcname2 = strArr[1];
            }
            for(int z=0; z< DevArray.count(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname == pGDDC->m_strName)
                    {
                        if(pGDDC->getQDState(QDZY))
                        {
                            warn |= JLWARNING_QDZY;
                            return true;
                        }
                    }
                }
            }
            for(int z=0; z< DevArray.count() && dcname2!=""; z++)
            {
                if (DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname2 == pGDDC->m_strName)
                    {
                        if(pGDDC->getQDState(QDZY))
                        {
                            warn |= JLWARNING_QDZY;
                            return true;
                        }
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z< DevArray.count(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        if(pGD->getState(QDZY))
                        {
                            warn |= JLWARNING_QDZY;
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}
//进路表中是否有区段拥有此状态 QDZY QDSB
bool MyStation::IsQDHaveStateInLSB(int _IndexRoute, int _state, int &warn, QString &msg)
{
    if(_IndexRoute < 0 || _IndexRoute>=vectInterlockRoute.count())
    {
        return false;
    }

    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_IndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);

    int c = pQDArray.count();
    for(int i = 0; i < c; i++)
    {
        QString qdname = pQDArray[i];
        CGD *pGD;
        CGDDC *pGDDC;

        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            QString dcname2 = "";
            int pos = dcname.indexOf("-");
            if( pos > 0 )
            {
                QStringList strArr;
                QString ch = "-";
                StringSplit(dcname, ch, strArr);
                dcname = strArr[0];
                dcname2 = strArr[1];
            }
            for(int z=0; z< DevArray.count(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname == pGDDC->m_strName)
                    {
                        if(pGDDC->getQDState(_state))
                        {
                            if(QDZY == _state)
                            {
                                warn |= JLWARNING_QDZY;
                            }
                            else if(QDSB == _state)
                            {
                                warn |= JLWARNING_QDSB;
                            }
                            msg = pGDDC->m_strName;
                            return true;
                        }
                    }
                }
            }
            for(int z=0; z< DevArray.count() && dcname2!=""; z++)
            {
                if (DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname2 == pGDDC->m_strName)
                    {
                        if(pGDDC->getQDState(_state))
                        {
                            if(QDZY == _state)
                            {
                                warn |= JLWARNING_QDZY;
                            }
                            else if(QDSB == _state)
                            {
                                warn |= JLWARNING_QDSB;
                            }
                            msg = pGDDC->m_strName;
                            return true;
                        }
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z< DevArray.count(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        if(pGD->getState(_state))
                        {
                            if(QDZY == _state)
                            {
                                warn |= JLWARNING_QDZY;
                            }
                            else if(QDSB == _state)
                            {
                                warn |= JLWARNING_QDSB;
                            }
                            msg = pGD->m_strName;
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}
//进路表中是否有区段是分录不良
bool MyStation::isQDHaveFLBLInLSB(int _IndexRoute, int &warn, QString &msg)
{
    if(_IndexRoute < 0 || _IndexRoute>=vectInterlockRoute.count())
    {
        return false;
    }

    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_IndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);

    int c = pQDArray.size();
    for(int i = 0; i < c; i++)
    {
        QString qdname = (QString)pQDArray[i];

        CGD *pGD;
        CGDDC *pGDDC;

        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            int pos = dcname.indexOf("-");
            if( pos > 0 )//示例：6-12DG
            {
                QStringList strArr;
                int dcCount = StringSplit(dcname, "-", strArr);
                for(int dc = 0; dc<dcCount; dc++)
                {
                    dcname = strArr[dc];
                    for(int z=0; z< DevArray.size(); z++)//获取站场大小
                    {
                        if (DevArray[z]->getDevType() == Dev_DC)
                        {
                            pGDDC=(CGDDC *)(DevArray[z]);
                            if(dcname == pGDDC->m_strName)
                            {
                                //分路不良,0无，1分路，2确认空闲
                                if(pGDDC->flblStatusCQ==1 || pGDDC->flblStatusDW==1 || pGDDC->flblStatusFW==1)
                                {
                                    warn |= JLWARNING_FLBL_DC;
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            else //示例：4DG
            {
                for(int z=0; z< DevArray.size(); z++)//获取站场大小
                {
                    if (DevArray[z]->getDevType() == Dev_DC)
                    {
                        pGDDC = (CGDDC *)(DevArray[z]);
                        if(dcname == pGDDC->m_strName)
                        {
                            //分路不良,0无，1分路，2确认空闲
                            if(pGDDC->flblStatusCQ==1 || pGDDC->flblStatusDW==1 || pGDDC->flblStatusFW==1)
                            {
                                warn |= JLWARNING_FLBL_DC;
                                return true;
                            }
                        }
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z< DevArray.size(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        //分路不良,0无，1分路，2确认空闲
                        if(pGD->flblStatus==1)
                        {
                            if(pGD->getGDType() == GD_QD)
                            {
                                warn |= JLWARNING_FLBL_GD;
                            }
                            else
                            {
                                warn |= JLWARNING_FLBL_WCQD;
                            }
                            return true;
                        }
                        //分路不良,0无，1分路，2确认空闲
                        if(pGD->flblStatus==2 && !GDFLBLKXAutoTouch)
                        {
                            if(pGD->getGDType() == GD_QD)
                            {
                                warn |= JLWARNING_FLBL_GDKX;
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}
//进路表中是否有区段是无电
bool MyStation::isQDHavePowerCutInLSB(int _IndexRoute, int &warn, QString &msg)
{
    if(_IndexRoute < 0 || _IndexRoute>=vectInterlockRoute.count())
    {
        return false;
    }
    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_IndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);

    int c = pQDArray.size();
    for(int i = 0; i <  c; i++)
    {
        QString qdname = (QString)pQDArray[i];
        CGD *pGD;
        CGDDC *pGDDC;

        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            int pos = dcname.indexOf("-");
            if( pos > 0 )//示例：6-12DG
            {
                QStringList strArr;
                int dcCount = StringSplit(dcname, "-", strArr);
                for(int dc = 0; dc<dcCount; dc++)
                {
                    dcname = strArr[dc];
                    for(int z=0; z<DevArray.size(); z++)
                    {
                        if (DevArray[z]->getDevType() == Dev_DC)
                        {
                            pGDDC=(CGDDC *)(DevArray[z]);
                            if(dcname == pGDDC->m_strName)
                            {
                                if(pGDDC->isPowerCutDW || pGDDC->isPowerCutFW || pGDDC->isPowerCutCQ)
                                {
                                    warn |= JLWARNING_QDPOWERCUT;
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            else //示例：4DG
            {
                for(int z=0; z<DevArray.size(); z++)
                {
                    if (DevArray[z]->getDevType() == Dev_DC)
                    {
                        pGDDC=(CGDDC *)(DevArray[z]);
                        if(dcname == pGDDC->m_strName)
                        {
                            if(pGDDC->isPowerCutDW || pGDDC->isPowerCutFW || pGDDC->isPowerCutCQ)
                            {
                                warn |= JLWARNING_QDPOWERCUT;
                                return true;
                            }
                        }
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z<DevArray.size(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        if(pGD->isPowerCut)//股道接触网无电
                        {
                            warn |= JLWARNING_QDPOWERCUT;
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}
//进路表中的区段是否都是此状态 QDKX QDSB
bool MyStation::IsQDStateInLSB(int _IndexRoute, int _state)
{
    if(_IndexRoute < 0 || _IndexRoute>=vectInterlockRoute.count())
    {
        return true;
    }

    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_IndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);
    int c = pQDArray.count();
    for(int i = 0; i < c; i++)
    {
        QString qdname = (QString)pQDArray[i];
        CGD *pGD;
        CGDDC *pGDDC;
        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            QString dcname2 = "";
            int pos = dcname.indexOf("-");
            if( pos > 0 )
            {
                QStringList strArr;
                StringSplit(dcname, "-", strArr);
                dcname = strArr[0];
                dcname2 = strArr[1];
            }
            for(int z=0; z< DevArray.size(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname == pGDDC->m_strName)
                    {
                        if(!pGDDC->getQDState(_state))
                        {
                            return false;
                        }
                    }
                }
            }
            for(int z=0; z< DevArray.size() && dcname2!=""; z++)
            {
                if (DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname2 == pGDDC->m_strName)
                    {
                        if(!pGDDC->getQDState(_state))
                        {
                            return false;
                        }
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z< DevArray.count(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        if(!pGD->getState(_state))
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}
//进路表中的区段是否封锁 FS
bool MyStation::IsQDFSInLSB(int _IndexRoute, int &warn, QString &msg)
{
    if(_IndexRoute < 0 || _IndexRoute>=vectInterlockRoute.count())
    {
        return false;
    }

    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_IndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);
    int c = pQDArray.count();
    for(int i = 0; i < c; i++)
    {
        QString qdname = (QString)pQDArray[i];
        int nArraySize =  DevArray.count();//获取站场大小
        CGD *pGD;
        CGDDC *pGDDC;
        if(qdname.right(2) == "DG")//道岔区段
        {
            QStringList dcNameArr;
            QString dcname = qdname.left(qdname.length() - 2);
            dcNameArr.append(dcname);
            int pos = dcname.indexOf("-");
            if( pos > 0 )
            {
                dcNameArr.clear();
                StringSplit(dcname, "-", dcNameArr);
            }
            for(int d=0; d<dcNameArr.count(); d++)
            {
                for(int z=0; z< DevArray.count(); z++)
                {
                    if (DevArray[z]->getDevType() == Dev_DC)
                    {
                        pGDDC=(CGDDC *)(DevArray[z]);
                        if(dcNameArr[d] == pGDDC->m_strName)
                        {
                            if(pGDDC->getIsFS())
                            {
                                warn |= JLWARNING_FS_DC;
                                msg = pGDDC->m_strName;//QString("道岔封锁");
                                return true;
                            }
                            ////道岔四开 或 封锁，不可办理
                            //if( pGDDC->getState(DCSK) || pGDDC->m_nFS)
                            //	return FALSE;
                        }
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z< DevArray.count(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        if(pGD->isLock)//区段封锁
                        {
                            warn |= JLWARNING_FS_GD;
                            msg = pGD->m_strName;//QString("区段封锁");
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}
//进路表中的道岔是否四开 DCSK
bool MyStation::IsQDDCSKInLSB(int _IndexRoute, int &warn, QString &msg)
{
    if(_IndexRoute < 0 || _IndexRoute>=vectInterlockRoute.count())
    {
        return false;
    }

    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_IndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);
    int c = pQDArray.count();
    for(int i = 0; i < c; i++)
    {
        QString qdname = (QString)pQDArray[i];
        //int nArraySize =  DevArray.count();//获取站场大小
        //CGD *pGD;
        CGDDC *pGDDC;
        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            QString dcname2 = "";
            int pos = dcname.indexOf("-");
            if( pos > 0 )
            {
                QStringList strArr;
                StringSplit(dcname, "-", strArr);
                dcname = strArr[0];
                dcname2 = strArr[1];
            }
            for(int z=0; z< DevArray.count(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname == pGDDC->m_strName)
                    {
                        //道岔四开，不可办理
                        if(pGDDC->getState(DCSK))
                        {
                            warn |= JLWARNING_DCSK;
                            return true;
                        }
                    }
                }
            }
            for(int z=0; z< DevArray.count() && dcname2 != ""; z++)
            {
                if (DevArray[z]->getDevType() == Dev_DC)
                {
                    pGDDC=(CGDDC *)(DevArray[z]);
                    if(dcname2 == pGDDC->m_strName)
                    {
                        //道岔四开，不可办理
                        if(pGDDC->getState(DCSK))
                        {
                            warn |= JLWARNING_DCSK;
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}
//进路表中的股道防溜是否清除
int MyStation::isGDFLClear(int _IndexRoute)
{
    if(_IndexRoute < 0 || _IndexRoute>=vectInterlockRoute.count())
    {
        return 0;
    }

    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_IndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);
    int c = pQDArray.count();
    for(int i = 0; i < c; i++)
    {
        QString qdname = (QString)pQDArray[i];
        //int nArraySize =  DevArray.count();//获取站场大小
        CGD *pGD;
        //CGDDC *pGDDC;
        if(qdname.right(2) == "DG")//道岔区段
        {
            continue;
        }
        else//区段、股道
        {
//            for(int z=0; z< DevArray.count(); z++)
//            {
//                if (DevArray[z]->getDevType() == Dev_GD)
//                {
//                    pGD=(CGD *)(DevArray[z]);
//                    if(qdname == pGD->m_strName)
//                    {
//                        if(pGD->m_nLAntiSlipType > 0 || pGD->m_nRAntiSlipType > 0)
//                        {
//                            return false;
//                        }
//                    }
//                }
//            }
            for (int f=0; f<m_ArrayGDAntiSlip.size(); f++)
            {
                pGD = m_ArrayGDAntiSlip[f];
                if(qdname == pGD->getName())
                {
                    if(pGD->m_nLAntiSlipType > 0 && pGD->m_nRAntiSlipType > 0)
                    {
                        return 3;//上行+下行
                    }
                    else if(pGD->m_nLAntiSlipType > 0)
                    {
                        if(StaConfigInfo.bStaSXLORR)//下行
                        {
                            return 2;//下行
                        }
                        else
                        {
                            return 1;//上行
                        }
                    }
                    else if(pGD->m_nRAntiSlipType > 0)
                    {
                        if(StaConfigInfo.bStaSXLORR)//下行
                        {
                            return 1;//上行
                        }
                        else
                        {
                            return 2;//下行
                        }
                    }
                }
            }
        }
    }

    return 0;
}
//检查股道防溜是否撤除,0无防溜，1上行，2下行，3上行+下行
int MyStation::CheckGDFL(QString gdName)
{
    int flType = 0;
    int devindex = GetIndexByStrName(gdName);
    if (devindex < 0)
        return 0;
    for (int i=0; i<m_ArrayGDAntiSlip.size(); i++)
    {
        CGD* pGD = m_ArrayGDAntiSlip[i];
        if(gdName == pGD->getName())
        {
            if(pGD->m_nLAntiSlipType > 0 && pGD->m_nRAntiSlipType > 0)
            {
                flType = 3;//上行+下行
            }
            else if(pGD->m_nLAntiSlipType > 0)
            {
                if(StaConfigInfo.bStaSXLORR)//下行
                {
                    flType = 2;//下行
                }
                else
                {
                    flType = 1;//上行
                }
            }
            else if(pGD->m_nRAntiSlipType > 0)
            {
                if(StaConfigInfo.bStaSXLORR)//下行
                {
                    flType = 1;//上行
                }
                else
                {
                    flType = 2;//下行
                }
            }
            break;
        }
    }

    return flType;
}
//进路表中的区段是否都分录不良空闲
bool MyStation::isQDFLBLKXInLSB(QStringList &pQDArray, int &warn, QString &msg)
{
    int c = pQDArray.size();
    for(int i = 0; i < c; i++)
    {
        QString qdname = (QString)pQDArray[i];

        CGD *pGD;
        CGDDC *pGDDC;

        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            int pos = dcname.indexOf("-");
            if( pos > 0 )//示例：6-12DG
            {
                QStringList strArr;
                int dcCount = StringSplit(dcname, "-", strArr);
                for(int dc = 0; dc<dcCount; dc++)
                {
                    dcname = strArr[dc];
                    for(int z=0; z< DevArray.size(); z++)//获取站场大小
                    {
                        if (DevArray[z]->getDevType() == Dev_DC)
                        {
                            pGDDC=(CGDDC *)(DevArray[z]);
                            if(dcname == pGDDC->m_strName)
                            {
                                //分路不良,0无，1分路，2确认空闲
                                if(pGDDC->flblStatusCQ==1 || pGDDC->flblStatusDW==1 || pGDDC->flblStatusFW==1)
                                {
                                    warn |= JLWARNING_FLBL_DC;
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
            else //示例：4DG
            {
                for(int z=0; z< DevArray.size(); z++)//获取站场大小
                {
                    if (DevArray[z]->getDevType() == Dev_DC)
                    {
                        pGDDC = (CGDDC *)(DevArray[z]);
                        if(dcname == pGDDC->m_strName)
                        {
                            //分路不良,0无，1分路，2确认空闲
                            if(pGDDC->flblStatusCQ==1 || pGDDC->flblStatusDW==1 || pGDDC->flblStatusFW==1)
                            {
                                warn |= JLWARNING_FLBL_DC;
                                return false;
                            }
                        }
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z< DevArray.size(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        //分路不良,0无，1分路，2确认空闲
                        if(pGD->flblStatus==1)
                        {
                            warn |= JLWARNING_FLBL_GD;
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}
//进路表中的区段是否都有电
bool MyStation::isQDPowerInLSB(QStringList &pQDArray, int &warn, QString &msg)
{
    int c = pQDArray.size();
    for(int i = 0; i <  c; i++)
    {
        QString qdname = (QString)pQDArray[i];

        CGD *pGD;
        CGDDC *pGDDC;

        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            int pos = dcname.indexOf("-");
            if( pos > 0 )//示例：6-12DG
            {
                QStringList strArr;
                int dcCount = StringSplit(dcname, "-", strArr);
                for(int dc = 0; dc<dcCount; dc++)
                {
                    dcname = strArr[dc];
                    for(int z=0; z<DevArray.size(); z++)
                    {
                        if (DevArray[z]->getDevType() == Dev_DC)
                        {
                            pGDDC=(CGDDC *)(DevArray[z]);
                            if(dcname == pGDDC->m_strName)
                            {
                                if(pGDDC->isPowerCutDW || pGDDC->isPowerCutFW || pGDDC->isPowerCutCQ)
                                {
                                    warn |= JLWARNING_QDPOWERCUT;
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
            else //示例：4DG
            {
                for(int z=0; z<DevArray.size(); z++)
                {
                    if (DevArray[z]->getDevType() == Dev_DC)
                    {
                        pGDDC=(CGDDC *)(DevArray[z]);
                        if(dcname == pGDDC->m_strName)
                        {
                            if(pGDDC->isPowerCutDW || pGDDC->isPowerCutFW || pGDDC->isPowerCutCQ)
                            {
                                warn |= JLWARNING_QDPOWERCUT;
                                return false;
                            }
                        }
                    }
                }
            }
        }
        else//区段、股道
        {
            for(int z=0; z<DevArray.size(); z++)
            {
                if (DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->m_strName)
                    {
                        if(pGD->isPowerCut)//股道接触网无电
                        {
                            warn |= JLWARNING_QDPOWERCUT;
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}
//检查进路可否执行（检查类型1-空闲检查，2-分路不良检查，3-接触网供电检查，0-进路是否存在）
bool MyStation::CheckRouteCanCmd(int checkType, unsigned char *pCmdUDPDate, QStringList &routeBtnTempArray)
{
    //进路报警类型(临时变量)
    int JLWarningType = 0;
    //进路报警信息(临时变量)
    QString JLWarningMsg = "";

    QString jlType;
    if(pCmdUDPDate[10] == 0x01)
    {
        jlType = ROUTE_LC;
    }
    else if(pCmdUDPDate[10] == 0x02)
    {
        jlType = ROUTE_DC;
    }
    else if(pCmdUDPDate[10] == 0x03)
    {
        jlType = ROUTE_TG;
    }

    //匹配进路
    int index = FindRouteIndexInLSB(jlType, routeBtnTempArray);//xhdNameTempArray
    if(index <= -1)
    {
        if (0 == checkType)
        {
            return false;
        }
        return true;//没有该条进路时默认为TRUE
    }
    else if(0 == checkType)
    {
        return true;
    }
    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[index]);
    //CString msg;
    //msg.Format(_T("已匹配联锁表，进路序号：%d"), index);
    //AfxMessageBox(msg);
    QStringList rtQdArray;
    rtQdArray.append(pRut->QDArr);

    if(1 == checkType)
    {
        //判断是否空闲
        if(isQDKXInLSB(rtQdArray))
        {
            return true;
        }
        else
        {
            return false;
        }

    }
    else if(2 == checkType)
    {
        //判断分录不良空闲
        if(isQDFLBLKXInLSB(rtQdArray,JLWarningType,JLWarningMsg))
        {
            return true;
        }
        else
        {
            //AfxMessageBox(_T("股道分路不良，需要确认股道空闲！"));
            return false;
        }
    }
    else if(3 == checkType)
    {
        //判断进路区段是否有电
        if(isQDPowerInLSB(rtQdArray,JLWarningType,JLWarningMsg))
        {
            return true;
        }
        else
        {
            return false;
            //AfxMessageBox(_T("电力机车无法进入无电股道!"));
        }
    }

    return true;
}
//获取信号机关联的区段
QString MyStation::GetXHDcorrQDName(CXHD *pxhd)
{
    CGD *pGD;
    QPoint pt[] = {pxhd->p1, pxhd->p2};

    int m = (int)DevArray.size();
    for (int i = 0; i < m; i++)
    {
        if (DevArray[i]->getDevType() == Dev_GD)
        {
            pGD = (CGD*)DevArray[i];
            //if (JJ_QD == pGD->GD_Type)
            {
                QRect rectGD;
                QPoint pPtGD[] = {pGD->p1.toPoint(), pGD->p2.toPoint(), pGD->p3.toPoint(), pGD->p4.toPoint()};
                for (int k = 0; k<2; k++)
                {
                    for (int j = 0; j<4; j++)
                    {
                        rectGD.setRect(pPtGD[j].x()-5, pPtGD[j].y()-5, 10, 10);
                        //if (PtInRect(rectGD, *pt[k]))
                        if(rectGD.contains(pt[k]))
                        {
                            return pGD->m_strName;
                            break;
                        }
                    }
                }
            }
        }
    }
    return "";
}
//检查调车无电
bool MyStation::CheckRouteDCPower(unsigned char *pCmdUDPDate, QStringList &routeBtnTempArray, int &warn, QString &msg)
{
    CGD *pGD = NULL;

    QString jlType;
    if(pCmdUDPDate[10] == 0x01)
    {
        jlType = ROUTE_LC;
    }
    else if(pCmdUDPDate[10] == 0x02)
    {
        jlType = ROUTE_DC;
    }
    else if(pCmdUDPDate[10] == 0x03)
    {
        jlType = ROUTE_TG;
    }

    //if(jlType == ROUTE_DC)
    {
        for(int i=1;i<routeBtnTempArray.size();i++)
        {
            QString strBut;
            strBut = routeBtnTempArray[i];
            strBut.replace("DA","");
            strBut.replace("LA","");
            strBut.replace("A","");
            CXHD *pxhd;
            int jckXHDindex = GetIndexByStrName(strBut);
            if (jckXHDindex < 0)
                continue;
            pxhd = (CXHD *)(DevArray[jckXHDindex]);
            QString strGd = GetXHDcorrQDName(pxhd);
            if(strGd != "")
            {
                for(int z = 0; z < DevArray.size(); z++)
                {
                    if (DevArray[z]->getDevType() == Dev_GD)
                    {
                        pGD = (CGD*)DevArray[z];
                        if(strGd == pGD->m_strName)
                        {
                            if(pGD->isPowerCut)
                            {
                                warn |= JLWARNING_QDPOWERCUT;
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}
//检查调车分路不良
bool MyStation::CheckRouteDC_FLBL(unsigned char *pCmdUDPDate, QStringList &routeBtnTempArray, int &warn, QString &msg)
{
    CGD *pGD = NULL;
    QString jlType;
    if(pCmdUDPDate[10] == 0x01)
    {
        jlType = ROUTE_LC;
    }
    else if(pCmdUDPDate[10] == 0x02)
    {
        jlType = ROUTE_DC;
    }
    else if(pCmdUDPDate[10] == 0x03)
    {
        jlType = ROUTE_TG;
    }

    //if(jlType == ROUTE_DC)
    {
        for(int i=1;i<routeBtnTempArray.size();i++)
        {
            QString strBut;
            strBut = routeBtnTempArray[i];
            strBut.replace("DA","");
            strBut.replace("LA","");
            strBut.replace("A","");
            CXHD *pxhd;
            int jckXHDindex = GetIndexByStrName(strBut);
            if (jckXHDindex < 0)
                break;
            pxhd = (CXHD *)(DevArray[jckXHDindex]);
            QString strGd = GetXHDcorrQDName(pxhd);
            if(strGd != "")
            {
                for(int z = 0; z < DevArray.size(); z++)
                {
                    if (DevArray[z]->getDevType() == Dev_GD)
                    {
                        pGD = (CGD*)DevArray[z];
                        if((strGd == pGD->m_strName)/*||(strGd == pGD->m_strName1)*/)
                        {
                            if(pGD->flblStatus>0)
                            {
                                warn |= JLWARNING_FLBL_GD;
                                return false;
                            }

                        }
                    }
                }
            }
        }
    }
    return true;
}
 //检查调车封锁
bool MyStation::CheckRouteDCFS(unsigned char *pCmdUDPDate, QStringList &routeBtnTempArray, int &warn, QString &msg)
{
    CGD *pGD = NULL;
    QString jlType;
    if(pCmdUDPDate[10] == 0x01)
    {
        jlType = ROUTE_LC;
    }
    else if(pCmdUDPDate[10] == 0x02)
    {
        jlType = ROUTE_DC;
    }
    else if(pCmdUDPDate[10] == 0x03)
    {
        jlType = ROUTE_TG;
    }

    //if(jlType == ROUTE_DC)
    {
        for(int i=1;i<routeBtnTempArray.size();i++)
        {
            QString strBut;
            strBut = routeBtnTempArray[i];
            strBut.replace("DA","");
            strBut.replace("LA","");
            strBut.replace("A","");
            CXHD *pxhd;
            int jckXHDindex = GetIndexByStrName(strBut);
            if (jckXHDindex < 0)
                break;
            pxhd = (CXHD *)(DevArray[jckXHDindex]);
            QString strGd = GetXHDcorrQDName(pxhd);
            if(strGd != "")
            {
                for(int z = 0; z < DevArray.size(); z++)
                {
                    if (DevArray[z]->getDevType() == Dev_GD)
                    {
                        pGD = (CGD*)DevArray[z];
                        if(strGd == pGD->m_strName)
                        {
                            if(pGD->isLock)
                            {
                                warn |= JLWARNING_FS_GD;
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}
//设置行车日志的通过报点时间（联锁延续进路通过不报点BUG在CTC此处补充报点）
void MyStation::SetTrafficLogTGTime(QString _strCheci)
{
    for(int j = 0; j < m_ArrayTrafficLog.count(); j++)
    {
        TrafficLog *pTrafficLog = (TrafficLog *)m_ArrayTrafficLog[j];
        if( pTrafficLog->m_strDepartTrainNum == _strCheci
          || pTrafficLog->m_strReachTrainNum == _strCheci)
        {
            //还没有报点才处理
            if(pTrafficLog->m_timRealReachStation.date().year() == 1970
               || pTrafficLog->m_timRealReachStation.date().year() == 0)
            {
                QDateTime tmCurt = QDateTime::currentDateTime();
                pTrafficLog->m_timRealDepaTrain = tmCurt;
                pTrafficLog->m_timRealReachStation = tmCurt;
                SetTrafficLogProc(pTrafficLog);
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
            return;
        }
    }
}
//自动检查和设置进路的状态
void MyStation::AutoCheckAndSetRouteOrderState()
{
    //互斥锁
    QMutexLocker locker(&Mutex);

    CGD *pGD = NULL;
    TrainRouteOrder* pTrainRouteOrder;
    //进路报警类型(临时变量)
    int JLWarningType = 0;
    //进路报警信息(临时变量)
    QString JLWarningMsg = "";

    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //组合进路的主进路不执行
        if(pTrainRouteOrder->m_bZHJL)
        {
            continue;
        }
        if(pTrainRouteOrder->m_strTrainNum.length() <= 0 || pTrainRouteOrder->m_bDeleteFlag)
        {
            continue;
        }
        //从分散自律转到非常站控时，自触标志不变；由非常站控转回到车站控制时，自触标志全部清除。P275.
        //if(pDoc->m_nFCZKMode)
        //{
        //	pTrainRouteOrder->m_nAutoTouch = FALSE;
        //}

        SetRouteSuccStateByCfgXHD(pTrainRouteOrder);

        int xhIndex = this->GetIndexByStrName(pTrainRouteOrder->m_strXHDBegin);
        if(xhIndex<0)
        {
            continue;
        }
        CXHD *pXHDBg = (CXHD*)this->DevArray[xhIndex];
        if(pTrainRouteOrder->m_nGDPos != 0 && pXHDBg)
        {
            if(pTrainRouteOrder->m_nGDPos > 0)
            {
                pGD = (CGD*)this->DevArray[pTrainRouteOrder->m_nGDPos-1];
            }
            if(0 == pTrainRouteOrder->m_nOldRouteState)
            {
                //等待的状态不处理
                //pTrainRouteOrder->SetState(0);
            }
            else
            {
                //接车进路
                if(pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC)//0x00 表示到达
                {
                    //有联锁表
                    if(pTrainRouteOrder->m_nIndexRoute>-1)
                    {
                        //接车进路不能判断进路表，因为进路终端是股道,但是线路所可以判断（没有股道）
                        if(pTrainRouteOrder->bXianLuSuo)
                        {
                            //触发完成
                            if(2 == pTrainRouteOrder->m_nOldRouteState)
                            {
                                if(this->IsQDZYInLSB(pTrainRouteOrder->m_nIndexRoute,JLWarningType,JLWarningMsg))
                                {
                                    pTrainRouteOrder->SetState(3);//_T("占用");
                                    this->SetTrafficLogTGTime(pTrainRouteOrder->m_strTrainNum);
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                            }
                            //占用
                            else if(3 == pTrainRouteOrder->m_nOldRouteState)
                            {
                                if(!this->IsQDZYInLSB(pTrainRouteOrder->m_nIndexRoute,JLWarningType,JLWarningMsg))
                                {
                                    pTrainRouteOrder->SetState(4);//_T("出清");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                            }
                        }
                        //非线路所
                        else
                        {
                            //触发完成
                            if(2 == pTrainRouteOrder->m_nOldRouteState)
                            {
                                if(this->IsQDZYInLSB(pTrainRouteOrder->m_nIndexRoute,JLWarningType,JLWarningMsg))
                                {
                                    pTrainRouteOrder->SetState(3);//_T("占用");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                            }
                            //占用
                            else if(3 == pTrainRouteOrder->m_nOldRouteState)
                            {
                                //股道占用且停稳
                                if((pTrainRouteOrder->m_btBeginOrEndFlg!=JFC_TYPE_TG)
                                        && pGD && pGD->getState(QDZY) && pGD->m_bLCTW)
                                {
                                    pTrainRouteOrder->SetState(4);//_T("出清");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                                //进路全部为空闲
                                else if((pTrainRouteOrder->m_btBeginOrEndFlg==JFC_TYPE_TG)
                                        && (this->IsQDStateInLSB(pTrainRouteOrder->m_nIndexRoute, QDKX)
                                            || pTrainRouteOrder->m_bReportTime
                                            || (pGD && pGD->getState(QDZY) && pGD->m_bLCTW))
                                        )
                                {
                                    pTrainRouteOrder->SetState(4);//_T("出清");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                                //进路全部为空闲
                                else if((pTrainRouteOrder->m_btBeginOrEndFlg!=JFC_TYPE_TG)
                                        && this->IsQDStateInLSB(pTrainRouteOrder->m_nIndexRoute, QDKX))
                                {
                                    pTrainRouteOrder->SetState(4);//_T("出清");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                            }

                            //通过进路是延续进路的报点特殊处理
                            if(pTrainRouteOrder->m_btBeginOrEndFlg==JFC_TYPE_TG && pTrainRouteOrder->m_bYXJL)
                            {
                                if(pGD && pTrainRouteOrder->m_strTrainNum==pGD->m_strCheCiNum && pGD->getState(QDZY))
                                {
                                    this->SetTrafficLogTGTime(pTrainRouteOrder->m_strTrainNum);
                                }
                            }
                        }
                    }
                    //无匹配的联锁表
                    else
                    {
                        //线路所
                        if(pTrainRouteOrder->bXianLuSuo)
                        {
                            ;// 无此进路的联锁表时不予处理，必须配置联锁表
                        }
                        //非线路所
                        else
                        {
                            //触发完成->占用
                            if(2 == pTrainRouteOrder->m_nOldRouteState)
                            {
                                //车次已在股道上
                                if(pGD && pTrainRouteOrder->m_strTrainNum==pGD->m_strCheCiNum)//(pXHDBg->getXHDState()==XHD_HD) /*pGD->getState(QDZY)*/
                                {
                                    pTrainRouteOrder->SetState(3);//_T("占用");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                            }
                            //占用->出清
                            else if(3 == pTrainRouteOrder->m_nOldRouteState)
                            {
                                //非通过计划 股道占用且停稳
                                if((pTrainRouteOrder->m_btBeginOrEndFlg!=JFC_TYPE_TG)
                                    && pGD && pGD->getState(QDZY) && pGD->m_bLCTW)
                                {
                                    pTrainRouteOrder->SetState(4);//_T("出清");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                                //通过计划 进路全部为
                                else if((pTrainRouteOrder->m_btBeginOrEndFlg==JFC_TYPE_TG)
                                        && pTrainRouteOrder->m_bReportTime
                                        && (pGD && pGD->getState(QDKX) )
                                  )
                                {
                                    pTrainRouteOrder->SetState(4);//_T("出清");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                            }

                            //通过进路是延续进路的报点特殊处理
                            if(pTrainRouteOrder->m_btBeginOrEndFlg==JFC_TYPE_TG && pTrainRouteOrder->m_bYXJL)
                            {
                                if(pGD && pTrainRouteOrder->m_strTrainNum==pGD->m_strCheCiNum && pGD->getState(QDZY))
                                {
                                    this->SetTrafficLogTGTime(pTrainRouteOrder->m_strTrainNum);
                                }
                            }
                        }
                    }
                }
                //发车进路
                else if(pTrainRouteOrder->m_btRecvOrDepart == ROUTE_FC)//0x01 表示出发
                {
                    //有联锁表
                    if(pTrainRouteOrder->m_nIndexRoute>-1)
                    {
                        //触发完成->占用
                        if(2 == pTrainRouteOrder->m_nOldRouteState)
                        {
                            if(this->IsQDZYInLSB(pTrainRouteOrder->m_nIndexRoute,JLWarningType,JLWarningMsg))
                            {
                                pTrainRouteOrder->SetState(3);//_T("占用");
                                //更新数据库
                                m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                //发送同步消息
                                //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                            }
                        }
                        //占用->出清
                        else if(3 == pTrainRouteOrder->m_nOldRouteState)
                        {
                            //if(!this->IsQDZYInLSB(pTrainRouteOrder->m_nIndexRoute))
                            //进路区段全部空闲
                            if(this->IsQDStateInLSB(pTrainRouteOrder->m_nIndexRoute, QDKX))
                            {
                                pTrainRouteOrder->SetState(4);//_T("出清");
                                //更新数据库
                                m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                //发送同步消息
                                //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                            }
                        }
                    }
                    //无匹配的联锁表
                    else
                    {
                        //if(pTrainRouteOrder->m_btBeginOrEndFlg!=JFC_TYPE_TG)//非通过进路
                        //非通过进路+通过进路处理方式一样
                        {
                            //触发完成->占用
                            if(2 == pTrainRouteOrder->m_nOldRouteState)
                            {
                                //信号关闭、股道占用、车次已出站显示
                                if((pXHDBg->getXHDState()==XHD_HD) && pGD && pGD->getState(QDZY)
                                        && pTrainRouteOrder->m_strTrainNum == "")
                                {
                                    pTrainRouteOrder->m_ntime=0;
                                    pTrainRouteOrder->SetState(3);//_T("占用");
                                    //更新数据库
                                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                    //发送同步消息
                                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                }
                            }
                            //占用->出清
                            else if(3 == pTrainRouteOrder->m_nOldRouteState)
                            {
                                //股道空闲
                                if(pGD && pGD->getState(QDKX))
                                {
                                    pTrainRouteOrder->m_ntime++;
                                    if(pTrainRouteOrder->m_ntime > (4*20))//线程1秒4次，20秒
                                    {
                                        pTrainRouteOrder->SetState(4);//_T("出清");
                                        //更新数据库
                                        m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                        //发送同步消息
                                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }
}
//根据配置的信号机判断设置进路的办理成功状态
bool MyStation::SetRouteSuccStateByCfgXHD(TrainRouteOrder *_pTrainRouteOrder)
{
    //qDebug()<<"in 1";
    if(!this->m_bRouteStateCheckXHD)
    {
        return false;
    }
    //qDebug()<<"in 2";
    if(_pTrainRouteOrder->m_nOldRouteState != 1)
    {
        //不是正在触发的不判断
        return false;
    }
    //qDebug()<<"in 3";
    for(int i=0; i<this->m_vectRouteCheckXhd.count(); i++)
    {
        //进路序列描述一致
        if(this->m_vectRouteCheckXhd[i].strRoute == _pTrainRouteOrder->m_strRouteDescrip)
        {
            //qDebug()<<"in 4";
            BOOL bOpen = false;
            for(int j=0; j<this->m_vectRouteCheckXhd[i].vectXHD.count(); j++)
            {
                QString xhdName = this->m_vectRouteCheckXhd[i].vectXHD[j];
                int xhPos = this->GetIndexByStrName(xhdName);
                if(xhPos > 0)
                {
                    //qDebug()<<"in 5";
                    CXHD* pXHD = (CXHD*)this->DevArray[xhPos];
                    //不是红灯和白灯
                    if(!pXHD->getXHDState(XHD_HD) && !pXHD->getXHDState(XHD_BD) && !pXHD->getXHDState(XHD_YD))
                    {
                        bOpen = true;
                        //qDebug()<<xhdName<<" is open!";
                    }
                    else
                    {
                        bOpen = false;
                        //qDebug()<<xhdName<<" is not open!";
                        break;
                    }
                }
            }
            if(bOpen)
            {
                _pTrainRouteOrder->SetState(2);
                //更新数据库
                m_pDataAccess->UpdateRouteOrder(_pTrainRouteOrder);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,_pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                //更新行车日志的进路办理标志
                bool bFCJL = _pTrainRouteOrder->m_btRecvOrDepart==ROUTE_FC?true:false;
                UpdateTrafficlogJLSuccessed(bFCJL, _pTrainRouteOrder->m_strTrainNum, true);
                return true;
            }
        }
    }

    return false;
}
//进路序列按照开始时间自动升序排序
void MyStation::AutoSortRouteOrder()
{
    //互斥锁
    QMutexLocker locker(&Mutex);

    //临时数组变量
    QVector<TrainRouteOrder*>  ArrayRouteTemp;
    int sizeRoute = m_ArrayRouteOrder.count();
    if(sizeRoute <= 1)
    {
        return;
    }
    else
    {
        //先增加第0个
        ArrayRouteTemp.append((TrainRouteOrder*)m_ArrayRouteOrder[0]);
    }

    //将既有数据按照时间先后插入到临时变量的相应位置
    for(int i=1; i<m_ArrayRouteOrder.count(); i++)
    {
        bool theLastOne = true;//最后一个
        //int sizeTemp = ArrayRouteTemp.count();
        for(int j=0; j<ArrayRouteTemp.count(); j++)
        {
            if(((TrainRouteOrder*)m_ArrayRouteOrder[i])->m_timBegin < ((TrainRouteOrder*)ArrayRouteTemp[j])->m_timBegin)
            {
                ArrayRouteTemp.insert(j, (TrainRouteOrder*)m_ArrayRouteOrder[i]);
                theLastOne = false;
                break;
            }
        }
        if(theLastOne)
        {
            ArrayRouteTemp.append((TrainRouteOrder*)m_ArrayRouteOrder[i]);
        }
    }
    m_ArrayRouteOrder.clear();
    m_ArrayRouteOrder.append(ArrayRouteTemp);
}
//定时自动发送列车计划指令到联锁，Server逻辑卡控并自动发送
void MyStation::SendOrderToInterlockSys()
{
    //互斥锁
    QMutexLocker locker(&Mutex);

    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        //当前时间
        QDateTime timeNow = QDateTime::currentDateTime();

        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if(pTrainRouteOrder->m_bCreateByMan)
        {
            //人工排路自动增加的进路序列（灰色显示的）不做发送处理
            continue;
        }
        if(pTrainRouteOrder->m_bDeleteFlag)
        {
            continue;
        }
        //组合进路
        if(pTrainRouteOrder->m_bZHJL)
        {
            continue;
        }
        //获取进路索引并组织进路命令
        int interlkRouteIndex = this->GetTrainRouteOrderLSBRouteIndex(pTrainRouteOrder);

        //记录最近一次办理时间
        qint64 secondsVSLast = this->m_LastTimeOfRouteDo.secsTo(timeNow);
        if(secondsVSLast<10 && HaveRouteIsDoing())
        {
            return;//间隔10秒再执行，等待联锁将上一次的进路办理出来，避免同一咽喉同时办理进路联锁办理不出来
        }

        //进路正在办理，则后续的进路不再执行，等待本进路触发完毕
        if(pTrainRouteOrder->m_nOldRouteState == 1)
        {
            //每间隔30秒重新触发一次
            qint64 secondsTouched = pTrainRouteOrder->m_timRealTouching.secsTo(timeNow);
            if(secondsTouched>=30)
            {
                //自触的进路,每间隔30秒重新触发一次
                if(pTrainRouteOrder->m_nAutoTouch)
                {
                    //超过规定的持久触发时间则恢复等待状态（即超过规定的触发次数）
                    if(pTrainRouteOrder->m_nTouchingCount >(AutoTryMaxMinutesWhenTouchingRoute*60/30))
                    {
                        //超时后恢复“等待”且清除“自触”
                        pTrainRouteOrder->SetState(0);
                        pTrainRouteOrder->m_nAutoTouch = false;
                        //更新数据库
                        m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                        //发送同步消息
                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                        qDebug()<<QString("%1 进路%2(%3)进路触发超时，恢复等待状态！")
                                  .arg(pTrainRouteOrder->m_timRealTouching.toString(TIME_FORMAT_YMDHMSM))
                                  .arg(pTrainRouteOrder->m_strTrainNum)
                                  .arg(pTrainRouteOrder->m_strRouteDescrip)
                                  .arg(pTrainRouteOrder->m_nTouchingCount);
                    }
                    //否则继续尝试触发
                    else
                    {
                        //发送指令
                        SendOrderToInterlock(pTrainRouteOrder);
                    }
                }
                //人工触发的进路
                else
                {
                    //超过30秒没有排出进路则恢复成初始状态（等待）
                    pTrainRouteOrder->SetState(0);
                    //更新数据库
                    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                    //发送同步消息
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                    qDebug()<<QString("%1 进路%2(%3)进路触发超时，恢复等待状态！")
                              .arg(pTrainRouteOrder->m_timRealTouching.toString(TIME_FORMAT_YMDHMSM))
                              .arg(pTrainRouteOrder->m_strTrainNum)
                              .arg(pTrainRouteOrder->m_strRouteDescrip)
                              .arg(pTrainRouteOrder->m_nTouchingCount);
                }
                return;
            }
            //退出本次大循环
            break;
        }

        //自动触发的进路状态不是等待，则忽略
        if(pTrainRouteOrder->m_nAutoTouch && 0!=pTrainRouteOrder->m_nOldRouteState)
        {
            continue;
        }

        //自触进路超时则不再触发
        qint64 seconds1 = timeNow.secsTo(pTrainRouteOrder->m_timBegin);
        if(pTrainRouteOrder->m_nAutoTouch && (seconds1 > (TryAutoTouchMaxMinutes*60)))
        {
            return;
        }

        //计划时间到
        bool bTimeOut = false;
        //自触-接车进路
        if(pTrainRouteOrder->m_nAutoTouch && pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC)
        {
            //qint64 seconds = pTrainRouteOrder->m_timBegin.secsTo(timeNow);
            qint64 seconds = timeNow.secsTo(pTrainRouteOrder->m_timBegin);
            //接车进路提前办理时间，接车标准时间-可配置的
            int standardJCSeconds = AutoTouchReachRouteLeadMinutes*60;
            //qDebug()<<QString("接车设置时间%1秒，%2离办理时间还有%3秒").arg(standardJCSeconds).arg(pTrainRouteOrder->m_strTrainNum).arg(seconds);
            if(seconds <= standardJCSeconds || pTrainRouteOrder->m_bRunNow)
            {
                bTimeOut = true;
                //通过进路判定
                if(pTrainRouteOrder->m_btBeginOrEndFlg==JFC_TYPE_TG)
                {
                    qDebug()<<QString("通过进路开始检查办理，%1").arg(pTrainRouteOrder->m_strTrainNum);
#if 0
                    //获取组合进路 等待的进路
                    int nRindex = this->GetPopTrainRouteIndexOfZHJL(pTrainRouteOrder->m_strTrainNum, ROUTE_FC);
                    if(nRindex <= -1)
                    {
                        //非组合进路，则获取常规进路
                        nRindex = this->GetTrainRouteIndex(pTrainRouteOrder->m_strTrainNum, ROUTE_FC);
                    }
                    if(nRindex > -1)
                    {
                        TrainRouteOrder* pTrainRouteOrderFC = (TrainRouteOrder*)this->m_ArrayRouteOrder[nRindex];
                        //自触-发车进路
                        if(pTrainRouteOrderFC->m_nAutoTouch)
                        {
                            //办理中，则退出
                            if(1==pTrainRouteOrderFC->m_nOldRouteState)
                            {
                                return;
                            }
                            //办理完成
                            else if(2==pTrainRouteOrderFC->m_nOldRouteState)
                            {
                                bTimeOut = true;
                            }
                            //等待状态
                            else if(0==pTrainRouteOrderFC->m_nOldRouteState)
                            {
                                //检查接车进路
                                CheckResult* ckResultJC = CheckPreventConditionAll(pTrainRouteOrder);
                                //检查发车进路
                                CheckResult* ckResultFC = CheckPreventConditionAll(pTrainRouteOrderFC);
                                //接车和发车都检查通过，则先办理发车
                                if(ckResultJC->check==0 && ckResultFC->check==0)
                                {
                                    bTimeOut = false;
                                    this->SendOrderToInterlock(pTrainRouteOrderFC);
                                    return;
                                }
                                //检查不通过，则先办理接车
                                else
                                {
                                    bTimeOut = true;
                                    //系统报警信息
                                    if(ckResultJC->check > 0)
                                    {
                                        //this->sendWarningMsgToCTC(1,2,ckResultJC->checkMsg);
                                        //bTimeOut = false;
                                    }
                                    if(ckResultFC->check > 0)
                                    {
                                        //this->sendWarningMsgToCTC(1,2,ckResultFC->checkMsg);
                                    }
                                }
                            }
                        }
                    }
#else
                    //获取组合进路(未办理的第一个)
                    int nRindex = this->GetPopTrainRouteIndexOfZHJL(pTrainRouteOrder->m_strTrainNum, ROUTE_FC);
                    if(nRindex <= -1)
                    {
                        //非组合进路，则获取常规进路
                        nRindex = this->GetTrainRouteIndex(pTrainRouteOrder->m_strTrainNum, ROUTE_FC);
                    }
                    if(nRindex > -1)
                    {
                        TrainRouteOrder* pTrainRouteOrderFC = (TrainRouteOrder*)this->m_ArrayRouteOrder[nRindex];
                        //自触-发车进路
                        if(pTrainRouteOrderFC->m_nAutoTouch)
                        {
                            //办理中，则退出
                            if(1==pTrainRouteOrderFC->m_nOldRouteState)
                            {
                                return;
                            }
                            //办理完成
                            else if(2==pTrainRouteOrderFC->m_nOldRouteState)
                            {
                                bTimeOut = true;
                            }
                            //等待状态
                            else if(0==pTrainRouteOrderFC->m_nOldRouteState)
                            {
                                QString strMsg;
                                //检查接车进路
                                //CheckResult* ckResultJC = CheckPreventConditionAll(pTrainRouteOrder);
                                CheckResult* ckResultJC = CheckBrotherRouteOrder(pTrainRouteOrder, strMsg);
                                //检查发车进路
                                //CheckResult* ckResultFC = CheckPreventConditionAll(pTrainRouteOrderFC);
                                CheckResult* ckResultFC = CheckBrotherRouteOrder(pTrainRouteOrderFC, strMsg);
                                //接车和发车都检查通过，则先办理发车
                                if(ckResultJC->check==0 && ckResultFC->check==0)
                                {
                                    bTimeOut = false;
                                    this->SendOrderToInterlock(pTrainRouteOrderFC);
                                    return;
                                }
                                //检查不通过，则先办理接车
                                else
                                {
                                    if(ckResultJC->check==0)
                                    {
                                        bTimeOut = true;
                                    }
                                    else
                                    {
                                        //QString strSys = QString("%1次列车，进路检查不通过 %2").arg(pTrainRouteOrder->m_strTrainNum).arg(strMsg);
                                        QString strSys = GetWariningMsgByType(pTrainRouteOrder,ckResultJC);
                                        qDebug()<<strSys;
                                        this->sendWarningMsgToCTC(1,2,strSys);
                                        bTimeOut = false;
                                    }
//                                    //系统报警信息
//                                    if(ckResultJC->check > 0)
//                                    {
//                                        //this->sendWarningMsgToCTC(1,2,ckResultJC->checkMsg);
//                                        //bTimeOut = false;
//                                    }
//                                    if(ckResultFC->check > 0)
//                                    {
//                                        //this->sendWarningMsgToCTC(1,2,ckResultFC->checkMsg);
//                                    }
                                }
                            }
                        }
                    }

#endif
                }
            }
        }

        //非通过进路 且 发车进路
        if(pTrainRouteOrder->m_btBeginOrEndFlg!=JFC_TYPE_TG && pTrainRouteOrder->m_btRecvOrDepart==ROUTE_FC)
        {
            //发车股道停稳
            bool fcgdTw = CheckRouteGDZYTW(pTrainRouteOrder);
            //组合进路特殊判定，只判定组合进路的父进路
            if(pTrainRouteOrder->father_id>0)
            {
                //获取父进路
                TrainRouteOrder* pTrainRouteOrderFather = FindTrainRouteOrderById(pTrainRouteOrder->father_id);
                //通过父进路判断股道停稳
                fcgdTw = CheckRouteGDZYTW(pTrainRouteOrderFather);
            }
            if(fcgdTw)
            {
                //发车标准时间-客车3分钟，货车1分钟
                int standardSeconds = 3*60;
                if(pTrainRouteOrder->m_nLHFlg != LCTYPE_KC)
                {
                    standardSeconds = 1*60;
                }
                //开始时间比当前时间早，则为负值，负值表示早点
                qint64 seconds = timeNow.secsTo(pTrainRouteOrder->m_timBegin);
                if(seconds <= standardSeconds)
                {
                    bTimeOut = true;
                }
            }
        }

#if 1   //通过进路的发车必须结合接车进路的条件执行.lwm.2023.12.14
        //该功能是在排列通过进路时先自触接车进路,待车进站时再设置自触发车进路,此时需要列车不停车通过(包括进路是坡道进路)2024.2.22.zzl，增加对关联接车进路的判定
        //通过进路特殊判断
        else if(pTrainRouteOrder->m_btBeginOrEndFlg==JFC_TYPE_TG && pTrainRouteOrder->m_btRecvOrDepart==ROUTE_FC)
        {
            //接车进路均没有问题
            bool bJCOK = true;
            //接车是组合进路
            bool bJCZHJL = false;
            //组合进路判定
            for(int r = 0; r < m_ArrayRouteOrder.size(); r++)
            {
                TrainRouteOrder* pTrainRouteOrder1 = m_ArrayRouteOrder[r];
                if( ROUTE_JC == pTrainRouteOrder1->m_btRecvOrDepart
                   && pTrainRouteOrder->m_strTrainNum == pTrainRouteOrder1->m_strTrainNum
                   && 4 != pTrainRouteOrder1->m_nOldRouteState
                   && 0 == pTrainRouteOrder1->m_bZHJL
                   && 0 < pTrainRouteOrder1->father_id)
                {
                    bJCZHJL = true;
                    if(0==pTrainRouteOrder1->m_nOldRouteState)
                    {
                        bJCOK = false;
                        break;
                    }
                }
            }
            //非组合进路判定
            for(int r = 0; r < m_ArrayRouteOrder.size() && !bJCZHJL; r++)
            {
                TrainRouteOrder* pTrainRouteOrder1 = m_ArrayRouteOrder[r];
                if( ROUTE_JC == pTrainRouteOrder1->m_btRecvOrDepart
                   && pTrainRouteOrder->m_strTrainNum == pTrainRouteOrder1->m_strTrainNum
                   && 4 != pTrainRouteOrder1->m_nOldRouteState
                   && 0 == pTrainRouteOrder1->m_bZHJL
                   && 0 == pTrainRouteOrder1->father_id)
                {
                    if(0==pTrainRouteOrder1->m_nOldRouteState)
                    {
                        bJCOK = false;
                        break;
                    }
                }
            }
//            TrainRouteOrder* pTrainRouteOrderJC = nullptr;
//            //获取组合进路
//            int nRindex = this->GetPopTrainRouteIndexOfZHJL(pTrainRouteOrder->m_strTrainNum, ROUTE_JC);
//            if(nRindex <= -1)
//            {
//                //非组合进路，则获取常规进路
//                nRindex = this->GetTrainRouteIndex(pTrainRouteOrder->m_strTrainNum, ROUTE_JC);
//            }
//            if(nRindex > -1)
//            {
//                pTrainRouteOrderJC = (TrainRouteOrder*)this->m_ArrayRouteOrder[nRindex];
//            }
//            // 接车进路可以办理且已在执行
//            if(pTrainRouteOrderJC!=nullptr && pTrainRouteOrderJC->m_nOldRouteState>0)
            if(bJCOK)
            {
                QString strMsg;
                CheckResult* ckResult = CheckBrotherRouteOrder(pTrainRouteOrder, strMsg);
                if(ckResult->check!=0)
                {
                    //QString strSys = QString("%1次列车，进路检查不通过 %2").arg(pTrainRouteOrder->m_strTrainNum).arg(strMsg);
                    QString strSys = GetWariningMsgByType(pTrainRouteOrder,ckResult);
                    qDebug()<<strSys;
                    this->sendWarningMsgToCTC(1,2,strSys);
                    continue;//防错办检查不通过
                }

                //发车标准时间-客车3分钟，货车1分钟
                int standardSeconds = 3*60;
                if(pTrainRouteOrder->m_nLHFlg != LCTYPE_KC)
                {
                    standardSeconds = 1*60;
                }
                //开始时间比当前时间早，则为负值，负值表示早点
                qint64 seconds = timeNow.secsTo(pTrainRouteOrder->m_timBegin);
                if(seconds <= standardSeconds)
                {
                    bTimeOut = true;
                    qDebug()<<QString("通过计划-发车进路开始办理，%1").arg(pTrainRouteOrder->m_strTrainNum);
                }
            }
        }
#endif

        //Server根据进路的逻辑卡控结果办理进路
        //计划时间到 || 人工执行 || 接收到立即执行
        if((pTrainRouteOrder->m_nAutoTouch && bTimeOut) || pTrainRouteOrder->m_nManTouch || pTrainRouteOrder->m_bRunNow)
        {
            //等待 or 已取消，才可以办理 || 办理中且办理失败的再次尝试办理
            if((pTrainRouteOrder->m_nOldRouteState == 0 || pTrainRouteOrder->m_nOldRouteState == 5)
                || (pTrainRouteOrder->m_nOldRouteState == 1 && pTrainRouteOrder->m_bSuccessed == 2))
            {
                //QString checkMsg;
                if(-1 != interlkRouteIndex)
                {
                    //人工触发-正线通过进路检查
                    if(pTrainRouteOrder->m_nManTouch || pTrainRouteOrder->m_bRunNow)
                    {
                        //客车正线通过进路检查
                        if(pTrainRouteOrder->m_nLHFlg == LCTYPE_KC)
                        {
                            //正线通过进路检查
                            bool tgRouteIsZXTGJL = CheckZXTGJL(pTrainRouteOrder);
                            if(!tgRouteIsZXTGJL)
                            {
                                CheckResult* ckResult = new CheckResult;
                                QDateTime timeNow = QDateTime::currentDateTime();
                                ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
                                ckResult->route_id = pTrainRouteOrder->route_id;
                                ckResult->bEnforced = true;
                                ckResult->checkMsg = QString("车次%1触发未知的返回结果，客车%2次在股道%3侧线通过")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                                //人工办理时才发送强制执行窗口
                                //防错办报警信息
                                this->SendRouteCheckResult(ckResult, -1);//所有终端
                                UpdateCheckResultArray(ckResult);//交互，则增加

                                pTrainRouteOrder->m_nManTouch = false;
                                pTrainRouteOrder->m_bRunNow = false;
                                continue;
                            }
                        }
                    }

                    //上次检查结果
                    int checkOld1 = pTrainRouteOrder->checkResultStaDetails;
                    int checkOld2 = pTrainRouteOrder->checkResultSequence;
                    int checkOld3 = pTrainRouteOrder->checkResultInterlock;
                    int checkOld4 = pTrainRouteOrder->checkResultFirstCheCi;

                    if(pTrainRouteOrder->m_nAutoTouch && pTrainRouteOrder->m_timCheck.date().year()>1970
                       && (checkOld1>0 || checkOld2>0 || checkOld3>0 || checkOld4>0))
                    {
                        //检查
                        //当前时间比检查时间晚，则为正值
                        qint64 seprtSeconds = pTrainRouteOrder->m_timCheck.secsTo(timeNow);
//                        qDebug()<<QString("%1%2进路间隔检查时间=%3").arg(pTrainRouteOrder->m_strTrainNum)
//                                  .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车").arg(seprtSeconds);
                        //自动触发进路指令发往联锁后，如果在45秒内进路没有排列出来，则CTC判断为联锁操作超时，等待45秒后，CTC将再次尝试自动触发进路。
                        //当前进路间隔30秒检查一次
                        if(seprtSeconds<30)//记得要修改为30秒
                        {
                            //continue;
                            //计划时间在前的某列列车的进路没有触发时，后续的进路也无法自动触发。
                            return;//直接退出后续进路的判定
                        }
                    }
                    pTrainRouteOrder->m_timCheck = timeNow;

                    //早点列车超过规定时间范围则不能触发
                    if(pTrainRouteOrder->m_nAutoTouch || pTrainRouteOrder->m_nManTouch)
                    {
                        qint64 seconds2 = timeNow.secsTo(pTrainRouteOrder->m_timBegin);
                        if(seconds2 > (EarlyTrainsTouchRangeMinutes*60))
                        {
                            pTrainRouteOrder->checkResultFirstCheCi = JLWARNING_SEQU_TIME;
                            qDebug()<<QString("%1次列车早点%2分钟，不可办理，规定时间%3分钟")
                                      .arg(pTrainRouteOrder->m_strTrainNum)
                                      .arg(seconds2/60)
                                      .arg(EarlyTrainsTouchRangeMinutes);
                            if(pTrainRouteOrder->m_nManTouch)
                            {
                                pTrainRouteOrder->m_nManTouch = false;
                            }
                            QString strSys = QString("%1次列车%2进路(%3)，触发失败，进路计划时间不在当前系统设定时间范围内（%4分钟）")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(pTrainRouteOrder->m_strRouteDescrip)
                                    .arg(EarlyTrainsTouchRangeMinutes);
                            //系统报警信息
                            this->sendWarningMsgToCTC(3,2,strSys);
                            continue;
                        }
                    }

                    QString strUnSameCheci;
                    bool bSameCheci = true;//CheckJCRouteSameCheciInJJQD(pTrainRouteOrder,strUnSameCheci);
                    //接车进路判定-检查接近区段的第一趟车次
                    if(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC)
                    {
                        bSameCheci = CheckJCRouteSameCheciInJJQD(pTrainRouteOrder,strUnSameCheci);
                    }
                    //发车进路判定-站内股道上的车次
                    else
                    {
                        bSameCheci = CheckFCRouteSameCheciInGD(pTrainRouteOrder,strUnSameCheci);
                    }
                    if(!bSameCheci)
                    {
                        CheckResult* ckResult = new CheckResult;
                        QDateTime timeNow = QDateTime::currentDateTime();
                        ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
                        ckResult->route_id = pTrainRouteOrder->route_id;
                        ckResult->bEnforced = false;
                        ckResult->checkMsg = QString("办理%1次进路与实际车次%2不一致")
                            .arg(pTrainRouteOrder->m_strTrainNum)
                            .arg(strUnSameCheci);
                        //人工办理时才发送强制执行窗口
                        if(pTrainRouteOrder->m_nManTouch)
                        {
                            //防错办报警信息
                            this->SendRouteCheckResult(ckResult, -1);//所有终端
                        }
                        QString strSys = QString("实际接车车次%1与计划接车车次%2不一致，请检查计划")
                                .arg(strUnSameCheci)
                                .arg(pTrainRouteOrder->m_strTrainNum);
                        this->sendWarningMsgToCTC(2,2,strSys);
                        pTrainRouteOrder->m_nManTouch = false;
                        pTrainRouteOrder->m_bRunNow = false;
                        
                        continue;
                    }

                    //检查所有条件
                    CheckResult* ckResult = CheckPreventConditionAll(pTrainRouteOrder);
                    //不满足
                    if(ckResult!=nullptr && ckResult->check!=0)
                    {
                        //************* 防溜检查 *************
                        //系统报警信息
                        QString strSys = ckResult->checkMsg;
                        //有防溜设备
                        if(JLWARNING_HAVEFLDEVSX == ckResult->check
                            || JLWARNING_HAVEFLDEVXX == ckResult->check
                            || JLWARNING_HAVEFLDEVSXX == ckResult->check)
                        {
                            QString strSXXFL;
                            if(JLWARNING_HAVEFLDEVSX == ckResult->check)
                            {
                                strSXXFL = QString("股道上行有防溜设备");
                            }
                            else if(JLWARNING_HAVEFLDEVXX == ckResult->check)
                            {
                                strSXXFL = QString("股道下行有防溜设备");
                            }
                            else
                            {
                                strSXXFL = QString("股道下行有防溜设备，股道上行有防溜设备");
                            }
                            strSys = QString("%1次股道%2%3%4")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack)
                                    .arg(strSXXFL)
                                    .arg(pTrainRouteOrder->m_nAutoTouch?QString("，不能自动办理"):QString(""));
                            //修改报警信息
                            ckResult->checkMsg = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，%2次股道%3%4")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack)
                                    .arg(strSXXFL);
                        }

                        //************* 联锁条件 *************
                        //区段占用
                        else if(JLWARNING_QDZY == ckResult->check)
                        {
                            strSys = QString("%1次%2进路(%3)触发时联锁条件不满足，进路不能办理，有区段占用")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(pTrainRouteOrder->m_strRouteDescrip);
                            //修改报警信息
                            ckResult->checkMsg = QString("车次%1 触发%2进路触发失败(与联锁逻辑冲突)，进路不能办理，有区段占用")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站");
                        }
                        //区段锁闭
                        else if(JLWARNING_QDSB == ckResult->check)
                        {
                            strSys = QString("%1次%2进路(%3)触发时联锁条件不满足，进路不能办理，有区段锁闭")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(pTrainRouteOrder->m_strRouteDescrip);
                            //修改报警信息
                            ckResult->checkMsg = QString("车次%1 触发%2进路触发失败(与联锁逻辑冲突)，进路不能办理，有区段锁闭")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站");
                        }
                        //道岔四开
                        else if(JLWARNING_DCSK == ckResult->check)
                        {
                            strSys = QString("%1次%2进路(%3)触发时联锁条件不满足，进路不能办理，道岔四开")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(pTrainRouteOrder->m_strRouteDescrip);
                            //修改报警信息
                            ckResult->checkMsg = QString("车次%1 触发%2进路触发失败(与联锁逻辑冲突)，进路不能办理，道岔四开")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站");
                        }
                        //封锁
                        else if(JLWARNING_FS_DC == ckResult->check
                           || JLWARNING_FS_GD == ckResult->check)
                        {
                            strSys = QString("%1次%2进路(%3)触发时联锁条件不满足，进路不能办理，进路设备封锁%4")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(pTrainRouteOrder->m_strRouteDescrip)
                                    .arg(ckResult->checkMsg);
                            //修改报警信息
                            ckResult->checkMsg = QString("车次%1 触发%2进路触发失败(与联锁逻辑冲突)，进路不能办理，进路设备封锁%3")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(ckResult->checkMsg);
                        }
                        //分路不良
                        else if(JLWARNING_FLBL_GD == ckResult->check
                           || JLWARNING_FLBL_WCQD == ckResult->check
                           || JLWARNING_FLBL_DC == ckResult->check)
                        {
                            strSys = QString("%1次%2进路(%3)联锁逻辑检查失败，进路不能办理，存在分路不良区段")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(pTrainRouteOrder->m_strRouteDescrip);
                            //修改报警信息
                            ckResult->checkMsg = QString("车次:%1触发未知的返回结果，进路不能办理，存在分路不良区段")
                                    .arg(pTrainRouteOrder->m_strTrainNum);
                        }
                        //分路不良空闲
                        else if(JLWARNING_FLBL_GDKX == ckResult->check)
                        {
                            strSys = QString("%1次%2进路(%3)联锁逻辑检查失败，进路不能办理，存在分路不良区段")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                                    .arg(pTrainRouteOrder->m_strRouteDescrip);
                            //修改报警信息
                            ckResult->checkMsg = QString("车次:%1触发未知的返回结果，进路不能办理，存在分路不良区段")
                                    .arg(pTrainRouteOrder->m_strTrainNum);
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }

                        //************* 站细条件 *************
                        //不满足-股道类型
                        else if(JLWARNING_ATTR_GDTYPE == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，列车%2次%3类型不满足")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            ckResult->checkMsg = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，列车%2次%3类型不满足")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                        }
                        //不满足-超限条件
                        else if(JLWARNING_ATTR_LEVELCX == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发未知的返回结果，%2次股道%3超限条件不满足")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次股道%3超限条件不满足")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }
                        //不满足-客运设备股道（站台）
                        else if(JLWARNING_ATTR_PLATFORM1 == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，旅客列车%2次无法接入无客运设备%3")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            ckResult->checkMsg = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，旅客列车%2次无法接入无客运设备%3")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                        }
                        //不满足-货车不可以接入高站台
                        else if(JLWARNING_ATTR_PLATFORM2 == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发未知的返回结果，货车%2次不能通过高站台股道%3")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            ckResult->checkMsg = QString("车次%1 触发未知的返回结果，货车%2次不能通过高站台股道%3")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }
                        //不满足-上水
                        else if(JLWARNING_ATTR_FLOWSS == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发未知的返回结果，%2次股道%3无上水设备")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次股道%3无上水设备")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }
                        //不满足-吸污
                        else if(JLWARNING_ATTR_FLOWXW == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发未知的返回结果，%2次股道%3无吸污设备")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次股道%3无吸污设备")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }
                        //不满足-出入口超限
                        else if(JLWARNING_ENEX_LEVELCX == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发未知的返回结果，%2次%3方向不允许超限车(出入口%4)")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
                            ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次%3方向不允许超限车(出入口%4)")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }
                        //不满足-客货类型错误
                        else if(JLWARNING_ENEX_KHTYPE == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发未知的返回结果，%2次%3方向客货类型错误(出入口%4)")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
                            ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次%3方向客货类型错误(出入口%4)")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }
                        //不满足-军运
                        else if(JLWARNING_ATTR_ARMY == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发未知的返回结果，军用列车%2次无法进入非军用股道%3")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            ckResult->checkMsg = QString("车次%1 触发未知的返回结果，军用列车%2次无法进入非军用股道%3")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrack);
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }
                        //不满足-列车固定径路信息
                        else if(JLWARNING_ENEX_UNSAME == ckResult->check)
                        {
                            //修改报警信息
                            strSys = QString("车次%1 触发未知的返回结果，%2次%3方向与固定路径不一致")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车");
                            ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次%3方向与固定路径不一致")
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_strTrainNum)
                                    .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车");
                            //可以强制执行
                            ckResult->bEnforced = true;
                        }

                        //************* 时序条件 *************
                        //不满足-车次冲突
                        else if((JLWARNING_SEQU_CCCT == ckResult->check || JLWARNING_SEQU_CROSS == ckResult->check)
                            && pTrainRouteOrder->m_nAutoTouch)
                        {
                            //修改报警信息
//                            strSys = QString("实际接车车次%1与计划接车车次%2不一致，请检查计划")
//                                    .arg(pTrainRouteOrder->m_strTrainNum)
//                                    .arg(ckResult->checkMsg);
//                            ckResult->checkMsg = QString("实际接车车次%1与计划接车车次%2不一致，请检查计划")
//                                    .arg(pTrainRouteOrder->m_strTrainNum)
//                                    .arg(ckResult->checkMsg);
                            strSys = ckResult->checkMsg;
                        }
                        //不满足-车次冲突
                        else if((JLWARNING_SEQU_CCCT == ckResult->check || JLWARNING_SEQU_CROSS == ckResult->check)
                            && pTrainRouteOrder->m_nManTouch)
                        {
                            strSys = ckResult->checkMsg;
                        }

                        //发送报警信息
                        //this->sendWarningMsgToCTC(4,2,pTrainRouteOrder->checkMsg);
                        //可强制执行，增加交互
                        if(ckResult->bEnforced)
                        {
                            UpdateCheckResultArray(ckResult);//交互，则增加
                        }
                        //人工触发才发送强制执行信息
//                        if(!pTrainRouteOrder->m_nAutoTouch
//                          ||(pTrainRouteOrder->m_nAutoTouch && JLWARNING_FLBL_GDKX==ckResult->check && !GDFLBLKXAutoTouch)
//                        )
                        if(  (pTrainRouteOrder->m_nManTouch && JLWARNING_FLBL_GDKX!=ckResult->check)
                           ||(pTrainRouteOrder->m_nManTouch && JLWARNING_FLBL_GDKX==ckResult->check && !GDFLBLKXAutoTouch) )
                        {
                            //防错办报警信息
                            this->SendRouteCheckResult(ckResult, -1);//所有终端//默认发送给第一个终端
                        }
                        ////自触模式只发送报警信息
                        //else
                        {
                            //系统报警信息
                            //TODO:实际接车车次与计划接车车次不一致报警等级又1级修改为2级 yzr
                            if(JLWARNING_SEQU_CCCT == ckResult->check)
                            {
                                this->sendWarningMsgToCTC(2,2,strSys);
                            }
                            //其余报警为一级
                            else
                            {
                                this->sendWarningMsgToCTC(1,2,strSys);
                            }
                        }
                        pTrainRouteOrder->m_nManTouch = false;
                        pTrainRouteOrder->m_bRunNow = false;

                        continue;
                    }
                    //满足条件
                    else
                    {
                        //若存在本进路的报警信息则删除，并继续执行后续的命令下达操作
                        UpdateCheckResultArray(ckResult, true);
                    }

                }

                //取消单机开行
                pTrainRouteOrder->m_bOnlyLocomotive = false;
                //满足条件
                //发送办理指令
                {
                    //发送指令
                    SendOrderToInterlock(pTrainRouteOrder);
                }
            }
        }
    }
}
//发送进路序列办理指令给联锁
void MyStation::SendOrderToInterlock(TrainRouteOrder *pTrainRouteOrder)
{
    //发送指令
    //数据帧转换
    QByteArray qSendArray = UnsignedCharToQByteArray(pTrainRouteOrder->m_byArrayUDPJLOrderDate, 30);
    //信号-发送数据给联锁
    emit sendDataToLSSignal(this, qSendArray, 30);
    qDebug()<<"进路序列办理命令:"<<ByteArrayToString0x(qSendArray);
    qDebug()<<pTrainRouteOrder->m_strTrainNum<<"进路序列办理:"<<pTrainRouteOrder->m_strRouteDescrip;
    //进路表单状态刷新
    pTrainRouteOrder->SetState(1);
    pTrainRouteOrder->m_nManTouch = false;
    pTrainRouteOrder->m_bRunNow = false;
    //记录最近一次办理时间
    this->m_LastTimeOfRouteDo = QDateTime::currentDateTime();
    //记录进路实际触发时间
    pTrainRouteOrder->m_timRealTouching = this->m_LastTimeOfRouteDo;
    pTrainRouteOrder->m_nTouchingCount++;
    //更新数据库
    m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
    qDebug()<<QString("%1 进路%2(%3)尝试第%4次触发")
              .arg(pTrainRouteOrder->m_timRealTouching.toString(TIME_FORMAT_YMDHMSM))
              .arg(pTrainRouteOrder->m_strTrainNum)
              .arg(pTrainRouteOrder->m_strRouteDescrip)
              .arg(pTrainRouteOrder->m_nTouchingCount);
    //发送同步消息
    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
}
//获取进路序列的联锁表进路索引并组织进路命令
int MyStation::GetTrainRouteOrderLSBRouteIndex(TrainRouteOrder *_pTrainRouteOrder)
{
    QStringList tempXhdArray;
    QStringList tempRouteBtnArray;
    QStringList strArray;
    //int count = StringSplit(_pTrainRouteOrder->m_strRouteDescripReal, _T(","), strArray); //_T("-");
    int count = 0;
    count = StringSplit(_pTrainRouteOrder->m_strRouteDescripReal, ",", strArray);
    //接车进路指令
    if (ROUTE_JC == _pTrainRouteOrder->m_btRecvOrDepart)
    {
        for (int t = 0; t<count; t++)
        {
            int xhCode = -1;
            QString jlxhName = strArray[t];
            QString devName = jlxhName;
            bool bDA = false;//调车按钮
            //if (devName.indexOf("DA") > -1)
            if (devName.right(2)== "DA")
            {
                bDA = true;
                //devName.replace("DA", "");
                devName = devName.left(devName.length()-2);
            }
            xhCode = GetCodeByStrName(devName);
            if (-1 < xhCode)
            {
                CXHD *pXHD = (CXHD*)DevArray[GetIndexByStrName(devName)];
                memcpy(&_pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2], &xhCode, 2);
                //int xhdType = GetXHDType(xhCode);
                int xhdType = pXHD->getXHDType();//进路序列不能使用GetXHDType()判定信号机类型，虚拟信号会显示为列按、调按两种
                _pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2 + 1] &= MASK05;
                if (xhdType == DC_XHJ || bDA)
                {
                    _pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2 + 1] |= JLBTN_DC;
                }
                else
                {
                    _pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2 + 1] |= JLBTN_LC;
                }
                if (pXHD->getSignalType() && (1==GetXXHDBtnCmdType(pXHD->getName())) )
                {
                    _pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2 + 1] |= JLBTN_DC;
                }
                //组织进路按钮
                tempXhdArray.append(strArray[t]);
                if (pXHD->getSignalType())
                {
                    tempRouteBtnArray.append(pXHD->getName());
                }
                else if (pXHD->getXHDType() == DC_XHJ)
                {
                    tempRouteBtnArray.append(pXHD->getName() + "A");
                }
                else if (bDA)//若需要按下列车信号机的调车按钮，则变通进路中的列车信号机需要配置“DA”！
                {
                    tempRouteBtnArray.append(jlxhName);
                }
                else
                {
                    tempRouteBtnArray.append(pXHD->getName() + "LA");
                }
            }
        }
    }
    //发车进路指令
    else if (ROUTE_FC == _pTrainRouteOrder->m_btRecvOrDepart)
    {
        for (int t = 0; t<count; t++)
        {
            int xhCode = -1;
            QString jlxhName = strArray[t];
            QString devName = jlxhName;
            bool bDA = false;//调车按钮
            if (devName.right(2) == ("DA") && GetCodeByStrName(devName) > -1)
            {
                //信号机名称包含DA
                bDA = false;
            }
            else if (devName.right(2) == ("DA")) //devName.indexOf("DA") > -1
            {
                bDA = true;
                devName.replace("DA", "");
            }
            xhCode = GetCodeByStrName(devName);
            if (-1 < xhCode)
            {
                CXHD *pXHD = (CXHD*)DevArray[GetIndexByStrName(devName)];
                memcpy(&_pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2], &xhCode, 2);
                //int xhdType = GetXHDType(xhCode);
                int xhdType = pXHD->getXHDType();//进路序列不能使用GetXHDType()判定信号机类型，虚拟信号会显示为列按、调按两种
                _pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2 + 1] &= MASK05;
                if (xhdType == DC_XHJ || bDA)
                {
                    _pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2 + 1] |= JLBTN_DC;
                }
                else
                {
                    _pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2 + 1] |= JLBTN_LC;
                }
                if (pXHD->getSignalType() && (1==GetXXHDBtnCmdType(pXHD->getName())) )
                {
                    _pTrainRouteOrder->m_byArrayUDPJLOrderDate[11 + t * 2 + 1] |= JLBTN_DC;
                }
                //组织进路按钮
                tempXhdArray.append(strArray[t]);
                if (pXHD->getSignalType())
                {
                    tempRouteBtnArray.append(pXHD->getName());
                }
                else if (pXHD->getXHDType() == DC_XHJ)
                {
                    tempRouteBtnArray.append(pXHD->getName() + "A");
                }
                else if (bDA)//若需要按下列车信号机的调车按钮，则变通进路中的列车信号机需要配置“DA”！
                {
                    tempRouteBtnArray.append(jlxhName);
                }
                else
                {
                    tempRouteBtnArray.append(pXHD->getName() + "LA");
                }
            }
        }
    }
    _pTrainRouteOrder->tempXhdArray.clear();
    _pTrainRouteOrder->tempXhdArray.append(tempXhdArray);
    _pTrainRouteOrder->tempRouteBtnArray.clear();
    _pTrainRouteOrder->tempRouteBtnArray.append(tempRouteBtnArray);
    //进路索引获取
    _pTrainRouteOrder->m_nIndexRoute = FindRouteIndexInLSB(ROUTE_LC, tempRouteBtnArray);

    return _pTrainRouteOrder->m_nIndexRoute;
}
//获得信号机的类型 DC_XHJ CZ_XHJ JZ_XHJ
int MyStation::GetXHDType(int xhdCode)
{
    CXHD *pXHD = NULL;
    for (int i = 0; i < DevArray.count(); i++)
    {
        if (DevArray[i]->getDevType() == Dev_XH)
        {
            pXHD = (CXHD*)DevArray[i];
            if (xhdCode == (int)pXHD->m_nCode)
            {
                if (pXHD->getSignalType())
                {
                    return DC_XHJ;
                }
                else
                {
                    return pXHD->getXHDType();
                }
            }
        }
    }
    return -1;
}
//查找将要办理的进路在联锁表中的索引
int MyStation::FindRouteIndexInLSB(QString JLType, QStringList &routeBtnArray)
{
    //进路按钮按下的个数或顺序
    int nbtnNum = routeBtnArray.count();//xhdNameArray.count();
    int rtCount = vectInterlockRoute.count();
    bool bMatch = false;

    //进路按钮组合成一个字符串，再对比，更精准
    QString strAllRouteBtn;
    for(int i=0; i<routeBtnArray.count(); i++)
    {
        if(i!=0)
        {
            strAllRouteBtn.append(",");
        }
        strAllRouteBtn.append(routeBtnArray[i]);
    }

    //查找匹配的联锁表进路
    for(int i = 0; i < rtCount && nbtnNum>=2; i++)
    {
        InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[i]);

        //进路按钮组合成一个字符串
        QString strAllRouteBtn1;
        for(int j=0; j<pRut->BtnArr.count(); j++)
        {
            if(j!=0)
            {
                strAllRouteBtn1.append(",");
            }
            strAllRouteBtn1.append(pRut->BtnArr[j]);
        }
        if(strAllRouteBtn == strAllRouteBtn1)
        {
            return i;
        }
        else
        {
            continue;
        }

        //if(pRut->Type != JLType)
        //{
        //	continue;
        //}

        ////联锁表进路按钮个数
        //int routeBtnCount = pRut->BtnArr.count();
        ////仅判断按钮个数一致的
        //if(routeBtnCount != nbtnNum)
        //{
        //	continue;
        //}

        ////缩小范围，进路始端一致的
        //if(xhdNameArray[0] == pRut->strXHD)
        //{
        //	//联锁表进路按钮信号机的名称数组
        //	QStringArray rtXhdArray;
        //	for(int b=0; b<routeBtnCount; b++)
        //	{
        //		rtXhdArray.Add(GetLSBRouteBtnXhdName(pRut->BtnArr[b]));
        //	}

        //	//比较信号机名称
        //	for(int b=0; b<routeBtnCount; b++)
        //	{
        //		if(rtXhdArray[b] != xhdNameArray[b])
        //		{
        //			bMatch = FALSE;
        //			break;
        //		}
        //		else
        //		{
        //			bMatch = TRUE;
        //		}
        //	}
        //	//已匹配进路
        //	if(bMatch)
        //	{
        //		return i;
        //	}
        //}
    }
    if(!bMatch)
    {
        //AfxMessageBox(_T("没有找到匹配的联锁表进路!"));
    }

    return -1;
}

//更新出入口进路预告信息
void MyStation::UpdateRoutePreWndInfo()
{
    //互斥锁
    QMutexLocker locker(&Mutex);
    //Mutex.lock();

    //进路预告窗口
    for(int i=0; i<vectRoutePreWnd.size(); i++)
    {
        RoutePreWnd *pRoutePreWnd = vectRoutePreWnd[i];
        //先清除，再增加
        pRoutePreWnd->vectRouteInfo.clear();
        //匹配进路序列并增加
        for(int j=0; j<m_ArrayRouteOrder.size(); j++)
        {
            TrainRouteOrder* pTrainRouteOrder = m_ArrayRouteOrder[j];
            if(pTrainRouteOrder->m_nOldRouteState == 4)// >= 2 出清
            {
                continue;//已结出清的进路则删除进路预告窗中相应的车次
            }
            if(pTrainRouteOrder->m_bDeleteFlag)//删除标志
            {
                continue;
            }
            //组合进路的子序列不显示
            if(pTrainRouteOrder->father_id > 0)
            {
                continue;
            }
            //人工办理进路时生成的序列不显示
            if(pTrainRouteOrder->m_bCreateByMan)
            {
                continue;
            }

            if(pRoutePreWnd->juncXHDName == pTrainRouteOrder->m_strXHD_JZk
                    || pRoutePreWnd->juncXHDName == pTrainRouteOrder->m_strXHD_CZk)
            {
                if(pRoutePreWnd->vectRouteInfo.size()<3)
                {
                    RoutePreWnd::RouteInfo route;
                    route.route_id = pTrainRouteOrder->route_id;
                    route.CheCi = pTrainRouteOrder->m_strTrainNum;
                    route.GDName = pTrainRouteOrder->m_strTrack;
                    route.m_nKHType = pTrainRouteOrder->m_nLHFlg;
                    if(pTrainRouteOrder->m_btBeginOrEndFlg==JFC_TYPE_TG)
                    {
                        route.routeType = 3;//通过
                    }
                    else if(pTrainRouteOrder->m_btRecvOrDepart==0)
                    {
                        route.routeType = 1;//接车
                    }
                    else
                    {
                        route.routeType = 2;//发车
                    }
                    if(pTrainRouteOrder->m_nOldRouteState==2)
                    {
                        route.routeState = 2;//触发完成
                    }
                    else if(pTrainRouteOrder->m_nOldRouteState==3)
                    {
                        route.routeState = 3;//占用
                    }
                    else if(pTrainRouteOrder->m_nAutoTouch)
                    {
                        route.routeState = 1;//自触
                    }
                    else
                    {
                        route.routeState = 0;//非自触
                    }
                    //组合进路的主序列状态特殊判定
                    if(pTrainRouteOrder->m_bZHJL)
                    {
                        if(IsSonRouteOrderHaveState(pTrainRouteOrder->route_id,3))
                        {
                            route.routeState = 3;//占用
                        }
                        else if(IsSonRouteOrderHaveState(pTrainRouteOrder->route_id,2))
                        {
                            route.routeState = 2;//触发完成
                        }
                        else if(IsSonRouteOrderHaveState(pTrainRouteOrder->route_id,1))
                        {
                            route.routeState = 1;//自触
                        }
                        else
                        {
                            route.routeState = 0;//非自触
                        }
                    }
                    pRoutePreWnd->vectRouteInfo.append(route);
                }
            }
        }
    }
    //Mutex.unlock();
}

//检查股道车次及信号状态，并发送发车命令
void MyStation::CheckGdTrainStatus()
{
    if(this->ModalSelect.nModeState != 0)//0中心模式
    {
        return;//不是中心模式，则退出
    }
    TrainRouteOrder* pTrainRouteOrder;
    CGD *pGD;
    CXHD *pXHD;
    for(int i=0; i<this->vectStationGDNode.size(); i++)
    {
        int mgdCode = 0;
        mgdCode = this->GetIndexByStrName(this->vectStationGDNode[i].strGDName);
        if(mgdCode == -1)
            continue;//防错
        pGD = (CGD *)this->DevArray[mgdCode];
        if(pGD->m_strCheCiNum != "" && (pGD->m_bLCTW==TRUE) && (pGD->getGDType() == GD_QD))//车次停稳
        {
            //qDebug()<<"int CheckGdTrainStatus() 0";
            int xhdCode=0;
            //车次左行右行 0 左行 1右行
            if(0 == pGD->m_nSXCheCi)
            {
                QString leftXHD = this->vectStationGDNode[i].strLeftNode;
                xhdCode = this->GetIndexByStrName(leftXHD);
            }
            else
            {
                QString RightXHD = this->vectStationGDNode[i].strRightNode;
                xhdCode = this->GetIndexByStrName(RightXHD);
            }
            if(xhdCode > 0)
            {
                pXHD = (CXHD *)this->DevArray[xhdCode];
                //qDebug()<<"int CheckGdTrainStatus() "<<pXHD->getName();
                int state = pXHD->getXHDState();
                //出站信号机绿灯 or 绿黄 or 单黄 or 双黄 or 双绿
                if((state == XHD_LD) || (state == XHD_LU) || (state == XHD_UD) || (state == XHD_UU) || (state == XHD_LL))
                {
                    //qDebug()<<"int CheckGdTrainStatus() 1";
                    //SendTrainStartCmd(pGD->m_strCheCiNum, pGD->m_nCode);
                    //pGD->m_nCheciLost = FALSE;
                    //break;

                    int nRindex = this->GetTrainRouteIndex(pGD->m_strCheCiNum, ROUTE_FC);
                    if(nRindex > -1)
                    {
                        //qDebug()<<"int CheckGdTrainStatus() 2";
                        pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[nRindex];
                        QDateTime timeNow = QDateTime::currentDateTime();
                        //按照计划时间进行发车，20190315
                        if(timeNow.toString(TIME_FORMAT_HM) == pTrainRouteOrder->m_timPlanned.toString(TIME_FORMAT_HM))
                        {
                            //qDebug()<<"int CheckGdTrainStatus() 3";
                            SendTrainStartCmd(pGD->m_strCheCiNum, pGD->m_nCode);
                            pGD->m_nCheciLost = FALSE;
                            break;
                        }
                    }

                }
            }
        }
    }
}
//自动发车 发送发车命令
void MyStation::SendTrainStartCmd(QString checi, int gdCode)
{
    //与联锁通信，将车次发送给联锁
    int nLength = 0;  //升级为不定长通信协议
    unsigned int count=0;
    unsigned int nameLength=0;
    unsigned char byArrayUDPJLOrderDate[1024];
    memset(byArrayUDPJLOrderDate,0,1024);

    byArrayUDPJLOrderDate[9]  = 0xac;//信息分类码：车次
    byArrayUDPJLOrderDate[10] = 0x05;//操作类型：发车
    QByteArray barray = checi.toLatin1();
    nameLength=barray.count();
    memcpy(&byArrayUDPJLOrderDate[11], &nameLength, 1);
    //车次 升级为不定长
    for(int u=0; u<barray.count(); u++)
    {
        byArrayUDPJLOrderDate[12+u] = barray[u];
    }
    count = 12+nameLength;
    memcpy(&byArrayUDPJLOrderDate[count], &gdCode, 2);//股道号
    count += 2;

    nLength=count+4;
    memcpy(&byArrayUDPJLOrderDate[4], &nLength, 2);//帧长度
    for(int i = 0; i < 4; i++)
    {
        byArrayUDPJLOrderDate[i] = 0xEF;
        byArrayUDPJLOrderDate[nLength - i -1] = 0xFE;
    }
    //发送信息
    //数据帧转换
    QByteArray qSendArray = UnsignedCharToQByteArray(byArrayUDPJLOrderDate, nLength);
    //信号-发送数据给联锁
    emit sendDataToLSSignal(this, qSendArray, nLength);
}

//列车位置信息处理
//CTC3.0占线板列车接近信息实时更新
void MyStation::SetTrainPosStatusInTrafficLog()
{
    for (int i = 0; i < this->m_ArrayTrafficLog.size(); i++)
    {
        TrafficLog* pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[i];
        bool bUpdate = this->SetPlanTrainPosStatus(pTrafficLog);
        //有变化时更新数据库
        //if(pTrafficLog->m_strTrainPosStatusOld != pTrafficLog->m_strTrainPosStatus)
        if(bUpdate && pTrafficLog->m_strTrainPosStatusOld != pTrafficLog->m_strTrainPosStatus)
        {
            m_pDataAccess->UpdateTrafficLogOtherInfo(pTrafficLog);
            //发送同步消息
            //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
        }
    }
}
//设置计划的列车位置状态,返回是否已更新
bool MyStation::SetPlanTrainPosStatus(TrafficLog *pTrafficLog)
{
    Train* pTrain = nullptr;
    CGDDC* pGDDC = nullptr;
    for (int j = 0; j < m_ArrayTrain.size(); j++)
    {
        pTrain = (Train *)m_ArrayTrain[j];
        if (pTrafficLog->m_strTrainNum == pTrain->m_strCheCi)
        {
            pGDDC = (CGDDC*)GetDCQDByCode(pTrain->nLastGDCode);
            if(pTrain->nNowGDCode != pTrain->m_nPosCodeReal)
            {
                pTrain->nLastGDCode = pTrain->nNowGDCode;
                pTrain->nNowGDCode = pTrain->m_nPosCodeReal;
            }
            break;
        }
    }

    bool bUpdate = false;
    int nJJindex = GetTrainJJQDindex(pTrafficLog);
    if (0 == nJJindex)
    {
        pTrafficLog->m_strTrainPosStatusOld = pTrafficLog->m_strTrainPosStatus;
        pTrafficLog->m_strTrainPosStatus = STATUS_LC1FQ;//列车距离本站1个分区
        bUpdate = true;
    }
    else if (1 == nJJindex)
    {
        pTrafficLog->m_strTrainPosStatusOld = pTrafficLog->m_strTrainPosStatus;
        pTrafficLog->m_strTrainPosStatus = STATUS_LC2FQ;//列车距离本站2个分区
        bUpdate = true;
    }
    else if (2 == nJJindex)
    {
        pTrafficLog->m_strTrainPosStatusOld = pTrafficLog->m_strTrainPosStatus;
        pTrafficLog->m_strTrainPosStatus = STATUS_LC3FQ;//列车距离本站3个分区
        bUpdate = true;
    }
    else if (true == FindCheCiInTrainArr(pTrafficLog->m_strTrainNum) && pTrafficLog->m_strTrainPosStatusOld == STATUS_LC1FQ)
    {
        pTrafficLog->m_strTrainPosStatusOld = pTrafficLog->m_strTrainPosStatus;
        pTrafficLog->m_strTrainPosStatus = STATUS_LCYJZ;//列车进站
        bUpdate = true;
    }
    else if(GetGdStatusGDZY(pTrafficLog->m_strTrainTrack, pTrafficLog->m_strTrainNum) && pTrain->m_nPosCode == pTrain->m_nPosCodeReal)
    {
        if(pTrafficLog->m_btBeginOrEndFlg == 0xBB)
        {
            pTrafficLog->m_strTrainPosStatusOld = pTrafficLog->m_strTrainPosStatus;
            pTrafficLog->m_strTrainPosStatus = STATUS_LCYWZJRGD;//列车完整进入股道
            bUpdate = true;
        }
        else if(pGDDC)
        {
            if(pGDDC->getQDState() == QDZY || pGDDC->getQDState() == QDSB)
            {
                pTrafficLog->m_strTrainPosStatusOld = pTrafficLog->m_strTrainPosStatus;
                pTrafficLog->m_strTrainPosStatus = STATUS_LCYKSJRGD;//列车开始进入股道
                bUpdate = true;
            }
            else if(pGDDC->getQDState() == QDKX)
            {
                pTrafficLog->m_strTrainPosStatusOld = pTrafficLog->m_strTrainPosStatus;
                pTrafficLog->m_strTrainPosStatus = STATUS_LCYWZJRGD;//列车完整进入股道
                bUpdate = true;
            }
        }
    }
    else if (FindCheCiInTrainArr(pTrafficLog->m_strTrainNum)
             && (pTrafficLog->m_strTrainPosStatusOld == STATUS_LCYWZJRGD || pTrafficLog->m_strTrainPosStatusOld == STATUS_LCYKSJRGD)
             && (false == GetJLXLState_FC(pTrafficLog->m_strTrainTrack, pTrafficLog->m_strTrainNum)))
    {
        pTrafficLog->m_strTrainPosStatusOld = pTrafficLog->m_strTrainPosStatus;
        pTrafficLog->m_strTrainPosStatus = STATUS_LCYCZ;//列车已出站
        bUpdate = true;
    }
    else if(FindCheCiInTrainArr(pTrafficLog->m_strTrainNum) == false && pTrafficLog->m_strTrainPosStatus == STATUS_LCYCZ)
    {
        pTrafficLog->m_strTrainPosStatus = STATUS_LCYWC;//列车已完成
        bUpdate = true;
    }
    SetTrafficLogProc(pTrafficLog);
    return bUpdate;
}
//获取股道车次的占用情况
bool MyStation::GetGdStatusGDZY(QString strGdname, QString strCheCi)
{
    int mgdCode = -1;
    mgdCode = GetIndexByStrName(strGdname);
    if (mgdCode > -1)
    {
        CGD* pGD = (CGD *)DevArray[mgdCode];
//        if (pGD->getState(QDZY) && (pGD->m_strCheCiNum == strCheCi))
//            return true;
        if(pGD->m_strCheCiNum == strCheCi)
        {
            if (pGD->getState(QDZY))
            {
                return true;
            }
        }
    }
    return false;
}
//获取列车在接近区段的索引
int MyStation::GetTrainJJQDindex(TrafficLog *pTrafficLog)
{
    for (int i = 0; i < StaConfigInfo.JCKCount; i++)
    {
        if (pTrafficLog->m_strXHD_JZk == StaConfigInfo.JFCKInfo[i].strJCKName)
        {
            int jjCount = StaConfigInfo.JFCKInfo[i].strArrJckJJQD.size();
            for (int j = 0; j < jjCount && j < 3; j++)
            {
                QString strJJQDName = StaConfigInfo.JFCKInfo[i].strArrJckJJQD[j];
                if (true == GetGdStatusGDZY(strJJQDName, pTrafficLog->m_strTrainNum))
                {
                    return j;
                }
            }
        }
    }
    return -1;
}
//查找车次是否存在
bool MyStation::FindCheCiInTrainArr(QString strCheCi)
{
    int ccCount = m_ArrayTrain.size();
    for (int j = 0; j < ccCount; j++)
    {
        Train* pTrain = (Train *)m_ArrayTrain[j];
        if (strCheCi == pTrain->m_strCheCi && 0 < pTrain->m_nPosCode)
        {
            return true;
        }
    }
    return false;
}
//判断进路序列是否已执行完毕
bool MyStation::GetJLXLState_JC(QString strGdname, QString strCheCi)
{
    TrainRouteOrder* pTrainRouteOrder;
    for(int i = 0; i < m_ArrayRouteOrder.size(); i++)
    {
        pTrainRouteOrder = (TrainRouteOrder*)m_ArrayRouteOrder[i];
        if (((pTrainRouteOrder->m_btRecvOrDepart==0)&&((pTrainRouteOrder->m_nOldRouteState==1)||(pTrainRouteOrder->m_nOldRouteState==2)||(pTrainRouteOrder->m_nOldRouteState==3)))
            &&(pTrainRouteOrder->m_strTrack==strGdname)&&(pTrainRouteOrder->m_strTrainNum==strCheCi))
        {
            return true;
        }
    }
    return false;
}
//判断进路序列是否已执行完毕
bool MyStation::GetJLXLState_FC(QString strGdname, QString strCheCi)
{
    TrainRouteOrder* pTrainRouteOrder;
    for(int i = 0; i < m_ArrayRouteOrder.size(); i++)
    {
        pTrainRouteOrder = (TrainRouteOrder*)m_ArrayRouteOrder[i];
        if ((pTrainRouteOrder->m_btRecvOrDepart==1)&&((pTrainRouteOrder->m_nOldRouteState==0)||(pTrainRouteOrder->m_nOldRouteState==1)||(pTrainRouteOrder->m_nOldRouteState==2)/*||(pTrainRouteOrder->m_nOldRouteState==3)*/)
            &&(pTrainRouteOrder->m_strTrack==strGdname)&&(pTrainRouteOrder->m_strTrainNum==strCheCi))
        {
            return true;
        }
    }
    return false;
}
//判断接车进路或者发车进路//0 为接车,1为发车，2为不动作
int MyStation::GetJLXLState_JC_FC(QString strGdname, QString strCheCi)
{
    TrainRouteOrder* pTrainRouteOrder;
    for(int i = 0; i < m_ArrayRouteOrder.size(); i++)
    {
        pTrainRouteOrder = (TrainRouteOrder*)m_ArrayRouteOrder[i];
        if((pTrainRouteOrder->m_strTrack==strGdname) && (pTrainRouteOrder->m_strTrainNum==strCheCi))
        {
            if (pTrainRouteOrder->m_btRecvOrDepart==0)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }
    return 2;
}

//由非常站控转回到车站控制时，自触标志全部清除。
void MyStation::ClearTrainRouteAutoTouch()
{
    bool update = false;
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //等待状态的自动触发进路
        if(0 == pTrainRouteOrder->m_nOldRouteState && pTrainRouteOrder->m_nAutoTouch)
        {
            update = true;
            pTrainRouteOrder->m_nAutoTouch = false;
            m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
        }
    }
    if(update)
    {
        //发送同步消息
        //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
    }
}
//接收联锁数据-非常站控转换
void MyStation::recvLSData_FCZK(unsigned char *recvArray, int recvlen)
{
    //转为分散自律
    if((int)recvArray[10] == 0x11)
    {
        this->m_nFCZKMode = false;
        this->ModalSelect.nModeState = 1;
        this->nOldModeState = -1;
        this->m_bHaveRoutePermissions = true;
        ClearTrainRouteAutoTouch();
        m_pDataAccess->UpdateStationInfo(this);
        //发送同步消息
        //sendUpdateDataMsg(this, UPDATETYPE_KZMS);
        QString msg = QString("车站控制模式已转换");//or控制模式已转换?
        qDebug()<<this->getStationName()<<msg;
        //发送报警提示信息
        this->sendWarningMsgToCTC(3,1,QString("车站控制模式已转换"));
        //语音播报
        this->SendSpeachText(msg);
        //从分散自律转到非常站控时，自触标志不变；由非常站控转回到车站控制时，自触标志全部清除。
        this->SetRouteAutoTouchState(false);
        CancleFlblKXFalg();
    }
    //转为非常站控
    else if((int)recvArray[10] == 0x22)
    {
        this->m_nFCZKMode = true;
        this->RoutePermit = 0;
        this->nOldModeState = -1;
        if(FCZKSetAutoTouch)
        {
            for(int i = 0; i < m_ArrayRouteOrder.size(); i++)
            {
                if(m_ArrayRouteOrder.at(i)->m_bZHJL == 0)
                {
                    m_ArrayRouteOrder.at(i)->m_nAutoTouch = true;
                    m_pDataAccess->UpdateRouteOrder(m_ArrayRouteOrder.at(i));
                    this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,m_ArrayRouteOrder.at(i),SYNC_FLAG_UPDATE,1,1);
                }
            }
        }
        m_pDataAccess->UpdateStationInfo(this);
        //发送同步消息
        //sendUpdateDataMsg(this, UPDATETYPE_KZMS);
        QString msg = QString("非常站控模式已转换");//or车站进入非常站控?
        qDebug()<<this->getStationName()<<msg;
        //发送报警提示信息
        this->sendWarningMsgToCTC(3,1,msg);
        //语音播报
        this->SendSpeachText(msg);
    }
}
//更新计划的进路办理状态
void MyStation::UpdateTrafficlogJLSuccessed(bool bFCJL, QString strcheci, bool bSucc, bool bIng)
{
    for (int i = 0; i < this->m_ArrayTrafficLog.count(); i++)
    {
        TrafficLog* pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[i];
        //发车
        if (true == bFCJL && strcheci == pTrafficLog->m_strDepartTrainNum)
        {
            int oldStatus = pTrafficLog->m_btFCJLStatus;
            if(bIng)//正在办理
            {
                pTrafficLog->m_btFCJLStatus = STATUS_JLBL_ING;
            }
            else
            {
                if (bSucc)
                {
                    pTrafficLog->m_btFCJLStatus = STATUS_JLBL_SUCC;
                    if (pTrafficLog->bXianLuSuo)
                    {
                        //线路所为通过，仅一条进路
                        pTrafficLog->m_btJCJLStatus = STATUS_JLBL_SUCC;
                    }
                }
                else
                {
                    pTrafficLog->m_btFCJLStatus = STATUS_JLBL_ING;//STATUS_JLBL_NO;
                }
                SetTrafficLogProc(pTrafficLog);
            }
            if(oldStatus != pTrafficLog->m_btFCJLStatus)
            {
                //更新数据库
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
        //接车
        else if (false == bFCJL && strcheci == pTrafficLog->m_strReachTrainNum)
        {
            int oldStatus = pTrafficLog->m_btJCJLStatus;
            if(bIng)
            {
                pTrafficLog->m_btJCJLStatus = STATUS_JLBL_ING;
            }
            else
            {
                if (bSucc)
                {
                    pTrafficLog->m_btJCJLStatus = STATUS_JLBL_SUCC;
                    if (pTrafficLog->bXianLuSuo)
                    {
                        //线路所为通过，仅一条进路
                        pTrafficLog->m_btFCJLStatus = STATUS_JLBL_SUCC;
                    }
                }
                else
                {
                    pTrafficLog->m_btJCJLStatus = STATUS_JLBL_ING;//STATUS_JLBL_NO;
                }
                SetTrafficLogProc(pTrafficLog);
            }
            if(oldStatus != pTrafficLog->m_btJCJLStatus)
            {
                //更新数据库
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
    }
}

//接收CTC数据-处理行车日志计划信息
void MyStation::recvCTCData_TrafficLogInfo(unsigned char *recvArray, int recvlen, int currCtcIndex, int currZxbIndex)
{
    if(FUNCTYPE_TRAFFIC != (int)recvArray[9])
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);

    //行车日志报点操作
    if(0x01 == (int)recvArray[10])
    {
        //互斥锁，作用域结束自动解锁
        QMutexLocker locker(&Mutex);

        int idLog = (int)(recvArray[11] | (recvArray[12]<<8));
        for(int i=0; i<this->m_ArrayTrafficLog.count(); i++)
        {
            TrafficLog* pTrafficLog = (TrafficLog* )this->m_ArrayTrafficLog[i];
            if(idLog == pTrafficLog->log_id)
            {
                //物理删除
                bool bDelete = false;
                //时间
                QDateTime tmRecv;
                int year = (int)(recvArray[14] | recvArray[15]<<8);
                int mouth = (int)recvArray[16];
                int day = (int)recvArray[17];
                int hour = (int)recvArray[18];
                int mini = (int)recvArray[19];
                int second = (int)recvArray[20];
                QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
                //qDebug()<<"strDateTime="<<strDateTime;
                tmRecv = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");

                QDateTime tmDefault;
                if(PLAN_CMD_FCYG == (int)recvArray[13])//预告（发预）
                {
                    //pTrafficLog->m_timToAdjtStaAgrDepaTrain = tmRecv;

                    SendReportTimeToLS(pTrafficLog->m_strDepartTrainNum, PLAN_CMD_FCYG);
                    //给邻站报点(报给下一个站)
                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_CZk);//出站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strDepartTrainNum, PLAN_CMD_FCYG, tmRecv);
                    }
                    else
                    {
                        //未接邻站时，模拟邻站自动同意
                        pTrafficLog->m_timToAdjtStaAgrDepaTrain = tmRecv;//邻站同意发车时间
                    }
                    SetTrafficLogProc(pTrafficLog);
                    //设置自动触发
                    if(AutoTouchMinitesWhenNoticed>=0)
                    {
                        //找发车进路
                        int routIndex = this->GetTrainRouteIndex(pTrafficLog->m_strDepartTrainNum, ROUTE_FC);
                        if(-1 < routIndex)
                        {
                            TrainRouteOrder* pRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[routIndex];
                            //方法1：数组存储对象，线程定时去执行
                            pRouteOrder->m_timAutoTouchTemp = QDateTime::currentDateTime().addSecs(AutoTouchMinitesWhenNoticed*60);
                            AddLZMNRouteOrder(pRouteOrder);
                            //方法2： 下面的方法QTimer::singleShot无法执行，原因未知！！2024.2.21.lwm
//                            //启动个1分钟的定时器
//                            //定时60秒后执行，仅执行一次
//                            //毫秒
//                            int milliseconds = AutoTouchMinitesWhenNoticed*60*1000;
//                            qDebug()<<QString("定时%1毫秒将执行进路自动办理").arg(milliseconds);
//                            QTimer::singleShot(milliseconds, this, [&, pRouteOrder](){
//                                qDebug()<<QString("定时%1毫秒执行函数AutoTouchLZMNRouteOrder，车次=%2")
//                                          .arg(milliseconds).arg(pRouteOrder->m_strTrainNum);
//                                AutoTouchLZMNRouteOrder(pRouteOrder);
//                            }
//                            );
                        }
                    }
                }
                else if(PLAN_CMD_TYYG == (int)recvArray[13])//同意（接预）
                {
                    pTrafficLog->m_timAgrFromAdjtStaDepaTrain = tmRecv;
                    SendReportTimeToLS(pTrafficLog->m_strReachTrainNum, PLAN_CMD_TYYG);
                    //给邻站报点(报给上一个站)
                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_JZk);//进站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strReachTrainNum, PLAN_CMD_TYYG, tmRecv);
                    }
                    SetTrafficLogProc(pTrafficLog);
                    //设置自动触发
                    if(AutoTouchMinitesWhenNoticed>=0)
                    {
                        //找接车进路
                        int routIndex = this->GetTrainRouteIndex(pTrafficLog->m_strReachTrainNum, ROUTE_JC);
                        if(-1 < routIndex)
                        {
                            TrainRouteOrder* pRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[routIndex];
                            //方法1：数组存储对象，线程定时去执行
                            pRouteOrder->m_timAutoTouchTemp = QDateTime::currentDateTime().addSecs(AutoTouchMinitesWhenNoticed*60);
                            AddLZMNRouteOrder(pRouteOrder);
                            //方法2：下面的方法QTimer::singleShot无法执行，原因未知！！2024.2.21.lwm
//                            //启动个1分钟的定时器
//                            //定时60秒后执行，仅执行一次
//                            //毫秒
//                            int milliseconds = 3000;//AutoTouchMinitesWhenNoticed*60*1000;
//                            qDebug()<<QString("定时%1毫秒将执行进路自动办理").arg(milliseconds);
//                            QTimer::singleShot(milliseconds, this, [&, pRouteOrder](){
//                                qDebug()<<QString("定时%1毫秒执行函数AutoTouchLZMNRouteOrder，车次=%2")
//                                          .arg(milliseconds).arg(pRouteOrder->m_strTrainNum);
//                                AutoTouchLZMNRouteOrder(pRouteOrder);
//                            }
//                            );

                        }
                    }
                }
                else if(PLAN_CMD_DDBD == (int)recvArray[13])//到达
                {
                    if(pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_SF)
                    {
                        pTrafficLog->m_timRealReachStation = tmRecv;
                        QDateTime tempDate;
                        if(pTrafficLog->m_timRealDepaTrain.isNull())
                            tempDate = pTrafficLog->m_timProvDepaTrain;
                        else tempDate = pTrafficLog->m_timRealDepaTrain;

                        if(tempDate.toString("hh:mm") == pTrafficLog->m_timRealReachStation.toString("hh:mm"))
                        {
                            pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_TG;
                        }
                        else if(pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_SF && pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_ZD)
                        {
                            pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_JF;
                        }
                        SetTrafficLogProc(pTrafficLog);
                        //给邻站报点(报给上一个站)
                        int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_JZk);//进站口
                        if(lzId > 0)
                        {
                            emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strReachTrainNum, 0x22, tmRecv);
                        }
                    }
                }
                else if(PLAN_CMD_CFBD == (int)recvArray[13])//出发
                {
                    if(pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_ZD)
                    {
                        pTrafficLog->m_timRealDepaTrain = tmRecv;//CTime::GetCurrentTime();

                        QDateTime tempDate;
                        if(pTrafficLog->m_timRealReachStation.isNull())
                            tempDate = pTrafficLog->m_timProvReachStation;
                        else tempDate = pTrafficLog->m_timRealReachStation;

                        if(pTrafficLog->m_timRealDepaTrain.toString("hh:mm") == tempDate.toString("hh:mm"))
                        {
                            pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_TG;
                        }
                        else if(pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_SF && pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_ZD)
                        {
                            pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_JF;
                        }

                        //给邻站报点(报给下一个站)
                        int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_CZk);//出站口
                        if(lzId > 0)
                        {
                            emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strDepartTrainNum, 0x11, tmRecv);
                        }
                        SetTrafficLogProc(pTrafficLog);
                    }
                }
                else if(PLAN_CMD_TGBD == (int)recvArray[13])//通过
                {
                    pTrafficLog->m_timRealReachStation = tmRecv;//CTime::GetCurrentTime();
                    pTrafficLog->m_timRealDepaTrain = pTrafficLog->m_timRealReachStation;
                    pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_TG;
                    //给邻站报点(报给下一个站)
                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_CZk);//出站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strDepartTrainNum, 0x11, tmRecv);
                    }
                    //给邻站报点(报给上一个站)
                    lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_JZk);//进站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strReachTrainNum, 0x22, tmRecv);
                    }
                    SetTrafficLogProc(pTrafficLog);
                }
                else if(PLAN_CMD_LZCF == (int)recvArray[13])//邻站(邻站同意)
                {
                    pTrafficLog->m_timToAdjtStaAgrDepaTrain = tmRecv;
                    //pTrafficLog->m_timFromAdjtStaDepaTrain = tmRecv;//CTime::GetCurrentTime();
                }
                else if(PLAN_CMD_QXJC == (int)recvArray[13])//取消接车0x07
                {
                    pTrafficLog->m_timRealReachStation = tmDefault;
                    if(pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_SF && pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_ZD)
                    {
                        QDateTime tempDate;
                        if(pTrafficLog->m_timRealDepaTrain.isNull())
                        {
                            tempDate = pTrafficLog->m_timProvDepaTrain;
                        }
                        else
                        {
                            tempDate = pTrafficLog->m_timRealDepaTrain;
                        }
                        //如果实际到达时间与实际发车时间相同，自动改为通过
                        if(pTrafficLog->m_timProvReachStation != tempDate)
                            pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_JF;
                        else pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_TG;
                        SetTrafficLogProc(pTrafficLog);
                    }
                    //本站处理，不影响邻站
//                    //给邻站报点(报给上一个站)
//                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_JZk);//进站口
//                    if(lzId > 0)
//                    {
//                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strReachTrainNum, 0x07, tmRecv);
//                    }
                }
                else if(PLAN_CMD_QXBS == (int)recvArray[13])//取消闭塞0x08
                {
                    pTrafficLog->m_timToAdjtStaAgrDepaTrain = tmDefault;
                    //给邻站报点(报给下一个站)
                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_CZk);//出站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strDepartTrainNum, 0x08, tmRecv);
                    }
                }
                else if(PLAN_CMD_QXFC == (int)recvArray[13])//取消发车0x09
                {
                    pTrafficLog->m_timRealDepaTrain = tmDefault;
                    if(pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_SF && pTrafficLog->m_btBeginOrEndFlg != JFC_TYPE_ZD)
                    {
                        QDateTime tempDate;
                        if(pTrafficLog->m_timRealReachStation.isNull())
                            tempDate = pTrafficLog->m_timProvReachStation;
                        else tempDate = pTrafficLog->m_timRealDepaTrain;
                        //如果实际到达时间与实际发车时间相同，自动改为通过
                        if(pTrafficLog->m_timProvDepaTrain != tempDate)
                            pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_JF;
                        else pTrafficLog->m_btBeginOrEndFlg = JFC_TYPE_TG;
                    }
                    SetTrafficLogProc(pTrafficLog);
                    //本站处理，不影响邻站
//                    //给邻站报点(报给下一个站)
//                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_CZk);//出站口
//                    if(lzId > 0)
//                    {
//                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strDepartTrainNum, 0x09, tmRecv);
//                    }
                }
                else if(PLAN_CMD_QXFY == (int)recvArray[13])//取消发预0x0A
                {
                    pTrafficLog->m_timToAdjtStaAgrDepaTrain = tmDefault;

                    //给邻站报点(报给下一个站)
                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_CZk);//出站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strDepartTrainNum, 0x0A, tmRecv);
                    }
                    SetTrafficLogProc(pTrafficLog);
                }
                else if(PLAN_CMD_QXJY == (int)recvArray[13])//取消接预0x0B
                {
                    pTrafficLog->m_timAgrFromAdjtStaDepaTrain = tmDefault;

                    //给邻站报点(报给上一个站)
                    int lzId = GetStationJFCKLZStationId(pTrafficLog->m_strXHD_JZk);//进站口
                    if(lzId > 0)
                    {
                        emit UpdateLZReportTimeSignal(lzId, pTrafficLog->m_strReachTrainNum, 0x22, tmRecv);
                    }
                    SetTrafficLogProc(pTrafficLog);
                }
                else if(PLAN_CMD_LZFC == (int)recvArray[13])//邻站发车0x0C
                {
                    pTrafficLog->m_timFromAdjtStaDepaTrain = tmRecv;
                }
                else if(PLAN_CMD_LZDD == (int)recvArray[13])//邻站到达0x0D
                {
                    pTrafficLog->m_timtoAdjtStation = tmRecv;
                }
                else if(PLAN_CMD_DELE == (int)recvArray[13])//删除0x1C
                {
                    bDelete = true;
                }
                qDebug() << "recvCTCData TrafficLogInfo" << pTrafficLog->log_id << (int)recvArray[13];

                if(ManSignRouteSynType && bDelete)
                {
                    pTrafficLog->m_bDelete = true;
                    m_ArrayTrafficLogChg.append(pTrafficLog);
                    return;
                }

                //删除
                if(bDelete)
                {
                    m_pDataAccess->DeleteTrafficLog(pTrafficLog);
                    //发送同步消息
                    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_DELETE,1,1);
                    //删除相应的进路序列
                    DeleteRouteOrderByTrafficLog(pTrafficLog);
                    qDebug() << "删除行车日志" << pTrafficLog->m_strTrainNum;
                    this->m_ArrayTrafficLog.removeAt(i);
                }
                //更新
                else
                {
                    m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                    //发送同步消息
                    this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                    //qDebug() << "send sync11111 " << pTrafficLog->m_nIndexLCLX << pTrafficLog->m_nIndexYXLX;
                }
                return;
            }
        }
    }
    //行车日志其他内容操作
    if(0x02 == (int)recvArray[10])
    {
        //互斥锁，作用域结束自动解锁
        QMutexLocker locker(&Mutex);

        int idLog = (int)(recvArray[11] | (recvArray[12]<<8));
        for(int i=0; i<this->m_ArrayTrafficLog.count(); i++)
        {
            TrafficLog* pTrafficLog = (TrafficLog* )this->m_ArrayTrafficLog[i];
            if(idLog == pTrafficLog->log_id)
            {
                int nCount = 13;
                //记事
                int txtLen = (int)recvArray[nCount];
                nCount += 1;
                unsigned char strRecvTxt[100];
                memset(strRecvTxt,0,sizeof(strRecvTxt));
                memcpy(strRecvTxt, &recvArray[nCount], txtLen);
                QString strNotes = UnsignedCharArrayToString(strRecvTxt);
                //QString strNotes = ByteArrayToUnicode(qRecvArray.mid(nCount,txtLen));
                pTrafficLog->m_strNotes = strNotes;
                nCount += txtLen;
                qDebug()<<"行车日志记事："<<pTrafficLog->m_strNotes;

                if(ManSignRouteSynType)
                {
                    m_ArrayTrafficLogChg.append(pTrafficLog);
                    return;
                }
                //更新数据库
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                //退出
                return;
            }
        }
    }
}
//根据行车日志删除进路序列
void MyStation::DeleteRouteOrderByTrafficLog(TrafficLog *pTrafficLog)
{
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //接车进路
        if(pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC
            && pTrainRouteOrder->m_strTrainNum == pTrafficLog->m_strReachTrainNum)
        {
            //更新数据库
            m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
            //发送同步消息
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
            this->m_ArrayRouteOrder.removeAt(i);
            break;
        }
    }
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //发车进路
        if(pTrainRouteOrder->m_btRecvOrDepart == ROUTE_FC
            && pTrainRouteOrder->m_strTrainNum == pTrafficLog->m_strDepartTrainNum)
        {
            //更新数据库
            m_pDataAccess->DeleteRouteOrder(pTrainRouteOrder);
            qDebug() << "DeleteRouteOrder" << pTrainRouteOrder->route_id << pTrainRouteOrder->m_strTrainNum;
            //发送同步消息
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_DELETE,1,1);
            this->m_ArrayRouteOrder.removeAt(i);
            break;
        }
    }
    //删除信息发送给联锁
    this->MakeAndSendPlanUDP(this, pTrafficLog, 0x01);
}

//向联锁发送车次报点信息供判定
void MyStation::SendReportTimeToLS(QString strCheCi, int type)
{
    int nLength = 40;
    QByteArray qSendArray;
    qSendArray.append(nLength, char(0));
    for(int u=0;u<4;u++)
    {
        qSendArray[u] = 0xEF;
        qSendArray[nLength-u-1] = 0xFE;
    }
    qSendArray[4] = nLength;
    qSendArray[9] = 0x05;//（CTC车务终端----->联锁仿真机）
    qSendArray[10] = type;
    QString strNumber = strCheCi;
    QByteArray barray = strNumber.toLatin1();
    for(int u=0; u<barray.count(); u++)
    {
        qSendArray[11+u] = barray[u];
    }
    //信号-发送数据给联锁
    emit sendDataToLSSignal(this, qSendArray, nLength);
}
//接收CTC数据-处理调度命令(签收)
void MyStation::recvCTCData_DisOrderInfo(unsigned char *recvArray, int recvlen)
{
    if(FUNCTYPE_DISPTCH != (int)recvArray[9])
    {
        return;
    }
    //数据帧
    QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);

    //签收操作
    if(0x01 == (int)recvArray[10])
    {
        int id = (int)(recvArray[11] | (recvArray[12]<<8));
        for(int i=0; i<this->m_ArrayDisOrderRecv.count(); i++)
        {
            DispatchOrderStation* pDisOrderRecv = (DispatchOrderStation* )this->m_ArrayDisOrderRecv[i];
            if(id == pDisOrderRecv->order_id)
            {
                if(0xAA == (int)recvArray[13])//签收
                {
                    pDisOrderRecv->nStateDisOrder = 3;
                    pDisOrderRecv->timSign = QDateTime::currentDateTime();
                    pDisOrderRecv->strSignName = "值班员";
                    {
                        //转发回执给联锁
                        //数据帧
                        QByteArray qSendArray;
                        qSendArray.append(40, char(0));
                        for(int u=0;u<4;u++)
                        {
                            qSendArray[u] = 0xEF;
                            qSendArray[40-u-1] = 0xFE;
                        }
                        qSendArray[4] = 0x28;//40
                        qSendArray[9] = 0x04;//信息分类码（CTC->联锁）调度命令签收
                        QString strNumber = pDisOrderRecv->strNumber;
                        QByteArray barray = strNumber.toLatin1();
                        for(int u=0; u<barray.count(); u++)
                        {
                            qSendArray[10+u] = barray[u];
                        }
                        //信号-发送数据给联锁
                        emit sendDataToLSSignal(this, qSendArray, 40);
                    }
                }
                else if(0xBB == (int)recvArray[13])//拒签
                {
                    pDisOrderRecv->nStateDisOrder = 4;
                    pDisOrderRecv->strSignName = "值班员";
                    pDisOrderRecv->timSign = QDateTime::currentDateTime();
                }
                //更新数据库
                m_pDataAccess->UpdateDisOrderRecv(pDisOrderRecv);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_DDML);
                //处理和发送1个数据
                this->sendOneDisOrderToSoft(DATATYPE_ALL,pDisOrderRecv,SYNC_FLAG_UPDATE,1,1);
                return;
            }
        }
    }
    //调度命令转发机车（新建机车命令）
    else if(0x02 == (int)recvArray[10])
    {
        DispatchOrderLocomotive* pDispOrderJC = new DispatchOrderLocomotive;
        int nCount = 11;
        //调度命令ID
        int id = (int)(qRecvArray[nCount] | (qRecvArray[nCount+1]<<8));
        nCount+=2;
        DispatchOrderStation* pDispOrderRecv = this->GetDisOrderRecvById(id);
        if(nullptr == pDispOrderRecv)
        {
            return;//防错
        }
        pDispOrderJC->station_id = pDispOrderRecv->station_id;
        pDispOrderJC->orderType = 0x01;//调度命令
        pDispOrderJC->uNumber = pDispOrderRecv->uNumber;
        pDispOrderJC->strType = pDispOrderRecv->strType;
        pDispOrderJC->strContent = pDispOrderRecv->strContent;
        pDispOrderJC->strStation = this->getStationName();
        pDispOrderJC->strDutyName = pDispOrderRecv->strSignName;
        pDispOrderJC->timCreate = pDispOrderRecv->timOrder;
        QDateTime timeNow = QDateTime::currentDateTime();
        pDispOrderJC->timSend = timeNow;
        pDispOrderJC->bSend=true;
        //车次（机车）个数
        int countjc = (int)(qRecvArray[nCount]&0xFF);
        nCount++;
        for(int i=0; i<countjc; i++)
        {
            int len = 0;
            QString str;
            LocomotiveInfo locomInfo;
            //车次号
            len = (int)(qRecvArray[nCount]&0xFF);
            nCount++;
            str = ByteArrayToUnicode(qRecvArray.mid(nCount,len));
            nCount += len;
            locomInfo.strCheCi = str;
            //机车号
            len = (int)(qRecvArray[nCount]&0xFF);
            nCount++;
            str = ByteArrayToUnicode(qRecvArray.mid(nCount,len));
            nCount += len;
            locomInfo.strLocomotive = str;
            locomInfo.timRecv = timeNow;
            locomInfo.nRecvState = 1;
            //增加一条
            pDispOrderJC->vectLocmtInfo.append(locomInfo);
        }
        //插入数据库
        pDispOrderJC->order_id = m_pDataAccess->InsertDisOrderLocom(pDispOrderJC);
        //增加一条
        this->m_ArrayDisOrderLocomot.append(pDispOrderJC);
        //发送同步消息-处理和发送1个数据
        this->sendOneDisOrderJCToSoft(DATATYPE_ALL,pDispOrderJC,SYNC_FLAG_ADD,1,1);
        return;
    }
    //调度命令转发给信号员
    else if(0x03 == (int)recvArray[10])
    {
        //TODO??
        ;
    }
    //新建的调度台调度命令
    else if(0x04 == (int)recvArray[10])
    {
        DispatchOrderDispatcher* disOrderDDT = this->updateDisorderDDT(qRecvArray);
        if(disOrderDDT->order_id==0)
        {
            //插入数据库
            disOrderDDT->order_id = m_pDataAccess->InsertDisOrderDisp(disOrderDDT);
            //本站添加
            this->m_ArrayDisOrderDisp.append(disOrderDDT);
            //发送同步数据消息-处理和发送1个数据
            this->sendOneDisOrderDDTToSoft(DATATYPE_ALL,disOrderDDT,SYNC_FLAG_ADD,1,1);
        }
        else
        {
            if(disOrderDDT->bDel)
            {
                //更新数据库
                m_pDataAccess->DeleteDisOrderDisp(disOrderDDT);
                //发送同步数据消息-处理和发送1个数据
                this->sendOneDisOrderDDTToSoft(DATATYPE_ALL,disOrderDDT,SYNC_FLAG_DELETE,1,1);
                m_pDataAccess->SelectAllDisOrderDisp(this);
            }
            else
            {
                //更新数据库
                m_pDataAccess->UpdateDisOrderDisp(disOrderDDT);
                //发送同步数据消息-处理和发送1个数据
                this->sendOneDisOrderDDTToSoft(DATATYPE_ALL,disOrderDDT,SYNC_FLAG_UPDATE,1,1);
                m_pDataAccess->SelectAllDisOrderDisp(this);
            }
        }
    }
    //新建的机车调度命令
    else if(0x05 == (int)recvArray[10])
    {
        DispatchOrderLocomotive* pDispOrderJC = this->updateDisorderJC(qRecvArray);
        if(pDispOrderJC->order_id==0)
        {
            //插入数据库
            pDispOrderJC->order_id = m_pDataAccess->InsertDisOrderLocom(pDispOrderJC);
            //本站添加
            this->m_ArrayDisOrderLocomot.append(pDispOrderJC);
            //发送同步数据消息-处理和发送1个数据
            this->sendOneDisOrderJCToSoft(DATATYPE_ALL,pDispOrderJC,SYNC_FLAG_ADD,1,1);
        }
        else
        {
            if(pDispOrderJC->bDel)
            {
                //更新数据库
                m_pDataAccess->DeleteDisOrderLocom(pDispOrderJC);
                //发送同步数据消息-处理和发送1个数据
                this->sendOneDisOrderJCToSoft(DATATYPE_ALL,pDispOrderJC,SYNC_FLAG_DELETE,1,1);
                m_pDataAccess->SelectAllDisOrderLocom(this);
            }
            else
            {
                //更新数据库
                m_pDataAccess->UpdateDisOrderLocom(pDispOrderJC);
                //发送同步数据消息-处理和发送1个数据
                this->sendOneDisOrderJCToSoft(DATATYPE_ALL,pDispOrderJC,SYNC_FLAG_UPDATE,1,1);
                m_pDataAccess->SelectAllDisOrderLocom(this);
            }
        }
    }
}
//接收CTC数据-处理作业流程信息（行车日志计划）
void MyStation::recvCTCData_JobFlowInfo(unsigned char *RecvArray, int recvlen, int currCtcIndex, int currZxbIndex)
{
    if(FUNCTYPE_FLOWS != (int)RecvArray[9])
    {
        return;
    }

    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    //站内作业流程操作
    //防溜设置操作
    if(0x01 == RecvArray[10])
    {
        int devCode = (int)(RecvArray[11] | (RecvArray[12]<<8));
        //QString devName = this->GetStrNameByCode(devCode);
        //int devIndex = this->GetIndexByStrName(devName);
        //if(devIndex <= -1)
        //    return;
        //CGD* pGD = (CGD*)this->DevArray[devIndex];
        for (int i=0; i<m_ArrayGDAntiSlip.size(); i++)
        {
            CGD* pGD = m_ArrayGDAntiSlip[i];
            if(devCode == (int)pGD->getCode())
            {
                //防溜类型
                int flType = (int)RecvArray[15];
                if(0xAA == (int)RecvArray[13])//左侧
                {
                    if(0xAA == (int)RecvArray[14])//设置防溜
                    {
                        pGD->m_nLAntiSlipType = flType;
                        pGD->m_nLTxNum = (int)RecvArray[16];//左侧铁鞋号
                        pGD->m_nLJgqNum = (int)RecvArray[17];//左侧紧固器号
                        pGD->m_nLJnMeters = (int)(RecvArray[18] | (RecvArray[19]<<8));//左侧警内米数
                        pGD->m_nTrainNums = (int)(RecvArray[20] | (RecvArray[21]<<8));//存车信息
                    }
                    else if(0xBB == (int)RecvArray[14])//取消防溜
                    {
                        pGD->m_nLAntiSlipType = 0;//&= ~flType;
                        pGD->m_nLTxNum = 0;//左侧铁鞋号
                        pGD->m_nLJgqNum = 0;//左侧紧固器号
                        pGD->m_nLJnMeters = 0;//左侧警内米数
                        pGD->m_nTrainNums = (int)(RecvArray[20] | (RecvArray[21]<<8));//存车信息
                    }
                    if(pGD->m_nLTxNum==0)
                    {
                        int a;
                        a=0;
                    }
                    qDebug()<<"recvCTCData_JobFlowInfo"<<pGD->getName()<<pGD->m_nLTxNum<<pGD->m_nRTxNum;
                }
                if(0xBB == (int)RecvArray[13])//右侧
                {
                    if(0xAA == (int)RecvArray[14])//设置防溜
                    {
                        pGD->m_nRAntiSlipType = flType;
                        pGD->m_nRTxNum = (int)RecvArray[16];//右侧铁鞋号
                        pGD->m_nRJgqNum = (int)RecvArray[17];//右侧紧固器号
                        pGD->m_nRJnMeters = (int)(RecvArray[18] | (RecvArray[19]<<8));//右侧警内米数
                        pGD->m_nTrainNums = (int)(RecvArray[20] | (RecvArray[21]<<8));//存车信息
                    }
                    else if(0xBB == (int)RecvArray[14])//取消防溜
                    {
                        pGD->m_nRAntiSlipType = 0;//&= ~flType;
                        pGD->m_nRTxNum = 0;//右侧铁鞋号
                        pGD->m_nRJgqNum = 0;//右侧紧固器号
                        pGD->m_nRJnMeters = 0;//右侧警内米数
                        pGD->m_nTrainNums = (int)(RecvArray[20] | (RecvArray[21]<<8));//存车信息
                    }
                }
                //存车信息 股道栏显示 新协议字符串显示
                int nlen=(int)(RecvArray[22]&0xFF);
                QString strTxt;
                QByteArray mArray= UnsignedCharToQByteArray(RecvArray,recvlen);
                strTxt = ByteArrayToUnicode(mArray.mid(23,nlen));
                pGD->m_sTrainInfoShow= strTxt;
                //qDebug()<<"fangliu"<<pGD->m_sTrainInfoShow;
                //更新数据库
                m_pDataAccess->UpdateGDAntiSlip(this->getStationID(), pGD);
                //更新防溜
                this->SetAllGDAntiSlip();
                //发送同步消息
                this->sendAllGDAntiSlipToSoft(DATATYPE_ALL,SYNC_FLAG_UPDATE);
                break;
            }
        }

    }
    //作业流程操作
    else if(0x02 == RecvArray[10])
    {
        int idLog = (int)(RecvArray[11] | (RecvArray[12]<<8));
        for(int i=0; i<this->m_ArrayTrafficLog.count(); i++)
        {
            TrafficLog* pTrafficLog = (TrafficLog* )this->m_ArrayTrafficLog[i];
            if(idLog == pTrafficLog->log_id)
            {
                //作业类型
                BYTE flowType = (BYTE)RecvArray[13];
                //作业操作
                BYTE flowOper = (BYTE)RecvArray[14];
                //操作时间
                QDateTime tmRecv;// = CTime::GetCurrentTime();
                int year = (int)(RecvArray[15] | RecvArray[16]<<8);
                int mouth = (int)RecvArray[17];
                int day = (int)RecvArray[18];
                int hour = (int)RecvArray[19];
                int mini = (int)RecvArray[20];
                int second = (int)RecvArray[21];
                QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
                tmRecv = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
                //作业
                if(DEF_FLOW_JAL == flowType)
                {
                    pTrafficLog->m_btJALStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_LJ == flowType)
                {
                    pTrafficLog->m_btLJStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_SS == flowType)
                {
                    pTrafficLog->m_btSSStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_XW == flowType)
                {
                    pTrafficLog->m_btXWStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_JP == flowType)
                {
                    pTrafficLog->m_btJPStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_CJ == flowType)
                {
                    pTrafficLog->m_btCJStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_ZG == flowType)
                {
                    pTrafficLog->m_btZGStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_LW == flowType)
                {
                    pTrafficLog->m_btLWStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_HJ == flowType)
                {
                    pTrafficLog->m_btHJStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_HC == flowType)
                {
                    pTrafficLog->m_btHCStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_ZX == flowType)
                {
                    pTrafficLog->m_btZXStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_JC == flowType)
                {
                    pTrafficLog->m_btJCStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_DK == flowType)
                {
                    pTrafficLog->m_btDKStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_CH == flowType)
                {
                    pTrafficLog->m_btCHStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_ZK == flowType)
                {
                    pTrafficLog->m_btZKStatus = GetJobFlowStatusFlag(flowOper);
                }
                else if(DEF_FLOW_ZW == flowType)
                {
                    pTrafficLog->m_btZWStatus = GetJobFlowStatusFlag(flowOper);
                }

                if(ManSignRouteSynType)
                {
                    m_ArrayTrafficLogChg.append(pTrafficLog);
                    return;
                }
                //更新数据库
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ, -1, currZxbIndex);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                return;
            }
        }
    }
}
//设置该站所有的股道防溜
void MyStation::SetAllGDAntiSlip()
{
    //互斥锁
    //QMutexLocker locker(&Mutex);

    for(int i=0; i<m_ArrayGDAntiSlip.count(); i++)
    {
        CGD* pGDTemp = (CGD*)m_ArrayGDAntiSlip[i];
        int devIndex = GetIndexByStrName(pGDTemp->m_strName);
        if(devIndex <= -1)
            break;
        CGD* pGD = (CGD*)DevArray[devIndex];
        pGD->m_nLAntiSlipType = pGDTemp->m_nLAntiSlipType;
        pGD->m_nLTxNum = pGDTemp->m_nLTxNum;
        pGD->m_nLJgqNum = pGDTemp->m_nLJgqNum;
        pGD->m_nLJnMeters =pGDTemp->m_nLJnMeters;
        pGD->m_nRAntiSlipType = pGDTemp->m_nRAntiSlipType;
        pGD->m_nRTxNum = pGDTemp->m_nRTxNum;
        pGD->m_nRJgqNum = pGDTemp->m_nRJgqNum;
        pGD->m_nRJnMeters = pGDTemp->m_nRJnMeters;
        pGD->m_nTrainNums = pGDTemp->m_nTrainNums;
        pGD->m_sTrainInfoShow = pGDTemp->m_sTrainInfoShow;
    }
}
//清除该站所有的股道防溜
void MyStation::ClearAllGDAntiSlip()
{
    for(int i=0; i<m_ArrayGDAntiSlip.count(); i++)
    {
        CGD* pGD = (CGD*)m_ArrayGDAntiSlip[i];
        pGD->m_nLAntiSlipType = 0;
        pGD->m_nLTxNum = 0;
        pGD->m_nLJgqNum = 0;
        pGD->m_nLJnMeters = 0;
        pGD->m_nRAntiSlipType = 0;
        pGD->m_nRTxNum = 0;
        pGD->m_nRJgqNum = 0;
        pGD->m_nRJnMeters = 0;
        pGD->m_nTrainNums = 0;
    }
    this->SetAllGDAntiSlip();
}
//接收占线板数据-计划执行命令信息
void MyStation::recvBoradData_PlanCmdInfo(unsigned char *recvArray, int recvlen)
{
    //行车日志车次操作
    if((int)recvArray[9] == PLAN_CMD_TYPE)//0x51
    {
        int nCount=10;
        //车次长度
        int lenCC = recvArray[11];
        //车次
        QString strCheci;
        unsigned char strRecvCC[20];
        memset(strRecvCC,0,sizeof(strRecvCC));
        memcpy(strRecvCC, &recvArray[12], lenCC);
        strCheci = UnsignedCharArrayToString(strRecvCC);

#pragma region //计划车次-接车预告（同意预告）
        if((int)recvArray[10] == PLAN_CMD_TYYG)//0x02
        {
            //行车日志中找到该车次，并赋值同意预告时间
            int index = this->GetIndexInTrafficArray(strCheci);
            if(index > -1)
            {
                TrafficLog *pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[index];
                pTrafficLog->m_timAgrFromAdjtStaDepaTrain = QDateTime::currentDateTime();
                //给联锁发送回执
                //同意预告网络接口
                {
                    QByteArray qSendArray;
                    qSendArray.append(20, char(0));
                    for(int u=0;u<4;u++)
                    {
                        qSendArray[u] = 0xEF;
                        qSendArray[20-u-1] = 0xFE;
                    }
                    qSendArray[4] = 0x14;//20
                    qSendArray[9] = 0x06;//信息分类码
                    QString strNumber = strCheci;
                    QByteArray barray = strNumber.toLatin1();
                    for(int u=0; u<barray.count(); u++)
                    {
                        qSendArray[10+u] = barray[u];
                    }
                    //SendUDPDataToLS(pStation, qSendArray, 20);
                    //信号-发送数据给联锁
                    emit sendDataToLSSignal(this, qSendArray, 20);
                }
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
#pragma endregion

#pragma region //计划车次-接车进路办理
        else if((int)recvArray[10] == PLAN_CMD_JCJL)//0x07
        {
            //进路序列中找到该车次的接车进路并发送进路命令
            this->SendRouteCommandBoard1(strCheci, ROUTE_JC);
            //return;
        }
#pragma endregion

#pragma region //计划车次-到点
        else if((int)recvArray[10] == PLAN_CMD_DDBD)//0x03
        {
            //行车日志中找到该车次，并赋值到点时间
            int index = this->GetIndexInTrafficArray(strCheci);
            if(index > -1)
            {
                TrafficLog *pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[index];
                pTrafficLog->m_timRealReachStation = QDateTime::currentDateTime();
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
#pragma endregion

#pragma region //计划车次-发车预告
        else if((int)recvArray[10] == PLAN_CMD_FCYG)//0x01
        {
            //行车日志中找到该车次，并赋值同意预告时间
            int index = this->GetIndexInTrafficArray(strCheci);
            if(index > -1)
            {
                TrafficLog *pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[index];
                pTrafficLog->m_timToAdjtStaAgrDepaTrain = QDateTime::currentDateTime();//自动同意
                //给联锁发送回执
                //发车预告网络接口
                {
                    QByteArray qSendArray;
                    qSendArray.append(20, char(0));
                    for(int u=0;u<4;u++)
                    {
                        qSendArray[u] = 0xEF;
                        qSendArray[20-u-1] = 0xFE;
                    }
                    qSendArray[4] = 0x14;//20
                    qSendArray[9] = 0x05;//信息分类码
                    QString strNumber = strCheci;
                    QByteArray barray = strNumber.toLatin1();
                    for(int u=0; u<barray.count(); u++)
                    {
                        qSendArray[10+u] = barray[u];
                    }
                    //SendUDPDataToLS(pStation, qSendArray, 20);
                    //信号-发送数据给联锁
                    emit sendDataToLSSignal(this, qSendArray, 20);
                }
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                //return;
            }
        }
#pragma endregion

#pragma region //计划车次-发车进路办理
        else if((int)recvArray[10] == PLAN_CMD_FCJL)//0x08
        {
            //进路序列中找到该车次的发车进路并发送进路命令
            this->SendRouteCommandBoard1(strCheci, ROUTE_FC);
        }
#pragma endregion

#pragma region //计划车次-发点
        else if((int)recvArray[10] == PLAN_CMD_CFBD)//0x04
        {
            //行车日志中找到该车次，并赋值到点时间
            int index = this->GetIndexInTrafficArray(strCheci);
            if(index > -1)
            {
                TrafficLog *pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[index];
                pTrafficLog->m_timRealDepaTrain = QDateTime::currentDateTime();
                SetTrafficLogProc(pTrafficLog);
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
#pragma endregion

#pragma region //计划车次-取消接车进路
        else if((int)recvArray[10] == PLAN_CMD_QXJL)//0x09
        {
            //查找列车进路序列并更新为未办理
            FindAndUpdateRouteWaite(strCheci, ROUTE_JC);
        }
#pragma endregion

#pragma region //计划车次-取消发车进路
        else if((int)recvArray[10] == PLAN_CMD_QXFL)//0x0A
        {
            //查找列车进路序列并更新为未办理
            FindAndUpdateRouteWaite(strCheci, ROUTE_FC);
        }
#pragma endregion

#pragma region //计划车次-取消接车预告
        else if ((int)recvArray[10] == PLAN_CMD_QXJY)//0x0B
        {
            //AfxMessageBox(TEXT("取消接车预告") + strCheci);
            int index = this->GetIndexInTrafficArray(strCheci);
            if (index > -1)
            {
                TrafficLog *pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[index];
                QDateTime time;
                pTrafficLog->m_timAgrFromAdjtStaDepaTrain = time;
                if(pTrafficLog->m_timProvReachStation == pTrafficLog->m_timProvDepaTrain)
                    pTrafficLog->m_btBeginOrEndFlg = 0xdd;
                else pTrafficLog->m_btBeginOrEndFlg = 0xaa;
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
#pragma endregion

#pragma region //计划车次-取消发车预告
        else if ((int)recvArray[10] == PLAN_CMD_QXFY)//0x0C
        {
            //AfxMessageBox(TEXT("取消发车预告") + strCheci);
            int index = this->GetIndexInTrafficArray(strCheci);
            if (index > -1)
            {
                TrafficLog *pTrafficLog = (TrafficLog*)this->m_ArrayTrafficLog[index];
                QDateTime time;
                pTrafficLog->m_timToAdjtStaAgrDepaTrain = time;
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_XCRZ);
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            }
        }
#pragma endregion

#pragma region //计划车次-通过进路办理
        else if((int)recvArray[10] == PLAN_CMD_TGJL)//0x0D
        {
            //进路序列中找到该车次的进路并发送进路命令
            this->SendRouteCommandBoard1(strCheci, ROUTE_TONGG);
        }
#pragma endregion

#pragma region //计划车次-变通进路处理
        //计划车次-修改接车变通进路
        else if((int)recvArray[10] == PLAN_CMD_CHGJCBTJL)
        {
            //双机版时CTC转发给CTC的CTC通道
            //if(CTCDEVTYPE_SINGLE != pFram->GlobalInfo.nCTCDevType)
//            {
//                //数据帧转换
//                QByteArray qSendArray = UnsignedCharToQByteArray(m_chArrayRecvDataBuffToBoard1, nFrameLength);
//                SendUDPDataToCTC(pStation, qSendArray, nFrameLength);
//            }
            int desIndex = recvArray[21];
            ChangeRouteCommand(strCheci, ROUTE_JC, desIndex);
        }
        //计划车次-修改发车变通进路
        else if((int)recvArray[10] == PLAN_CMD_CHGFCBTJL)
        {
            //双机版时CTC转发给CTC的CTC通道
            //if(CTCDEVTYPE_SINGLE != pFram->GlobalInfo.nCTCDevType)
//            {
//                //数据帧转换
//                QByteArray qSendArray = UnsignedCharToQByteArray(m_chArrayRecvDataBuffToBoard1, nFrameLength);
//                SendUDPDataToCTC(pStation, qSendArray, nFrameLength);
//            }
            int desIndex = recvArray[21];
            ChangeRouteCommand(strCheci, ROUTE_FC, desIndex);
        }
        //计划车次-修改接车变通进路并办理进路
        else if((int)recvArray[10] == PLAN_CMD_JCBTJL)
        {
            //双机版时CTC转发给CTC的CTC通道
            //if(CTCDEVTYPE_SINGLE != pFram->GlobalInfo.nCTCDevType)
//            {
//                //数据帧转换
//                QByteArray qSendArray = UnsignedCharToQByteArray(m_chArrayRecvDataBuffToBoard1, nFrameLength);
//                SendUDPDataToCTC(pStation, qSendArray, nFrameLength);
//            }
            int desIndex = recvArray[21];
            ChangeRouteCommand(strCheci, ROUTE_JC, desIndex);
            this->SendRouteCommandBoard1(strCheci, ROUTE_JC);
        }
        //计划车次-修改发车变通进路并办理进路
        else if((int)recvArray[10] == PLAN_CMD_FCBTJL)
        {
            //双机版时CTC转发给CTC的CTC通道
            //if(CTCDEVTYPE_SINGLE != pFram->GlobalInfo.nCTCDevType)
//            {
//                //数据帧转换
//                QByteArray qSendArray = UnsignedCharToQByteArray(m_chArrayRecvDataBuffToBoard1, nFrameLength);
//                SendUDPDataToCTC(pStation, qSendArray, nFrameLength);
//            }
            int desIndex = recvArray[21];
            ChangeRouteCommand(strCheci, ROUTE_FC, desIndex);
            this->SendRouteCommandBoard1(strCheci, ROUTE_FC);
        }
#pragma endregion

    }
}
//进路序列中找到该车次相应类型的进路并发送进路命令（定时器自动发送）
void MyStation::SendRouteCommandBoard1(QString strCheci, int jltype)
{
    if(ROUTE_JC !=jltype && ROUTE_FC !=jltype && ROUTE_TONGG != jltype)
        return;
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if( strCheci == pTrainRouteOrder->m_strTrainNum
            && 	jltype == pTrainRouteOrder->m_btRecvOrDepart)
        {
            pTrainRouteOrder->m_bRunNow = true;
            return;
        }
        else if (strCheci == pTrainRouteOrder->m_strTrainNum
            && 	ROUTE_TONGG == jltype)
        {
            pTrainRouteOrder->m_bRunNow = true;
        }
    }
}
//查找列车进路序列并更新为未办理
void MyStation::FindAndUpdateRouteWaite(QString strCheci, int jltype)
{
    if (ROUTE_JC != jltype && ROUTE_FC != jltype)
            return;
    for (int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if (strCheci == pTrainRouteOrder->m_strTrainNum
            && 	jltype == pTrainRouteOrder->m_btRecvOrDepart)
        {
            pTrainRouteOrder->SetState(0);

            int startXHDcode = this->GetCodeByStrName(pTrainRouteOrder->m_strXHDBegin);
            int nLength = 18;
            unsigned char byArrayUDPJLOrderDate[100] = {0xEF, 0xEF, 0xEF, 0xEF,	//帧头
                0x12,	//帧长低位
                0x00,	//帧长高位
                0x00,	//目标地址码
                0x00,	//本机地址码
                0x00,	//车站标志
                0x88,	//信息分类码 （CTC车务终端----->联锁仿真机）
                0x00,	//功能按钮类型
                0x00,	//设备号
                0x00,	//设备号
                0x00,	//设备号
                0xFE, 0xFE, 0xFE, 0xFE};	//帧尾
            byArrayUDPJLOrderDate[10] = 0x14;//总人解
            byArrayUDPJLOrderDate[11] = (BYTE)(startXHDcode);
            byArrayUDPJLOrderDate[12] = (BYTE)(startXHDcode>>8);
            //命令数据转发给联锁
            //数据帧转换
            QByteArray qSendArray = UnsignedCharToQByteArray(byArrayUDPJLOrderDate, 18);
            //SendUDPDataToLS(this, qRecvArray,nLength);
            //信号-发送数据给联锁
            emit sendDataToLSSignal(this, qSendArray, nLength);
            //更新数据库
            m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
            //发送同步消息
            //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
            this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
            return;
        }
    }
}
//修改进路变通进路
void MyStation::ChangeRouteCommand(QString strCheci, int jltype, int nIndexOfBTJL)
{
    if(ROUTE_JC !=jltype && ROUTE_FC !=jltype)
            return;
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if( strCheci == pTrainRouteOrder->m_strTrainNum
            && 	jltype == pTrainRouteOrder->m_btRecvOrDepart)
        {
            //变通进路判断
            int btCount = pTrainRouteOrder->strArrayBTJL.count();
            if(0 < btCount && -1 < nIndexOfBTJL && nIndexOfBTJL < btCount)
            {
                QString strDesNew = pTrainRouteOrder->strArrayBTJL[nIndexOfBTJL];
                QString tempJL = strDesNew;
                tempJL.replace(",","-");
                pTrainRouteOrder->m_strRouteDescrip = tempJL;
                pTrainRouteOrder->m_strRouteDescripReal = strDesNew;
                //pTrainRouteOrder->m_ArrXHDBt.RemoveAll();
                QStringList strArray;
                int ct = StringSplit(strDesNew, ",", strArray);
                if (3 <= ct)
                {
                    pTrainRouteOrder->m_bBTJL = TRUE;
                    pTrainRouteOrder->m_strXHDYXEnd = strArray[ct-1];
                    ////始端和终端除外
                    //for (int n = 1; n<ct - 1; n++)
                    //{
                    //	pTrainRouteOrder->m_ArrXHDBt.Add(strArray[n]);
                    //}
                }
                else
                {
                    //始端、终端不变
                    pTrainRouteOrder->m_bBTJL = FALSE;
                }
                //更新数据库
                m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                //发送同步消息
                //sendUpdateDataMsg(this, UPDATETYPE_JLXL);
                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
            }
            break;
        }
    }
}

//根据线路号和公里标设置临时限速
void MyStation::SetTempLimitSpeedByGLB(int xlNum, int glbStart, int glbFinish, int speed, bool bSet)
{
    for(int i=0; i<this->XLGLBArray.size(); i++)
    {
        if(xlNum == this->XLGLBArray[i].XLNum)
        {
            for(int j=0; j<this->XLGLBArray[i].DevArray.size(); j++)
            {
                if (Dev_GD == this->XLGLBArray[i].DevArray[j]->getDevType())
                {
                    CGD* pGDDev = (CGD*)XLGLBArray[i].DevArray[j];
                    if(glbStart <= pGDDev->GLB &&  pGDDev->GLB <= glbFinish)
                    {
                        int zcDevIndex = this->GetIndexByStrName(pGDDev->m_strName);
                        if(zcDevIndex > -1)
                        {
                            CGD* pGD = (CGD*)this->DevArray[zcDevIndex];
                            pGD->isSpeedLimit = bSet;//true;
                            pGD->LimitSpeed = bSet?speed:0;
                            if(pGD->isSpeedLimit)
                            {
                                pGD->speedLimitStatus = 1;
                            }
                            else
                            {
                                pGD->speedLimitStatus = 0;
                            }
                        }
                    }
                }
                else if (Dev_DC == this->XLGLBArray[i].DevArray[j]->getDevType())
                {
                    CGDDC* pGDDCDev = (CGDDC*)XLGLBArray[i].DevArray[j];
                    if(glbStart <= pGDDCDev->GLB &&  pGDDCDev->GLB <= glbFinish)
                    {
                        int zcDevIndex = this->GetIndexByStrName(pGDDCDev->m_strName);
                        if(zcDevIndex > -1)
                        {
                            CGDDC* pGDDC = (CGDDC*)this->DevArray[zcDevIndex];
                            pGDDC->isSpeedLimit = bSet;//true;
                            pGDDC->LimitSpeed = bSet?speed:0;
                            if (DCDW == pGDDC->getDCWZ() && bSet)
                            {
                                pGDDC->speedLimitStatus = 1;
                            }
                            else if (DCFW == pGDDC->getDCWZ() && bSet)
                            {
                                pGDDC->speedLimitStatus = 2;
                            }
                            else
                            {
                                pGDDC->speedLimitStatus = 0;
                            }
                        }
                    }
                }
            }
            break;
        }
    }
}

//接收教师的临时限速数据
void MyStation::recvTeacherData_LimitSpeed(unsigned char *StatusArray, int nLength)
{
    //帧长，动态增加
    int nCount = 9+1;
    //限速命令编号
    int cmdNum = (int)(StatusArray[nCount] | StatusArray[nCount+1]<<8);
    nCount+=2;
    //线路号
    int lineNum =  (int)(StatusArray[nCount]);
    nCount++;
    //限速列车数
    int trainCount =  (int)(StatusArray[nCount]);
    nCount++;
    //车次号
    nCount += 10;
    //限速值
    int speed =  (int)(StatusArray[nCount]);
    bool bSet = (speed!=0xFF)?true:false;
    speed = speed*5;
    nCount++;
    //起始公里标
    int glbA =  (int)(StatusArray[nCount]);
    int glbB =  (int)(StatusArray[nCount+1]);
    int glbC =  (int)(StatusArray[nCount+2]);
    int glbD =  (int)(StatusArray[nCount+3]);
    int glbStart = glbA*1000 + glbB*100000 + glbC + glbD*100;
    nCount += 4;
    //结束公里标
    glbA =  (int)(StatusArray[nCount]);
    glbB =  (int)(StatusArray[nCount+1]);
    glbC =  (int)(StatusArray[nCount+2]);
    glbD =  (int)(StatusArray[nCount+3]);
    int glbFinish = glbA*1000 + glbB*100000 + glbC + glbD*100;
    nCount += 4;

    //调换为由小到大
    if(glbStart>glbFinish)
    {
        int glbTemp = glbFinish;
        glbFinish = glbStart;
        glbStart = glbTemp;
    }
    //DEBUG
    //SetTempLimitSpeedByGLB(1, 1000100, 1200200, 45);
    //SetTempLimitSpeedByGLB(2, 1000100, 1200200, 45);
    SetTempLimitSpeedByGLB(lineNum, glbStart, glbFinish, speed, bSet);
}
//接收新版教师的临时限速数据
void MyStation::recvTeacherData_LimitSpeedNew(QByteArray recvArray, int nLength)
{
    LimitSpeed* limitSpeed=nullptr;

    //帧长，动态增加
    int nCount = 9+1;
    //命令号
    int len = (int)(recvArray[nCount]&0xFF);
    nCount += 1;
    QString cmdNum = ByteArrayToUnicode(recvArray.mid(nCount,len));
    nCount += len;
    //设置或取消
    bool bSet = ((int)(recvArray[nCount]&0xFF)==0x11)?true:false;
    nCount += 1;

    //限速值
    int speed =  ByteArrayToUInt(recvArray.mid(nCount,2));
    nCount += 2;
    //开始时间
    QDateTime startTime;
    {
        int year  = ByteArrayToUInt(recvArray.mid(nCount,2));
        int mouth = ByteArrayToUInt(recvArray.mid(nCount+2,1));
        int day   = ByteArrayToUInt(recvArray.mid(nCount+3,1));
        int hour  = ByteArrayToUInt(recvArray.mid(nCount+4,1));
        int mini  = ByteArrayToUInt(recvArray.mid(nCount+5,1));
        int second = 0;//(int)recvArray[nCount+6];
        //"2019-03-31 12:24:36";
        QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
        qDebug()<<"开始时间="<<strDateTime;
        startTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
        nCount += 7;
    }
    //结束时间
    QDateTime finishTime;
    {
        int year  = ByteArrayToUInt(recvArray.mid(nCount,2));
        int mouth = ByteArrayToUInt(recvArray.mid(nCount+2,1));
        int day   = ByteArrayToUInt(recvArray.mid(nCount+3,1));
        int hour  = ByteArrayToUInt(recvArray.mid(nCount+4,1));
        int mini  = ByteArrayToUInt(recvArray.mid(nCount+5,1));
        int second = 0;//(int)recvArray[nCount+6];
        //"2019-03-31 12:24:36";
        QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
        qDebug()<<"结束时间="<<strDateTime;
        finishTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
        nCount += 7;
    }
    //立即开始标志位
    bool bStartNow = ((int)(recvArray[nCount]&0xFF)==0x11)?true:false;
    nCount += 1;

    //道岔列表
    QVector<CGDDC*> vectGDDC;
    //道岔设备个数
    int dcCount = (int)(recvArray[nCount]&0xFF);
    nCount += 1;
    //遍历道岔
    for(int i=0; i<dcCount; i++)
    {
        int devCode = ByteArrayToUInt(recvArray.mid(nCount,2));
        QString devName = this->GetStrNameByCode(devCode);
        int devIndex = this->GetIndexByStrName(devName);
        //qDebug()<<"dc"<<devCode<<devName<<devIndex;
        if(devIndex > -1)
        {
            CGDDC* pGDDC = (CGDDC*)this->DevArray[devIndex];
            if(bSet)
            {
                pGDDC->speedLimitStatus = 1;
            }
            else
            {
                pGDDC->speedLimitStatus = 0;
            }
            vectGDDC.append(pGDDC);
        }
        nCount += 2;
    }

    //区段列表
    QVector<CGD*> vectGD;
    //区段设备个数
    int qdCount = (int)(recvArray[nCount]&0xFF);
    nCount += 1;
    //遍历区段
    for(int i=0; i<qdCount; i++)
    {
        int devCode = ByteArrayToUInt(recvArray.mid(nCount,2));
        QString devName = this->GetStrNameByCode(devCode);
        int devIndex = this->GetIndexByStrName(devName);
        //qDebug()<<"qd"<<devCode<<devName<<devIndex;
        if(devIndex > -1)
        {
            CGD* pGD = (CGD*)this->DevArray[devIndex];
            if(bSet)
            {
                pGD->speedLimitStatus = 1;
            }
            else
            {
                pGD->speedLimitStatus = 0;
            }
            vectGD.append(pGD);
        }
        nCount += 2;
    }

    QVector<CGD*> vectQJ;//区间列表
    //区间设备个数
    int qjCount = (int)(recvArray[nCount]&0xFF);
    nCount += 1;
    //遍历区间
    for(int i=0; i<qjCount; i++)
    {
        int devCode = ByteArrayToUInt(recvArray.mid(nCount,2));
        QString devName = this->GetStrNameByCode(devCode);
        int devIndex = this->GetIndexByStrName(devName);
        //qDebug()<<"qj"<<devCode<<devName<<devIndex;
        if(devIndex > -1)
        {
            CGD* pGD = (CGD*)this->DevArray[devIndex];
            if(bSet)
            {
                pGD->speedLimitStatus = 1;
            }
            else
            {
                pGD->speedLimitStatus = 0;
            }
            vectQJ.append(pGD);
        }
        nCount += 2;
    }

    //设置
    if(bSet)
    {
        limitSpeed = new LimitSpeed();
        limitSpeed->strCmdNum = cmdNum;
        limitSpeed->speed = speed;
        limitSpeed->startTime = startTime;
        limitSpeed->finishTime = finishTime;
        limitSpeed->vectGD.append(vectGD);
        limitSpeed->vectGDDC.append(vectGDDC);
        limitSpeed->vectQJ.append(vectQJ);
        limitSpeed->bSet = bSet;
        limitSpeed->bStartNow = bStartNow;
        //add
        vectLimitSpeed.append(limitSpeed);
    }
    //取消
    else
    {
        limitSpeed = FindLimitSpeedByNum(cmdNum);
        //limitSpeed->bSet = bSet;
        //if(!limitSpeed)
        if(limitSpeed!=nullptr)
        {
            limitSpeed->bSet = bSet;
            limitSpeed = new LimitSpeed();
            limitSpeed->strCmdNum = cmdNum;
            limitSpeed->speed = speed;
            limitSpeed->startTime = startTime;
            limitSpeed->finishTime = finishTime;
            limitSpeed->vectGD.append(vectGD);
            limitSpeed->vectGDDC.append(vectGDDC);
            limitSpeed->vectQJ.append(vectQJ);
            limitSpeed->bSet = bSet;
            limitSpeed->bStartNow = bStartNow;
            //add
            vectLimitSpeed.append(limitSpeed);
            vectLimitSpeed.append(limitSpeed);
        }
    }

    //信号-发送数据回执
    emit sendDataToTeacherSignal(this, recvArray, nLength);
}

//检查列车在限速区域的运行
void MyStation::CheckTrainInLimitSpeed()
{
    Train *pTrain;
    for(int i=0; i<this->m_ArrayTrain.size(); i++)
    {
        pTrain = (Train *)this->m_ArrayTrain[i];
        if(pTrain->m_bRunning)
        {
            int speed = GetQDLimitSpeed(pTrain->m_nPosCode);
            if(speed > 0)
            {
                SendLimitSpeedToLS(pTrain->m_strCheCi, pTrain->m_nPosCode, speed);
            }
        }
    }
}
//获取区段的限速值
int MyStation::GetQDLimitSpeed(int qdCode)
{
    CGD *pGD;
    CGDDC *pGDDC;
    for(int i=0; i< this->DevArray.size(); i++)
    {
        if (Dev_GD == this->DevArray[i]->getDevType())
        {
            pGD=(CGD *)(this->DevArray[i]);
            if((int)pGD->m_nCode == qdCode)
            {
                if(pGD->isSpeedLimit)
                {
                    return pGD->LimitSpeed;
                }
                else
                {
                    return 0;
                }
            }
        }
        else if (Dev_DC == this->DevArray[i]->getDevType())
        {
            pGDDC=(CGDDC *)(this->DevArray[i]);
            if((int)pGDDC->m_nCode == qdCode)
            {
                if(pGDDC->isSpeedLimit)
                {
                    return pGDDC->LimitSpeed;
                }
                else
                {
                    return 0;
                }
            }
        }
    }
    return 0;
}
//给联锁发送车次限速信息
void MyStation::SendLimitSpeedToLS(QString strCheci, int posCode, int speed)
{
//    int nCount = 0;
//    //数据接口
//    unsigned char pSendDate[30] = {0};
//    InitSendDate(pSendDate, 30);
//    pSendDate[6] = this->getStationID();
//    pSendDate[8] = 0;
//    pSendDate[9] = FUNCTYPE_LIMITCC;
//    //车次长度
//    nCount = 10;
//    pSendDate[nCount] = strCheci.GetLength();
//    nCount++;
//    //车次号
//    for(int i=0;i<strCheci.GetLength();i++)
//    {
//        pSendDate[nCount++]=strCheci.GetAt(i);
//    }
//    //区段
//    memcpy(&pSendDate[nCount], &posCode, 2);
//    nCount += 2;
//    //速度
//    memcpy(&pSendDate[nCount], &speed, 2);
//    nCount += 2;
//    //发送数据
//    SendUDPDataToServer(pSendDate, 30);

    QByteArray dataArray;
    int nCount = 10;
    //添加nCount个字节并全部置零
    dataArray.append(nCount, char(0));
    //标志位-车次限速信息
    dataArray[nCount-1] = FUNCTYPE_LIMITCC;
    //车次号
    QByteArray byteArray = strCheci.toLocal8Bit();
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
    //所在区段设备号
    nCount+=2;
    dataArray.append(2, char(0));
    dataArray[nCount-2] = posCode&0xFF;
    dataArray[nCount-1] = (posCode<<8)&0xFF;
    //车速km/h
    nCount+=2;
    dataArray.append(2, char(0));
    dataArray[nCount-2] = speed&0xFF;
    dataArray[nCount-1] = (speed<<8)&0xFF;

    //帧尾4+空白2
    nCount+=6;
    dataArray.append(6, char(0));
    //打包数据
    packHeadAndTail(&dataArray, nCount);
    qDebug()<<"limit speed data to ls, dataArray="<<ByteArrayToString(dataArray);
    //信号-发送数据给联锁
    emit sendDataToLSSignal(this, dataArray, nCount);

}
//接收版教师的邻站模拟进出站信息
void MyStation::recvTeacherData_LZNMJCZ(QByteArray recvArray, int nLength)
{
    //帧长，动态增加
    int nCount = 10;
    //邻站列车进站或出站
    int jczType = (int)(recvArray[nCount]&0xFF);
    nCount += 1;
    //关联信号机设备code
    int jzxhdCode =  ByteArrayToUInt(recvArray.mid(nCount,2));
    nCount += 2;
    //设置接口
    SetLZMNJCZ(jzxhdCode, jczType);
}
//设置邻站模拟进出站(信号机设备号，进出站类型)
void MyStation::SetLZMNJCZ(int xhdCode, int jczType)
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    QString strXHD = this->GetStrNameByCode(xhdCode);
    qDebug()<<QString("收到教师机接发车口%1邻站模拟列车%2").arg(strXHD).arg(0x01==jczType?"[进站]":"[出站]");
    //列车进站
    if(0x01 == jczType)
    {
        for(int i=0; i<m_ArrayTrafficLog.count(); i++)
        {
            TrafficLog* pTrafficLog = m_ArrayTrafficLog[i];
            //已执行、匹配出站口
            if (pTrafficLog->m_nExecuteFlag>0 && strXHD==pTrafficLog->m_strXHD_CZk)
            {
                //到达邻站时间
                pTrafficLog->m_timtoAdjtStation = QDateTime::currentDateTime();
                //同步数据库
                m_pDataAccess->UpdateTrafficLog(pTrafficLog);
                //发送同步消息
                this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
                break;
            }
        }
    }
    //列车出站
    else if(0x02 == jczType)
    {
        //遍历进路
        for(int i=0; i<this->m_ArrayRouteOrder.size(); i++)
        {
            TrainRouteOrder* pRouteOrder = this->m_ArrayRouteOrder[i];
            //接车进路、匹配进站口、自触模式，先到先得
            if(ROUTE_JC == pRouteOrder->m_btRecvOrDepart
               && strXHD == pRouteOrder->m_strXHD_JZk
               /*&& pRouteOrder->m_nAutoTouch*/)
            {
                if(AutoTouchMinitesWhenLZMNJCZ>=0)
                {
                    //方法1：数组存储对象，线程定时去执行
                    pRouteOrder->m_timAutoTouchTemp = QDateTime::currentDateTime().addSecs(AutoTouchMinitesWhenLZMNJCZ*60);
                    AddLZMNRouteOrder(pRouteOrder);
                    //方法2： 下面的方法QTimer::singleShot无法执行，原因未知！！2024.2.21.lwm
//                    //启动个1分钟的定时器
//                    //定时60秒后执行，仅执行一次
//                    //毫秒
//                    int milliseconds = AutoTouchMinitesWhenLZMNJCZ*60*1000;
//                    QTimer::singleShot(milliseconds, this, [&, pRouteOrder](){
//                        AutoTouchLZMNRouteOrder(pRouteOrder);
//                    }
//                    );
                }
                break;//退出
            }
        }
    }
}
//增加邻站模拟的进路序列
void MyStation::AddLZMNRouteOrder(TrainRouteOrder *_pTrainRouteOrder)
{
    bool bAdd = false;
    //仅处理自触模式的进路
    if(_pTrainRouteOrder->m_nAutoTouch)
    {
        bAdd = true;
    }
    else if(_pTrainRouteOrder->m_bZHJL)
    {
        //遍历进路
        for(int i=0; i<this->m_ArrayRouteOrder.size(); i++)
        {
            TrainRouteOrder* pRouteOrder = this->m_ArrayRouteOrder[i];
            //子序列
            if(_pTrainRouteOrder->route_id == pRouteOrder->father_id
               && pRouteOrder->m_nAutoTouch)
            {
                 bAdd = true;
                 break;
            }
        }
    }

    if(vectLZMNTrainRouteOrder.contains(_pTrainRouteOrder))
    {
        //去重
        vectLZMNTrainRouteOrder.removeOne(_pTrainRouteOrder);
    }
    if(bAdd)
    {
        vectLZMNTrainRouteOrder.append(_pTrainRouteOrder);
        qDebug()<<QString("增加一条邻站模拟触发进路，车次=%1").arg(_pTrainRouteOrder->m_strTrainNum);
    }
}
//自动触发邻站模拟的进路序列
void MyStation::AutoTouchLZMNRouteOrder()
{
    QDateTime timNow = QDateTime::currentDateTime();
    for(int i=0; i<vectLZMNTrainRouteOrder.size(); i++)
    {
        TrainRouteOrder *_pTrainRouteOrder = vectLZMNTrainRouteOrder[i];
        //时间到
        if(timNow.toString(TIME_FORMAT_HMS) == _pTrainRouteOrder->m_timAutoTouchTemp.toString(TIME_FORMAT_HMS))
        {
            //组合进路
            if(_pTrainRouteOrder->m_bZHJL)
            {
                //遍历进路
                for(int i=0; i<this->m_ArrayRouteOrder.size(); i++)
                {
                    TrainRouteOrder* pRouteOrder = this->m_ArrayRouteOrder[i];
                    //子序列
                    if(_pTrainRouteOrder->route_id == pRouteOrder->father_id
                       && pRouteOrder->m_nAutoTouch)
                    {
                         pRouteOrder->m_bRunNow = true;
                         qDebug()<<QString("触发一条邻站模拟触发进路，车次=%1，进路（%2）")
                                   .arg(pRouteOrder->m_strTrainNum)
                                   .arg(pRouteOrder->m_strRouteDescrip);
                    }
                }
            }
            //非组合进路
            else if(_pTrainRouteOrder->m_nAutoTouch)
            {
                _pTrainRouteOrder->m_bRunNow = true;
                qDebug()<<QString("触发一条邻站模拟触发进路，车次=%1，进路（%2）")
                          .arg(_pTrainRouteOrder->m_strTrainNum)
                          .arg(_pTrainRouteOrder->m_strRouteDescrip);
            }

            //执行完后移除
            vectLZMNTrainRouteOrder.removeOne(_pTrainRouteOrder);
        }
    }
}

//发送语音播报文字（播报内容，播报次数-默认1次）
void MyStation::SendSpeachText(QString strText, int count)
{
    QByteArray dataArray;
    int nCount = 10;
    //添加nCount个字节并全部置零
    dataArray.append(nCount, char(0));
    //标志位-语音播报类型
    dataArray[nCount-1] = FUNCTYPE_SPEACH;
    //播报次数
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = count;
    //播报
    QByteArray byteArray = strText.toLocal8Bit();
    int len = byteArray.count();
    //播报长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = len;
    //播报内容
    nCount+=len;
    dataArray.append(len, char(0));
    for(int u=0; u<len; u++)
    {
        dataArray[nCount-len+u] = byteArray[u];
    }

    //帧尾4+空白2
    nCount+=6;
    dataArray.append(6, char(0));
    //打包数据
    packHeadAndTail(&dataArray, nCount);
    //qDebug()<<"Speach data to CTC, dataArray="<<ByteArrayToString(dataArray);
    //信号-发送数据给CTC
    emit sendDataToCTCSignal(this, dataArray, nCount);
    //信号-发送数据给占线板
    emit sendDataToBoardSignal(this, dataArray, nCount);
    //信号-发送数据给集控台
    emit sendDataToJKSignal(this, dataArray, nCount);
    //信号-发送数据给占线图
    emit sendDataToZXTSignal(this, dataArray, nCount);
}
//发送进路检查结果-防错办
void MyStation::SendRouteCheckResult(CheckResult* checkResult,int currCtcIndex)
{
    QByteArray dataArray;
    int nCount = 10;
    //添加nCount个字节并全部置零
    dataArray.append(nCount, char(0));
    //标志位-防错办检查信息
    dataArray[nCount-1] = FUNCTYPE_CHECK;
    //子分类码
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = 0x01;//逻辑反馈
    //信息id
    nCount+=4;
    dataArray.append(4, char(0));
    memcpy(dataArray.data()+(nCount-4),&checkResult->id,4);
    //是否可以强制执行
    nCount+=1;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = checkResult->bEnforced?0x01:0x00;
    //检查信息
    QByteArray byteArray = checkResult->checkMsg.toLocal8Bit();
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
    //关联的进路序列id
    nCount+=2;
    dataArray.append(2, char(0));
    if(checkResult->route_id>0)
    {
        memcpy(dataArray.data()+(nCount-2),&checkResult->route_id,2);
    }

    //帧尾4+空白2
    nCount+=6;
    dataArray.append(6, char(0));
    //打包数据
    packHeadAndTail(&dataArray, nCount);
    qDebug()<<"RouteCheckResult data to CTC, dataArray="<<ByteArrayToString(dataArray);
    //信号-发送数据给联锁
    emit sendDataToCTCSignal(this, dataArray, nCount, currCtcIndex);
    emit sendDataToJKSignal(this, dataArray, nCount, currCtcIndex);
    emit sendDataToBoardSignal(this, dataArray, nCount);
    qDebug()<<QString("触发防错办报警提示-%1,%2").arg(this->getStationName()).arg(checkResult->checkMsg);
}
//检查车次是否是电力
bool MyStation::CheckCheciElectric(QString _strCheci)
{
    bool bElect = false;

    //检查显示的车次
    int ccCount = m_ArrayTrain.size();
    for(int i=0; i<ccCount; i++)
    {
        Train* pTrain = (Train *)m_ArrayTrain[i];
        if(_strCheci == pTrain->m_strCheCi)
        {
            return pTrain->m_bElectric;
        }
    }
    //检查阶段计划行车日志
    int logSize = m_ArrayTrafficLog.size();
    for(int i=0; i<logSize; i++)
    {
        TrafficLog* pTrafficLog = (TrafficLog*)m_ArrayTrafficLog[i];
        if(_strCheci == pTrafficLog->m_strReachTrainNum
            || _strCheci == pTrafficLog->m_strDepartTrainNum)
        {
            return pTrafficLog->m_bElectric;
        }
    }
    //检查进路序列
    int routSize = m_ArrayRouteOrder.size();
    for(int i=0; i<routSize; i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)m_ArrayRouteOrder[i];
        if(_strCheci == pTrainRouteOrder->m_strTrainNum)
        {
            return pTrainRouteOrder->m_bElectric;
        }
    }

    return bElect;
}
//列车进路序列防错办检查-联锁条件，返回0可办理，非0表示有不满足的情况
CheckResult*  MyStation::CheckPreventConditionInterlock(TrainRouteOrder *_pTrainRouteOrder)
{
    CheckResult* ckResult = new CheckResult;
    QDateTime timeNow = QDateTime::currentDateTime();
    ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
    ckResult->route_id = _pTrainRouteOrder->route_id;
    //数据帧转换
    QByteArray qSendArray = UnsignedCharToQByteArray(_pTrainRouteOrder->m_byArrayUDPJLOrderDate, 30);
    ckResult->dataArray.append(qSendArray);
    //联锁表索引
    int idxLsb = GetTrainRouteOrderLSBRouteIndex(_pTrainRouteOrder);
    ckResult->indexRoute = idxLsb;
    //提示内容
    QString strMsg = "";
    //进路报警类型(临时变量)
    int JLWarningType = 0;
    //进路报警信息(临时变量)
    QString JLWarningMsg = "";

    //无匹配的联锁表进路
    if(idxLsb <= -1)
    {
        //无岔区段有电判断
        bool bWCQDPower = true;
        //电力车次
        if(_pTrainRouteOrder->m_bElectric)
        {
            //进路接触网供电判断
           bWCQDPower = CheckRouteDCPower(_pTrainRouteOrder->m_byArrayUDPJLOrderDate, _pTrainRouteOrder->tempRouteBtnArray
                                          ,JLWarningType,JLWarningMsg);
        }
        //无岔区段分路不良判断
        bool bWCQDFLBL = CheckRouteDC_FLBL(_pTrainRouteOrder->m_byArrayUDPJLOrderDate, _pTrainRouteOrder->tempRouteBtnArray
                                           ,JLWarningType,JLWarningMsg);
        //无岔区段封锁判断
        bool bWCQDFS = CheckRouteDCFS(_pTrainRouteOrder->m_byArrayUDPJLOrderDate, _pTrainRouteOrder->tempRouteBtnArray
                                      ,JLWarningType,JLWarningMsg);
        //条件
        if(!bWCQDPower || !bWCQDFLBL || !bWCQDFS)
        {
            QString str = QString("联锁条件检查失败");
            //strMsg.append("\r\n");
            strMsg.append(str);
            ckResult->checkMsg = strMsg;
            ckResult->check = JLWarningType;
        }

        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
        _pTrainRouteOrder->checkResultInterlock = ckResult->check;
        //检查不通过则返回
        if(ckResult->check>0)
        {
            return ckResult;
        }
    }
    //匹配的联锁表进路
    else
    {
        //【显示优先级顺序：无电>占用>锁闭>封锁>分路不良,当前时刻按照优先级只能显示一种。】

        //进路接触网无电判断
        bool bRoutePowerCut = false;
        //电力车次
        if(_pTrainRouteOrder->m_bElectric)
        {
            //进路接触网供电判断
            bRoutePowerCut = isQDHavePowerCutInLSB(idxLsb,JLWarningType,JLWarningMsg);
        }
        if(bRoutePowerCut)
        {
            QString str = QString("列车车次:%1触发进路触发失败(与联锁逻辑冲突)，电力牵引列车%2无法通过无电进路(%3)")
                    .arg(_pTrainRouteOrder->m_strTrainNum).arg(_pTrainRouteOrder->m_strTrainNum)
                    .arg(_pTrainRouteOrder->m_strRouteDescrip);
            //strMsg.append("\r\n");
            strMsg.append(str);
            ckResult->bEnforced = false;//不可强制执行
            ckResult->checkMsg = strMsg;
            ckResult->check = JLWarningType;
        }
        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
        _pTrainRouteOrder->checkResultInterlock = ckResult->check;
        //检查不通过则返回
        if(ckResult->check>0)
        {
            return ckResult;
        }

        QString str;
        //进路区段空闲
        bool bRouteKX = true;
        //进路区段占用判断
        bool bRouteZY = IsQDZYInLSB(idxLsb,JLWarningType,JLWarningMsg);
        if(bRouteZY)
        {
            bRouteKX = false;
            str = QString("联锁逻辑检查失败，区段占用");
            strMsg.append(str);
            ckResult->bEnforced = false;//不可强制执行
            ckResult->checkMsg = strMsg;
            ckResult->check |= JLWarningType;
        }
        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
        _pTrainRouteOrder->checkResultInterlock = ckResult->check;
        //检查不通过则返回
        if(ckResult->check>0)
        {
            return ckResult;
        }

        //进路道岔四开检查
        bool bRouteSK = IsQDDCSKInLSB(idxLsb,JLWarningType,JLWarningMsg);
        //进路空闲判断
        if(bRouteSK)
        {
            bRouteKX = false;
            str = QString("联锁逻辑检查失败，道岔四开");
            strMsg.append(str);
            ckResult->bEnforced = false;//不可强制执行
            ckResult->checkMsg = strMsg;
            ckResult->check |= JLWarningType;
        }
        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
        _pTrainRouteOrder->checkResultInterlock = ckResult->check;
        //检查不通过则返回
        if(ckResult->check>0)
        {
            return ckResult;
        }

        //进路区段锁闭判断
        //bool bRouteSB = IsQDStateInLSB(idxLsb, QDSB);
        bool bRouteSB = IsQDHaveStateInLSB(idxLsb, QDSB, JLWarningType, JLWarningMsg);
        //进路空闲判断
        //（非延续进路 或 非通过进路）时才判断区段锁闭卡控,通过进路发车,不需要判断区段锁闭卡控(考虑接车进路延续)
        if((bRouteSB)
            &&(((_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC)&&(_pTrainRouteOrder->m_bYXJL==0))
               ||((_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_FC)&&(_pTrainRouteOrder->m_btBeginOrEndFlg!=JFC_TYPE_TG))))
        {
            bRouteKX = false;
            str = QString("联锁逻辑检查失败，区段锁闭");
            strMsg.append(str);
            ckResult->bEnforced = false;//不可强制执行
            ckResult->checkMsg = strMsg;
            ckResult->check |= JLWarningType;
        }
        //接车进路为延续的发车进路不判定发车进路的锁闭
        if(bRouteSB && (_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_FC)
           && _pTrainRouteOrder->m_bYXJLOfJC)
        {
            ckResult->check = 0;
            ckResult->checkMsg = "";
        }
        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
        _pTrainRouteOrder->checkResultInterlock = ckResult->check;
        //检查不通过则返回
        if(ckResult->check>0)
        {
            return ckResult;
        }

//        //进路空闲判断
//        if(bRouteZY || bRouteSB || bRouteSK)
//        {
//            strMsg.append(str);
//            ckResult->bEnforced = false;//不可强制执行
//            ckResult->checkMsg = strMsg;
//            ckResult->check |= JLWarningType;
//        }
//        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
//        _pTrainRouteOrder->checkResultInterlock = ckResult->check;
//        //检查不通过则返回
//        if(ckResult->check>0)
//        {
//            return ckResult;
//        }

        //封锁检查
        bool bRouteFS = IsQDFSInLSB(idxLsb,JLWarningType,JLWarningMsg);
        if(bRouteFS)
        {
            //设备名称
            QString str = /*QString("进路检查失败，进路设备封锁") + */JLWarningMsg;
            //strMsg.append("\r\n");
            strMsg.append(str);
            ckResult->bEnforced = false;//不可强制执行
            ckResult->checkMsg = strMsg;
            ckResult->check |= JLWarningType;
        }
        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
        _pTrainRouteOrder->checkResultInterlock = ckResult->check;
        //检查不通过则返回
        if(ckResult->check>0)
        {
            return ckResult;
        }

        //进路联锁关系判断
        //单独判断股道封路不良
        if(_pTrainRouteOrder->m_nGDPos>0)
        {
            CGD *pGD=(CGD*)DevArray[_pTrainRouteOrder->m_nGDPos-1];
            //分路不良,0无，1分路，2确认空闲
            if(pGD->flblStatus==1)
            {
                JLWarningType |= JLWARNING_FLBL_GD;
                //分路
                ckResult->bEnforced = false;//不可强制执行
                QString str = QString("%1次股道%2为分路不良区段").arg(_pTrainRouteOrder->m_strTrainNum)
                        .arg(_pTrainRouteOrder->m_strTrack);
                //strMsg.append("\r\n");
                strMsg.append(str);
                ckResult->checkMsg = strMsg;
                ckResult->check = JLWarningType;
            }
            //股道分路不良确认空闲触发时弹出强制执行框(自动触发||人工触发)
//            else if(pGD->flblStatus==2
//                    && ((!GDFLBLKXAutoTouch && _pTrainRouteOrder->m_nAutoTouch)
//                        || _pTrainRouteOrder->m_nManTouch && !_pTrainRouteOrder->m_nAutoTouch)
//              )
            //股道分路不良确认空闲触发时弹出强制执行框(人工触发时)
            else if(pGD->flblStatus==2 && (!GDFLBLKXAutoTouch && _pTrainRouteOrder->m_nManTouch))
            {
                JLWarningType |= JLWARNING_FLBL_GDKX;
                //分路
                ckResult->bEnforced = true;//可强制执行
                QString str = QString("%1次股道%2为分路不良区段").arg(_pTrainRouteOrder->m_strTrainNum)
                        .arg(_pTrainRouteOrder->m_strTrack);
                //strMsg.append("\r\n");
                strMsg.append(str);
                ckResult->checkMsg = strMsg;
                ckResult->check = JLWarningType;
            }

            _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
            _pTrainRouteOrder->checkResultInterlock = ckResult->check;
            //检查不通过则返回
            if(ckResult->check>0)
            {
                return ckResult;
            }
        }

        //分路不良检查
        //进路分路不良并确认空闲判断
        //bool bRouteFLBLKX = CheckRouteCanCmd(2, _pTrainRouteOrder->m_byArrayUDPJLOrderDate, _pTrainRouteOrder->tempRouteBtnArray);
        bool bRouteFLBL = isQDHaveFLBLInLSB(idxLsb,JLWarningType,JLWarningMsg);
        if(bRouteFLBL)
        {
            QString str = QString("进路分路不良未确认空闲");
            //strMsg.append("\r\n");
            strMsg.append(str);
            if((JLWarningType&JLWARNING_FLBL_DC) /*&& !(JLWarningType&JLWARNING_FLBL_GD)*/
              )
            {
                ckResult->bEnforced = true;//可强制执行，道岔分路不良
            }
            else
            {
                ckResult->bEnforced = false;//不可强制执行，股道分路不良
            }
            ckResult->checkMsg = strMsg;
            ckResult->check = JLWarningType;
        }
        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
        _pTrainRouteOrder->checkResultInterlock = ckResult->check;
        //检查不通过则返回
        if(ckResult->check>0)
        {
            return ckResult;
        }

        //进路办理
        //进路检查通过
        ckResult->check = 0;
        ckResult->checkMsg = "";
    }
    _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
    _pTrainRouteOrder->checkResultInterlock = ckResult->check;
    //检查不通过则返回
    if(ckResult->check>0)
    {
        return ckResult;
    }

    return ckResult;
}
//列车进路序列防错办检查-时序，返回0可办理，非0表示有不满足的情况
CheckResult *MyStation::CheckPreventConditionSequence(TrainRouteOrder *_pTrainRouteOrder)
{
    CheckResult* ckResult = new CheckResult;
    QDateTime timeNow = QDateTime::currentDateTime();
    ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
    ckResult->route_id = _pTrainRouteOrder->route_id;
    //数据帧转换
    QByteArray qSendArray = UnsignedCharToQByteArray(_pTrainRouteOrder->m_byArrayUDPJLOrderDate, 30);
    ckResult->dataArray.append(qSendArray);
    //联锁表索引
    int idxLsb = GetTrainRouteOrderLSBRouteIndex(_pTrainRouteOrder);
    ckResult->indexRoute = idxLsb;

    //冲突的车次（或计划第一个车次）
    QString strCheCiConflect;
    //时序判定
    bool bFirst = CheckRouteIsFirst(_pTrainRouteOrder, strCheCiConflect);
    //判定进路交叉
    if(JudgeRouteCrossed && !bFirst)
    {
        //冲突的进路序列（目前只能识别一个）
        TrainRouteOrder *_pTrainRouteOrderConflect = new TrainRouteOrder;
        //进路交叉判定
        bool bCross = CheckRouteIsCross(_pTrainRouteOrder, _pTrainRouteOrderConflect);
        if(bCross)
        {
            if(_pTrainRouteOrderConflect->m_nOldRouteState==0
               || _pTrainRouteOrderConflect->m_nOldRouteState>=4)
            {
                ckResult->bEnforced = true;//可强制办理
            }
            else
            {
                ckResult->bEnforced = false;//不可强制办理
            }
            ckResult->check = JLWARNING_SEQU_CROSS;//JLWARNING_SEQU_CCCT;
            ckResult->checkMsg = QString("车次:%7触发未知的返回结果，%1次%2进路(%3)，与%4次%5进路(%6)进路交叉")
                    .arg(_pTrainRouteOrder->m_strTrainNum)
                    .arg(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                    .arg(_pTrainRouteOrder->m_strRouteDescrip)
                    .arg(_pTrainRouteOrderConflect->m_strTrainNum)
                    .arg(_pTrainRouteOrderConflect->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                    .arg(_pTrainRouteOrderConflect->m_strRouteDescrip)
                    .arg(_pTrainRouteOrder->m_strTrainNum);
            _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
            _pTrainRouteOrder->checkResultSequence = ckResult->check;
            return ckResult;
        }
    }

//    //时序判定
//    bool bFirst = CheckRouteIsFirst(_pTrainRouteOrder, strCheCiConflect);
    //不是第一个 且 自触模式
    if(!bFirst && _pTrainRouteOrder->m_nAutoTouch)
    {
        ckResult->bEnforced = false;//不可强制办理
        ckResult->check = JLWARNING_SEQU_CCCT;
//        ckResult->checkMsg = QString("实际%1车次%2与计划%3车次%4不一致，请检查计划")
//                .arg(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
//                .arg(_pTrainRouteOrder->m_strTrainNum)
//                .arg(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
//                .arg(strCheCiConflect);
        ckResult->checkMsg = QString("计划%1顺序与实际不一致，检查列车%2与%3的计划时间")
                .arg(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                .arg(_pTrainRouteOrder->m_strTrainNum)
                .arg(strCheCiConflect);
        //ckResult->checkMsg = strCheCiConflect;
    }
    //人工模式下触发有冲突，由人负责
    else if(!bFirst /*&& _pTrainRouteOrder->m_nManTouch*/)
    {
        ckResult->bEnforced = true;//可强制办理
        ckResult->check = JLWARNING_SEQU_CCCT;
//        ckResult->checkMsg = QString("实际%1车次%2与计划%3车次%4不一致，请检查计划")
//                .arg(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
//                .arg(_pTrainRouteOrder->m_strTrainNum)
//                .arg(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
//                .arg(strCheCiConflect);
        ckResult->checkMsg = QString("计划%1顺序与实际不一致，检查列车%2与%3的计划时间")
                .arg(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                .arg(_pTrainRouteOrder->m_strTrainNum)
                .arg(strCheCiConflect);
        //ckResult->checkMsg = strCheCiConflect;
    }
    //人工模式下没有冲突，由人负责
    else
    {
        ckResult->check = 0;
        ckResult->checkMsg = "";
    }
    _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
    _pTrainRouteOrder->checkResultSequence = ckResult->check;

    return ckResult;
}
//列车进路序列防错办检查-站细，返回0可办理，非0表示有不满足的情况
CheckResult *MyStation::CheckPreventConditionStaDetails(TrainRouteOrder *_pTrainRouteOrder)
{
    CheckResult* ckResult = new CheckResult;
    QDateTime timeNow = QDateTime::currentDateTime();
    ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
    ckResult->route_id = _pTrainRouteOrder->route_id;
    //数据帧转换
    QByteArray qSendArray = UnsignedCharToQByteArray(_pTrainRouteOrder->m_byArrayUDPJLOrderDate, 30);
    ckResult->dataArray.append(qSendArray);
    ////联锁表索引
    //int idxLsb = GetTrainRouteOrderLSBRouteIndex(_pTrainRouteOrder);
    //ckResult->indexRoute = idxLsb;

    //提示信息
    QString msg;

    //关联行车日志
    TrafficLog* pTrafficLog = GetTrafficLogByCheCi(_pTrainRouteOrder->m_strTrainNum);

    //（防错办）股道属性
    for(int i=0; i<this->vectGDAttr.size(); i++)
    {
        CGD* pGDAttr = this->vectGDAttr[i];
        if((int)pGDAttr->getCode() == _pTrainRouteOrder->m_nTrackCode)
        {
            //接车进路检查 //发车进路也检查
            //if(_pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC)
            {
                //客车接入了货车股道 || 货车接入了客车股道
                if((_pTrainRouteOrder->m_nLHFlg == LCTYPE_KC && !(pGDAttr->jfcAttr==0 || pGDAttr->jfcAttr==2))
                  || (_pTrainRouteOrder->m_nLHFlg == LCTYPE_HC && !(pGDAttr->jfcAttr==1 || pGDAttr->jfcAttr==2))
                  )
                {
                    ckResult->bEnforced = false;//不可强制办理
                    ckResult->check = JLWARNING_ATTR_GDTYPE;
                    ckResult->checkMsg = QString("列车%1次股道%2类型不满足")
                            .arg(_pTrainRouteOrder->m_strTrainNum)
                            .arg(_pTrainRouteOrder->m_strTrack);
                    //return ckResult;
                    break;
                }
                //计划超限超过股道超限属性
                //qDebug() << "股道超限属性" << _pTrainRouteOrder->m_nLevelCX;
                if(_pTrainRouteOrder->m_nLevelCX > pGDAttr->overLimit)
                {
                    ckResult->bEnforced = false;//不可强制办理
                    ckResult->check = JLWARNING_ATTR_LEVELCX;
                    ckResult->checkMsg = QString("%1次股道%2超限条件不满足")
                            .arg(_pTrainRouteOrder->m_strTrainNum)
                            .arg(_pTrainRouteOrder->m_strTrack);
                    //return ckResult;
                    break;
                }
                //（非通过列车） && 客车 && 没有站台 && 办理客运
                if((_pTrainRouteOrder->m_btBeginOrEndFlg != JFC_TYPE_TG)
                    && _pTrainRouteOrder->m_nLHFlg == LCTYPE_KC
                    && pGDAttr->platform==0 && (pTrafficLog!=nullptr && pTrafficLog->m_bBLKY))
                {
                    ckResult->bEnforced = false;//不可强制办理
                    ckResult->check = JLWARNING_ATTR_PLATFORM1;
                    ckResult->checkMsg = QString("客车%1次无法接入无客运设备股道%2")
                            .arg(_pTrainRouteOrder->m_strTrainNum)
                            .arg(_pTrainRouteOrder->m_strTrack);
                    //return ckResult;
                    break;
                }
                //货车 && 高站台
                if(_pTrainRouteOrder->m_btBeginOrEndFlg == JFC_TYPE_TG &&
                    _pTrainRouteOrder->m_nLHFlg == LCTYPE_HC && pGDAttr->platform==1)
                {
                    ckResult->bEnforced = false;//不可强制办理
                    ckResult->check = JLWARNING_ATTR_PLATFORM2;
                    ckResult->checkMsg = QString("货车%1次不能通过高站台股道%2")
                            .arg(_pTrainRouteOrder->m_strTrainNum)
                            .arg(_pTrainRouteOrder->m_strTrack);
                    //return ckResult;
                    break;
                }
                if(pTrafficLog != nullptr)
                {
                    //军用列车 && 股道非军运
                    if(pTrafficLog->m_bArmy && pGDAttr->army==0)
                    {
                        ckResult->bEnforced = false;//不可强制办理
                        ckResult->check = JLWARNING_ATTR_ARMY;
                        ckResult->checkMsg = QString("军用列车%1次无法进入非军用股道%2")
                                .arg(_pTrainRouteOrder->m_strTrainNum)
                                .arg(_pTrainRouteOrder->m_strTrack);
                        //return ckResult;
                        break;
                    }
                    //上水
                    if(pTrafficLog->m_btSSStatus>0 && pGDAttr->isWater==0)
                    {
                        ckResult->bEnforced = false;//不可强制办理
                        ckResult->check = JLWARNING_ATTR_FLOWSS;
                        ckResult->checkMsg = QString("%1次股道%2无上水设备")
                                .arg(_pTrainRouteOrder->m_strTrainNum)
                                .arg(_pTrainRouteOrder->m_strTrack);
                        //return ckResult;
                        break;
                    }
                    //吸污
                    if(pTrafficLog->m_btXWStatus>0 && pGDAttr->isBlowdown==0)
                    {
                        ckResult->bEnforced = false;//不可强制办理
                        ckResult->check = JLWARNING_ATTR_FLOWXW;
                        ckResult->checkMsg = QString("%1次股道%2无吸污设备")
                                .arg(_pTrainRouteOrder->m_strTrainNum)
                                .arg(_pTrainRouteOrder->m_strTrack);
                        //return ckResult;
                        break;
                    }
                }
            }
            break;
        }
    }

    _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
    _pTrainRouteOrder->checkResultStaDetails = ckResult->check;
    //检查不通过则返回
    if(ckResult->check>0)
    {
        return ckResult;
    }

    //（防错办）出入口属性
    for(int i=0; i<this->vectGatewayAttr.size(); i++)
    {
        CXHD* pXHDAttr = this->vectGatewayAttr[i];
        //入口检查
        if((int)pXHDAttr->getCode() == _pTrainRouteOrder->m_nCodeReachStaEquip
            && _pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC)
        {
            //接车口不允许进入超限列车
            if(_pTrainRouteOrder->m_nLevelCX>0 && pXHDAttr->allowOverLimit==0)
            {
                ckResult->bEnforced = false;//不可强制办理
                ckResult->check = JLWARNING_ENEX_LEVELCX;
                ckResult->checkMsg = QString("%1次接车方向不允许超限车（出入口%2）")
                        .arg(_pTrainRouteOrder->m_strTrainNum)
                        .arg(_pTrainRouteOrder->m_strXHD_JZk);
                //return ckResult;
                break;
            }
            //接车口不允许某些列类型列车通过
            if((_pTrainRouteOrder->m_nLHFlg == LCTYPE_KC && pXHDAttr->allowPassenger==0)
               || (_pTrainRouteOrder->m_nLHFlg == LCTYPE_HC && pXHDAttr->allowFreight==0)
               )
            {
                ckResult->bEnforced = false;//不可强制办理
                ckResult->check = JLWARNING_ENEX_KHTYPE;
                ckResult->checkMsg = QString("%1次接车方向客货类型错误（出入口%2）")
                        .arg(_pTrainRouteOrder->m_strTrainNum)
                        .arg(_pTrainRouteOrder->m_strXHD_JZk);
                //return ckResult;
                break;
            }
        }

        //出口检查
        if((int)pXHDAttr->getCode() == _pTrainRouteOrder->m_nCodeDepartStaEquip
            && _pTrainRouteOrder->m_btRecvOrDepart == ROUTE_FC)
        {
            //发车口不允许进入超限列车
            if(_pTrainRouteOrder->m_nLevelCX>0 && pXHDAttr->allowOverLimit==0)
            {
                ckResult->bEnforced = false;//不可强制办理
                ckResult->check = JLWARNING_ENEX_LEVELCX;
                ckResult->checkMsg = QString("%1次发车方向不允许超限车（出入口%2）")
                        .arg(_pTrainRouteOrder->m_strTrainNum)
                        .arg(_pTrainRouteOrder->m_strXHD_CZk);
                //return ckResult;
                break;
            }
            //发车口不允许某些列类型列车通过
            if((_pTrainRouteOrder->m_nLHFlg == LCTYPE_KC && pXHDAttr->allowPassenger==0)
               || (_pTrainRouteOrder->m_nLHFlg == LCTYPE_HC && pXHDAttr->allowFreight==0)
               )
            {
                ckResult->bEnforced = false;//不可强制办理
                ckResult->check = JLWARNING_ENEX_KHTYPE;
                ckResult->checkMsg = QString("%1次发车方向客货类型错误（出入口%2）")
                        .arg(_pTrainRouteOrder->m_strTrainNum)
                        .arg(_pTrainRouteOrder->m_strXHD_CZk);
                //return ckResult;
                break;
            }
        }
    }
    _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
    _pTrainRouteOrder->checkResultStaDetails = ckResult->check;
    //检查不通过则返回
    if(ckResult->check>0)
    {
        return ckResult;
    }

    //（防错办）列车固定径路信息
    if(pTrafficLog!=nullptr && !pTrafficLog->m_bAllowCRKNotMatch)
    {
        for(int i=0; i<this->vectFixedRoute.size(); i++)
        {
            FixedRoute* pFixedRoute = this->vectFixedRoute[i];
            //接车进路检查
            if(_pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC
               && pFixedRoute->arrivalnum == _pTrainRouteOrder->m_strTrainNum)
            {
                if(pFixedRoute->entrXHDName != _pTrainRouteOrder->m_strXHD_JZk)
                {
                    ckResult->bEnforced = false;//不可强制办理
                    ckResult->check = JLWARNING_ENEX_UNSAME;
                    ckResult->checkMsg = QString("列车%1次接车方向与固定路径不一致")
                            .arg(_pTrainRouteOrder->m_strTrainNum);
                    //return ckResult;
                    break;
                }
            }

            //发车进路检查
            if(_pTrainRouteOrder->m_btRecvOrDepart == ROUTE_FC
                && pFixedRoute->departnum == _pTrainRouteOrder->m_strTrainNum)
            {
                if(pFixedRoute->exitXHDName != _pTrainRouteOrder->m_strXHD_CZk)
                {
                    ckResult->bEnforced = false;//不可强制办理
                    ckResult->check = JLWARNING_ENEX_UNSAME;
                    ckResult->checkMsg = QString("列车%1次发车方向与固定路径不一致")
                            .arg(_pTrainRouteOrder->m_strTrainNum);
                    //return ckResult;
                    break;
                }
            }
        }
        _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
        _pTrainRouteOrder->checkResultStaDetails = ckResult->check;
        //检查不通过则返回
        if(ckResult->check>0)
        {
            return ckResult;
        }
    }

    //（防错办）车次股道列表信息
    for(int i=0; i<this->vectTrainNumTrack.size(); i++)
    {
        ;//暂不实现.lwm.2023.6.26
    }
    _pTrainRouteOrder->checkMsg = ckResult->checkMsg;
    _pTrainRouteOrder->checkResultStaDetails = ckResult->check;
    //检查不通过则返回
    if(ckResult->check>0)
    {
        return ckResult;
    }

    return ckResult;
}
//列车进路序列防错办检查-（站细+时序+联锁条件），返回0可办理，非0表示有不满足的情况
CheckResult *MyStation::CheckPreventConditionAll(TrainRouteOrder *_pTrainRouteOrder)
{
    //检查结果
    CheckResult* ckResult = nullptr;
    CheckResult* ckResult1 = nullptr;
    CheckResult* ckResult2 = nullptr;
    CheckResult* ckResult3 = nullptr;

    //单机开行时不判断防溜
    if(!_pTrainRouteOrder->m_bOnlyLocomotive)
    {
        //检查股道防溜是否撤除
        QString strGD = _pTrainRouteOrder->m_strTrack;//this->GetGDNameInGDNodeList();
        int flType = this->CheckGDFL(strGD);
        if(flType > 0)
        {
            ckResult = new CheckResult;
            QDateTime timeNow = QDateTime::currentDateTime();
            //ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
            ckResult->route_id = _pTrainRouteOrder->route_id;
            //进路序列操作时，使用进路序列id作为检查报警信息的id，用于和前端交互。
            ckResult->id = ckResult->route_id;
            if(1==flType)
            {
                ckResult->check = JLWARNING_HAVEFLDEVSX;
            }
            else if(2==flType)
            {
                ckResult->check = JLWARNING_HAVEFLDEVXX;
            }
            else
            {
                ckResult->check = JLWARNING_HAVEFLDEVSXX;
            }
            _pTrainRouteOrder->checkResultInterlock = ckResult->check;
            //返回值
            return ckResult;
        }
    }

    //防错办检查
    //站细
    ckResult1 = this->CheckPreventConditionStaDetails(_pTrainRouteOrder);
    if(ckResult1->check==0)
    {
        //时序
        ckResult2 = this->CheckPreventConditionSequence(_pTrainRouteOrder);
        if(ckResult2->check==0)
        {
            //联锁条件
            ckResult3 = this->CheckPreventConditionInterlock(_pTrainRouteOrder);
            if(ckResult3->check==0)
            {
                //全部检查通过
                ckResult = new CheckResult;
            }
            else
            {
                ckResult = ckResult3;
            }
        }
        else
        {
            ckResult = ckResult2;
        }
    }
    else
    {
        ckResult = ckResult1;
    }
    //进路序列操作时，使用进路序列id作为检查报警信息的id，用于和前端交互。
    ckResult->id = ckResult->route_id;
    //返回值
    return ckResult;
}

//检查进路的办理条件并自动设置触发标记，进路正常则返回true，否则返回false
bool MyStation::CheckTrainRouteOrder(TrainRouteOrder *_pTrainRouteOrder)
{
    //检查站细，满足才可设置自触
    CheckResult* ckResult1 = this->CheckPreventConditionStaDetails(_pTrainRouteOrder);
    //非常站控模式下，阶段计划自动触发
    if(this->m_nFCZKMode)
    {
        if(ckResult1->check==0)
        {
            if(FCZKSetAutoTouch)
            {
                _pTrainRouteOrder->m_nAutoTouch = true;
            }
            else
            {
                _pTrainRouteOrder->m_nAutoTouch = false;
            }
        }
    }
    else
    {
        //中心和车站调车模式下为自动触发
        if(this->ModalSelect.nModeState != 1)
        {
            if(ckResult1->check==0)
            {
                _pTrainRouteOrder->m_nAutoTouch = true;
            }
        }
        //车站控制模式下为人工触发
        else if(this->ModalSelect.nModeState == 1)
        {
            _pTrainRouteOrder->m_nAutoTouch = false;
        }
    }
    return _pTrainRouteOrder->checkResultStaDetails==0?true:false;
}
//检查进路及其兄弟进路的防错办信息，检查通过则返回true，否则返回false
CheckResult* MyStation::CheckBrotherRouteOrder(TrainRouteOrder *_pTrainRouteOrder, QString &msg)
{
    //是组合进路
    bool bZHJL = false;
    //组合进路判定 等待状态
    for(int r = 0; r < m_ArrayRouteOrder.size(); r++)
    {
        TrainRouteOrder* pTrainRouteOrder1 = m_ArrayRouteOrder[r];
        if( _pTrainRouteOrder->m_btRecvOrDepart == pTrainRouteOrder1->m_btRecvOrDepart
           && _pTrainRouteOrder->m_strTrainNum == pTrainRouteOrder1->m_strTrainNum
           && 0 == pTrainRouteOrder1->m_nOldRouteState
           && 0 == pTrainRouteOrder1->m_bZHJL
           && 0 < pTrainRouteOrder1->father_id )
        {
            bZHJL = true;
            //检查进路
            CheckResult* ckResult = CheckPreventConditionAll(pTrainRouteOrder1);
            if(ckResult->check > 0)
            {
                msg = ckResult->checkMsg;
                return ckResult;
            }
        }
    }
    //非组合进路判定 等待状态
    for(int r = 0; r < m_ArrayRouteOrder.size() && !bZHJL; r++)
    {
        TrainRouteOrder* pTrainRouteOrder1 = m_ArrayRouteOrder[r];
        if( _pTrainRouteOrder->m_btRecvOrDepart == pTrainRouteOrder1->m_btRecvOrDepart
           && _pTrainRouteOrder->m_strTrainNum == pTrainRouteOrder1->m_strTrainNum
           && 0 == pTrainRouteOrder1->m_nOldRouteState
           && 0 == pTrainRouteOrder1->m_bZHJL
           && 0 == pTrainRouteOrder1->father_id)
        {
            //检查发车进路
            CheckResult* ckResult = CheckPreventConditionAll(pTrainRouteOrder1);
            if(ckResult->check > 0)
            {
                msg = ckResult->checkMsg;
                return ckResult;
            }
            break;//仅匹配一个
        }
    }

    CheckResult* ckResult = new CheckResult;
    return ckResult;
}
//根据报警类型获取报警信息内容
QString MyStation::GetWariningMsgByType(TrainRouteOrder *pTrainRouteOrder, CheckResult* ckResult)
{
    //系统报警信息
    QString strSys = ckResult->checkMsg;
    //************* 防溜条件 *************
    //有防溜设备
    if(JLWARNING_HAVEFLDEVSX == ckResult->check
        || JLWARNING_HAVEFLDEVXX == ckResult->check
        || JLWARNING_HAVEFLDEVSXX == ckResult->check)
    {
        QString strSXXFL;
        if(JLWARNING_HAVEFLDEVSX == ckResult->check)
        {
            strSXXFL = QString("股道上行有防溜设备");
        }
        else if(JLWARNING_HAVEFLDEVXX == ckResult->check)
        {
            strSXXFL = QString("股道下行有防溜设备");
        }
        else
        {
            strSXXFL = QString("股道下行有防溜设备，股道上行有防溜设备");
        }
        strSys = QString("%1次股道%2%3%4")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack)
                .arg(strSXXFL)
                .arg(pTrainRouteOrder->m_nAutoTouch?QString("，不能自动办理"):QString(""));
        //修改报警信息
        ckResult->checkMsg = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，%2次股道%3%4")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack)
                .arg(strSXXFL);
    }

    //************* 联锁条件 *************
    //区段占用
    else if(JLWARNING_QDZY == ckResult->check)
    {
        strSys = QString("%1次%2进路(%3)触发时联锁条件不满足，进路不能办理，有区段占用")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                .arg(pTrainRouteOrder->m_strRouteDescrip);
        //修改报警信息
        ckResult->checkMsg = QString("车次%1 触发%2进路触发失败(与联锁逻辑冲突)，进路不能办理，有区段占用")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站");
    }
    //区段锁闭
    else if(JLWARNING_QDSB == ckResult->check)
    {
        strSys = QString("%1次%2进路(%3)触发时联锁条件不满足，进路不能办理，有区段锁闭")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                .arg(pTrainRouteOrder->m_strRouteDescrip);
        //修改报警信息
        ckResult->checkMsg = QString("车次%1 触发%2进路触发失败(与联锁逻辑冲突)，进路不能办理，有区段锁闭")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站");
    }
    //道岔四开
    else if(JLWARNING_DCSK == ckResult->check)
    {
        strSys = QString("%1次%2进路(%3)触发时联锁条件不满足，进路不能办理，道岔四开")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                .arg(pTrainRouteOrder->m_strRouteDescrip);
        //修改报警信息
        ckResult->checkMsg = QString("车次%1 触发%2进路触发失败(与联锁逻辑冲突)，进路不能办理，道岔四开")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站");
    }
    //封锁
    else if(JLWARNING_FS_DC == ckResult->check
       || JLWARNING_FS_GD == ckResult->check)
    {
        strSys = QString("%1次%2进路(%3)触发时联锁条件不满足，进路不能办理，进路设备封锁%4")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                .arg(pTrainRouteOrder->m_strRouteDescrip)
                .arg(ckResult->checkMsg);
        //修改报警信息
        ckResult->checkMsg = QString("车次%1 触发%2进路触发失败(与联锁逻辑冲突)，进路不能办理，进路设备封锁%3")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                .arg(ckResult->checkMsg);
    }
    //分路不良
    else if(JLWARNING_FLBL_GD == ckResult->check
       || JLWARNING_FLBL_WCQD == ckResult->check
       || JLWARNING_FLBL_DC == ckResult->check)
    {
        strSys = QString("%1次%2进路(%3)联锁逻辑检查失败，进路不能办理，存在分路不良区段")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                .arg(pTrainRouteOrder->m_strRouteDescrip);
        //修改报警信息
        ckResult->checkMsg = QString("车次:%1触发未知的返回结果，进路不能办理，存在分路不良区段")
                .arg(pTrainRouteOrder->m_strTrainNum);
    }
    //分路不良空闲
    else if(JLWARNING_FLBL_GDKX == ckResult->check)
    {
        strSys = QString("%1次%2进路(%3)联锁逻辑检查失败，进路不能办理，存在分路不良区段")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"进站":"出站")
                .arg(pTrainRouteOrder->m_strRouteDescrip);
        //修改报警信息
        ckResult->checkMsg = QString("车次:%1触发未知的返回结果，进路不能办理，存在分路不良区段")
                .arg(pTrainRouteOrder->m_strTrainNum);
        //可以强制执行
        ckResult->bEnforced = true;
    }

    //************* 站细条件 *************
    //不满足-股道类型
    else if(JLWARNING_ATTR_GDTYPE == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，列车%2次%3类型不满足")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        ckResult->checkMsg = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，列车%2次%3类型不满足")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
    }
    //不满足-超限条件
    else if(JLWARNING_ATTR_LEVELCX == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发未知的返回结果，%2次股道%3超限条件不满足")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次股道%3超限条件不满足")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        //可以强制执行
        ckResult->bEnforced = true;
    }
    //不满足-客运设备股道（站台）
    else if(JLWARNING_ATTR_PLATFORM1 == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，旅客列车%2次无法接入无客运设备%3")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        ckResult->checkMsg = QString("车次%1 触发进路触发失败(与联锁逻辑冲突)，旅客列车%2次无法接入无客运设备%3")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
    }
    //不满足-货车不可以接入高站台
    else if(JLWARNING_ATTR_PLATFORM2 == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发未知的返回结果，货车%2次不能通过高站台股道%3")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        ckResult->checkMsg = QString("车次%1 触发未知的返回结果，货车%2次不能通过高站台股道%3")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        //可以强制执行
        ckResult->bEnforced = true;
    }
    //不满足-上水
    else if(JLWARNING_ATTR_FLOWSS == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发未知的返回结果，%2次股道%3无上水设备")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次股道%3无上水设备")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        //可以强制执行
        ckResult->bEnforced = true;
    }
    //不满足-吸污
    else if(JLWARNING_ATTR_FLOWXW == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发未知的返回结果，%2次股道%3无吸污设备")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次股道%3无吸污设备")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        //可以强制执行
        ckResult->bEnforced = true;
    }
    //不满足-出入口超限
    else if(JLWARNING_ENEX_LEVELCX == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发未知的返回结果，%2次%3方向不允许超限车(出入口%4)")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
        ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次%3方向不允许超限车(出入口%4)")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
        //可以强制执行
        ckResult->bEnforced = true;
    }
    //不满足-客货类型错误
    else if(JLWARNING_ENEX_KHTYPE == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发未知的返回结果，%2次%3方向客货类型错误(出入口%4)")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
        ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次%3方向客货类型错误(出入口%4)")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车")
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?pTrainRouteOrder->m_strXHD_JZk:pTrainRouteOrder->m_strXHD_CZk);
        //可以强制执行
        ckResult->bEnforced = true;
    }
    //不满足-军运
    else if(JLWARNING_ATTR_ARMY == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发未知的返回结果，军用列车%2次无法进入非军用股道%3")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        ckResult->checkMsg = QString("车次%1 触发未知的返回结果，军用列车%2次无法进入非军用股道%3")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrack);
        //可以强制执行
        ckResult->bEnforced = true;
    }
    //不满足-列车固定径路信息
    else if(JLWARNING_ENEX_UNSAME == ckResult->check)
    {
        //修改报警信息
        strSys = QString("车次%1 触发未知的返回结果，%2次%3方向与固定路径不一致")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车");
        ckResult->checkMsg = QString("车次%1 触发未知的返回结果，%2次%3方向与固定路径不一致")
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_strTrainNum)
                .arg(pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC?"接车":"发车");
        //可以强制执行
        ckResult->bEnforced = true;
    }

    //************* 时序条件 *************
    //不满足-车次冲突
    else if((JLWARNING_SEQU_CCCT == ckResult->check || JLWARNING_SEQU_CROSS == ckResult->check)
        && pTrainRouteOrder->m_nAutoTouch)
    {
        strSys = ckResult->checkMsg;
    }
    //不满足-车次冲突
    else if((JLWARNING_SEQU_CCCT == ckResult->check || JLWARNING_SEQU_CROSS == ckResult->check)
        && pTrainRouteOrder->m_nManTouch)
    {
        strSys = ckResult->checkMsg;
    }

    return strSys;
}
//更新检查信息数组
void MyStation::UpdateCheckResultArray(CheckResult *checkResult, bool bDelete)
{
    if(checkResult == nullptr)
    {
        return;
    }

    for(int i=0; i<vectCheckResult.size(); i++)
    {
        //匹配和更新
        if(checkResult->route_id == vectCheckResult[i]->route_id)
        {
            //删除
            if(bDelete)
            {
                vectCheckResult.remove(i);
                return;
            }
            //更新
            vectCheckResult[i]->id = checkResult->id;
            vectCheckResult[i]->indexRoute = checkResult->indexRoute;
            vectCheckResult[i]->bEnforced = checkResult->bEnforced;
            vectCheckResult[i]->check = checkResult->check;
            vectCheckResult[i]->checkMsg = checkResult->checkMsg;
            vectCheckResult[i]->dataArray = checkResult->dataArray;
            return;
        }
    }
    //没找到则新增
    vectCheckResult.append(checkResult);
}
//发送提示信息-QMessageBox显示内容(type=1基本信息information,2疑问question,3警告warning,4错误critical)
void MyStation::SendMessageBoxMsg(int type, QString strMsg,int currCtcIndex)
{
    QByteArray dataArray;
    int nCount = 10;
    //添加nCount个字节并全部置零
    dataArray.append(nCount, char(0));
    //标志位
    dataArray[nCount-1] = FUNCTYPE_SYSMSG;
    //子分类码-提示信息
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = 0x02;

    //信息类别
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = type;
    //信息描述
    QByteArray byteArray = strMsg.toLocal8Bit();//toLatin1();
    int msgLen = byteArray.count();
    //描述长度
    nCount++;
    dataArray.append(1, char(0));
    dataArray[nCount-1] = msgLen;
    //描述内容
    nCount+=msgLen;
    dataArray.append(msgLen, char(0));
    for(int u=0; u<msgLen; u++)
    {
        dataArray[nCount-msgLen+u] = byteArray[u];
    }

    //帧尾4+空白2
    nCount+=6;
    dataArray.append(6, char(0));
    //打包数据
    packHeadAndTail(&dataArray, nCount);
    qDebug()<<"MessageBoxMsg ="<<strMsg;
    qDebug()<<"MessageBoxMsg dataArray="<<ByteArrayToString(dataArray);
    //信号-发送数据给CTC
    emit sendDataToCTCSignal(this, dataArray, nCount, currCtcIndex);
}
//接收CTC数据-强制执行操作
void MyStation::recvCTCData_ForceExecute(QByteArray recvArray)
{
    int nCount = 10;
    //子分类码
    int type = (int)(recvArray[nCount]&0xFF);
    nCount++;
    //操作指令
    if(0x02 == type)
    {
        int id = ByteArrayToUInt(recvArray.mid(nCount,4));
        nCount += 4;
        //操作
        int excut = (int)(recvArray[nCount]&0xFF);
        nCount++;

        //遍历查找
        for(int i=0; i<vectCheckResult.size(); i++)
        {
            CheckResult* checkResult = vectCheckResult[i];
            //匹配
            if(id == checkResult->id)
            {
                if(checkResult->route_id>0 && checkResult->bEnforced)
                {
                    TrainRouteOrder *pTrainRouteOrder = this->GetTrainRouteOrderById(checkResult->route_id);
                    if(pTrainRouteOrder != nullptr)
                    {
                        //强制执行
                        if(0x01 == excut)
                        {
                            //发送办理指令
                            SendOrderToInterlock(pTrainRouteOrder);
                        }
                        //放弃执行
                        else if(0x02 == excut)
                        {
                            if(pTrainRouteOrder->m_nAutoTouch)
                            {
                                pTrainRouteOrder->m_nAutoTouch = false;
                                pTrainRouteOrder->m_nManTouch = false;
                                pTrainRouteOrder->m_bRunNow = false;
                                //更新数据库
                                m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                                //发送同步消息
                                this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                            }
                        }
                        //执行组合进路其他兄弟进路
                        ForceExecuteZHJL(pTrainRouteOrder,excut);
                    }
                }
                else if(checkResult->cmdArray.size()>0)
                {
                    //强制执行
                    if(0x01 == excut)
                    {
                        //信号-发送数据给联锁
                        emit sendDataToLSSignal(this, checkResult->cmdArray, checkResult->cmdArray.size());
                    }
                    //放弃执行
                    else if(0x02 == excut)
                    {
                        //不执行操作
                    }
                }
                int indexx = vectCheckResult.indexOf(checkResult);
                qDebug()<<"vectCheckResult-indexx="<<indexx;
                vectCheckResult.removeAt(indexx);
                return;
            }
        }
    }
}
//强制执行组合进路的兄弟进路
void MyStation::ForceExecuteZHJL(TrainRouteOrder *pTrainRouteOrderSon, int excut)
{
    int fatherId = pTrainRouteOrderSon->father_id;
    //是否为组合进路
    if(fatherId<=0)
    {
        //常规进路则退出
        return;
    }

    //遍历查找
    for(int j=0; j<vectCheckResult.size(); j++)
    {
        CheckResult* checkResult = vectCheckResult[j];
        if(checkResult->route_id == pTrainRouteOrderSon->route_id)
        {
            continue;//本进路已处理，则忽略
        }
        for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
        {
            TrainRouteOrder* pTrainRouteOrder = this->m_ArrayRouteOrder[i];
            if(pTrainRouteOrderSon->route_id == pTrainRouteOrder->route_id)
            {
                continue;//本进路已处理，则忽略
            }
            //匹配
            if((fatherId == pTrainRouteOrder->father_id)
                && (checkResult->route_id==pTrainRouteOrder->route_id))
            {
                //强制执行
                if(0x01 == excut)
                {
                    //发送办理指令
                    SendOrderToInterlock(pTrainRouteOrder);
                }
                //放弃执行
                else if(0x02 == excut)
                {
                    if(pTrainRouteOrder->m_nAutoTouch)
                    {
                        pTrainRouteOrder->m_nAutoTouch = false;
                        pTrainRouteOrder->m_nManTouch = false;
                        pTrainRouteOrder->m_bRunNow = false;
                        //更新数据库
                        m_pDataAccess->UpdateRouteOrder(pTrainRouteOrder);
                        //发送同步消息
                        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_UPDATE,1,1);
                    }
                }
                //匹配
                if(checkResult->route_id == pTrainRouteOrder->route_id)
                {
                    //移除
                    //vectCheckResult.removeAt(j);
                    int indexx = vectCheckResult.indexOf(checkResult);
                    qDebug()<<"vectCheckResult-indexx="<<indexx;
                    vectCheckResult.removeAt(indexx);
                }
            }
        }
    }
}
//根据id获取列车进路序列
TrainRouteOrder *MyStation::GetTrainRouteOrderById(int routeId)
{
    TrainRouteOrder* pTrainRouteOrder = nullptr;
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = this->m_ArrayRouteOrder[i];
        if(routeId == pTrainRouteOrder->route_id)
        {
            return pTrainRouteOrder;
        }
    }
    return pTrainRouteOrder;
}
//根据进路指令和输入车次查找相应进路序列
TrainRouteOrder *MyStation::FindTrainRouteIndexByCmd(QByteArray recvArray, QString _strCheci)
{
    TrainRouteOrder* pTrainRouteOrder = nullptr;
    //列车进路
    if((int)(recvArray[10]&0xFF) == 0x01)
    {
        //判断进路始端是否为进站信号机
        BYTE jfcType = 0x00;
        int firstXHDCode  = (int)(recvArray[11]&0xFF | ((recvArray[12]&MASK05)<<8));
        int xhId = this->GetIndexByStrName(this->GetStrNameByCode(firstXHDCode));
        if(xhId>-1)
        {
            CXHD* pXHD = (CXHD*)this->DevArray[xhId];
            if(pXHD->getXHDType() == JZ_XHJ)
            {
                jfcType = ROUTE_JC;
            }
            else
            {
                jfcType = ROUTE_FC;
            }
            int nSize = this->m_ArrayRouteOrder.size();
            for(int i=0; i<nSize; i++)
            {
                pTrainRouteOrder = this->m_ArrayRouteOrder[i];
                //根据接车找发车、根据发车找接车
                if(pTrainRouteOrder->m_strTrainNum == _strCheci && pTrainRouteOrder->m_btRecvOrDepart == jfcType)
                {
                    return pTrainRouteOrder;
                }
            }
        }
    }

    return nullptr;//pTrainRouteOrder;
}
//检查列车接近进站信号机信号未开放报警
void MyStation::CheckTrainCloseToJZXHD()
{
    for(int i=0; i<this->StaConfigInfo.JCKCount; i++)
    {
        int nxhIndex = -1;
        nxhIndex = this->GetIndexByStrName(this->StaConfigInfo.JFCKInfo[i].strJCKName);
        if(nxhIndex < 0)
        {
            continue;
        }
        CXHD *pXHD = (CXHD *)this->DevArray[nxhIndex];
        int state = pXHD->getXHDState();
        //进站信号机开放（绿灯 or 绿黄 or 单黄 or 双黄 or 双绿 , or 引导 or 二黄 or 黄闪黄）
        if((state == XHD_LD) || (state == XHD_LU) || (state == XHD_UD) || (state == XHD_UU) || (state == XHD_LL)
            || (state == XHD_YD) || (state == XHD_2U) || (state == XHD_USU) )
        {
            continue;
        }
        else//进站信号机没开放
        {
            for(int j=0; j<this->StaConfigInfo.JFCKInfo[i].strArrJckJJQD.size(); j++)
            {
                int ngdIndex = -1;
                ngdIndex = this->GetIndexByStrName(this->StaConfigInfo.JFCKInfo[i].strArrJckJJQD[j]);//strJCKjucJJQD
                if(ngdIndex > 0)
                {
                    CGD *pGD = (CGD *)this->DevArray[ngdIndex];
                    //接近区段占用且有车次-闭塞分区由近及远的第一个车次
                    if(pGD->getState(QDZY) && (pGD->m_strCheCiNum.length() > 0))
                    {
                        bool bWarning = FALSE;
                        QString strCheci = pGD->m_strCheCiNum;
                        //右行为下行
                        if(this->StaConfigInfo.bStaSXLORR)
                        {
                            //pGD->m_nSXCheCi;//1右行 0左行
                            if(pXHD->m_nSX)//上行
                            {
                                if(!pGD->m_nSXCheCi)//0左行
                                {
                                    //信号机和车次方向一致
                                    bWarning = TRUE;
                                    CheckAndTouchJCRouteOnAutoMode(strCheci,pXHD->m_strName);
                                    break;//触发第一列车，后面紧跟的车不再触发
                                }
                            }
                            else//下行
                            {
                                if(pGD->m_nSXCheCi)//1右行
                                {
                                    //信号机和车次方向一致
                                    bWarning = TRUE;
                                    CheckAndTouchJCRouteOnAutoMode(strCheci,pXHD->m_strName);
                                    break;//触发第一列车，后面紧跟的车不再触发
                                }
                            }
                        }
                        else//右行为上行
                        {
                            if(pXHD->m_nSX)//上行
                            {
                                if(pGD->m_nSXCheCi)//1右行
                                {
                                    //信号机和车次方向一致
                                    bWarning = TRUE;
                                    CheckAndTouchJCRouteOnAutoMode(strCheci,pXHD->m_strName);
                                    break;//触发第一列车，后面紧跟的车不再触发
                                }
                            }
                            else//下行
                            {
                                if(!pGD->m_nSXCheCi)//0左行
                                {
                                    //信号机和车次方向一致
                                    bWarning = TRUE;
                                    CheckAndTouchJCRouteOnAutoMode(strCheci,pXHD->m_strName);
                                    break;//触发第一列车，后面紧跟的车不再触发
                                }
                            }
                        }
                        //信号没及时开放则报警
                        if(bWarning)
                        {
//                            pFrame->m_strArrTrainCloseToCheci.Add(strCheci);
//                            pFrame->m_bTrainCloseToNoSignal = TRUE;
//                            //CString strSpeakCheci = TranslateCHECISpeek(strCheci) + _T("次列车接近，信号未开放！");
//                            //pFrame->SpeakFun(strSpeakCheci,1);
                        }
                    }

                }
            }
        }

    }
}
//列车进路序列自动触发模式下，列车接近自动办理接车进路
void MyStation::CheckAndTouchJCRouteOnAutoMode(QString strCheCi, QString strJZXHD)
{
    //互斥锁
    QMutexLocker locker(&Mutex);
    //qDebug()<<QString("列车接近%1，判定进路...").arg(strCheCi);
    for(int i = 0; i < this->m_ArrayRouteOrder.size(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if(pTrainRouteOrder->m_bZHJL)
        {
            continue;
        }
        //接车进路指令
        if(ROUTE_JC == pTrainRouteOrder->m_btRecvOrDepart)
        {
            //正常序列判定
            if(pTrainRouteOrder->father_id==0)
            {
                if((strCheCi == pTrainRouteOrder->m_strTrainNum)
                    && (0 == pTrainRouteOrder->m_nOldRouteState)
                    && (strJZXHD == pTrainRouteOrder->m_strXHDBegin)
                    )
                {
                    QString strMsg;
                    CheckResult* ckResult = CheckBrotherRouteOrder(pTrainRouteOrder, strMsg);
                    if(ckResult->check!=0)
                    {
                        //QString strSys = QString("%1次列车，进路检查不通过 %2").arg(pTrainRouteOrder->m_strTrainNum).arg(strMsg);
                        QString strSys = GetWariningMsgByType(pTrainRouteOrder,ckResult);
                        qDebug()<<strSys;
                        this->sendWarningMsgToCTC(1,2,strSys);
                        break;//防错办检查不通过
                    }

                    if(true == pTrainRouteOrder->m_nAutoTouch)
                    {
                        pTrainRouteOrder->m_bRunNow = true;
                    }
                    else
                    {
                        pTrainRouteOrder->m_bRunNow = false;
                    }
                    break;
                }
            }
            //组合进路的子序列判定
            else if(pTrainRouteOrder->father_id>0)
            {
                if((strCheCi == pTrainRouteOrder->m_strTrainNum)
                    && (0 == pTrainRouteOrder->m_nOldRouteState)
                    )
                {
                    QString strMsg;
                    CheckResult* ckResult = CheckBrotherRouteOrder(pTrainRouteOrder, strMsg);
                    pTrainRouteOrder->m_timCheck = QDateTime::currentDateTime();
                    if(ckResult->check!=0)
                    {
                        //QString strSys = QString("%1次列车，进路检查不通过 %2").arg(pTrainRouteOrder->m_strTrainNum).arg(strMsg);
                        QString strSys = GetWariningMsgByType(pTrainRouteOrder,ckResult);
                        qDebug()<<strSys;
                        this->sendWarningMsgToCTC(1,2,strSys);
                        break;//防错办检查不通过
                    }

                    if(true == pTrainRouteOrder->m_nAutoTouch)
                    {
                        pTrainRouteOrder->m_bRunNow = true;
                    }
                    else
                    {
                        pTrainRouteOrder->m_bRunNow = false;
                    }
                    //组合进路的子序列继续循环
                    //break;
                }
            }
        }
    }
}
//检查进路是否可以触发，自触模式的，(进路，冲突车次)
bool MyStation::CheckRouteIsFirst(TrainRouteOrder *pCheckRoute, QString &strCheciConflict)
{
    //检查进路是否可以触发(进路，进路索引)(自动进路时序性检查，本进路之前是否还有未执行的进路)
    //目标进路最早则自动触发、目标进路靠前的优先或者没有找到更早的则为目标进路自动触发、目标进路时间不是最早则不自动触发

    int chkIndex = -1;
    for(int i = 0; i < this->m_ArrayRouteOrder.size(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if(pCheckRoute->route_id == pTrainRouteOrder->route_id)
        {
            chkIndex = i;
            break;
        }
    }

    //进路始端信号机状态判断
    int xhdIndex = this->GetIndexByStrName(pCheckRoute->m_strXHDBegin);
    if(xhdIndex > -1)
    {
        CXHD *pXHD = (CXHD*)this->DevArray[xhdIndex];
        //若始端信号机不是红灯，则说明列车进路无法正常办理，则本进路不可触发
        if(pXHD->getXHDState() != XHD_HD)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    bool rtValue = false;
    int timeFirstRouteIndex = chkIndex;//0;//-1;//时间最早的进路索引
    QDateTime timeFirst = pCheckRoute->m_timBegin;
    //int nRouteOrderSize = this->m_ArrayRouteOrder.size();

    //接车进路判断 HavaAnotherRouteBeforeTheRoute
    if(pCheckRoute->m_btRecvOrDepart == ROUTE_JC)
    {
        for(int i = 0; i < this->m_ArrayRouteOrder.size(); i++)
        {
            TrainRouteOrder* pRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
            //不判断自己 和 出清的进路
            //if(i == chkIndex)
            if(pCheckRoute->m_strTrainNum == pRouteOrder->m_strTrainNum)
            {
                //timeFirst = pRouteOrder->m_timBegin;
                //timeFirstRouteIndex = i;
                //continue;
                break;
            }
            //不判断出清的进路
            else if(4 == pRouteOrder->m_nOldRouteState)
            {
                continue;
            }
            //不判断组合进路的主进路
            else if(pRouteOrder->m_bZHJL)
            {
                continue;
            }
            //判断其他进路
            //等待状态(或触发中、触发完成、占用)、接车、X进站口一致、X自触模式，取得最早的时间
            if(4 > pRouteOrder->m_nOldRouteState /*&& pRouteOrder->m_btRecvOrDepart == ROUTE_JC*/
                /*&& pRouteOrder->m_nCodeReachStaEquip == pCheckRoute->m_nCodeReachStaEquip*/
                /*&& pRouteOrder->m_nAutoTouch*/
                )
            {
                //时间早于最早的时间
                if(pRouteOrder->m_timBegin <= timeFirst)
                {
                    timeFirst = pRouteOrder->m_timBegin;
                    timeFirstRouteIndex = i;
                    //检查的进路靠后，则认定前面的进路
                    if(chkIndex>i)
                    {
                        strCheciConflict = pRouteOrder->m_strTrainNum;
                    }
                }
            }
        }
        //目标进路最早
        if(pCheckRoute->m_timBegin < timeFirst)
        {
            timeFirst = pCheckRoute->m_timBegin;
            timeFirstRouteIndex = chkIndex;
            rtValue = true;
        }
        //目标进路时间一致
        else if(pCheckRoute->m_timBegin == timeFirst)
        {
            //靠前的优先 或者 没有找到更早的则为自己
//            if(chkIndex <= timeFirstRouteIndex || timeFirstRouteIndex == 0)//-1
//            {
//                timeFirst = pCheckRoute->m_timBegin;
//                rtValue = true;
//            }
            if(chkIndex <= timeFirstRouteIndex /*|| timeFirstRouteIndex == 0*/)
            {
                rtValue = true;
            }
            else
            {
                TrainRouteOrder* pRouteOrderFirst = (TrainRouteOrder*)this->m_ArrayRouteOrder[timeFirstRouteIndex];
                if(pCheckRoute->m_strTrainNum == pRouteOrderFirst->m_strTrainNum)
                {
                    rtValue = true;
                }
                else
                {
                    strCheciConflict = pRouteOrderFirst->m_strTrainNum;
                    rtValue = false;
                }
            }
        }
        //目标进路时间不是最早
        else
        {
            rtValue = false;
        }
    }
    //发车进路判断 HavaAnotherRouteBeforeTheRoute
    else if(pCheckRoute->m_btRecvOrDepart == ROUTE_FC)
    {
        for(int i = 0; i < this->m_ArrayRouteOrder.size(); i++)
        {
            TrainRouteOrder* pRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
            //不判断自己
            //if(i == chkIndex)
            if(pCheckRoute->m_strTrainNum == pRouteOrder->m_strTrainNum)
            {
                break;
            }
            //只判断比自己早的计划
            if(pCheckRoute->m_timBegin < pRouteOrder->m_timBegin)
            {
                continue;
            }
            //判断其他进路
            //等待状态(或触发中、触发完成、占用)、发车、X出站口一致、X自触模式，取得最早的时间
            if(4 > pRouteOrder->m_nOldRouteState /*&& pRouteOrder->m_btRecvOrDepart == ROUTE_FC*/
                /*&& pRouteOrder->m_nCodeDepartStaEquip == pCheckRoute->m_nCodeDepartStaEquip*/
                /*&& pRouteOrder->m_nAutoTouch*/
                )
            {
                //时间早于最早的时间
                if(pRouteOrder->m_timBegin <= timeFirst)
                {
                    timeFirst = pRouteOrder->m_timBegin;
                    timeFirstRouteIndex = i;
                    //检查的进路靠后，则认定前面的进路
                    if(chkIndex>i)
                    {
                        strCheciConflict = pRouteOrder->m_strTrainNum;
                    }
                }
            }
        }
        //目标进路最早
        if(pCheckRoute->m_timBegin < timeFirst)
        {
            timeFirst = pCheckRoute->m_timBegin;
            timeFirstRouteIndex = chkIndex;
            rtValue = true;
        }
        //目标进路时间一致
        else if(pCheckRoute->m_timBegin == timeFirst)
        {
            //靠前的优先 或者 没有找到更早的则为自己
//            if(chkIndex <= timeFirstRouteIndex || timeFirstRouteIndex == 0)//-1
//            {
//                timeFirst = pCheckRoute->m_timBegin;
//                rtValue = true;
//            }
            if(chkIndex <= timeFirstRouteIndex /*|| timeFirstRouteIndex == 0*/)
            {
                rtValue = true;
            }
            else
            {
                TrainRouteOrder* pRouteOrderFirst = (TrainRouteOrder*)this->m_ArrayRouteOrder[timeFirstRouteIndex];
                if(pCheckRoute->m_strTrainNum == pRouteOrderFirst->m_strTrainNum)
                {
                    rtValue = true;
                }
                else
                {
                    strCheciConflict = pRouteOrderFirst->m_strTrainNum;
                    rtValue = false;
                }
            }
        }
        //目标进路时间不是最早
        else
        {
            rtValue = false;
        }
    }
    return rtValue;
}
//检查进路是否交叉(进路，冲突车次)
//bool MyStation::CheckRouteIsCross(TrainRouteOrder *pCheckRoute, QString &strCheciConflict)
bool MyStation::CheckRouteIsCross(TrainRouteOrder* pCheckRoute, TrainRouteOrder* pCheckRouteConflict)
{
    for(int i = 0; i < this->m_ArrayRouteOrder.size(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //检查比目标进路更早的进路
        if(pCheckRoute->m_timBegin >= pTrainRouteOrder->m_timBegin)
        {
            //不检查同车次、已出清、等待的进路
            if(pCheckRoute->m_strTrainNum == pTrainRouteOrder->m_strTrainNum
               || 4 == pTrainRouteOrder->m_nOldRouteState
               /*|| 0 == pTrainRouteOrder->m_nOldRouteState*/)
            {
                continue;
            }
            if(pCheckRoute->m_nIndexRoute>-1 && pTrainRouteOrder->m_nIndexRoute>-1)
            {
                bool bCross = CheckInterlockRouteIsCross(pCheckRoute->m_nIndexRoute,pTrainRouteOrder->m_nIndexRoute);
                if(bCross)
                {
                    //strCheciConflict = pTrainRouteOrder->m_strTrainNum;
                    pCheckRouteConflict->route_id = pTrainRouteOrder->route_id;
                    pCheckRouteConflict->m_strTrainNum = pTrainRouteOrder->m_strTrainNum;
                    pCheckRouteConflict->m_btRecvOrDepart = pTrainRouteOrder->m_btRecvOrDepart;
                    pCheckRouteConflict->m_strRouteDescrip = pTrainRouteOrder->m_strRouteDescrip;
                    pCheckRouteConflict->m_nOldRouteState = pTrainRouteOrder->m_nOldRouteState;
                    return true;
                }
            }
        }
    }

    return false;
}
//检查两条联锁表进路是否交叉
bool MyStation::CheckInterlockRouteIsCross(int _idxRoute1, int _idxRoute2)
{
    if(_idxRoute1 <=-1 || _idxRoute2<=-1)
    {
        return false;
    }

    InterlockRoute *pRut1 = (InterlockRoute *)(vectInterlockRoute[_idxRoute1]);
    //检查区段
    QStringList pQDArray1;
    pQDArray1.append(pRut1->QDArr);
    int c1 = pQDArray1.count();
    for(int i = 0; i < c1; i++)
    {
        QString qdname = pQDArray1[i];
        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            QString dcname2 = "";
            int pos = dcname.indexOf("-");
            if( pos > 0 )
            {
                QStringList strArr;
                QString ch = "-";
                StringSplit(dcname, ch, strArr);
                dcname = strArr[0];
                dcname2 = strArr[1];
            }
            //联锁表中查找
            if(IsDevInInterlockRoute(_idxRoute2, dcname))
            {
                return true;
            }
            if(dcname2 != "" && IsDevInInterlockRoute(_idxRoute2, dcname2))
            {
                return true;
            }
        }
        else//区段、股道
        {
            if(IsDevInInterlockRoute(_idxRoute2, qdname))
            {
                return true;
            }
        }
    }
#if 0 //判定类似平行进路时双动道岔误报，所以不再单独判定道岔
    //检查道岔
    QStringList pDCArray1;
    pDCArray1.append(pRut1->DCArr);
    c1 = pDCArray1.count();
    for(int i = 0; i < c1; i++)
    {
        QString dcname = pDCArray1[i];
        QString dcname2 = "";
        dcname.replace("(","");
        dcname.replace(")","");
        dcname.replace("[","");
        dcname.replace("]","");
        dcname.replace("{","");
        dcname.replace("}","");
        int pos = dcname.indexOf("-");
        if( pos > 0 )
        {
            QStringList strArr;
            QString ch = "-";
            StringSplit(dcname, ch, strArr);
            dcname = strArr[0];
            dcname2 = strArr[1];
        }
        //联锁表中查找
        if(IsDevInInterlockRoute(_idxRoute2, dcname))
        {
            return true;
        }
        if(dcname2 != "" && IsDevInInterlockRoute(_idxRoute2, dcname2))
        {
            return true;
        }
    }
#endif

    return false;
}
//设备是否在联锁表中
bool MyStation::IsDevInInterlockRoute(int _idxRoute, QString _strDevName)
{
    if(_idxRoute <=-1)
    {
        return false;
    }

    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[_idxRoute]);
    //检查区段
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);
    int c = pQDArray.count();
    for(int i = 0; i < c; i++)
    {
        QString qdname = pQDArray[i];
        if(qdname.right(2) == "DG")//道岔区段
        {
            QString dcname = qdname.left(qdname.length() - 2);
            QString dcname2 = "";
            int pos = dcname.indexOf("-");
            if( pos > 0 )
            {
                QStringList strArr;
                QString ch = "-";
                StringSplit(dcname, ch, strArr);
                dcname = strArr[0];
                dcname2 = strArr[1];
            }
            //联锁表中查找
            if(_strDevName == dcname)
            {
                return true;
            }
            if(dcname2 != "" && (_strDevName == dcname2))
            {
                return true;
            }
        }
        else//区段、股道
        {
            if(_strDevName == qdname)
            {
                return true;
            }
        }
    }

#if 0 //判定类似平行进路时双动道岔误报，所以不再单独判定道岔
    //检查道岔
    QStringList pDCArray;
    pDCArray.append(pRut->DCArr);
    c = pDCArray.count();
    for(int i = 0; i < c; i++)
    {
        QString dcname = pDCArray[i];
        QString dcname2 = "";
        dcname.replace("(","");
        dcname.replace(")","");
        dcname.replace("[","");
        dcname.replace("]","");
        dcname.replace("{","");
        dcname.replace("}","");
        int pos = dcname.indexOf("-");
        if( pos > 0 )
        {
            QStringList strArr;
            QString ch = "-";
            StringSplit(dcname, ch, strArr);
            dcname = strArr[0];
            dcname2 = strArr[1];
        }
        if(_strDevName == dcname)
        {
            return true;
        }
        if(dcname2 != "" && (_strDevName == dcname2))
        {
            return true;
        }
    }
#endif

    return false;
}
//检查进路的股道-占用并停稳(若股道有中岔或者股道为两段，则要判断另外一段区段)
bool MyStation::CheckRouteGDZYTW(TrainRouteOrder *pCheckRoute)
{
    for(int i=0; i<this->vectStationGDNode.size();i++)
    {
        if(pCheckRoute->m_strTrack == this->vectStationGDNode[i].strGDName)
        {
            bool bOK = false;
            int gdIndex = this->GetIndexByStrName(pCheckRoute->m_strTrack);
            if(gdIndex<=-1)
            {
                return false;//防护
            }
            CGD* pGD = (CGD*)this->DevArray[gdIndex];
            //股道上有车占用，且列车停稳，且车次一致
            if(pGD->getState(QDZY) && pGD->m_bLCTW
                    && pGD->m_strCheCiNum==pCheckRoute->m_strTrainNum)
            {
                bOK = true;
                return true;
            }
            if(!bOK)
            {
                if(this->vectStationGDNode[i].strGDName1 != "")
                {
                    CGD* pGD1 = NULL;
                    CGDDC* pGDDC1 = NULL;
                    QString gd1Name = this->vectStationGDNode[i].strGDName1;
                    int gdIndex1 = this->GetIndexByStrName(gd1Name);
                    //在股道或无岔区段上
                    if(gdIndex1 > -1)
                    {
                        pGD1 = (CGD*)this->DevArray[gdIndex1];
                        //股道上有车占用，且列车停稳，且车次一致，才可自动办理发车进路
                        if(pGD1->getState(QDZY) && pGD1->m_bLCTW
                                && pGD1->m_strCheCiNum==pCheckRoute->m_strTrainNum)
                        {
                            bOK = true;
                            return true;
                        }
                    }
                    //在道岔上
                    else
                    {
                        int dcindex = this->GetDCIndexByDCQDName(gd1Name);
                        if(dcindex>=0)
                        {
                            pGDDC1 = (CGDDC*)this->DevArray[dcindex];
                            //股道上有车占用，且列车停稳，且车次一致，才可自动办理发车进路
                            if(pGDDC1->getState(QDZY) && pGDDC1->m_bLCTW
                                    && pGDDC1->m_strCheCiNum==pCheckRoute->m_strTrainNum)
                            {
                                bOK = true;
                                return true;
                            }
                        }
                    }
                }
            }
            break;
        }
    }

    return false;
}
//根据道岔区段名称获取道岔设备索引
int MyStation::GetDCIndexByDCQDName(QString qdname)
{
    CGDDC *pDC;
    CText *pTxt;
    QString strDCname = "";

    //取道岔名称
    for(int i = 0; i < (int)this->DevArray.size(); i++)
    {
        if(this->DevArray[i]->getDevType() == Dev_TEXT)
        {
            pTxt = (CText*)this->DevArray[i];
            if(qdname == pTxt->m_strName)
            {
                strDCname = pTxt->m_DCname;
                break;
            }
        }
    }
    //取道岔索引
    for(int i = 0; i < (this->DevArray.size() && (strDCname!="")); i++)
    {
        if(this->DevArray[i]->getDevType() == Dev_DC)
        {
            pDC = (CGDDC*)this->DevArray[i];
            if(strDCname == pDC->m_strName)
            {
                return i;
            }
        }
    }
    return -1;
}

//判断信号机是否灯丝断丝并报警
void MyStation::JudgXHDDSstate(QString xhdName)
{
    for(int i = 0; i < (int)this->DevArray.count(); i++)
    {
        if(this->DevArray[i]->getDevType() == Dev_XH)
        {
            CXHD *pXHD = (CXHD *)this->DevArray[i];
            if(pXHD->getName()==xhdName)
            {
                if((pXHD->getXHDState() == XHD_DS)
                    ||((GetXHDType(pXHD->m_nCode)==CZ_XHJ)&&(pXHD->XHD_ds_LD))
                    ||((GetXHDType(pXHD->m_nCode)==CZ_XHJ)&&(pXHD->XHD_ds_UD))
                    ||((GetXHDType(pXHD->m_nCode)==JZ_XHJ)&&(pXHD->XHD_ds_YBD))
                    ||((GetXHDType(pXHD->m_nCode)==JZ_XHJ)&&(pXHD->XHD_ds_UD))
                    ||((GetXHDType(pXHD->m_nCode)==DC_XHJ)&&(pXHD->XHD_ds_BD))
                    )
                {
                    QString msg = QString("信号机灯丝断丝！");
                    QString msg_XHD = QString("%1信号机灯丝断丝！").arg(xhdName);
                    //发送报警提示信息
                    this->sendWarningMsgToCTC(3,1,msg_XHD);
                    //语音播报
                    this->SendSpeachText(msg);
                }
                return;
            }
        }
    }
}

//获取车站接发车口相应邻站的车站id
int MyStation::GetStationJFCKLZStationId(QString strJFCK)
{
    for (int j = 0; j< (this->StaConfigInfo.JCKCount); j++)
    {
        if(this->StaConfigInfo.JFCKInfo[j].strJCKName == strJFCK
          || this->StaConfigInfo.JFCKInfo[j].strFCKName == strJFCK)
        {
            return this->StaConfigInfo.JFCKInfo[j].nLZStationId;
        }
    }
    return 0;
}
//更新邻站报点（邻站到达、邻站出发）(type类型-0x22到达，0x11出发，0x33通过)
void MyStation::UpdateLZReportTime(QString checi,int type, QDateTime dateTime)
{
    QDateTime tmDefault;
    int logIndex = this->GetIndexInTrafficArray(checi);
    if(logIndex > -1)
    {
        TrafficLog* pTrafficLog = (TrafficLog *)this->m_ArrayTrafficLog[logIndex];
        //出发、通过(报给下一个站)
        if(0x11 == type || 0x33 == type || PLAN_CMD_CFBD == type)
        {
            pTrafficLog->m_timFromAdjtStaDepaTrain = dateTime;//邻站出发时间
            SetTrafficLogProc(pTrafficLog);
            m_pDataAccess->UpdateTrafficLog(pTrafficLog);
            //发送同步消息
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            //语音播报
            this->SendSpeachText("请注意，邻站发车");
        }
        //到达、通过(报给上一个站)
        if(0x22 == type || 0x33 == type || PLAN_CMD_DDBD == type)
        {
            pTrafficLog->m_timtoAdjtStation = dateTime;//到达邻站时间
            m_pDataAccess->UpdateTrafficLog(pTrafficLog);
            //发送同步消息
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
        }
        //预告（发预）
        if(PLAN_CMD_FCYG == type)
        {
            ;//无数据变化，仅单元格显红
            //语音播报
            this->SendSpeachText("请注意，邻站要排预告");
        }
        //同意（接预）
        else if(PLAN_CMD_TYYG == type)
        {
            pTrafficLog->m_timToAdjtStaAgrDepaTrain = dateTime;//邻站同意发车时间
            SetTrafficLogProc(pTrafficLog);
            m_pDataAccess->UpdateTrafficLog(pTrafficLog);
            //发送同步消息
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            //语音播报
            this->SendSpeachText("请注意，邻站同意发车");
        }
        //取消闭塞(报给下一个站)
        else if(0x08 == type)
        {
            pTrafficLog->m_timAgrFromAdjtStaDepaTrain = tmDefault;//同意邻站发车时间
            SetTrafficLogProc(pTrafficLog);
            m_pDataAccess->UpdateTrafficLog(pTrafficLog);
            //发送同步消息
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            //语音播报
            this->SendSpeachText("请注意，邻站取消闭塞");
        }
        //取消发预(报给下一个站)
        else if(0x0A == type)
        {
            pTrafficLog->m_timAgrFromAdjtStaDepaTrain = tmDefault;//同意邻站发车时间
            SetTrafficLogProc(pTrafficLog);
            m_pDataAccess->UpdateTrafficLog(pTrafficLog);
            //发送同步消息
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
        }
        //取消接预(报给上一个站)
        else if(0x0B == type)
        {
            pTrafficLog->m_timToAdjtStaAgrDepaTrain = tmDefault;//邻站同意发车时间
            SetTrafficLogProc(pTrafficLog);
            m_pDataAccess->UpdateTrafficLog(pTrafficLog);
            //发送同步消息
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
        }

        //打包预告数据发给本站
        {
            int nCount = 9;
            QByteArray dataArray;
            dataArray.append(30, char(0));
            //主功能码-行车日志计划
            dataArray[nCount] = FUNCTYPE_TRAFFIC;
            nCount++;
            //行车日志报点操作
            dataArray[nCount] = 0x01;
            nCount++;
            //行车日志ID（数据库中自增）
            memcpy(dataArray.data()+nCount,&pTrafficLog->log_id,2);
            nCount+=2;
            //报点类型
            dataArray[nCount] = type;
            nCount++;
            //报点时间
            int Year1 = dateTime.date().year();
            int Month1 = dateTime.date().month();
            int Day1 = dateTime.date().day();
            int Hour1 = dateTime.time().hour();
            int Minu1 = dateTime.time().minute();
            int Secd1 = dateTime.time().second();
            memcpy(dataArray.data()+(nCount), &Year1, 2);
            memcpy(dataArray.data()+(nCount+2), &Month1, 1);
            memcpy(dataArray.data()+(nCount+3), &Day1, 1);
            memcpy(dataArray.data()+(nCount+4), &Hour1, 1);
            memcpy(dataArray.data()+(nCount+5), &Minu1, 1);
            memcpy(dataArray.data()+(nCount+6), &Secd1, 1);
            //制作数据帧头帧尾和关键信息
            packHeadAndTail(&dataArray, dataArray.size());
            nCount = dataArray.size();
            //信号-发送数据给CTC
            emit sendDataToCTCSignal(this, dataArray, nCount);
            //调试
            qDebug()<<"预告数据已发送给邻站CTC"<<this->getStationName();
        }
    }
}

//重置进路的操作权限(type-1CTC,2占线板)
void MyStation::ResetRoutePermit(int type)
{
    //权限在CTC，则释放CTC的权限
    if(1==type && this->RoutePermit==1)
    {
        this->RoutePermit = 0;
        m_pDataAccess->UpdateStationInfo(this);
    }
    //权限在占线板，则释放占线板的权限
    else if(2==type && this->RoutePermit==2)
    {
        this->RoutePermit = 0;
        m_pDataAccess->UpdateStationInfo(this);
    }
}

//生成新的进路序列
void MyStation::MakeNewRouteOrder(int type,int nIndexRoute,QStringList devNameList,QString inputCheci)
{
    if(nIndexRoute<=-1 || nIndexRoute>=this->vectInterlockRoute.size())
    {
        return;
    }
    if(devNameList.length()==0)
    {
        return;
    }
    int devIndex = this->GetIndexByStrName(devNameList[0]);
    if(devIndex == -1)
    {
        return;
    }
    CXHD *pXHDFirst = (CXHD*)this->DevArray[devIndex];
    CXHD *pXHDEnd = nullptr;
    //匹配后续的第一个列车信号机
    for(int i=1;i<devNameList.size();i++)
    {
        devIndex = this->GetIndexByStrName(devNameList[i]);
        if(devIndex == -1)
        {
            return;
        }
        CXHD *pXHDTemp = (CXHD*)this->DevArray[devIndex];
        //非虚信号 且 是列车信号机
        if(!pXHDTemp->getSignalType()
           && (pXHDTemp->getXHDType()==JZ_XHJ
               || pXHDTemp->getXHDType()==CZ_XHJ
               || pXHDTemp->getXHDType()==FCJL_XHJ
               || pXHDTemp->getXHDType()==SXCZ_XHJ)
         )
        {
            pXHDEnd = pXHDTemp;
            break;
        }
    }
    if(pXHDEnd == nullptr)
    {
        return;
    }
    //时间
    QDateTime timeNow = QDateTime::currentDateTime();

    //列车进路
    if(0x01==type)
    {
        TrainRouteOrder* pTrainRouteOrder = new TrainRouteOrder;
        pTrainRouteOrder->m_bCreateByMan = true;
        pTrainRouteOrder->station_id = this->getStationID();
        pTrainRouteOrder->m_nPlanNumber = 0;
        pTrainRouteOrder->bXianLuSuo = false;
        pTrainRouteOrder->m_btRecvOrDepart = (pXHDFirst->getXHDType()==JZ_XHJ)?ROUTE_JC:ROUTE_FC;
        pTrainRouteOrder->m_btBeginOrEndFlg = 0x00;//置0
        pTrainRouteOrder->m_strTrainNum = inputCheci;
        //接车进路
        if(pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC)
        {
            pTrainRouteOrder->m_strTrack = GetGDNameInLSB(nIndexRoute);
            pTrainRouteOrder->m_nTrackCode = GetCodeByStrName(pTrainRouteOrder->m_strTrack);
            pTrainRouteOrder->m_nGDPos = GetGDPosInzcArray(pTrainRouteOrder->m_nTrackCode);
            pTrainRouteOrder->m_nCodeReachStaEquip = pXHDFirst->getCode();
            pTrainRouteOrder->m_strXHD_JZk = pXHDFirst->getName();
            //获取进路方向
            pTrainRouteOrder->m_strDirection = GetDirectByCode(pTrainRouteOrder->m_nCodeReachStaEquip,0);
            //获取进路描述
            //int nCode = GetCodeByRecvEquipGD(pTrainRouteOrder->m_nCodeReachStaEquip,pTrainRouteOrder->m_nTrackCode);
            pTrainRouteOrder->m_strXHDBegin = pXHDFirst->getName();
            pTrainRouteOrder->m_strXHDEnd   = pXHDEnd->getName();//GetStrNameByCode(nCode);
        }
        //发车进路
        else
        {
            pTrainRouteOrder->m_strTrack = GetGDNameInGDNodeList(pXHDFirst->getName());
            pTrainRouteOrder->m_nTrackCode = GetCodeByStrName(pTrainRouteOrder->m_strTrack);
            pTrainRouteOrder->m_nGDPos = GetGDPosInzcArray(pTrainRouteOrder->m_nTrackCode);
            pTrainRouteOrder->m_nCodeDepartStaEquip = pXHDEnd->getCode();
            pTrainRouteOrder->m_strXHD_CZk = pXHDEnd->getName();
            //获取进路方向
            pTrainRouteOrder->m_strDirection = GetDirectByCode(pTrainRouteOrder->m_nCodeDepartStaEquip,1);
            //获取进路描述
            pTrainRouteOrder->m_strXHDBegin = pXHDFirst->getName();
            pTrainRouteOrder->m_strXHDEnd   = pXHDEnd->getName();
        }
        pTrainRouteOrder->m_timPlanned = timeNow;
        pTrainRouteOrder->m_timBegin = timeNow;
        pTrainRouteOrder->m_bElectric = true;//默认true
        pTrainRouteOrder->m_nLevelCX = 0;//默认0
        pTrainRouteOrder->m_nLHFlg = LCTYPE_KC;//默认客车
        pTrainRouteOrder->m_strRouteDescrip = devNameList.join("-"); //pTrainRouteOrder->m_strXHDBegin + "-" + pTrainRouteOrder->m_strXHDEnd;
        pTrainRouteOrder->m_strRouteDescripReal =  devNameList.join(",");//pTrainRouteOrder->m_strXHDBegin + "," + pTrainRouteOrder->m_strXHDEnd;

        //设置办理状态
        pTrainRouteOrder->SetState(1);
        //插入数据库
        m_pDataAccess->InsetRouteOrder(pTrainRouteOrder);
        //重新读取数据
        m_pDataAccess->SelectAllRouteOrder(this);
        //处理和发送1个数据
        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pTrainRouteOrder,SYNC_FLAG_ADD,1,1);
    }
    //通过进路
    else if(0x03==type)
    {
        //接车进路
        TrainRouteOrder* pRecvTrainRouteOrder = new TrainRouteOrder;
        {
            pRecvTrainRouteOrder->m_bCreateByMan = true;
            pRecvTrainRouteOrder->station_id = this->getStationID();
            pRecvTrainRouteOrder->m_nPlanNumber = 0;
            pRecvTrainRouteOrder->bXianLuSuo = false;
            pRecvTrainRouteOrder->m_btRecvOrDepart = ROUTE_JC;
            pRecvTrainRouteOrder->m_btBeginOrEndFlg = 0x00;//置0
            pRecvTrainRouteOrder->m_strTrainNum = inputCheci;

            //接车进路
            pRecvTrainRouteOrder->m_strTrack = GetGDNameInLSB(nIndexRoute);
            pRecvTrainRouteOrder->m_nTrackCode = GetCodeByStrName(pRecvTrainRouteOrder->m_strTrack);
            pRecvTrainRouteOrder->m_nGDPos = GetGDPosInzcArray(pRecvTrainRouteOrder->m_nTrackCode);
            pRecvTrainRouteOrder->m_nCodeReachStaEquip = pXHDFirst->getCode();
            pRecvTrainRouteOrder->m_strXHD_JZk = pXHDFirst->getName();
            //获取进路方向
            pRecvTrainRouteOrder->m_strDirection = GetDirectByCode(pRecvTrainRouteOrder->m_nCodeReachStaEquip,0);
            //获取进路描述
            int nCode = GetCodeByRecvEquipGD(pRecvTrainRouteOrder->m_nCodeReachStaEquip,
                pRecvTrainRouteOrder->m_nTrackCode,0);
            pRecvTrainRouteOrder->m_strXHDBegin = pXHDFirst->getName();
            pRecvTrainRouteOrder->m_strXHDEnd   = GetStrNameByCode(nCode);

            pRecvTrainRouteOrder->m_timPlanned = timeNow;
            pRecvTrainRouteOrder->m_timBegin = timeNow;
            pRecvTrainRouteOrder->m_bElectric = true;//默认true
            pRecvTrainRouteOrder->m_nLevelCX = 0;//默认0
            pRecvTrainRouteOrder->m_nLHFlg = LCTYPE_KC;//默认客车
            pRecvTrainRouteOrder->m_strRouteDescrip = pRecvTrainRouteOrder->m_strXHDBegin + "-" + pRecvTrainRouteOrder->m_strXHDEnd;
            pRecvTrainRouteOrder->m_strRouteDescripReal =  pRecvTrainRouteOrder->m_strXHDBegin + "," + pRecvTrainRouteOrder->m_strXHDEnd;

            //设置办理状态
            pRecvTrainRouteOrder->SetState(1);
            //插入数据库
            m_pDataAccess->InsetRouteOrder(pRecvTrainRouteOrder);
        }

        //发车进路
        TrainRouteOrder* pDepartTrainRouteOrder = new TrainRouteOrder;
        {
            pDepartTrainRouteOrder->m_bCreateByMan = true;
            pDepartTrainRouteOrder->station_id = this->getStationID();
            pDepartTrainRouteOrder->m_nPlanNumber = 0;
            pDepartTrainRouteOrder->bXianLuSuo = false;
            pDepartTrainRouteOrder->m_btRecvOrDepart = ROUTE_FC;
            pDepartTrainRouteOrder->m_btBeginOrEndFlg = 0x00;//置0
            pDepartTrainRouteOrder->m_strTrainNum = inputCheci;

            //发车进路
            pDepartTrainRouteOrder->m_strTrack = GetGDNameInLSB(nIndexRoute);//GetGDNameInGDNodeList(pXHDFirst->getName());
            pDepartTrainRouteOrder->m_nTrackCode = GetCodeByStrName(pDepartTrainRouteOrder->m_strTrack);
            pDepartTrainRouteOrder->m_nGDPos = GetGDPosInzcArray(pDepartTrainRouteOrder->m_nTrackCode);
            pDepartTrainRouteOrder->m_nCodeDepartStaEquip = pXHDEnd->getCode();
            pDepartTrainRouteOrder->m_strXHD_CZk = pXHDEnd->getName();
            //获取进路方向
            pDepartTrainRouteOrder->m_strDirection = GetDirectByCode(pDepartTrainRouteOrder->m_nCodeDepartStaEquip,1);
            int nCode = GetCodeByRecvEquipGD(pDepartTrainRouteOrder->m_nCodeDepartStaEquip,
                pDepartTrainRouteOrder->m_nTrackCode,1);
            //获取进路描述
            pDepartTrainRouteOrder->m_strXHDBegin = GetStrNameByCode(nCode);//pXHDFirst->getName();
            pDepartTrainRouteOrder->m_strXHDEnd   = pXHDEnd->getName();

            pDepartTrainRouteOrder->m_timPlanned = timeNow;
            pDepartTrainRouteOrder->m_timBegin = timeNow;
            pDepartTrainRouteOrder->m_bElectric = true;//默认true
            pDepartTrainRouteOrder->m_nLevelCX = 0;//默认0
            pDepartTrainRouteOrder->m_nLHFlg = LCTYPE_KC;//默认客车
            pDepartTrainRouteOrder->m_strRouteDescrip = pDepartTrainRouteOrder->m_strXHDBegin + "-" + pDepartTrainRouteOrder->m_strXHDEnd;
            pDepartTrainRouteOrder->m_strRouteDescripReal =  pDepartTrainRouteOrder->m_strXHDBegin + "," + pDepartTrainRouteOrder->m_strXHDEnd;

            //设置办理状态
            pDepartTrainRouteOrder->SetState(1);
            //插入数据库
            m_pDataAccess->InsetRouteOrder(pDepartTrainRouteOrder);
        }

        //重新读取数据
        m_pDataAccess->SelectAllRouteOrder(this);
        //处理和发送1个数据
        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pRecvTrainRouteOrder,SYNC_FLAG_ADD,1,1);
        //处理和发送1个数据
        this->sendOneTrainRouteOrderToSoft(DATATYPE_ALL,pDepartTrainRouteOrder,SYNC_FLAG_ADD,1,1);
    }
}

//根据进路序列索引找到该条联锁表中的股道名称
QString MyStation::GetGDNameInLSB(int nIndexRoute)
{
    if(nIndexRoute<=-1 || nIndexRoute>=this->vectInterlockRoute.size())
    {
        return "";
    }
    InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[nIndexRoute]);
    QStringList pQDArray;
    pQDArray.append(pRut->QDArr);
    //遍历区段
    for(int i = 0; i < pQDArray.size(); i++)
    {
        QString qdname = (QString)pQDArray[i];
        int nArraySize =  this->DevArray.size();//获取站场大小
        CGD *pGD;
        if(qdname.right(2) == "DG")//道岔区段
        {
            continue;
        }
        else//无岔区段、股道
        {
            for(int z=0; z<nArraySize; z++)
            {
                if(DevArray[z]->getDevType() == Dev_GD)
                {
                    pGD=(CGD *)(DevArray[z]);
                    if(qdname == pGD->getName())
                    {
                        //股道区段
                        if( pGD->getGDType() == GD_QD)
                        {
                            return pGD->getName();
                        }
                    }
                }
            }
        }
    }
    return "";
}
//根据信号机名称在股道信号机列表中找到该信号机关联的股道名称
QString MyStation::GetGDNameInGDNodeList(QString xhdName)
{
    for(int i=0; i<vectStationGDNode.size(); i++)
    {
        if(xhdName == vectStationGDNode[i].strLeftNode
          || xhdName == vectStationGDNode[i].strRightNode)
        {
            return vectStationGDNode[i].strGDName;
        }
    }
    return "";
}

//根据命令号查找临时限速命令
LimitSpeed *MyStation::FindLimitSpeedByNum(QString cmdNum)
{
    for(int i=0; i<this->vectLimitSpeed.size(); i++)
    {
        LimitSpeed* limitSpeed = this->vectLimitSpeed[i];
        if(limitSpeed->strCmdNum == cmdNum)
        {
            return limitSpeed;
        }
    }
    return nullptr;
}

////整理组合进路//初始化读取数据库时调用
//void MyStation::ArrangeCombinedRouteOrder()
//{
//    if(this->m_ArrayRouteOrder.size()<=0)
//    {
//        return;
//    }
//    //遍历子进路
//    for(int i=0; i<this->m_ArrayRouteOrderSon.size(); i++)
//    {
//        TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrderSon[i];
//        if(pRouteOrderSon->father_id>0)
//        {
//            TrainRouteOrder* pRouteOrderFather = FindTrainRouteOrderById(pRouteOrderSon->father_id);
//            pRouteOrderFather->vectRouteOrder.append(pRouteOrderSon);
//        }
//    }
//    //this->m_ArrayRouteOrderSon.clear();
//}
//根据id查找进路序列
TrainRouteOrder *MyStation::FindTrainRouteOrderById(int id)
{
    //遍历进路
    for(int i=0; i<this->m_ArrayRouteOrder.size(); i++)
    {
        TrainRouteOrder* pRouteOrder = this->m_ArrayRouteOrder[i];
        if(id == pRouteOrder->route_id)
        {
            return pRouteOrder;
        }
    }
    return nullptr;
}
//根据父id查找子进路序列（即组合进路的子序列）
TrainRouteOrder *MyStation::FindSonRouteOrderByFatherId(int fatherId)
{
    //遍历进路
    for(int i=0; i<this->m_ArrayRouteOrder.size(); i++)
    {
        TrainRouteOrder* pRouteOrder = this->m_ArrayRouteOrder[i];
        if(fatherId == pRouteOrder->father_id)
        {
            return pRouteOrder;
        }
    }
    return nullptr;
}

//组合进路的子序列是否拥有此状态（state-2触发完成/3占用/1自触/0非自触）
bool MyStation::IsSonRouteOrderHaveState(int fatherId, int state)
{
    //遍历进路
    for(int i=0; i<this->m_ArrayRouteOrder.size(); i++)
    {
        TrainRouteOrder* pRouteOrderSon = this->m_ArrayRouteOrder[i];
        if(fatherId == pRouteOrderSon->father_id)
        {
            if(3==state)
            {
                if(pRouteOrderSon->m_nOldRouteState==3)
                {
                    return true;
                }
            }
            else if(2==state)
            {
                if(pRouteOrderSon->m_nOldRouteState==2)
                {
                    return true;
                }
            }
            else if(1==state)
            {
                if(pRouteOrderSon->m_nAutoTouch)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
//在独立的信号机按钮数组中查找按钮名称(设备号,类型0列按1调按2通按)
QString MyStation::GetBtnNameInAloneXHD(int code, int type)
{
    //独立的列车调车按钮
    int anCount = vectXhBtn.size();
    for (int i = 0; i<anCount; i++)
    {
        XhBtn *pXHAN = (XhBtn*)vectXhBtn[i];
        if (code == pXHAN->m_nCode)
        {
            if(1==type && pXHAN->m_nANTYPE==DCAN)
            {
                return pXHAN->m_strName;
            }
            else if(0==type && pXHAN->m_nANTYPE==LCAN)
            {
                return pXHAN->m_strName;
            }
        }
    }
    return "";
}

//是否有进路状态为正在办理
bool MyStation::HaveRouteIsDoing()
{
    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        //进路正在办理
        if(pTrainRouteOrder->m_nOldRouteState == 1)
        {
            return true;
        }
    }
    return false;
}

//针对发车进路而言，检查并设置其相应的接车进路是否是延续进路，用于接车延续发车进路的办理判定条件
bool MyStation::CheckJCRouteIsYXJL(TrainRouteOrder* _pTrainRouteOrderFC)
{
    //进路是接车进路的不判断
    if(_pTrainRouteOrderFC->m_btRecvOrDepart==ROUTE_JC)
    {
        return false;
    }

    for(int i = 0; i < this->m_ArrayRouteOrder.count(); i++)
    {
        TrainRouteOrder* pTrainRouteOrder = (TrainRouteOrder*)this->m_ArrayRouteOrder[i];
        if(pTrainRouteOrder->station_id == _pTrainRouteOrderFC->station_id
           && pTrainRouteOrder->m_strTrainNum == _pTrainRouteOrderFC->m_strTrainNum
           && pTrainRouteOrder->m_btRecvOrDepart == ROUTE_JC
         )
        {
            //关联的接车进路是延续进路（针对发车进路而言）
            _pTrainRouteOrderFC->m_bYXJLOfJC = pTrainRouteOrder->m_bYXJL;
            return true;
        }
    }

    return false;
}

//检查进路的车次和接近区段上的第一趟车次是否一致,不一致时返回false和不一致车次
bool MyStation::CheckJCRouteSameCheciInJJQD(TrainRouteOrder *_pTrainRouteOrder, QString &strUnSameCheci)
{
    if(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_FC)
    {
        _pTrainRouteOrder->checkResultFirstCheCi = 0;
        return true;
    }

    for(int i=0; i<this->StaConfigInfo.JCKCount; i++)
    {
        QString jzxhdName = this->StaConfigInfo.JFCKInfo[i].strJCKName;
        //匹配进站口
        if(_pTrainRouteOrder->m_strXHDBegin == jzxhdName)
        {
            for(int j=0; j<this->StaConfigInfo.JFCKInfo[i].strArrJckJJQD.size(); j++)
            {
                int ngdIndex = -1;
                ngdIndex = this->GetIndexByStrName(this->StaConfigInfo.JFCKInfo[i].strArrJckJJQD[j]);//strJCKjucJJQD
                if(ngdIndex > 0)
                {
                    CGD *pGD = (CGD *)this->DevArray[ngdIndex];
                    //接近区段占用且有车次-闭塞分区由近及远的第一个车次
                    if(pGD->getState(QDZY) && (pGD->m_strCheCiNum.length() > 0))
                    {
                        QString strCheci = pGD->m_strCheCiNum;
                        if(_pTrainRouteOrder->m_strTrainNum == strCheci)
                        {
                            _pTrainRouteOrder->checkResultFirstCheCi = 0;
                            return true;
                        }
                        else
                        {
                            //车次不一致
                            strUnSameCheci = strCheci;
                            _pTrainRouteOrder->checkResultFirstCheCi = JLWARNING_SEQU_CCCT;
                            return false;
                        }
                    }
                }
            }
        }
    }
    _pTrainRouteOrder->checkResultFirstCheCi = 0;
    return true;
}
//检查进路的车次和股道上的实际车次是否一致,不一致时返回false和不一致车次
bool MyStation::CheckFCRouteSameCheciInGD(TrainRouteOrder *_pTrainRouteOrder, QString &strUnSameCheci)
{
    if(_pTrainRouteOrder->m_btRecvOrDepart==ROUTE_JC)
    {
        _pTrainRouteOrder->checkResultFirstCheCi = 0;
        return true;
    }

    for(int i=0; i<m_ArrayTrain.size(); i++)
    {
        Train *pTrain = m_ArrayTrain[i];
        if(pTrain->m_nPosCodeReal==_pTrainRouteOrder->m_nTrackCode
             || pTrain->m_nPosCode==_pTrainRouteOrder->m_nTrackCode)
        {
            if(pTrain->m_strCheCi==_pTrainRouteOrder->m_strTrainNum)
            {
                return true;
            }
            else
            {
                //车次不一致
                strUnSameCheci = pTrain->m_strCheCi;
                _pTrainRouteOrder->checkResultFirstCheCi = JLWARNING_SEQU_CCCT;
                return false;
            }
        }
    }
    for(int i=0; i<m_ArrayTrainManType.size(); i++)
    {
        Train *pTrain = m_ArrayTrainManType[i];
        if(pTrain->m_nPosCode==_pTrainRouteOrder->m_nTrackCode)
        {
            if(pTrain->m_strCheCi==_pTrainRouteOrder->m_strTrainNum)
            {
                return true;
            }
            else
            {
                //车次不一致
                strUnSameCheci = pTrain->m_strCheCi;
                _pTrainRouteOrder->checkResultFirstCheCi = JLWARNING_SEQU_CCCT;
                return false;
            }
        }

    }
    _pTrainRouteOrder->checkResultFirstCheCi = 0;
    return true;
}
//判断通过进路是否为正线通过
bool MyStation::CheckZXTGJL(TrainRouteOrder *pTrainRouteOrder)
{
    if(pTrainRouteOrder->m_btBeginOrEndFlg==JFC_TYPE_TG)//&&(pTrainRouteOrder->m_nManTouch)
    {
        for(int ii=0;ii<vectTGJL.size();ii++)
        {
            if((pTrainRouteOrder->m_strTrack==vectTGJL[ii].ZXTGGDName)
               &&((pTrainRouteOrder->m_strXHDBegin==vectTGJL[ii].SDXHName)||(pTrainRouteOrder->m_strXHDEnd==vectTGJL[ii].ZDXHName)))
            {
                return true;
            }
        }
        return false;
    }
    return true;
}
//检查进路关联的计划信息是否已经预告
bool MyStation::CheckTrafficLogIsNoticed(TrainRouteOrder *pTrainRouteOrder)
{
    //关联行车日志
    TrafficLog* pTrafficLog = GetTrafficLogByCheCi(pTrainRouteOrder->m_strTrainNum);
    //非终到计划
    if(pTrafficLog->m_btBeginOrEndFlg!=JFC_TYPE_ZD)
    {
        if(pTrafficLog->m_timToAdjtStaAgrDepaTrain.date().year()>1970)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}

//响应CTC点击的“发送计划”
void MyStation::SendPlan()
{
    //互斥锁，作用域结束自动解锁
    QMutexLocker locker(&Mutex);

    if(ManSignRouteSynType == 0)
        return;

    int count = this->m_ArrayStagePlan.count();
    StagePlan *pStagePlan;
    for(int i = 0; i < count; i++)
    {
        pStagePlan = this->m_ArrayStagePlan[i];
        //已签收到行车日志，暂未发送到进路序列
        if(pStagePlan->m_nStateSignPlan == 1)
        {
            //按图排路，才会生成进路序列
            if(this->ModalSelect.nStateSelect == 1)
            {
                //通知阶段计划表签收状态已发生改变，列车进路表开始更新
                this->StagePlanToRouteOrder(pStagePlan);
                //更新阶段计划数组类签收状态
                pStagePlan->m_nStateSignPlan = 2;
            }
            //数据库访问
            m_pDataAccess->UpdateStagePlanDetail(pStagePlan);
            //处理和发送1个数据
            this->sendOneStagePlanToSoft(DATATYPE_ALL,pStagePlan,SYNC_FLAG_UPDATE,1,1);
        }
    }

    //AutoRecvModifiedPlan();
    for (int i = 0; i < this->m_ArrayStagePlanChg.count(); i++)
    {
        StagePlan *pStagePlanChg = (StagePlan *)this->m_ArrayStagePlanChg[i];
        //更新计划进路信息
        ModifyTrainRouteOrder(pStagePlanChg);
        //更新计划日志信息
        ModifyTrafficLog(pStagePlanChg);
    }
    //清空
    this->m_ArrayStagePlanChg.clear();

    for (int i = 0; i < this->m_ArrayTrafficLogChg.count(); i++)
    {
        qDebug() << "count" << this->m_ArrayTrafficLogChg.count() << this->m_ArrayTrafficLogChg.size();
        TrafficLog *pTrafficLog = (TrafficLog *)this->m_ArrayTrafficLogChg[i];
        //删除
        if(pTrafficLog->m_bDelete)
        {
            m_pDataAccess->DeleteTrafficLog(pTrafficLog);
            //发送同步消息
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_DELETE,1,1);
            //删除相应的进路序列
            DeleteRouteOrderByTrafficLog(pTrafficLog);
            this->m_ArrayTrafficLog.removeOne(pTrafficLog);
        }
        //更新
        else
        {
            m_pDataAccess->UpdateTrafficLog(pTrafficLog);
            //发送同步消息
            this->sendOneTrafficLogToSoft(DATATYPE_ALL,pTrafficLog,SYNC_FLAG_UPDATE,1,1);
            //qDebug() << "send sync11111 " << pTrafficLog->m_nIndexLCLX << pTrafficLog->m_nIndexYXLX;
        }
    }
    //清空
    this->m_ArrayTrafficLogChg.clear();
}

//获取虚信号机按钮命令类型，返回值0是列车按钮，1是调车按钮
int MyStation::GetXXHDBtnCmdType(QString _xxhdName)
{
    for(int i=0; i<mXXHDBtnCmdType.XXHDArray.size(); i++)
    {
        if(_xxhdName == mXXHDBtnCmdType.XXHDArray[i])
        {
            return mXXHDBtnCmdType.BtnCmdType;//返回类型
        }
    }
    return 1;//默认虚信号发送调车按钮
}


//检查进路序列中需要占用的区段和当前排列的调车进路是否存在冲突区段(即是否有相同区段)如果有返回true,如果没有返回false
bool MyStation::CheckDCRouteSameQDwithJLXL(QString SDAN, QString ZDAN,int takenMinutes,QString &m_TrainNum)
{
    int size = this->m_ArrayRouteOrder.size();
    int routeCount = vectInterlockRoute.size();
    for (int j = 0; j < routeCount; j++)
    {
        InterlockRoute *pRut = (InterlockRoute *)(vectInterlockRoute[j]);
        QStringList pQDArray;
        QStringList pBtnArray;
        pBtnArray.append(pRut->BtnArr);
        int b = pBtnArray.count();
        if((SDAN==pBtnArray[0])&&(ZDAN==pBtnArray[b-1]))//根据点击界面选择的始端信号和终端信号判断该调车进路所包含的区段有哪些
        {
            pQDArray.append(pRut->QDArr);
            int c = pQDArray.count();
            for(int i = 0; i < c; i++)
            {
                QString qdname = pQDArray[i];
                for(int p=0;p<size;p++)//通过进路序列选择所有进路序列的始端信号和终端信号
                {
                    TrainRouteOrder* pTrainRouteOrder = m_ArrayRouteOrder[p];
                    for(int q=0;q<routeCount;q++)//进路序列始端信号和终端信号判断出的进路的区段有哪些
                    {
                        InterlockRoute *pRut_route = (InterlockRoute *)(vectInterlockRoute[q]);
                        QStringList pQDArray_route;
                        QStringList pBtnArray_route;
                        pBtnArray_route.append(pRut_route->BtnArr);
                        int d = pBtnArray_route.count();
                        for(int r=0;r<d;r++)
                        {
                            if(pBtnArray_route[r].right(2)=="DA")
                            {
                                pBtnArray_route[r].replace("DA","");
                            }
                            else if(pBtnArray_route[r].right(2)=="LA")
                            {
                                pBtnArray_route[r].replace("LA","");
                            }
                            else if(pBtnArray_route[r].right(2)=="TA")
                            {
                                pBtnArray_route[r].replace("TA","");
                            }
                            else if(pBtnArray_route[r].right(1)=="A")
                            {
                                pBtnArray_route[r].replace("A","");
                            }
                        }
                        if((pTrainRouteOrder->m_strXHDBegin==pBtnArray_route[0])&&(pTrainRouteOrder->m_strXHDEnd==pBtnArray_route[d-1]))
                        {
                            //if(CheckJJQDZY(pTrainRouteOrder->m_strXHDBegin))//需要判断接近区段有车占用时,才使用该条件
                            {
                                pQDArray_route.append(pRut_route->QDArr);
                                int e = pQDArray_route.count();
                                for(int k=0;k<e;k++)//比较该两种区段的所有值,只要出现相同则返回false
                                {
                                    QString qdname_route = pQDArray_route[k];
                                    if(qdname==qdname_route)
                                    {
                                        if(CheckDCRouteTakenTime(pTrainRouteOrder,takenMinutes))
                                        {
                                            m_TrainNum = pTrainRouteOrder->m_strTrainNum;
                                            return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

//检查调车进路输入时间后,判断时间范围内
bool MyStation::CheckDCRouteTakenTime(TrainRouteOrder *pTrainRouteOrder,int takenMinutes)
{
    QDateTime currentTime;
    int inputTime_secs=takenMinutes*60;
    qint64 currentTime_Epoch = QDateTime::currentSecsSinceEpoch();

    int setTime_secs=10*60;
    qint64 JLXLplanTime_Epoch = pTrainRouteOrder->m_timPlanned.toSecsSinceEpoch();

    //当前时间+输入时间的值,大于计划时间-设置冲突时间的值;或者当前时间小于计划时间;此时为冲突时间
    if(((currentTime_Epoch+inputTime_secs)>(JLXLplanTime_Epoch-setTime_secs))&&(currentTime_Epoch<JLXLplanTime_Epoch))
    {
//        CheckResult* ckResult = new CheckResult;
//        QDateTime timeNow = QDateTime::currentDateTime();
//        ckResult->id = timeNow.toMSecsSinceEpoch()&0xFFFFFFFF;//取后4个字节
//        ckResult->route_id = pTrainRouteOrder->route_id;
//        ckResult->bEnforced = true;
//        ckResult->checkMsg = QString("命令与计划冲突\r\n与%1次的计划冲突")
//            .arg(pTrainRouteOrder->m_strTrainNum);
//        //人工办理时才发送强制执行窗口
//        //防错办报警信息
//        this->SendRouteCheckResult(ckResult, -1);//所有终端
//        UpdateCheckResultArray(ckResult);//交互，则增加

//        pTrainRouteOrder->m_nManTouch = false;
//        pTrainRouteOrder->m_bRunNow = false;
        return true;
    }
    return false;
}

//判断接近区段是否被占用
bool MyStation::CheckJJQDZY(QString strName)
{
#if 1
    int nArraySize =  this->DevArray.size();//获取站场大小
    int ct = StaConfigInfo.JCKCount;
    CGD *pGD;

    for (int t = 0; t < ct; t++)
    {
        //接车口关联的接近区段获取
        if (strName == StaConfigInfo.JFCKInfo[t].strJCKName)
        {
            int jckXHDindex = GetIndexByStrName(StaConfigInfo.JFCKInfo[t].strJCKName);
            if(jckXHDindex < 0)
                break;
            CXHD *pxhd=(CXHD *)(DevArray[jckXHDindex]);
            //QString str1 = GetXHDjucJJQDName(pxhd);//第1个接近
            QString str1 = GetXHDjucJJQDName(pxhd->p1, pxhd->p2);//第1个接近
            StaConfigInfo.JFCKInfo[t].strJCKjucJJQD = str1;
            if(str1 != "")
            {
                StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(str1);
                QString strNext2 = FindNextJJQD(t, str1);//第2个接近
                for (int z = 0; z< nArraySize; z++)
                {
                    if (DevArray[z]->getDevType() == Dev_GD)
                    {
                        pGD = (CGD *)(DevArray[z]);
                        if (str1 == pGD->m_strName)
                        {
                            if (!(pGD->getState(QDKX)))
                            {
                                return true;
                            }
                        }
                    }
                }
                if(strNext2 != "")
                {
                    StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext2);
                    QString strNext3 = FindNextJJQD(t, strNext2);//第3个接近
                    for (int z = 0; z< nArraySize; z++)
                    {
                        if (DevArray[z]->getDevType() == Dev_GD)
                        {
                            pGD = (CGD *)(DevArray[z]);
                            if (strNext2 == pGD->m_strName)
                            {
                                if (!(pGD->getState(QDKX)))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    if(strNext3 != "")
                    {
                        StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext3);
                        QString strNext4 = FindNextJJQD(t, strNext3);//第4个接近
                        for (int z = 0; z< nArraySize; z++)
                        {
                            if (DevArray[z]->getDevType() == Dev_GD)
                            {
                                pGD = (CGD *)(DevArray[z]);
                                if (strNext3 == pGD->m_strName)
                                {
                                    if (!(pGD->getState(QDKX)))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        if(strNext4 != "")
                        {
                            StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext4);
                            QString strNext5 = FindNextJJQD(t, strNext4);//第5个接近
                            for (int z = 0; z< nArraySize; z++)
                            {
                                if (DevArray[z]->getDevType() == Dev_GD)
                                {
                                    pGD = (CGD *)(DevArray[z]);
                                    if (strNext4 == pGD->m_strName)
                                    {
                                        if (!(pGD->getState(QDKX)))
                                        {
                                            return true;
                                        }
                                    }
                                }
                            }
                            if(strNext5 != "")
                            {
                                StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext5);
                                QString strNext6 = FindNextJJQD(t, strNext5);//第6个接近
                                for (int z = 0; z< nArraySize; z++)
                                {
                                    if (DevArray[z]->getDevType() == Dev_GD)
                                    {
                                        pGD = (CGD *)(DevArray[z]);
                                        if (strNext5 == pGD->m_strName)
                                        {
                                            if (!(pGD->getState(QDKX)))
                                            {
                                                return true;
                                            }
                                        }
                                    }
                                }
                                if(strNext6 != "")
                                {
                                    StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext6);
                                    QString strNext7 = FindNextJJQD(t, strNext6);//第7个接近
                                    for (int z = 0; z< nArraySize; z++)
                                    {
                                        if (DevArray[z]->getDevType() == Dev_GD)
                                        {
                                            pGD = (CGD *)(DevArray[z]);
                                            if (strNext6 == pGD->m_strName)
                                            {
                                                if (!(pGD->getState(QDKX)))
                                                {
                                                    return true;
                                                }
                                            }
                                        }
                                    }
                                    if(strNext7 != "")
                                    {
                                        StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext7);
                                        QString strNext8 = FindNextJJQD(t, strNext7);//第8个接近
                                        for (int z = 0; z< nArraySize; z++)
                                        {
                                            if (DevArray[z]->getDevType() == Dev_GD)
                                            {
                                                pGD = (CGD *)(DevArray[z]);
                                                if (strNext7 == pGD->m_strName)
                                                {
                                                    if (!(pGD->getState(QDKX)))
                                                    {
                                                        return true;
                                                    }
                                                }
                                            }
                                        }
                                        if(strNext8 != "")
                                        {
                                            StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext8);
                                            QString strNext9 = FindNextJJQD(t, strNext8);//第9个接近
                                            for (int z = 0; z< nArraySize; z++)
                                            {
                                                if (DevArray[z]->getDevType() == Dev_GD)
                                                {
                                                    pGD = (CGD *)(DevArray[z]);
                                                    if (strNext8 == pGD->m_strName)
                                                    {
                                                        if (!(pGD->getState(QDKX)))
                                                        {
                                                            return true;
                                                        }
                                                    }
                                                }
                                            }
                                            if(strNext9 != "")
                                            {
                                                StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext9);
                                                QString strNext10 = FindNextJJQD(t, strNext9);//第10个接近
                                                for (int z = 0; z< nArraySize; z++)
                                                {
                                                    if (DevArray[z]->getDevType() == Dev_GD)
                                                    {
                                                        pGD = (CGD *)(DevArray[z]);
                                                        if (strNext9 == pGD->m_strName)
                                                        {
                                                            if (!(pGD->getState(QDKX)))
                                                            {
                                                                return true;
                                                            }
                                                        }
                                                    }
                                                }
                                                if(strNext10 != "")
                                                {
                                                    StaConfigInfo.JFCKInfo[t].strArrJckJJQD.append(strNext10);
                                                    for (int z = 0; z< nArraySize; z++)
                                                    {
                                                        if (DevArray[z]->getDevType() == Dev_GD)
                                                        {
                                                            pGD = (CGD *)(DevArray[z]);
                                                            if (strNext10 == pGD->m_strName)
                                                            {
                                                                if (!(pGD->getState(QDKX)))
                                                                {
                                                                    return true;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        //发车口关联的离去区段获取
        if (strName == StaConfigInfo.JFCKInfo[t].strFCKName)
        {
            int fckXHDindex = GetIndexByStrName(StaConfigInfo.JFCKInfo[t].strFCKName);
            if (fckXHDindex < 0)
                break;
            CXHD *pxhd2 = (CXHD *)(DevArray[fckXHDindex]);
            //str1 = GetXHDjucJJQDName(pxhd);//第1个离去
            QString str11 = GetXHDjucJJQDName(pxhd2->p1, pxhd2->p2);//第1个接近
            StaConfigInfo.JFCKInfo[t].strFCKjucLQQD = str11;
            if (str11 != "")
            {
                StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(str11);
                QString strNext2 = FindNextJJQD(t, str11);//第2个离去
                for (int z = 0; z< nArraySize; z++)
                {
                    if (DevArray[z]->getDevType() == Dev_GD)
                    {
                        pGD = (CGD *)(DevArray[z]);
                        if (str11 == pGD->m_strName)
                        {
                            if (!(pGD->getState(QDKX)))
                            {
                                return true;
                            }
                        }
                    }
                }
                if (strNext2 != "")
                {
                    StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext2);
                    QString strNext3 = FindNextJJQD(t, strNext2);//第3个离去
                    for (int z = 0; z< nArraySize; z++)
                    {
                        if (DevArray[z]->getDevType() == Dev_GD)
                        {
                            pGD = (CGD *)(DevArray[z]);
                            if (strNext2 == pGD->m_strName)
                            {
                                if (!(pGD->getState(QDKX)))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    if (strNext3 != "")
                    {
                        StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext3);
                        QString strNext4 = FindNextJJQD(t, strNext3);//第4个离去
                        for (int z = 0; z< nArraySize; z++)
                        {
                            if (DevArray[z]->getDevType() == Dev_GD)
                            {
                                pGD = (CGD *)(DevArray[z]);
                                if (strNext3 == pGD->m_strName)
                                {
                                    if (!(pGD->getState(QDKX)))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        if (strNext4 != "")
                        {
                            StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext4);
                            QString strNext5 = FindNextJJQD(t, strNext4);//第5个离去
                            for (int z = 0; z< nArraySize; z++)
                            {
                                if (DevArray[z]->getDevType() == Dev_GD)
                                {
                                    pGD = (CGD *)(DevArray[z]);
                                    if (strNext4 == pGD->m_strName)
                                    {
                                        if (!(pGD->getState(QDKX)))
                                        {
                                            return true;
                                        }
                                    }
                                }
                            }
                            if (strNext5 != "")
                            {
                                StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext5);
                                QString strNext6 = FindNextJJQD(t, strNext5);//第6个离去
                                for (int z = 0; z< nArraySize; z++)
                                {
                                    if (DevArray[z]->getDevType() == Dev_GD)
                                    {
                                        pGD = (CGD *)(DevArray[z]);
                                        if (strNext5 == pGD->m_strName)
                                        {
                                            if (!(pGD->getState(QDKX)))
                                            {
                                                return true;
                                            }
                                        }
                                    }
                                }
                                if (strNext6 != "")
                                {
                                    StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext6);
                                    QString strNext7 = FindNextJJQD(t, strNext6);//第7个离去
                                    for (int z = 0; z< nArraySize; z++)
                                    {
                                        if (DevArray[z]->getDevType() == Dev_GD)
                                        {
                                            pGD = (CGD *)(DevArray[z]);
                                            if (strNext6 == pGD->m_strName)
                                            {
                                                if (!(pGD->getState(QDKX)))
                                                {
                                                    return true;
                                                }
                                            }
                                        }
                                    }
                                    if (strNext7 != "")
                                    {
                                        StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext7);
                                        QString strNext8 = FindNextJJQD(t, strNext7);//第8个离去
                                        for (int z = 0; z< nArraySize; z++)
                                        {
                                            if (DevArray[z]->getDevType() == Dev_GD)
                                            {
                                                pGD = (CGD *)(DevArray[z]);
                                                if (strNext7 == pGD->m_strName)
                                                {
                                                    if (!(pGD->getState(QDKX)))
                                                    {
                                                        return true;
                                                    }
                                                }
                                            }
                                        }
                                        if (strNext8 != "")
                                        {
                                            StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext8);
                                            QString strNext9 = FindNextJJQD(t, strNext8);//第9个离去
                                            for (int z = 0; z< nArraySize; z++)
                                            {
                                                if (DevArray[z]->getDevType() == Dev_GD)
                                                {
                                                    pGD = (CGD *)(DevArray[z]);
                                                    if (strNext8 == pGD->m_strName)
                                                    {
                                                        if (!(pGD->getState(QDKX)))
                                                        {
                                                            return true;
                                                        }
                                                    }
                                                }
                                            }
                                            if (strNext9 != "")
                                            {
                                                StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext9);
                                                QString strNext10 = FindNextJJQD(t, strNext9);//第10个离去
                                                for (int z = 0; z< nArraySize; z++)
                                                {
                                                    if (DevArray[z]->getDevType() == Dev_GD)
                                                    {
                                                        pGD = (CGD *)(DevArray[z]);
                                                        if (strNext9 == pGD->m_strName)
                                                        {
                                                            if (!(pGD->getState(QDKX)))
                                                            {
                                                                return true;
                                                            }
                                                        }
                                                    }
                                                }
                                                if (strNext10 != "")
                                                {
                                                    StaConfigInfo.JFCKInfo[t].strArrFckLQQD.append(strNext10);
                                                    for (int z = 0; z< nArraySize; z++)
                                                    {
                                                        if (DevArray[z]->getDevType() == Dev_GD)
                                                        {
                                                            pGD = (CGD *)(DevArray[z]);
                                                            if (strNext10 == pGD->m_strName)
                                                            {
                                                                if (!(pGD->getState(QDKX)))
                                                                {
                                                                    return true;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif
    return false;
}





