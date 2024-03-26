#include "mydoc.h"
#include <QtDebug>
#include <QSettings>
#include <QMessageBox>
#include <windows.h>
#include <QTextCodec>
#include <QCryptographicHash> //生成密码散列
#include "GlobalHeaders/GlobalFuntion.h"


//接车进路自动触发提前时间（分钟）
int AutoTouchReachRouteLeadMinutes=3;
//发车进路自动触发提前时间（分钟）
int AutoTouchDepartRouteLeadMinutes=1;
//进路序列可设置自动触发的时间范围（分钟）
int AutoTouchMinutes=60*12;//默认12小时
//进路序列尝试自动触发的最长时间范围（分钟）
int TryAutoTouchMaxMinutes=60;//默认1小时
//人工排路时生成进路序列
bool MakeRouteOrderWhenClick = false;//默认否
//股道分路不良确认空闲自触可办理
bool GDFLBLKXAutoTouch = true;//默认是
//非常站控模式下是否设置进路自触
bool FCZKSetAutoTouch = false;//默认否
//人工签收方式，0人工签收后自动同步进路信息，1人工签收后需点击“发送计划”才同步进路信息 //默认0
int ManSignRouteSynType = 0;
//正在触发、触发成功的进路是否可取消自触（默认不可取消）
bool TouchingRouteCancelAutoFlag = 0;
//“自触”标志的进路一直“正在触发”时，每间隔30秒重新触发一次，持久时间如下（分钟）
int AutoTryMaxMinutesWhenTouchingRoute = 5;
//早点列车可以办理进路的时间范围（分钟）
int EarlyTrainsTouchRangeMinutes = 720;//60*12 默认12小时
//设置自触标记时是否需要先完成预告（终到计划除外）
bool SetAutoTouchNeedNotice = false;
//设置自触的时间，当同意邻站预告/预告后，小于0则功能不启用
int AutoTouchMinitesWhenNoticed = -1;
//设置自触的时间，当邻站模拟进出站，小于0则功能不启用
int AutoTouchMinitesWhenLZMNJCZ = -1;
//调度中心的车次是否可以修改（默认可修改）
bool DispatchCenterTrainModify = true;
//是否判定进路交叉（默认否）
bool JudgeRouteCrossed = false;


MyDoc::MyDoc(QObject *parent) : QObject(parent)
{
//    //日志初始化
//    log.initLogger();

    //变量初始化
    SysLifeSeconds = 0;
    currStaIndex = 0;
    //bUdpMode = true;
    localServerPort1 = 0;
    localServerPort2 = 0;
    teacherWatchPort = 0;
    teacherPort = 0;
    localTeacherPort = 0;
    localTrainPort = 0;
    socketUDP = nullptr;
    serverTcp = nullptr;
    databasePort = 0;
    m_pDataAccess = nullptr;
    nElapsed = 0;

    initAllData();

    qDebug()<<"init MyDoc";
}
MyDoc::~MyDoc()
{
    socketUDP->socketUDP->close();
    delete socketUDP;
    serverTcp->tcpServer->close();
    delete serverTcp;
    stopGlobalLogic();
}
//获取MD5加密值
QString MyDoc::GetMd5(const QString &inputValue)
{
    QString strMd5;
    QByteArray array;
    array = QCryptographicHash::hash(inputValue.toUtf8(), QCryptographicHash::Md5);
    strMd5.append(array.toHex());
    return strMd5;
}

//初始化所有数据
void MyDoc::initAllData()
{
    readGlobalData();
    ReadTrainNumTypeTXT();
    initMySQL();
    initNetCom();
    initTimer();
    initGlobalLogic();
    //记录当前时间
    SysStartedDataTime = QDateTime::currentDateTime();
    m_timeLastResetStationAll = QDateTime::currentDateTime();//初始化
    //qDebug()<<"SysStartedDataTime="<<SysStartedDataTime.toMSecsSinceEpoch();
}
//读取全局数据
void MyDoc::readGlobalData()
{
    QString strFile="Data/Global.ini";
    QFileInfo fileInfo(strFile);
    if (!fileInfo.isFile())
    {
        QLOG_ERROR()<<QString::fromLocal8Bit("File %1 not existed!").arg(strFile);
        return;
    }

    //配置文件ini读取
    QSettings* settings = new QSettings(strFile, QSettings::IniFormat);
    // 指定为中文
    settings->setIniCodec("UTF-8");

    //读取配置-车站信息
    //int num = settings->value("Stations/Num").toInt();
    int num = StringToHex(settings->value("Stations/Num").toString());
    qDebug()<<num;
    for(int i=0; i<num; i++)
    {
        QString strNum = QString("Stations/Num%1").arg(i+1);
        qDebug()<<strNum;
        QString info = settings->value(strNum).toString();
        QStringList strArr;
        int count = StringSplit(info, "|", strArr);
        if (2 == count)
        {
            MyStation *pMyStation = new MyStation;
            pMyStation->setStationID(StringToHex(strArr[0]));//strArr[0].toInt()
            QLOG_INFO() << "stationID=" << pMyStation->getStationID();
            pMyStation->setStationName(strArr[1]);
            QLOG_INFO() << "stationName=" << pMyStation->getStationName();
            //防重判断
            for (int s = 0; s < vectMyStation.size(); s++)
            {
                if (pMyStation->getStationID() == vectMyStation[s]->getStationID())
                {
                    QString msg = "错误：存在重复的车站ID " + QString(pMyStation->getStationID());
                    QLOG_ERROR()<<msg;
                    return;
                }
            }
            //本站所有配置数据读取
            //读取站场数据
            QString staFile = QString("Data/data%1/Station.txt").arg(i+1);
            if(!pMyStation->readStationDev(staFile, pMyStation))
            {
                continue;
            }
            //读取配置信息
            staFile = QString("Data/data%1/StationConfig.txt").arg(i+1);//StationConfig.ini
            if(!pMyStation->readStationConfig(staFile, pMyStation))
            {
                continue;
            }
            //读取联锁表
            staFile = QString("Data/data%1/InterlockTable.txt").arg(i+1);
            if(!pMyStation->readInterlockTable(staFile, pMyStation))
            {
                continue;
            }
            //读取股道文件
            staFile = QString("Data/data%1/GDConfig.txt").arg(i+1);
            pMyStation->readGDConfigInfo(staFile, pMyStation);

            //读取进路描述的扩展信号机判定文档
            staFile = QString("Data/data%1/TempRouteXHD.txt").arg(i+1);
            pMyStation->readTempRouteXHD(staFile, pMyStation);
            //读取线路公里标临时限速判断配置文件
            staFile = QString("Data/data%1/XLGLB.txt").arg(i+1);
            pMyStation->readXLGLBConfigInfo(staFile, pMyStation);
            //初始化
            pMyStation->InitData();
            vectMyStation.append(pMyStation);

            //工作线程
            QThread *thread = new QThread(this);
            vectThread.append(thread);
            pMyStation->isThreadStart = true;
            pMyStation->moveToThread(thread);
            connect(thread, &QThread::started, pMyStation, &MyStation::startWorkSlot);
            connect(pMyStation, &MyStation::endWorkSignal, thread, &QThread::quit);
            //thread->start();//此处不启动，初始化结束后统一启动

            //单站收发数据绑定-UDP通道
            connect(pMyStation, &MyStation::sendDataToTeacherSignal, this, &MyDoc::sendDataToTeacherSlot);
            connect(pMyStation, &MyStation::sendDataToLSSignal, this, &MyDoc::sendDataToLSSlot);
            connect(pMyStation, &MyStation::sendDataToCTCSignal, this, &MyDoc::sendDataToCTCSlot);
            connect(pMyStation, &MyStation::sendDataToBoardSignal, this, &MyDoc::sendDataToBoardSlot);
            connect(pMyStation, &MyStation::sendDataToJKSignal, this, &MyDoc::sendDataToJKSlot);
            connect(pMyStation, &MyStation::sendDataToZXTSignal, this, &MyDoc::sendDataToZXTSlot);
            //更新数据消息绑定
            connect(pMyStation, &MyStation::sendUpdateDataMsgSignal, this, &MyDoc::sendUpdateDataMsgSlot);
            //单站收发数据绑定-TCP通道-CTC
            connect(pMyStation, &MyStation::sendDataToCTCSignal2, this, &MyDoc::sendDataToCTCSlot2);
            //单站收发数据绑定-TCP通道-占线板
            connect(pMyStation, &MyStation::sendDataToBoardSignal2, this, &MyDoc::sendDataToBoardSlot2);
            //单站收发数据绑定-TCP通道-JK
            connect(pMyStation, &MyStation::sendDataToJKSignal2, this, &MyDoc::sendDataToJKSlot2);
            //单站收发数据绑定-TCP通道-ZXT
            connect(pMyStation, &MyStation::sendDataToZXTSignal2, this, &MyDoc::sendDataToZXTSlot2);
            //邻站报点消息绑定
            connect(pMyStation, &MyStation::UpdateLZReportTimeSignal, this, &MyDoc::UpdateLZReportTimeSlot);
        }
    }

    //读取配置-通信地址端口信息
    //bUdpMode = settings->value("Address/UdpMode").toInt() == 1 ? true:false;
    localServerPort1 = settings->value("Address/LocalServerPort1").toInt();
    localServerPort2 = settings->value("Address/LocalServerPort2").toInt();
    teacherWatchPort = settings->value("Address/TeacherWatchPort").toInt();
    teacherAddr = settings->value("Address/TeacherAddr").toString();
    teacherPort = settings->value("Address/TeacherPort").toInt();
    localTeacherPort = settings->value("Address/LocalTeacherPort").toInt();
    localTrainPort = settings->value("Address/LocalTrainPort").toInt();

    //数据库地址账号信息
    databaseIP = settings->value("DataBase/HostIP").toString();
    databaseName = settings->value("DataBase/DataBase").toString();
    databaseUser = settings->value("DataBase/UserName").toString();
    databasePassWord = settings->value("DataBase/PassWord").toString();
    databasePort = settings->value("DataBase/Port").toInt();

    QString strKey;
    //读取配置-进路相关
    strKey = "Route/AutoTouchReachRouteLeadMinutes";
    if(settings->contains(strKey))
    {
        AutoTouchReachRouteLeadMinutes = settings->value(strKey).toInt();
        //    qDebug()<<"AutoTouchReachRouteLeadMinutes="<<AutoTouchReachRouteLeadMinutes;
    }
    strKey = "Route/AutoTouchDepartRouteLeadMinutes";
    if(settings->contains(strKey))
    {
        AutoTouchDepartRouteLeadMinutes = settings->value(strKey).toInt();
        //    qDebug()<<"AutoTouchDepartRouteLeadMinutes="<<AutoTouchDepartRouteLeadMinutes;
    }
    strKey = "Route/AutoTouchMinutes";
    if(settings->contains(strKey))
    {
        AutoTouchMinutes = settings->value(strKey).toInt();
        //    qDebug()<<"AutoTouchMinutes="<<AutoTouchMinutes;
    }
    strKey = "Route/TryAutoTouchMaxMinutes";
    if(settings->contains(strKey))
    {
        TryAutoTouchMaxMinutes = settings->value(strKey).toInt();
        //    qDebug()<<"TryAutoTouchMaxMinutes="<<TryAutoTouchMaxMinutes;
    }
    strKey = "Route/MakeRouteOrderWhenClick";
    if(settings->contains(strKey))
    {
        MakeRouteOrderWhenClick = settings->value(strKey).toBool();
        //    qDebug()<<"MakeRouteOrderWhenClick="<<MakeRouteOrderWhenClick;
    }
    strKey = "Route/GDFLBLKXAutoTouch";
    if(settings->contains(strKey))
    {
        GDFLBLKXAutoTouch = settings->value(strKey).toBool();
    }
    strKey = "Route/FCZKSetAutoTouch";
    if(settings->contains(strKey))
    {
        FCZKSetAutoTouch = settings->value(strKey).toBool();
        //    qDebug()<<"FCZKSetAutoTouch="<<FCZKSetAutoTouch;
    }
    strKey = "Route/ManSignRouteSynType";
    if(settings->contains(strKey))
    {
        ManSignRouteSynType = settings->value(strKey).toInt();
    }
    strKey = "Route/TouchingRouteCancelAutoFlag";
    if(settings->contains(strKey))
    {
        TouchingRouteCancelAutoFlag = settings->value(strKey).toBool();
    }
    strKey = "Route/AutoTryMaxMinutesWhenTouchingRoute";
    if(settings->contains(strKey))
    {
        AutoTryMaxMinutesWhenTouchingRoute = settings->value(strKey).toInt();
    }
    strKey = "Route/EarlyTrainsTouchRangeMinutes";
    if(settings->contains(strKey))
    {
        EarlyTrainsTouchRangeMinutes = settings->value(strKey).toInt();
    }
    strKey = "Route/SetAutoTouchNeedNotice";
    if(settings->contains(strKey))
    {
        SetAutoTouchNeedNotice = settings->value(strKey).toBool();
    }
    strKey = "Route/AutoTouchMinitesWhenNoticed";
    if(settings->contains(strKey))
    {
        AutoTouchMinitesWhenNoticed = settings->value(strKey).toInt();
        qDebug()<<"AutoTouchMinitesWhenNoticed="<<AutoTouchMinitesWhenNoticed;
    }
    strKey = "Route/AutoTouchMinitesWhenLZMNJCZ";
    if(settings->contains(strKey))
    {
        AutoTouchMinitesWhenLZMNJCZ = settings->value(strKey).toInt();
        qDebug()<<"AutoTouchMinitesWhenLZMNJCZ="<<AutoTouchMinitesWhenLZMNJCZ;
    }
    strKey = "Route/JudgeRouteCrossed";
    if(settings->contains(strKey))
    {
        JudgeRouteCrossed = settings->value(strKey).toBool();
    }

    //读取配置-列车相关
    strKey = "Train/DispatchCenterTrainModify";
    if(settings->contains(strKey))
    {
        DispatchCenterTrainModify = settings->value(strKey).toBool();
        qDebug()<<"DispatchCenterTrainModify="<<DispatchCenterTrainModify;
    }


    delete settings;
}

