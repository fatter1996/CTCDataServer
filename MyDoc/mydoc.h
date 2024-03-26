#ifndef MYDOC_H
#define MYDOC_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include "MyStation/mystation.h"
#include "SocketPro/socketudp.h"
#include "SocketPro/servertcp.h"
#include "SocketPro/sockettcp.h"
#include "DataAccess/dataaccess.h"
#include "Log/log.h"
#include <QtConcurrent>
#include <QFuture>

/*
*主文档类
*/
class MyDoc : public QObject
{
    Q_OBJECT
public:
    explicit MyDoc(QObject *parent = nullptr);

public:
    ~MyDoc();
    static MyDoc& GetInstance()
    {
        static MyDoc s_instance;
        return s_instance;
    }
    MyDoc(MyDoc const&) = delete ;
    MyDoc& operator= (MyDoc const&) = delete ;

public:
    //获取MD5加密值
    QString GetMd5(const QString &inputValue);

public:
    //资源互斥锁，用来保护数据防止被篡改
    QMutex Mutex;
    QDateTime SysStartedDataTime;//系统启动时的时间
    long SysLifeSeconds;//系统自启动以来已运行时间（秒）

public:
    //车站数组
    QVector<MyStation*> vectMyStation;
    //车站数组
    QVector<QThread*> vectThread;
    //当前车站的索引值
    int currStaIndex;
    ////主通信是否UDP模式（兼容旧版模式）
    //bool bUdpMode;
    //udp通信服务端（LS+CTC+ZXB主通信）
    SocketUDP* socketUDP;
    //udp通信服务端（教师机）
    SocketUDP* socketUDPTeacher;
    //udp通信服务端（培训软件）
    SocketUDP* socketUDPTrain;
    //tcp通信服务端（LS+CTC+ZXB主通信）
    ServerTCP* serverTcp;
    //tcp通信服务端（文字显示信息）
    SocketTCP* socketWatchTCP;
    //本机Server端口1（LS+CTC+ZXB）
    int localServerPort1;
    //本机Server端口2（CTC+ZXB）
    int localServerPort2;
    //本机Server端口3（文字显示信息）
    int teacherWatchPort;
    //教师机地址
    QString teacherAddr;
    //教师机端口
    int teacherPort;
    //本机教师机通信端口
    int localTeacherPort;
    //本机培训软件链接端口
    int localTrainPort;

    //数据库地址账号信息
    QString databaseIP;
    QString databaseName;
    QString databaseUser;
    QString databasePassWord;
    int databasePort;

    //服务终端定时器
    QTimer *pTimerServer;
    int nElapsed;

    //数据库访问接口
    DataAccess* m_pDataAccess;

public:
    //集控台接收的调度命令
    QVector<DispatchOrderStation*> vectRecvDisOrder;

public:
    //********** 培训软件 **********
    typedef  struct
    {
        QString FunStr;     //功能码
        QString ParaStr;    //参数码  分割前
        QString SubFunStr;       //子功能码
        QString TipShowStr;     //提示内容  比如：箭头旁边的文字
        QString DevName;    //每个操作标识对应的操作设备名称
        unsigned int DevCode;  //设备号
        bool SetFlag;
        void init()
        {
            FunStr= "";
            ParaStr= "";
            SubFunStr= "";
            TipShowStr= "";
            DevName="";
            DevCode=0xffff;
            SetFlag = false;
        }
    }OrderStr;
    QVector<OrderStr> v_OrderArray;
    unsigned char type;  //0 -没有进去场景设置模式   0x55-培训模式  0xaa-考试模式
    unsigned int PlanCode;
    char Num;
    //接收培训数据-分析数据
    void RecvTrainingData_AnalyUdpData(MyStation* pStation, unsigned char *Rec, int recvlen);
    //接收培训数据-调度命令解析函数
    void RecvTrainingData_DDMLAnalysis(MyStation* pStation, unsigned char *Rec_data);
    void ManageSpecialOrder(MyStation* pStation);
    QString getNameOfDevNode(MyStation* pStation, unsigned int devnode);//培训软件-根据设备编号获取设备名称
    unsigned int getDevNodeOfName(MyStation* pStation, QString strName);//培训软件-根据设备名称获取设备编号
    void JDJHAnalysis(MyStation* pStation, unsigned char *Rec_data); //培训软件-阶段计划解析函数
    void DrawSence();
    void DrawSence(MyStation* pStation, OrderStr ExapleStr,int index);//根据场景命令绘制界面演示信息