//初始化通信
void MyDoc::initNetCom()
{
    //主通信1-UDP通道
    //if(bUdpMode)
    {
        socketUDP = new SocketUDP;
        if(socketUDP->initAnyIP(localServerPort1))
        {
            connect(socketUDP,SIGNAL(recvDataSignal(QByteArray, QString, int)),this,SLOT(receiveAllLSCTCDataSlot(QByteArray, QString, int)));
            connect(this,SIGNAL(sendDataToMainSignal(QByteArray,QString,int,int)),socketUDP,SLOT(sendDataSlot(QByteArray,QString,int,int)));
            QLOG_INFO()<<QString("主通信1-%1初始化成功！").arg(localServerPort1);
        }
        else
        {
            QLOG_ERROR()<<QString("主通信1-%1初始化失败！").arg(localServerPort1);
        }
    }
    //主通信2-TCP通道(数据同步通道)
    {
        serverTcp = new ServerTCP;
        if(serverTcp->initAnyIP(localServerPort2))
        {
            connect(serverTcp,SIGNAL(recvDataSignal(QByteArray, QString, int)),this,SLOT(receiveAllDataSlot2(QByteArray, QString, int)));
            connect(serverTcp,SIGNAL(onDisconnectedSignal(QString, int)),this,SLOT(tcpClientDisconnectedSlot(QString, int)));
            connect(this,SIGNAL(sendDataToMainSignal2(QByteArray,QString,int,int)),serverTcp,SLOT(sendDataSlot(QByteArray,QString,int,int)));
            QLOG_INFO()<<QString("主通信2-%1初始化成功！").arg(localServerPort2);
        }
        else
        {
            QLOG_ERROR()<<QString("主通信2-%1初始化失败！").arg(localServerPort2);
        }
    }

    //初始化教师机通信
    {
        socketUDPTeacher = new SocketUDP;
        if(socketUDPTeacher->initAnyIP(localTeacherPort))
        {
            connect(socketUDPTeacher,SIGNAL(recvDataSignal(QByteArray, QString, int)),this,SLOT(receiveTeacherDataSlot(QByteArray, QString, int)));
            connect(this,SIGNAL(sendDataToTeacherSignal(QByteArray,QString,int,int)),socketUDPTeacher,SLOT(sendDataSlot(QByteArray,QString,int,int)));
            QLOG_INFO()<<QString("教师机通信-%1初始化成功！").arg(localTeacherPort);
        }
        else
        {
            //QMessageBox::information(NULL,VERSION,"教师机通信, 绑定套接字失败，请检查配置数据或设备环境!");
            QLOG_ERROR()<<QString("教师机通信-%1初始化失败！").arg(localTeacherPort);
        }
    }

    //初始化培训软件通信
    {
        socketUDPTrain = new SocketUDP;
        if(socketUDPTrain->initAnyIP(localTrainPort))
        {
            connect(socketUDPTrain,SIGNAL(recvDataSignal(QByteArray, QString, int)),this,SLOT(receiveTrainingDataSlot(QByteArray, QString, int)));
            connect(this,SIGNAL(sendDataToTrainingSignal(QByteArray,QString,int,int)),socketUDPTrain,SLOT(sendDataSlot(QByteArray,QString,int,int)));
            QLOG_INFO()<<QString("培训软件通信-%1初始化成功！").arg(localTrainPort);
        }
        else
        {
            //QMessageBox::information(NULL,VERSION,"培训软件通信, 绑定套接字失败，请检查配置数据或设备环境!");
            QLOG_ERROR()<<QString("培训软件通信-%1初始化失败！").arg(localTrainPort);
        }
    }
    //文字显示通信
    {
        if(teacherWatchPort == 0)
            return;
        socketWatchTCP = new SocketTCP;
        if(socketWatchTCP->initByIP(teacherAddr, teacherWatchPort))
        {
            connect(socketWatchTCP,SIGNAL(recvDataSignal(QByteArray, QString, int)),this,SLOT(receiveAllDataSlot2(QByteArray, QString, int)));
            connect(socketWatchTCP,SIGNAL(onDisconnectedSignal(QString, int)),this,SLOT(tcpClientDisconnectedSlot(QString, int)));
            //connect(this,SIGNAL(sendTextDataToJSJ(QByteArray,int)),serverTcpText,SLOT(sendDataToJSJ(QByteArray,QString,int,int)));
            connect(this,&MyDoc::sendTextDataToJSJ,[=](QByteArray dataArray,int len){
                //sendDataToJSJ(dataArray, len);
                if(teacherWatchPort > 0)
                {
                    socketWatchTCP->sendDataSlot(dataArray, len);
                }
                //emit sendDataToTeacherSignal(dataArray, teacherAddr, teacherWatchPort, len);
            });
            QLOG_INFO()<<QString("文字显示通信-%1初始化成功！").arg(teacherWatchPort);
        }
        else
        {
            QLOG_ERROR()<<QString("文字显示通信-%1初始化失败！").arg(teacherWatchPort);
        }
    }
}
//初始化数据库
void MyDoc::initMySQL()
{
    m_pDataAccess = new DataAccess();
    if(m_pDataAccess->openDataBase(databaseIP, databasePort, databaseName, databaseUser, databasePassWord))
    {
        //QLOG_INFO()<<"数据库链接成功！";
        QLOG_INFO()<<QString("数据库链接成功！%1:%2 %3").arg(databaseIP).arg(databasePort).arg(databaseName);
        //获取各站数据
        GetAllStationDataFromDatabase();
    }
    else
    {
        //QLOG_ERROR()<<"数据库链接失败！";
        QLOG_ERROR()<<QString("数据库链接失败！%1:%2 %3").arg(databaseIP).arg(databasePort).arg(databaseName);
    }
}
//和数据库定时保持连接
void MyDoc::KeepDatabaseConnAlive()
{
    m_pDataAccess->SelectStationCount();
}
//各站获取自己的数据
void MyDoc::GetAllStationDataFromDatabase()
{
    for(int s=0; s<vectMyStation.size(); s++)
    {
        MyStation *pStation = vectMyStation[s];
        //设置数据访问接口
        pStation->setDataAccess(m_pDataAccess);
        //获取车站基础配置状态
        m_pDataAccess->SelectStationInfo(pStation);
        m_pDataAccess->SelectAllStagePlanDetail(pStation);
        m_pDataAccess->SelectAllTrafficLog(pStation);
        m_pDataAccess->SelectAllRouteOrder(pStation);
        m_pDataAccess->SelectAllDisOrderRecv(pStation);
        m_pDataAccess->SelectAllDisOrderDisp(pStation);
        m_pDataAccess->SelectAllDisOrderLocom(pStation);
        {
            m_pDataAccess->SelectAllGDAntiSlip(pStation);
            pStation->SetAllGDAntiSlip();
        }
        //防错办数据读取
        m_pDataAccess->SelectAllGDAttribute(pStation);
        m_pDataAccess->SelectAllGatewayAttribute(pStation);
        m_pDataAccess->SelectAllFixedRoute(pStation);
        m_pDataAccess->SelectAllTrainNumTrack(pStation);
    }
}

//初始化全局逻辑
void MyDoc::initGlobalLogic()
{
    for (int i = 0; i < vectThread.size(); i++)
    {
        QThread* pThread = vectThread.at(i);
        pThread->start();
    }
}
//停止全局逻辑
void MyDoc::stopGlobalLogic()
{
    for (int i = 0; i < vectThread.size(); i++)
    {
        QThread* pThread = vectThread.at(i);
        if(pThread->isRunning())
        {
            pThread->quit();
        }
        vectThread.remove(i);
        delete pThread;
    }
}
//初始化定时器-1秒定时器
void MyDoc::initTimer()
{
    pTimerServer = new QTimer();
    pTimerServer->setInterval(1000);//1秒的定时器
    connect(pTimerServer, SIGNAL(timeout()), this, SLOT(timerOutSlot()));
    pTimerServer->start();
}

//界面绘制
void MyDoc::Draw(QPainter *painter, long nElapsed, double nDiploid)
{
    for(int i=0; i<vectMyStation.size(); i++)//绘制站场界面
    {
        if(i == currStaIndex)
        {
            //显示当前车站
            vectMyStation[i]->draw(painter,nElapsed,nDiploid);
        }
    }
}

//根据站名获取索引
int MyDoc::getIndexByStaNameInStaArray(QString strStation)
{
    for (int i = 0; i < vectMyStation.size(); i++)
    {
        MyStation* pStation = vectMyStation.at(i);
        if (strStation == pStation->getStationName())
        {
            return i;
        }
    }
    return -1;
}

//根据站名在车站数组中获取车站指针
MyStation* MyDoc::getMyStationByStaNameInStaArray(QString strStation)
{
    for (int i = 0; i < vectMyStation.size(); i++)
    {
        MyStation* pStation = vectMyStation.at(i);
        if (strStation == pStation->getStationName())
        {
            return pStation;
        }
    }
    return NULL;
}

//根据id在车站数组中获取车站指针
MyStation* MyDoc::getMyStationByStaIDInStaArray(int id)
{
    for (int i = 0; i < vectMyStation.size(); i++)
    {
        MyStation* pStation = vectMyStation.at(i);
        if (id == (int)pStation->getStationID())
        {
            return pStation;
        }
    }
    return NULL;
}

//根据索引在车站数组中获取车站指针
MyStation* MyDoc::getMyStationByIndexInStaArray(int idx)
{
    if (0 <= idx && idx <vectMyStation.size())
    {
        MyStation* pStation = vectMyStation.at(idx);
        return pStation;
    }
    return NULL;
}
//在车站数组中获取当前车站指针
MyStation *MyDoc::getCurrMyStationInStaArray()
{
    MyStation* pStation = getMyStationByIndexInStaArray(currStaIndex);
    return pStation;
}
//根据站名设置当前车站索引
int MyDoc::setCurrIndexByStaName(QString strStation)
{
    //设置车站索引
    currStaIndex = getIndexByStaNameInStaArray(strStation);
    if (currStaIndex > -1)
    {
        //用于控制命令栏的状态更新
        //GetMyStationByStaNameInStaArray(strStation)->m_bModeChanged = TRUE;
    }
    return currStaIndex;
}

//读取列车运行类型配置文件
void MyDoc::ReadTrainNumTypeTXT()
{
    QString filePath = GetWorkDirRoot()+"Data/TrainNumType.txt";
    QFile file(filePath);
    // 指定为GBK
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QLOG_ERROR()<<"打开TrainNumType.txt数据文件失败!";
        return;
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
        else if(m_str.left(2) == "//")
        {
            continue;
        }
        else if(m_str.indexOf("|") >= 0)
        {
            QStringList strArr;
            int c = StringSplit(m_str, "|", strArr);
            if(c == 2)
            {
                TrainNumType *trainNum_type = new TrainNumType;
                trainNum_type->strType = strArr[0].trimmed();
                trainNum_type->strTypeName = strArr[1].trimmed();
                v_TrainNumType.append(trainNum_type);
            }
        }
        else
        {
            v_TrainRunType.push_back(m_str.trimmed());
        }
    }
    file.close();
    //各站同步
    InitStationTrainNumType();
}
//各站列车运行类型数据同步初始化
void MyDoc::InitStationTrainNumType()
{
    for (int s = 0; s < vectMyStation.count(); s++)
    {
        MyStation* myStation = vectMyStation[s];
        for(int a=0; a<v_TrainNumType.count(); a++)
        {
            myStation->v_TrainNumType.append(v_TrainNumType[a]);
        }
        for(int a=0; a<v_TrainRunType.count(); a++)
        {
            myStation->v_TrainRunType.append(v_TrainRunType[a]);
        }
     }
}

//定时器槽
void MyDoc::timerOutSlot()
{
    //统计运行时间
    SysLifeSeconds = SysStartedDataTime.msecsTo(QDateTime::currentDateTime())/1000;
    //qDebug()<<"[SysLifeSeconds]="<<SysLifeSeconds;

    nElapsed++;
    if(nElapsed >= 9999999)
    {
        nElapsed=0;
    }

    //定时10分钟连接一次数据库（10*60）
    if(nElapsed%600==0)
    {
        KeepDatabaseConnAlive();
    }

//    //定时发送心跳给教师机
//    sendHeartBeatToJSJ();


    for (int s = 0; s < vectMyStation.count(); s++)
    {
        MyStation* myStation = vectMyStation[s];
        myStation->SetTrainPosStatusInTrafficLog();
    }

}