    ////清除车站的所有数据（是否全部车站）
    //void ResetStationInfo(MyStation* pStation, bool bAll=false);
    //发送更新数据消息（给所有连接终端-CTC、占线板）
    void SendUpdateDataMsg(MyStation *pStation, int _type=0x00, int currCtcIndex = -1, int currZxbIndex = -1);
    //培训软件数据处理（阶段计划，调度命令等）（是否自动签收）
    void RecvStagePlanDataOfTraining(unsigned char *recvArray, int recvlen, bool bAutoSign = false);


private:
    //初始化所有数据
    void initAllData();
    //读取全局数据
    void readGlobalData();
    //初始化通信
    void initNetCom();
    //初始化数据库
    void initMySQL();
    //和数据库定时保持连接
    void KeepDatabaseConnAlive();
    //各站获取自己的数据
    void GetAllStationDataFromDatabase();
    //初始化全局逻辑
    void initGlobalLogic();
    //停止全局逻辑
    void stopGlobalLogic();
    //初始化定时器
    void initTimer();

public:
    //界面绘制
    void Draw(QPainter *painter, long nElapsed, double nDiploid);

    //根据站名在车站数组中获取索引
    int getIndexByStaNameInStaArray(QString strStation);
    //根据站名在车站数组中获取车站指针
    MyStation* getMyStationByStaNameInStaArray(QString strStation);
    //根据id在车站数组中获取车站指针
    MyStation* getMyStationByStaIDInStaArray(int id);
    //根据索引在车站数组中获取车站指针
    MyStation* getMyStationByIndexInStaArray(int idx);
    //在车站数组中获取当前车站指针
    MyStation* getCurrMyStationInStaArray();
    //根据站名设置当前车站索引
    int setCurrIndexByStaName(QString strStation);

signals:
    //信号-发送数据给主通道-UDP
    void sendDataToMainSignal(QByteArray dataArray,QString OppAdress,int OppProt,int len);
    //信号-发送数据给教师机通道
    void sendDataToTeacherSignal(QByteArray dataArray,QString OppAdress,int OppProt,int len);
    //信号-发送数据给培训软件通道
    void sendDataToTrainingSignal(QByteArray dataArray,QString OppAdress,int OppProt,int len);

public:
    QVector<TrainNumType*> v_TrainNumType;
    QVector<QString> v_TrainRunType;
    // 读取列车运行类型配置文件
    void ReadTrainNumTypeTXT();
    // 各站列车运行类型数据同步初始化
    void InitStationTrainNumType();

public slots:
    //定时器槽
    void timerOutSlot();


/************ mydoccomdata.cpp函数定义 ************/
public slots:
    //解析LS+CTC+占线板的socket通道数据-UDP
    void receiveAllLSCTCDataSlot(QByteArray dataArray,QString clientAdd, int clientPort);
    //解析教师机的socket通道数据
    void receiveTeacherDataSlot(QByteArray array,QString client_add, int client_port);
    //解析培训软件的socket通道数据
    void receiveTrainingDataSlot(QByteArray array,QString client_add, int client_port);

    //槽-发送数据给教师机
    void sendDataToTeacherSlot(MyStation* pMyStation, QByteArray dataArray,int len);
    //槽-发送数据给联锁
    void sendDataToLSSlot(MyStation* pMyStation, QByteArray dataArray,int len);
    //槽-发送数据给CTC
    void sendDataToCTCSlot(MyStation* pMyStation, QByteArray dataArray,int len, int currCtcIndex=-1);
    //槽-发送数据给占线板
    void sendDataToBoardSlot(MyStation* pMyStation, QByteArray dataArray,int len);
    //槽-发送数据给集控台
    void sendDataToJKSlot(MyStation* pMyStation, QByteArray dataArray,int len, int currSoftIndex=-1);
    //槽-发送数据给占线图
    void sendDataToZXTSlot(MyStation* pMyStation, QByteArray dataArray,int len, int currSoftIndex=-1);

    //槽--发送更新数据消息（给所有连接终端-CTC、占线板）
    void sendUpdateDataMsgSlot(MyStation *pStation, int _type, int currCtcIndex, int currZxbIndex);