//接收培训软件终端数据
void MyDoc::RecvTrainningData(unsigned char *recvArray, int recvlen, QString client_add, int client_port)
{
    //车站id
    int nStaid = (int)recvArray[6];
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    pStation->trainningAddr = client_add;
    pStation->trainningPort = client_port;

    //非阶段计划+非调度命令
    if(0x12 != (int)recvArray[9] || 0x13 != (int)recvArray[9])
    {
        //数据帧
        QByteArray qRecvArray = UnsignedCharToQByteArray(recvArray, recvlen);
        qRecvArray[8] = DATATYPE_TRAIN;
        //转发给CTC各终端
        SendUDPDataToCTC(pStation, qRecvArray, recvlen);
        SendUDPDataToBoard(pStation, qRecvArray, recvlen);
    }

    Mutex.lock();
    RecvTrainingData_AnalyUdpData(pStation, recvArray+8, recvlen);
    if((int)recvArray[9] == 0x13)
    {
        RecvTrainingData_DDMLAnalysis(pStation, recvArray);
    }
    Mutex.unlock();
}
//向pStation车站所有的CTC终端发送数据
void MyDoc::SendUDPDataToCTC(MyStation *pStation, QByteArray pSendDate, int nLength, int currClientIndex)
{
    if(-1 == currClientIndex)
    {
        //发给本站所有CTC终端
        for (int j=0; j<10; j++)
        {
            emit sendDataToMainSignal(pSendDate, pStation->ctcAddr[j], pStation->ctcPort[j], nLength);
        }
    }
    else
    {
        //发给本站当前CTC终端以外的其余终端
        for (int j=0; j<10; j++)
        {
            if(currClientIndex != j)
            {
                emit sendDataToMainSignal(pSendDate, pStation->ctcAddr[j], pStation->ctcPort[j], nLength);
            }
        }
    }
}
//向pStation车站所有的占线板终端发送数据
void MyDoc::SendUDPDataToBoard(MyStation *pStation, QByteArray pSendDate, int nLength, int currClientIndex)
{
    if(-1 == currClientIndex)
    {
        //发给本站所有占线板终端
        for (int j=0; j<10; j++)
        {
            emit sendDataToMainSignal(pSendDate, pStation->zxbAddr[j], pStation->zxbPort[j], nLength);
        }
    }
    else
    {
        //发给本站当前CTC终端以外的其余终端
        for (int j=0; j<10; j++)
        {
            if(currClientIndex != j)
            {
                emit sendDataToMainSignal(pSendDate, pStation->zxbAddr[j], pStation->zxbPort[j], nLength);
            }
        }
    }
}
//接收培训数据-分析数据
void MyDoc::RecvTrainingData_AnalyUdpData(MyStation* pStation, unsigned char *Rec, int recvlen)
{
    OrderStr orderStr;
    int cmdLen = 0;
    unsigned char TXT[500] = { 0 };
    QString ParaStr;
    int index;
    index = 1;
    type = Rec[0];
    if(Rec[index] == 0x11)//场景设置
    {
        index++;
        cmdLen = Rec[++index];
        memcpy(TXT, &Rec[++index],cmdLen);
        orderStr.FunStr = UnsignedCharArrayToString(TXT);

        index = index + cmdLen;
        cmdLen = Rec[index];

        memset(TXT,0,sizeof(TXT));
        memcpy(TXT, &Rec[++index],cmdLen);
        ParaStr = UnsignedCharArrayToString(TXT);

        orderStr.ParaStr = ParaStr;

        QStringList StrArray;
        StringSplit(ParaStr,",",StrArray);
        if(StrArray.count()==1)
        {
            orderStr.DevName = StrArray[0];
        }
        else if(StrArray.count()==2)
        {
            orderStr.SubFunStr = StrArray[0];
            orderStr.DevName = StrArray[1];
        }
        else if(StrArray.count()==3)
        {
            orderStr.SubFunStr = StrArray[0];
            orderStr.TipShowStr = StrArray[1];
            orderStr.DevName = StrArray[2];
        }
        v_OrderArray.push_back(orderStr);
        ManageSpecialOrder(pStation);
    }
    else if(Rec[index] == 0x12)//阶段计划接收
    {
        JDJHAnalysis(pStation, Rec+2);
    }
    else if(Rec[index] == 0x66)//清除场景
    {
        v_OrderArray.clear();
        //ResetStationInfo(pStation,false);//true
        resetStationInfo(pStation,false);//true
    }
    else if(Rec[index] == 0xaa)//清除站场演示信息  返回到CTC主页面
    {
        v_OrderArray.clear();
        //ResetStationInfo(pStation,false);//true
        resetStationInfo(pStation,false);//true
    }
}
//接收培训数据-调度命令解析函数
void MyDoc::RecvTrainingData_DDMLAnalysis(MyStation* pStation, unsigned char *Rec_data)
{
    unsigned int AnalysisPos  = 10;

    //RecvDisOrder disOrder;
    //disOrder.Init();
    DispatchOrderStation* disOrder = new DispatchOrderStation();
    //车站id
    disOrder->station_id = pStation->getStationID();
    //命令类型
    disOrder->strType = GetStrFun(Rec_data,&AnalysisPos);
    //接受命令号
    disOrder->strNumber = GetStrFun(Rec_data,&AnalysisPos);
    /*20200806 增加调度号转换*/
    disOrder->uNumber = disOrder->strNumber.toInt();
    //命令时间
    disOrder->timOrder = QDateTime::currentDateTime();
    //调度员
    disOrder->strDisName = "调度员";
    //调度中心
    disOrder->strDisCenter = "CTC调度中心";

    //若收到联锁发送的调度命令，则长度为484。铁科版在StatusChangeNew()中修改。20180103
    //受令内容
    disOrder->strContent = GetStrFun_Lenth(Rec_data,&AnalysisPos,2);
    //受令单位
    disOrder->listRecvPlace.push_back(pStation->getStationName());
    //全局增加一条
    vectRecvDisOrder.append(disOrder);

    //数据库访问
    if(m_pDataAccess)
    {
        disOrder->order_id = m_pDataAccess->InsertDisOrderRecv(disOrder);
        //本站添加
        pStation->m_ArrayDisOrderRecv.append(disOrder);//同一个指针是否有问题
        //发送同步数据消息
        //SendUpdateDataMsg(pStation, UPDATETYPE_DDML);
        //处理和发送1个数据
        pStation->sendOneDisOrderToSoft(DATATYPE_ALL,disOrder,SYNC_FLAG_ADD,1,1);
        //报警信息
        pStation->sendWarningMsgToCTC(3,2,"收到调度命令");
        //语音播报
        pStation->SendSpeachText("调度命令下达请签收");
    }

    //(必须在存入数据库后再发送，否则CTC从数据库获取不到数据)
    //向集控站CTC转发教师机命令信息
    {
        int nLength = 60;
        unsigned char byArrayUDPDate[60];
        memset(byArrayUDPDate,0,nLength);
        for(int i = 0; i < 4; i++)
        {
            byArrayUDPDate[i] = 0xEF;
            byArrayUDPDate[nLength - i -1] = 0xFE;
        }
        //memcpy(&byArrayUDPDate[4], &nLength, 2);//帧长度
        //memcpy(&byArrayUDPDate[6], &StationID, 1);//帧长度
        memcpy(byArrayUDPDate+4, &nLength, 2);//帧长度
        //memcpy(byArrayUDPDate+6, &StationID, 1);//帧长度
        byArrayUDPDate[6] = pStation->getStationID();
        byArrayUDPDate[8] = DATATYPE_TCC;
        byArrayUDPDate[9] = 0x99;//分类信息码
        //byArrayUDPDate[10] = _type;//子分类信息码
        //发给本站所有的CTC、占线板终端
        {
            //数据帧转换
            QByteArray qSendArray = UnsignedCharToQByteArray(byArrayUDPDate, nLength);
            SendUDPDataToCTC(pStation, qSendArray, nLength);
            SendUDPDataToBoard(pStation, qSendArray, nLength);
        }
    }
}

void MyDoc::ManageSpecialOrder(MyStation* pStation)
{
    for(int i = 0; i < v_OrderArray.count(); i++)
    {
        if(v_OrderArray[i].FunStr.indexOf("JDJHML-MEMU")!=-1)//发送一包阶段计划命令
        {
            OrderStr ExapleStr = v_OrderArray[i];
            v_OrderArray.erase(v_OrderArray.begin()+i);
            if(ExapleStr.DevName == "QS")
            {
                //pFrame->m_wndDispatchOrderBar.OnBnClickedBtnStageplan();
            }
            else
            {
                //unsigned char JDJH[60]={
                //	0xEF, 0xEF, 0xEF, 0xEF, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x33, 0xE9, 0x03, 0x54, 0x31, 0x32, 0x33, 0x34, 0x00, 0x54, 0x31,
                //	0x32, 0x33, 0x34, 0x00, 0x11, 0x51, 0x00, 0xE4, 0x07, 0x05, 0x1B, 0x11, 0x1A, 0x00, 0x51, 0x00, 0xE4, 0x07, 0x05, 0x1B,
                //	0x11, 0x1A, 0x00, 0xcc, 0x19, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0xFE
                //};
                unsigned int GDCode = getDevNodeOfName(pStation, ExapleStr.TipShowStr);
                unsigned int XHCode = getDevNodeOfName(pStation, ExapleStr.DevName);
                QDateTime m_currTime, m_Time1, m_Time2;
                m_currTime = QDateTime::currentDateTime();
                m_Time1 = m_currTime.addSecs(2);// + CTimeSpan(0,0,2,0);//加2分钟
                m_Time2 = m_currTime.addSecs(4);// + CTimeSpan(0,0,4,0);//加3分钟

                //NEW.2021.8.28.LWM
                int nLength = 0;
                int nCount = 9;
                unsigned char JDJH[500];
                memset(JDJH, 0x00, 500);

                //车站id
                JDJH[6] = pStation->getStationID();

                //命令类型-阶段计划
                JDJH[nCount] = 0x33;
                nCount += 1;

                PlanCode ++;
                //memcpy(JDJH+10,&PlanCode,2);//计划号2B
                memcpy(&JDJH[nCount],&PlanCode,4);//计划号4B
                nCount += 4;

                //到达车次
                Num ++;
                unsigned char ch[6]={0x54, 0x31, 0x32, 0x33, 0x34,0x00};
                ch[4] = Num;
                //memcpy(&JDJH[12],ch,6);
                JDJH[nCount] = 0x05;
                nCount += 1;
                memcpy(&JDJH[nCount],ch,5);
                nCount += 5;

                //出发车次
                Num++;
                ch[4] = Num;
                //memcpy(&JDJH[18],ch,6);
                JDJH[nCount] = 0x05;
                nCount += 1;
                memcpy(&JDJH[nCount],ch,5);
                nCount += 5;

                //类型：添加还是删除
                JDJH[nCount] = 0x11;//添加
                nCount += 1;

                if(GDCode!=0xffff && XHCode!=0xffff)
                {
                    int  Year_DC = m_Time1.date().year();
                    int  Month_DC = m_Time1.date().month();
                    int  Day_DC = m_Time1.date().day();
                    int  Hour_DC = m_Time1.time().hour();
                    int  Minute_DC = m_Time1.time().minute();
                    //接车股道
                    //memcpy(JDJH+25,&GDCode,2);
                    memcpy(&JDJH[nCount],&GDCode,2);
                    nCount += 2;
                    ///到达时间
                    //memcpy(JDJH+27,&Year_DC,2);
                    //memcpy(JDJH+29,&Month_DC,1);
                    //memcpy(JDJH+30,&Day_DC,1);
                    //memcpy(JDJH+31,&Hour_DC,1);
                    //memcpy(JDJH+32,&Minute_DC,1);
                    memcpy(&JDJH[nCount],&Year_DC,2);
                    memcpy(&JDJH[nCount+2],&Month_DC,1);
                    memcpy(&JDJH[nCount+3],&Day_DC,1);
                    memcpy(&JDJH[nCount+4],&Hour_DC,1);
                    memcpy(&JDJH[nCount+5],&Minute_DC,1);
                    JDJH[nCount+6] = 0x00;//秒
                    nCount += 7;

                    Year_DC = m_Time2.date().year();
                    Month_DC = m_Time2.date().month();
                    Day_DC = m_Time2.date().day();
                    Hour_DC = m_Time2.time().hour();
                    Minute_DC = m_Time2.time().minute();
                    //发车股道
                    //memcpy(JDJH+34,&GDCode,2);
                    memcpy(&JDJH[nCount],&GDCode,2);
                    nCount += 2;
                    //出发时间
                    //memcpy(JDJH+36,&Year_DC,2);
                    //memcpy(JDJH+38,&Month_DC,1);
                    //memcpy(JDJH+39,&Day_DC,1);
                    //memcpy(JDJH+40,&Hour_DC,1);
                    //memcpy(JDJH+41,&Minute_DC,1);
                    memcpy(&JDJH[nCount],&Year_DC,2);
                    memcpy(&JDJH[nCount+2],&Month_DC,1);
                    memcpy(&JDJH[nCount+3],&Day_DC,1);
                    memcpy(&JDJH[nCount+4],&Hour_DC,1);
                    memcpy(&JDJH[nCount+5],&Minute_DC,1);
                    JDJH[nCount+6] = 0x00;//秒
                    nCount += 7;

                    //始发终到标志
                    if(ExapleStr.SubFunStr == "ZD")
                    {
                        //JDJH[43]=0xcc;
                        JDJH[nCount]=0xcc;
                    }
                    else if(ExapleStr.SubFunStr == "SF")
                    {
                        //JDJH[43]=0xbb;
                        JDJH[nCount]=0xbb;
                    }
                    else
                    {
                        return;
                    }
                    nCount += 1;

                    //接车口/进站口
                    //memcpy(JDJH+44,&XHCode,2);
                    memcpy(&JDJH[nCount],&XHCode,2);
                    nCount += 2;

                    //发车口/出站口
                    //memcpy(JDJH+46,&XHCode,2);
                    memcpy(&JDJH[nCount],&XHCode,2);
                    nCount += 2;

                    //电力
                    JDJH[nCount] = 0x11;
                    nCount += 1;
                    //超限
                    JDJH[nCount] = 0x00;
                    nCount += 1;
                    //列车客货类型
                    JDJH[nCount] = LCTYPE_KC;
                    nCount += 1;
                    //列车类型序号
                    JDJH[nCount] = 0x00;
                    nCount += 1;
                    //运行类型序号
                    JDJH[nCount] = 0x00;
                    nCount += 1;

                    //memcpy(pDoc->m_chArrayRecvDataBuffToJSJ[0],JDJH,60);
                    //pDoc->RecvStagePlanData();
                    nLength=nCount+4;
                    memcpy(&JDJH[4], &nLength, 2);//帧长度
                    for(int i = 0; i < 4; i++)
                    {
                        JDJH[i] = 0xEF;
                        JDJH[nLength - i -1] = 0xFE;
                    }
                    RecvStagePlanDataOfTraining(JDJH, nLength, true);
                }
            }
        }
        else if(v_OrderArray[i].FunStr.indexOf("DDMLJS-MEMU")!=-1)//调度命令接收
        {
            if(v_OrderArray[i].DevName == "OK")
            {

                //HWND  hWnd = ::FindWindow(NULL,_T("签收调度命令")); //查找消息框的标题
                //if (hWnd)
                //{
                //    ::PostMessage((HWND)hWnd,WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), NULL);
                //}
            }
            else
            {
                //DDMLCount = 4;
                //pFrame->m_wndDispatchOrderBar.OnBnClickedDispatchButton();
            }
            v_OrderArray.erase(v_OrderArray.begin()+i);
        }
        else if(v_OrderArray[i].FunStr.indexOf("DDMLCX-MEMU")!=-1)//调度命令查询
        {
            //pFrame->OnDispatchorderMenu();
            //if(v_OrderArray[i].DevName == "CX")
            //    pFrame->m_pDispatchOrderDlg->DDMLCheck();
            /*else if(v_OrderArray[i].DevName == _T("XS"))
                ((CDispatchFormView *)(pFrame->m_pDispatchFormView))->m_page3.ShowDisOrder(0);*/
            v_OrderArray.erase(v_OrderArray.begin()+i);
        }
        else if(v_OrderArray[i].FunStr.indexOf("XCPZBJ-MEMU")!=-1)//行车凭证页面
        {
            //pFrame->OnDispatchorderMenu();
            if(v_OrderArray[i].DevName == "MAIN")//主页面
            {
                //pFrame->m_pDispatchOrderDlg->TurnXCPZPage();
            }
            else if(v_OrderArray[i].DevName == "NEW")//新建行车凭证
            {
                //DDMLCount = 4;
                //CDlgSelectDisType dlgSelectDisType; //机车调度命令类型选择
                //dlgSelectDisType.DoModal();
            }
            else if(v_OrderArray[i].DevName == "路票")//路票演示
            {
                //XCPZCount = 4;
                //CDlgNewTrainDis dlgNewTrainDis(NULL,2);	//新建机车调度命令
                //dlgNewTrainDis.DoModal();
            }
            else if(v_OrderArray[i].DevName == "绿色许可证")//绿色许可证演示
            {
                //XCPZCount = 4;
                //CDlgNewTrainDis dlgNewTrainDis(NULL,3);	//新建机车调度命令
                //dlgNewTrainDis.DoModal();
            }
            else if(v_OrderArray[i].DevName == "红色许可证")//红色许可证演示
            {
                //XCPZCount = 4;
                //CDlgNewTrainDis dlgNewTrainDis(NULL,4);	//新建机车调度命令
                //dlgNewTrainDis.DoModal();
            }
            v_OrderArray.erase(v_OrderArray.begin()+i);
        }
        else if(v_OrderArray[i].FunStr.indexOf("FSFCQQ-MEMU")!=-1)//发车请求
        {
            //pDoc->SetXCRZTrainMessage(0,0x11);
            v_OrderArray.erase(v_OrderArray.begin()+i);
        }
        else if(v_OrderArray[i].FunStr.indexOf("TYLZFC-MEMU")!=-1)//同意邻站发车
        {
            //pDoc->SetXCRZTrainMessage(0,0x33);
            v_OrderArray.erase(v_OrderArray.begin()+i);
        }
    }
}
//培训软件-根据设备编号获取设备名称
QString MyDoc::getNameOfDevNode(MyStation* pStation, unsigned int devnode)
{
    for(auto ement:pStation->DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            if(devnode == (int)gddc->getCode())
            {
                return gddc->getName();
            }
        }
        else if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            if(devnode == (int)xhd->getCode())
            {
                return xhd->getName();
            }
        }
        else if(ement->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD*)ement;
            if(devnode == (int)gd->getCode())
            {
                return gd->getName();
            }
        }
    }
    return "";
}
//培训软件-根据设备名称获取设备编号
unsigned int MyDoc::getDevNodeOfName(MyStation* pStation, QString strName)
{
    for(auto ement:pStation->DevArray)
    {
        if(ement->getDevType() == Dev_DC)
        {
            CGDDC *gddc=(CGDDC *)ement;
            if(strName == gddc->getName())
            {
                return gddc->getCode();
            }
        }
        else if(ement->getDevType() == Dev_XH)
        {
            CXHD *xhd=(CXHD *)ement;
            if(strName == xhd->getName())
            {
                return xhd->getCode();
            }
        }
        else if(ement->getDevType() == Dev_GD)
        {
            CGD *gd=(CGD*)ement;
            if(strName == gd->getName())
            {
                return gd->getCode();
            }
        }
    }
    return 65535;
}
//培训软件-阶段计划解析函数
void MyDoc::JDJHAnalysis(MyStation *pStation, unsigned char *Rec_data)
{
    unsigned int AnalysisPos  = 0;
    QString GetString;
    StagePlan *GetStagePlan = new StagePlan();
    GetStagePlan->m_nPlanNumber = 0;
    GetStagePlan->station_id = pStation->getStationID();

    if((GetStagePlan->station_id<0)&&(GetStagePlan->station_id>255))
    {
        return;
    }

    GetStagePlan->m_btStagePlanKind = 0x11;
    GetStagePlan->m_strReachTrainNum = GetStrFun(Rec_data,&AnalysisPos);  //接车车次号
    GetStagePlan->m_strDepartTrainNum = GetStrFun(Rec_data,&AnalysisPos);  //发车车次号
    GetStagePlan->m_strReachTrainNumOld = GetStagePlan->m_strReachTrainNum;
    GetStagePlan->m_strDepartTrainNumOld = GetStagePlan->m_strDepartTrainNum;
    GetString =  GetStrFun(Rec_data,&AnalysisPos); //到达股道
    GetStagePlan->m_nRecvTrainTrack = getDevNodeOfName(pStation, GetString);
    GetStagePlan->m_strRecvTrainTrack = GetString;   //接车股道
    if(0xffff == GetStagePlan->m_nRecvTrainTrack)
        return ;
    //列车到达时间
    {
        int year = (int)(Rec_data[AnalysisPos] | Rec_data[AnalysisPos+1]<<8);
        int mouth = (int)Rec_data[AnalysisPos+2];
        int day = (int)Rec_data[AnalysisPos+3];
        int hour = (int)Rec_data[AnalysisPos+4];
        int mini = (int)Rec_data[AnalysisPos+5];
        int second = (int)Rec_data[AnalysisPos+6];
        QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
        QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
        //qDebug()<<"GetStagePlan->m_timProvReachStation="<<dateTime.toString("yyyy-MM-dd hh:mm:ss");
        GetStagePlan->m_timProvReachStation = dateTime;
        AnalysisPos += 7;
    }
    GetString =  GetStrFun(Rec_data,&AnalysisPos); //发到股道的长度
    GetStagePlan->m_nDepartTrainTrack = getDevNodeOfName(pStation, GetString);
    GetStagePlan->m_strDepartTrainTrack = GetString; //发车股道
    if(0xffff == GetStagePlan->m_nDepartTrainTrack)
        return ;
    //列车到达时间
    {
        int year = (int)(Rec_data[AnalysisPos] | Rec_data[AnalysisPos+1]<<8);
        int mouth = (int)Rec_data[AnalysisPos+2];
        int day = (int)Rec_data[AnalysisPos+3];
        int hour = (int)Rec_data[AnalysisPos+4];
        int mini = (int)Rec_data[AnalysisPos+5];
        int second = (int)Rec_data[AnalysisPos+6];
        QString strDateTime= TransfrmFullDateTimeString(year,mouth,day,hour,mini,second);
        QDateTime dateTime = QDateTime::fromString(strDateTime, "yyyy-MM-dd hh:mm:ss");
        //qDebug()<<"GetStagePlan->m_timProvDepaTrain="<<dateTime.toString("yyyy-MM-dd hh:mm:ss");
        GetStagePlan->m_timProvDepaTrain = dateTime;
        AnalysisPos += 7;
    }
    GetStagePlan->m_btBeginOrEndFlg = Rec_data[AnalysisPos];
    AnalysisPos++;
    GetString= GetStrFun(Rec_data,&AnalysisPos); //进站信号机名称
    if(GetStagePlan->m_btBeginOrEndFlg != 0xbb)
    {
        GetStagePlan->m_nCodeReachStaEquip = getDevNodeOfName(pStation, GetString);
        if(0xffff == GetStagePlan->m_nCodeReachStaEquip)
            return ;
    }
    GetString= GetStrFun(Rec_data,&AnalysisPos); //出站信号机名称
    if(GetStagePlan->m_btBeginOrEndFlg != 0xcc)
    {
        GetStagePlan->m_nCodeDepartStaEquip = getDevNodeOfName(pStation, GetString);
        if(0xffff == GetStagePlan->m_nCodeDepartStaEquip)
            return ;
    }

    bool bXianLuSuo = false;
    if(GetStagePlan->m_btBeginOrEndFlg!=JFC_TYPE_SF)
    {
        bXianLuSuo = pStation->IsXHDinXianLuSuo(pStation->GetStrNameByCode(GetStagePlan->m_nCodeReachStaEquip));
    }
    if(!bXianLuSuo && GetStagePlan->m_btBeginOrEndFlg!=JFC_TYPE_ZD)
    {
        bXianLuSuo = pStation->IsXHDinXianLuSuo(pStation->GetStrNameByCode(GetStagePlan->m_nCodeDepartStaEquip));
    }
    GetStagePlan->bXianLuSuo = bXianLuSuo;
    if(GetStagePlan->bXianLuSuo)
    {
        //线路所进路默认全部是通过
        GetStagePlan->m_btBeginOrEndFlg = JFC_TYPE_TG;
        GetStagePlan->m_strRecvTrainTrack = pStation->GetStrNameByCode(GetStagePlan->m_nCodeReachStaEquip) + "-" + pStation->GetStrNameByCode(GetStagePlan->m_nCodeDepartStaEquip);
        GetStagePlan->m_strDepartTrainTrack = pStation->GetStrNameByCode(GetStagePlan->m_nCodeReachStaEquip) + "-" + pStation->GetStrNameByCode(GetStagePlan->m_nCodeDepartStaEquip);
    }
    //终到
    if(GetStagePlan->m_btBeginOrEndFlg == JFC_TYPE_ZD)
    {
        GetStagePlan->m_strDepartTrainNum = "";
    }
    //始发
    else if(GetStagePlan->m_btBeginOrEndFlg == JFC_TYPE_SF)
    {
        GetStagePlan->m_strReachTrainNum = "";
    }
//    if((GetStagePlan->bXianLuSuo)&&(GetStagePlan->m_btBeginOrEndFlg != JFC_TYPE_SF)&&(GetStagePlan->m_btBeginOrEndFlg != JFC_TYPE_ZD))
//    {
//        //线路所进路默认全部是通过
//        GetStagePlan->m_btBeginOrEndFlg = JFC_TYPE_TG;
//    }
//    //else if((GetStagePlan->bXianLuSuo)&&(GetStagePlan->m_btBeginOrEndFlg == 0xbb))
//    //{
//    //}
//    else if((GetStagePlan->bXianLuSuo)&&(GetStagePlan->m_btBeginOrEndFlg == JFC_TYPE_ZD))
//    {
//        GetStagePlan->m_btBeginOrEndFlg = JFC_TYPE_ZD;
//        GetStagePlan->m_nCodeDepartStaEquip = pStation->GetCodeByStrName(GetString);

//        if(0xffff == GetStagePlan->m_nCodeDepartStaEquip)
//            return ;
//    }

    //NEW.2021.8.28.LWM
    //电力机车
    GetStagePlan->m_bElectric = true;
    //超限
    GetStagePlan->m_nLevelCX = 0;
    //列车客货类型
    GetStagePlan->m_nLHFlg = LCTYPE_KC;
    //列车类型序号（管内动车组、通勤列车等）
    GetStagePlan->m_nIndexLCLX = 0;
    //运行类型序号（动车组、快速旅客列车等）
    GetStagePlan->m_nIndexYXLX = 0;

    if(GetStagePlan->m_btBeginOrEndFlg == 0xbb)
        GetStagePlan->m_strReachTrainNum  = "";
    if(GetStagePlan->m_btBeginOrEndFlg == 0xcc)
        GetStagePlan->m_strDepartTrainNum  = "";
    /****************遍历所有的阶段计划类***************************/
    StagePlan *pStagePlan;
    int nIndex;
    int spCount = pStation->m_ArrayStagePlan.count();
    for(nIndex = 0; nIndex<spCount; nIndex++)
    {
        pStagePlan = (StagePlan *)pStation->m_ArrayStagePlan[nIndex];
        if(GetStagePlan->m_strReachTrainNum == pStagePlan->m_strReachTrainNum)
        {
            //替换相应的设备
            pStagePlan->m_nPlanNumber = GetStagePlan->m_nPlanNumber;
            pStagePlan->station_id = GetStagePlan->station_id;
            pStagePlan->m_btStagePlanKind = GetStagePlan->m_btStagePlanKind;
            pStagePlan->m_strReachTrainNum = GetStagePlan->m_strReachTrainNum;
            pStagePlan->m_strDepartTrainNum = GetStagePlan->m_strDepartTrainNum;
            pStagePlan->m_timProvReachStation = GetStagePlan->m_timProvReachStation;
            pStagePlan->m_timProvDepaTrain = GetStagePlan->m_timProvDepaTrain;
            pStagePlan->m_nRecvTrainTrack = GetStagePlan->m_nRecvTrainTrack;
            pStagePlan->m_nDepartTrainTrack = GetStagePlan->m_nDepartTrainTrack;
            pStagePlan->m_btBeginOrEndFlg = GetStagePlan->m_btBeginOrEndFlg;
            pStagePlan->m_nCodeReachStaEquip = GetStagePlan->m_nCodeReachStaEquip;
            pStagePlan->m_nCodeDepartStaEquip = GetStagePlan->m_nCodeDepartStaEquip;
            pStagePlan->m_nStateSignPlan = 0;//未签收
            //pStagePlan->m_btStagePlanKind = 0x11;
            //NEW.2021.8.28.LWM
            pStagePlan->m_bElectric = GetStagePlan->m_bElectric;
            pStagePlan->m_nLevelCX = GetStagePlan->m_nLevelCX;
            pStagePlan->m_nLHFlg = GetStagePlan->m_nLHFlg;
            pStagePlan->m_nIndexLCLX = GetStagePlan->m_nIndexLCLX;
            pStagePlan->m_nIndexYXLX = GetStagePlan->m_nIndexYXLX;
            //更新数据库
            m_pDataAccess->UpdateStagePlanDetail(pStagePlan);
            //发送同步数据消息
            SendUpdateDataMsg(pStation, UPDATETYPE_JDJH);
            //自动签收
            pStation->SignStagePlan(true);
            return;
        }
    }

//    /*阶段计划按钮闪烁 20200806*/
//    CMainFrame *pFram=(CMainFrame *)AfxGetApp()->m_pMainWnd;
//    pFram->m_wndDispatchOrderBar.m_nStagePlanBtnFlgFlash = TRUE;
//    pFram->m_wndDispatchOrderBar.m_nFlashNum = 5;
//    pFram->m_wndDispatchOrderBar.m_selfBtnStagePlan.Invalidate();
//    pFram->SpeakFun(_T("阶段计划下达请签收"), 1);
    /********************************/
    pStation->m_ArrayStagePlan.append(GetStagePlan);
    if(m_pDataAccess)
    {
        Stage* stage = new Stage;
        stage->station_id = pStation->getStationID();
        stage->m_nPlanNum = GetStagePlan->m_nPlanNumber;
        stage->m_timRecv = QDateTime::currentDateTime();
        //查数据库
        GetStagePlan->plan_id = m_pDataAccess->SelectPlanId(stage);
        //还没有父表内容，则插入
        if(GetStagePlan->plan_id <= 0)
        {
            GetStagePlan->plan_id = m_pDataAccess->InsetStage(stage);
        }
        m_pDataAccess->InsetStagePlanDetail(GetStagePlan);
        //发送同步消息
        //SendUpdateDataMsg(pStation, UPDATETYPE_JDJH);
        //自动签收
        pStation->SignStagePlan(true, true);
    }
    //(必须在存入数据库后再发送，否则CTC从数据库获取不到数据)
    //向集控站CTC转发教师机命令信息
    {
        int nLength = 60;
        unsigned char byArrayUDPDate[60];
        memset(byArrayUDPDate,0,nLength);
        for(int i = 0; i < 4; i++)
        {
            byArrayUDPDate[i] = 0xEF;
            byArrayUDPDate[nLength - i -1] = 0xFE;
        }
        //memcpy(&byArrayUDPDate[4], &nLength, 2);//帧长度
        //memcpy(&byArrayUDPDate[6], &StationID, 1);//帧长度
        memcpy(byArrayUDPDate+4, &nLength, 2);//帧长度
        //memcpy(byArrayUDPDate+6, &StationID, 1);//帧长度
        byArrayUDPDate[6] = GetStagePlan->station_id;
        byArrayUDPDate[8] = DATATYPE_TCC;
        byArrayUDPDate[9] = 0x33;//分类信息码
        //byArrayUDPDate[10] = _type;//子分类信息码
        //发给本站所有的CTC、占线板终端
        {
            //数据帧转换
            QByteArray qSendArray = UnsignedCharToQByteArray(byArrayUDPDate, nLength);
            SendUDPDataToCTC(pStation, qSendArray, nLength);
            SendUDPDataToBoard(pStation, qSendArray, nLength);
        }
    }
}