    //槽--更新邻站报点(邻站id，车次，报点类型-0x22到达，0x11出发，0x33通过，报点时间)
    void UpdateLZReportTimeSlot(int lzId,QString checi,int type, QDateTime dateTime);

public:
//    //发送心跳信息给教师机
//    void sendHeartBeatToJSJ();
    //发送数据至教师机
    void sendDataToJSJ(QByteArray pSendDate, int nLength);
    //发送状态给所有车站CTC（各终端的站间透明使用）
    void sendStatusToAllStationCTC(QByteArray pSendDate, int nLength);
    //发送状态给所有车站的Soft
    void sendStatusToAllStationSoft(QByteArray pSendDate, int nLength);
    //向pStation车站的联锁发送数据
    void sendDataToLS(MyStation* pStation, QByteArray pSendDate, int nLength);
    //向pStation车站所有的CTC终端发送数据
    void sendDataToCTC(MyStation* pStation, QByteArray pSendDate, int nLength, int currClientIndex = -1);
    //向pStation车站所有的占线板终端发送数据
    void sendDataToBoard(MyStation* pStation, QByteArray pSendDate, int nLength, int currClientIndex = -1);
    //向pStation车站所有的集控终端发送数据
    void sendDataToJK(MyStation* pStation, QByteArray pSendDate, int nLength, int currClientIndex = -1);
    //向pStation车站所有的占线图终端发送数据
    void sendDataToZXT(MyStation* pStation, QByteArray pSendDate, int nLength, int currClientIndex = -1);
    //发送更新数据消息（给所有连接终端-CTC、占线板）
    void sendUpdateDataMsg(MyStation *pStation, int _type=0x00, int currCtcIndex = -1, int currZxbIndex = -1);

public:
    //接收联锁数据处理
    void recvAllLSData(QByteArray recvArray, int recvlen, QString clientAdd, int clientPort);
    //接受的CTC+占线板数据处理-UDP
    void AnalyCTCData(MyStation* pStation, unsigned char *recvArray, int recvlen, int nClientIndex= -1);
    //接收CTC数据处理-UDP
    void recvAllCTCData(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort);
    //接收所有占线板终端数据
    void recvAllBoardData(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort);
    //接收所有集控终端数据
    void recvAllJKData(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort);
    //接收所有占线图终端数据
    void recvAllZXTData(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort);

    //接收CTC数据-用户登录
    void recvCTCDataUserLogin(MyStation* pStation, unsigned char *recvArray, int recvlen, int currCtcIndex = -1);

    //接收的教师机数据处理（阶段计划，调度命令等）（是否自动签收）
    void recvTeacherData(unsigned char *recvArray, int recvlen, bool bAutoSign = false);
    //上次重置所有站场的时间
    QDateTime m_timeLastResetStationAll;
    //清除车站的所有数据（是否全部车站）
    void resetStationInfo(MyStation* pStation, bool bAll=false);
    //重置数据库
    void resetDataBase();
    //接收培训软件终端数据
    void RecvTrainningData(unsigned char *recvArray, int recvlen, QString client_add, int client_port);
    //向pStation车站所有的CTC终端发送数据
    void SendUDPDataToCTC(MyStation* pStation, QByteArray pSendDate, int nLength, int currClientIndex = -1);
    //向pStation车站所有的占线板终端发送数据
    void SendUDPDataToBoard(MyStation* pStation, QByteArray pSendDate, int nLength, int currClientIndex = -1);

    //阶段计划接收回执发送给教师机
    void SendStagePlanReceipt(MyStation* pStation, StagePlan* pStagePlan);

/************ mydoccomdata2.cpp函数定义 ************/
public:
    //将接收到的数据根据帧头帧位进行分割并返回分割后的二维数组
    QVector<QByteArray> SplitReceiveData_SameHeadTail(QByteArray recvArray);

signals:
    //信号-发送数据给主通道-TCP
    void sendDataToMainSignal2(QByteArray dataArray,QString OppAdress,int OppProt,int len);
    void sendTextDataToJSJ(QByteArray dataArray, int len);
public slots:
    //TCP断开槽
    void tcpClientDisconnectedSlot(QString clientAdd, int clientPort);
    //解析所有终端的socket通道数据-TCP
    void receiveAllDataSlot2(QByteArray dataArray,QString clientAdd, int clientPort);
    //槽-发送数据给CTC-TCP通道
    void sendDataToCTCSlot2(MyStation* pMyStation, QByteArray dataArray,int len, int currCtcIndex);
    //槽-发送数据给占线板-TCP通道
    void sendDataToBoardSlot2(MyStation* pMyStation, QByteArray dataArray,int len, int currIndex);
    //槽-发送数据给集控JK-TCP通道
    void sendDataToJKSlot2(MyStation* pMyStation, QByteArray dataArray,int len, int currIndex);
    //槽-发送数据给占线图ZXT-TCP通道
    void sendDataToZXTSlot2(MyStation* pMyStation, QByteArray dataArray,int len, int currIndex);
public:
    //接收CTC数据处理-TCP数据同步
    void recvAllCTCData2(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort);
    //接收占线板数据处理-TCP数据同步
    void recvAllBoardData2(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort);
    //接收集控台数据处理-TCP数据同步
    void recvAllJKData2(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort);
    //接收占线图数据处理-TCP数据同步
    void recvAllZXTData2(unsigned char *recvArray, int recvlen, QString clientAdd, int clientPort);


};

#endif // MYDOC_H