//培训软件判别
void MyDoc::DrawSence()
{
    for (int s = 0; s < vectMyStation.count(); s++)
    {
        MyStation* pStation = (MyStation*)vectMyStation[s];
        if (pStation->StaConfigInfo.isHavePXRJ)
        {
            int count = v_OrderArray.size();
            for(int i = 0; i < v_OrderArray.size(); i++)
            {
                DrawSence(pStation,v_OrderArray[i],i);
            }
        }
    }
}

void MyDoc::DrawSence(MyStation *pStation, MyDoc::OrderStr ExapleStr, int index)
{
    QString Getstring;
    if(ExapleStr.FunStr.indexOf("DDML-MEMU")!=-1)
    {
        qDebug()<<"recived DDML-MEMU.";
        v_OrderArray.erase(v_OrderArray.begin()+index);
        unsigned char DDML[500]={
            0xEF, 0xEF ,0xEF, 0xEF, 0xF4, 0x01, 0x05, 0x01, 0xBB, 0x99, 0x31, 0x00 ,0x00, 0x00, 0x00, 0x00, 0xE4, 0x07, 0x05 ,0x14,
            0x0B, 0x08, 0x00, 0x44 ,0x45, 0x46 ,0x41 ,0x55 ,0x4C, 0x54 ,0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00 ,
            0x00, 0x00, 0x00, 0xBD, 0xD3, 0xB4, 0xA5, 0xCD, 0xF8, 0xCD, 0xA3, 0xA1, 0xA2, 0xCB, 0xCD, 0xB5, 0xE7, 0xBC, 0xB0, 0xCE,
            0xAC, 0xD0, 0xDE, 0xD7, 0xF7, 0xD2, 0xB5, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00 ,0x00 ,0x00 ,0x00, 0x00, 0x00, 0x00 ,
            0x00, 0x00, 0x00 ,0x31, 0x2E, 0xBD, 0xD3 ,0xB4 ,0xA5 ,0xCD, 0xF8, 0xD3, 0xD0, 0xBC, 0xC6, 0xB0, 0xAE, 0xCD, 0xA3, 0xA1 ,
            0xA2, 0xCB, 0xCD, 0xB5, 0xE7, 0xBC, 0xB0, 0xCE, 0xAC, 0xD0, 0xDE, 0xD7, 0xF7, 0xD2, 0xB5, 0x0D, 0x0A, 0x20, 0x20, 0x20,
            0xB8, 0xF9, 0xBE, 0xDD, 0xB9, 0xA9, 0xB5, 0xE7, 0xB5, 0xF7, 0xB6 ,0xC8, 0x20, 0x20, 0x20, 0x20, 0xBA, 0xC5, 0xC9, 0xEA ,
            0xC7, 0xEB, 0xA3, 0xAC, 0xD7, 0xD4, 0xBD, 0xD3, 0xC1, 0xEE, 0xCA, 0xB1, 0x28, 0xCA ,0xB1, 0x20, 0x20, 0x20 ,0x20, 0xB7 ,
            0xD6, 0x29, 0x20, 0x20, 0x20, 0xB4, 0xCE, 0xC1, 0xD0, 0xB3, 0xB5, 0xB5, 0xBD, 0xB4 ,0xEF, 0x20, 0x20, 0x20 ,0xD5, 0xBE ,
            0xA3, 0xA9, 0xC6, 0xF0, 0xD6, 0xC1, 0x20, 0x20, 0xCA, 0xB1, 0x20, 0x20, 0xB7, 0xD6, 0xD6, 0xB9, 0xA3, 0xAC ,0x0D, 0x0A ,
            0xD7, 0xBC, 0xD0, 0xED, 0x20, 0x20, 0x20, 0xD5, 0xBE, 0xD6, 0xC1, 0x20, 0x20, 0x20, 0xD5, 0xBE ,0xBC, 0xE4 ,0x20, 0x20 ,
            0x20, 0x20, 0xD0, 0xD0, 0xCF, 0xDF, 0xBD, 0xD3, 0xB4, 0xA5, 0xCD, 0xF8, 0xCD, 0xA3, 0xB5, 0xE7 ,0xA3, 0xAC ,0xD3 ,0xD0,
            0xB9, 0xD8, 0xB0, 0xB2, 0xC8, 0xAB, 0xB4, 0xEB, 0xCA, 0xA9, 0xBC, 0xB0, 0xCD, 0xA3, 0xB5, 0xE7 ,0xB7, 0xB6 ,0xCE ,0xA7,
            0xB0, 0xB4, 0x20, 0x20, 0xB9, 0xA9, 0xB5, 0xE7, 0xB6, 0xCE, 0x20, 0x20, 0x20, 0xB5, 0xA5, 0xD4 ,0xAA, 0xD6, 0xB4 ,0xD0,
            0xD0, 0xA1, 0xA3, 0x0D, 0x0A, 0x20, 0xCD, 0xAC, 0xCA, 0xB1, 0xD7, 0xBC, 0xD0, 0xED, 0xBD, 0xF8 ,0xD0, 0xD0 ,0xCE ,0xAC,
            0xD0, 0xDE, 0x28, 0xCA, 0xA9, 0xB9, 0xA4, 0x29, 0xD7, 0xF7, 0xD2, 0xB5, 0x2C, 0xB2, 0xA2, 0xB7, 0xE2, 0xCB ,0xF8 ,0x20,
            0x20, 0xD5, 0xBE, 0xD6, 0xC1, 0x20, 0x20, 0x20, 0xD5, 0xBE, 0xBC, 0xE4, 0x20, 0x20, 0xD0, 0xD0, 0xCF, 0xDF, 0xA1 ,0xA3,
            0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xA3, 0xA8, 0x31, 0xA3, 0xA9, 0xD7, 0xBC ,0xD0 ,0xED, 0xB9 ,0xA4, 0xCE,
            0xF1, 0xB2, 0xBF, 0xC3, 0xC5, 0xD4, 0xDA, 0x20, 0x20, 0x20, 0x6B, 0x6D, 0x20, 0x20, 0x20 ,0x6D ,0xD6, 0xC1 ,0x20, 0x20,
            0x20, 0x6B, 0x6D, 0x20, 0x20, 0x20, 0x6D, 0xB4, 0xA6, 0xCA, 0xA9, 0xB9, 0xA4, 0xA1, 0xA3, 0x0D, 0x0A, 0x20, 0x20, 0x20,
            0x20, 0x20, 0xA3, 0xA8, 0x32, 0xA3, 0xA9, 0xD7, 0xBC, 0xD0, 0xED, 0xB9, 0xA9, 0xB5, 0xE7, 0xB2 ,0xBF, 0xC3, 0xC5, 0xD4,
            0xDA, 0x20, 0x20, 0x20, 0x6B, 0x6D, 0x20, 0x20, 0x20, 0x6D, 0xD6, 0xC1, 0x20, 0x20, 0x20, 0x6B, 0x6D, 0x20, 0x20, 0x20,
            0x20, 0x6D, 0xB4, 0xA6, 0xCA, 0xA9, 0xB9, 0xA4, 0xA1, 0xA3, 0x0D, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0xA3, 0xA8, 0x33,
            0xA3, 0xA9, 0xD7, 0xBC, 0xD0, 0xED, 0xA1, 0xA1, 0xA1, 0xA1, 0xB2, 0xBF, 0xC3, 0xC5, 0xD4, 0xDA, 0x20, 0x20, 0x20, 0x6B,
            0x6D, 0x20, 0x20, 0x20, 0x6D, 0xD6, 0xC1, 0x20, 0x20, 0x20, 0x6B, 0x6D ,0x20 ,0x00, 0x00 ,0x00, 0xFE, 0xFE, 0xFE, 0xFE
        };
        DDML[6] = pStation->getStationID();
        RecvStagePlanDataOfTraining(DDML, 500, true);
        return;
    }
}

////清除车站的所有数据（是否全部车站）
//void MyDoc::ResetStationInfo(MyStation *pStation, bool bAll)
//{
//    //Mutex.lock();
//    //清除所有数据
//    vectRecvDisOrder.clear();//调度命令
//    pStation->m_ArrayRouteOrder.clear();
//    pStation->m_ArrayTrafficLog.clear();
//    pStation->m_ArrayStagePlan.clear();
//    pStation->m_ArrayStagePlanChg.clear();
//    pStation->m_ArrayTrain.clear();
//    pStation->m_ArrayDisOrderRecv.clear();
//    pStation->m_ArrayDisOrderDisp.clear();
//    pStation->m_ArrayDisOrderLocomot.clear();
//    pStation->ResetStationDevStatus();
//    pStation->clearCmdCountDown();//清除终端闪烁
//    pStation->vectCheckResult.clear();
//    if(bAll)
//    {
//        //数据库访问
//        m_pDataAccess->ResetAllTable();//数据库清除全部
//        ////保持操作数据库一致，所有站都清空
//        //for(int i=0; i<vectMyStation.count(); i++)
//        //{
//        //    //发送同步消息
//        //    SendUpdateDataMsg(vectMyStation[i], UPDATETYPE_ALL);
//        //}
//    }
//    else
//    {
//        //数据库访问
//        m_pDataAccess->ResetStationTable(pStation);//数据库清除本站
//        ////发送同步消息
//        //SendUpdateDataMsg(pStation, UPDATETYPE_ALL);
//    }
//    //Mutex.unlock();
//}

//发送更新数据消息（给所有连接终端-CTC、占线板）
void MyDoc::SendUpdateDataMsg(MyStation *pStation, int _type, int currCtcIndex, int currZxbIndex)
{
    int nLength = 60;
    QByteArray byArrayUDPDate;
    byArrayUDPDate.append(nLength, char(0));//添加60个字节并全部置零
    for(int i = 0; i < 4; i++)
    {
        byArrayUDPDate[i] = 0xEF;
        byArrayUDPDate[nLength - i -1] = 0xFE;
    }
    byArrayUDPDate[4] = nLength;//帧长度
    byArrayUDPDate[5] = nLength<<8;//帧长度
    byArrayUDPDate[6] = pStation->getStationID();
    byArrayUDPDate[8] = DATATYPE_SERVER;
    byArrayUDPDate[9] = FUNCTYPE_UPDATE;//分类信息码
    byArrayUDPDate[10] = _type;//子分类信息码
    //发给本站所有的CTC、占线板终端
    {
        SendUDPDataToCTC(pStation, byArrayUDPDate, nLength, currCtcIndex);
        SendUDPDataToBoard(pStation, byArrayUDPDate, nLength, currZxbIndex);
    }
}

//教师机数据处理（阶段计划，调度命令等）
void MyDoc::RecvStagePlanDataOfTraining(unsigned char *recvArray, int recvlen, bool bAutoSign)
{
    //车站id
    int nStaid = recvArray[6];
    //判断id是否已配置
    MyStation* pStation = getMyStationByStaIDInStaArray(nStaid);
    if (!pStation)
    {
        return;
    }
    //数据类型
    int revcType = (int)recvArray[9];

    //调度命令
    if(revcType == 0x99)
    {
        //Mutex.lock();//培训软件下发计划时会卡死
        DispatchOrderStation* disOrderRecv = pStation->updateDisorderSta(recvArray, recvlen);
        //CDisOrderRecv disOrderRecv = pStation->m_ArrayDisOrderRecv[pStation->m_ArrayDisOrderRecv.count()-1];
        disOrderRecv->order_id = m_pDataAccess->InsertDisOrderRecv(disOrderRecv);
        //全局添加
        vectRecvDisOrder.append(disOrderRecv);
        //本站添加
        pStation->m_ArrayDisOrderRecv.append(disOrderRecv);
        //发送同步数据消息
        SendUpdateDataMsg(pStation, UPDATETYPE_DDML);
        //Mutex.unlock();
    }
    //阶段计划
    else if(revcType == 0x33)
    {
        //Mutex.lock();
        //阶段计划详情
        StagePlan* stagePlan = pStation->updateStagePlan(recvArray, recvlen);
        //阶段计划大计划
        Stage* stage = new Stage();
        stage->station_id = pStation->getStationID();
        stage->m_nPlanNum = stagePlan->m_nPlanNumber;
        stage->m_timRecv = QDateTime::currentDateTime();
        //查数据库
        stagePlan->plan_id = m_pDataAccess->SelectPlanId(stage);
        //还没有父表内容，则插入
        if(stagePlan->plan_id <= 0)
        {
            stagePlan->plan_id = m_pDataAccess->InsetStage(stage);
        }
        stagePlan->detail_id = m_pDataAccess->InsetStagePlanDetail(stagePlan);
        pStation->m_ArrayStagePlan.append(stagePlan);
        //发送同步数据消息
        SendUpdateDataMsg(pStation, UPDATETYPE_JDJH);
        //qDebug()<<"bAutoSign="<<bAutoSign;
        //自动签收
        pStation->SignStagePlan(bAutoSign);//false
        //Mutex.unlock();
    }
    //站场重置
    else if(revcType == 0xEA)
    {
        //ResetStationInfo(pStation, false);//true
        resetStationInfo(pStation,false);//true
    }
    //控制模式转换申请结果
    else if(revcType == 0x2A)
    {
        int targtMode = (int)recvArray[11];
        int result    = (int)recvArray[12];
        int ctcMode;
        switch(pStation->ModalSelect.nModeState)
        {
        case 0: ctcMode = CTC_MODE_CENTER; break;
        case 1: ctcMode = CTC_MODE_STATION; break;
        case 2: ctcMode = CTC_MODE_NORMAL; break;
        //default: nMode = 1; break;
        }

        if(/*(currMode == ctcMode) && (currMode != targtMode) &&*/
            (ctcMode != targtMode) && (result == AGREE) )
        {
            int nMode;
            switch(targtMode)
            {
            case CTC_MODE_CENTER:  nMode = 0; break;
            case CTC_MODE_STATION: nMode = 1; break;
            case CTC_MODE_NORMAL:  nMode = 2; break;
            default: nMode = 1; break;
            }
            //0 中心控制 1 车站控制 2 分散自律
            pStation->ModalSelect.nModeState = nMode;
            pStation->ModalSelect.nRequestMode = -1;
            m_pDataAccess->UpdateStationInfo(pStation);
            //发送同步消息
            sendUpdateDataMsg(pStation, UPDATETYPE_KZMS);
            //当前模式-0 中心控制 1 车站控制 2 分散自律
            if(1==nMode)
            {
                QString msg = QString("%1的车站控制模式由中心控制转为车站控制").arg(pStation->getStationName());
                //发送报警提示信息
                pStation->sendWarningMsgToCTC(3,1,msg);
            }
        }
    }
    //时钟同步
    else if(revcType == 0xAA)
    {
        int nCount = 10;
        SYSTEMTIME sysTime;
        GetSystemTime(&sysTime);
        sysTime.wYear = (recvArray[nCount] | recvArray[nCount+1]<<8);
        sysTime.wMonth = recvArray[nCount+2];
        sysTime.wDay  = recvArray[nCount+3];
        sysTime.wHour = recvArray[nCount+4];
        sysTime.wMinute = recvArray[nCount+5];
        sysTime.wSecond = recvArray[nCount+6];
        sysTime.wDayOfWeek = -1;
        sysTime.wMilliseconds = 0;
        SetLocalTime(&sysTime);
    }
    //一键关机
    else if(revcType == 0xAB)
    {
        system("shutdown -s -t 00");
    }

    //(必须在存入数据库后再发送，否则CTC从数据库获取不到数据)
    //向车站CTC转发教师机命令信息
    {
        unsigned char newArray[2048];
        memset(newArray,0,2048);
        memcpy(newArray, recvArray, recvlen);
        newArray[8] = DATATYPE_TCC;
        //数据帧转换
        QByteArray qSendArray = UnsignedCharToQByteArray(newArray, recvlen);
        SendUDPDataToCTC(pStation, qSendArray, recvlen);
        SendUDPDataToBoard(pStation, qSendArray, recvlen);
    }

}

