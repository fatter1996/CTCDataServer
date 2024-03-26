#ifndef MYSTATION_H
#define MYSTATION_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "BaseDataPro/station.h"
#include "automaticblock.h"
#include "semiautomaticblock.h"
#include "axlecounter.h"
#include "interlockroute.h"
#include "xhbtn.h"
#include "sysuser.h"
#include "stage.h"
#include "stageplan.h"
#include "trainrouteorder.h"
#include "trafficlog.h"
#include "train.h"
#include "dispatchorderstation.h"
#include "dispatchorderdispatcher.h"
#include "dispatchorderlocomotive.h"
#include "routeprewnd.h"
#include "limitspeed.h"
#include "interfieldconnection.h"

#pragma execution_character_set("utf-8")

#include "DataAccess/dataaccess.h"
//class DataAccess;

//自定义车站类，车站的业务逻辑类
class MyStation : public CStation
{
    Q_OBJECT
public:
    MyStation();
    ~MyStation();

public:
    //资源互斥锁，用来保护数据防止被篡改
    QMutex Mutex;

public:
    //数据库访问接口
    DataAccess* m_pDataAccess;
    //设置数据访问接口
    void setDataAccess(DataAccess* _dataAccess);

public:
    //线程是否启动
    bool isThreadStart;
    //线程计时器
    int  ticksOfThread;

private:
    //通信更新判断
    void UpdateConnStatus();
public slots:
    //开始工作
    void startWorkSlot();
signals:
    //结束工作
    void endWorkSignal();
    //信号-发送数据给教师机
    void sendDataToTeacherSignal(MyStation* pMyStation, QByteArray dataArray,int len);
    //信号-发送数据给联锁
    void sendDataToLSSignal(MyStation* pMyStation, QByteArray dataArray,int len);
    //信号-发送数据给CTC
    void sendDataToCTCSignal(MyStation* pMyStation, QByteArray dataArray,int len, int currCtcIndex=-1);
    //信号-发送数据给占线板
    void sendDataToBoardSignal(MyStation* pMyStation, QByteArray dataArray,int len);
    //信号-发送数据给集控台
    void sendDataToJKSignal(MyStation* pMyStation, QByteArray dataArray,int len, int currSoftIndex=-1);
    //信号-发送数据给占线图
    void sendDataToZXTSignal(MyStation* pMyStation, QByteArray dataArray,int len, int currSoftIndex=-1);
    //信号-发送更新数据消息（给所有连接终端-CTC、占线板）
    void sendUpdateDataMsgSignal(MyStation *pStation, int _type, int currCtcIndex, int currZxbIndex);

public:
    //绘制站场
    void draw(QPainter *painter,long nElapsed, double nDiploid=0.0f);
private:
    //绘制CTC站场
    void drawMyStation(QPainter *painter,long nElapsed, double nDiploid=0.0f);
    //向教师机发送心跳信息
    void sendHeartBeatToTeacher();
    //发送更新数据消息（给所有连接终端-CTC、占线板）
    void sendUpdateDataMsg(MyStation *pStation, int _type=0x00, int currCtcIndex = -1, int currZxbIndex = -1);

public:
    QVector<SysUser*> vectLoginUser;//登录的用户信息
//    int stationID = 0;//车站id
//    QString stationName;//车站名称
    QVector<DispatchOrderStation*> m_ArrayDisOrderRecv;//本站接收的调度命令
    QVector<DispatchOrderDispatcher*> m_ArrayDisOrderDisp;//调度台调度命令
    QVector<DispatchOrderLocomotive*> m_ArrayDisOrderLocomot;//机车调度命令
    QVector<StagePlan*> m_ArrayStagePlan;//教师机阶段计划
    QVector<StagePlan*> m_ArrayStagePlanChg;//CTC修改的计划
    QVector<TrafficLog*> m_ArrayTrafficLog;//行车日志
    QVector<TrafficLog*> m_ArrayTrafficLogChg;//行车日志
    QVector<TrainRouteOrder*> m_ArrayRouteOrder;//列车进路序列数组
    QVector<CGD*> m_ArrayGDAntiSlip;//股道防溜
    QVector<Train*> m_ArrayTrain;//实际列车
    QVector<Train*> m_ArrayTrainManType;//人工标记的车次
    QVector<AutomaticBlock*> vectAutomaticBlock;//自动闭塞
    QVector<SemiAutomaticBlock*> vectSemiAutomaticBlock;//半自动闭塞
    QVector<AxleCounter*> vectAxleCounter;//计轴
    QVector<InterfieldConnection*> vectInterfieldConnection;//场联
    QVector<InterlockRoute*> vectInterlockRoute;//联锁进路表
    QVector<XhBtn*> vectXhBtn;//独立的信号按钮
    QVector<RoutePreWnd*> vectRoutePreWnd;//进路预告窗
    QVector<CGD*> vectGDAttr;//（防错办）股道属性
    QVector<CXHD*> vectGatewayAttr;//（防错办）出入口属性
    QVector<FixedRoute*> vectFixedRoute;//（防错办）列车固定径路信息
    QVector<TrainNumTrack*> vectTrainNumTrack;//（防错办）车次股道列表信息
    QVector<LimitSpeed*> vectLimitSpeed;//临时限速数组

    //通信终端最大计数器
    const int commBreakTimes = 4*6;//4*6=6秒//4*3=3秒  10*3
    //******** 本站联锁地址端口 ********
    QString lsAddr;//IP地址
    int lsPort;//端口
    int m_nTimeLSBreak;//本站LS终端通信中断计时器
    bool isLSConnected;//本站LS终端是否连接
    int nLastGDCode = -1;
    int nNowGDCode = -1;
    //******** 本站CTC地址端口-UDP ********
    QString ctcAddr[10];//IP地址
    int ctcPort[10];//端口
    int  m_nTimeCTCBreak[10];//本站CTC终端通信中断计时器
    bool isCTCConnected[10];//本站CTC终端是否连接
    //******** 本站CTC地址端口-TCP ********
    QString ctcAddr2[10];//IP地址
    int ctcPort2[10];//端口
    int  m_nTimeCTCBreak2[10];//本站CTC终端通信中断计时器
    bool isCTCConnected2[10];//本站CTC终端是否连接

    //******** 本站占线板地址端口-UDP ********
    QString zxbAddr[10];//IP地址
    int zxbPort[10];//端口
    int m_nTimeBoardBreak[10];//本站占线板终端通信中断计时器
    bool isBoardConnected[10];//本站占线板终端是否连接
    //******** 本站占线板地址端口-TCP ********
    QString zxbAddr2[10];//IP地址
    int zxbPort2[10];//端口
    int m_nTimeBoardBreak2[10];//本站占线板终端通信中断计时器
    bool isBoardConnected2[10];//本站占线板终端是否连接

    //******** 本站培训软件端口 ********
    QString trainningAddr;//IP地址
    int trainningPort;//端口

    //******** 本站集控终端地址端口-UDP ********
    QString jkAddr[10];//IP地址
    int jkPort[10];//端口
    int m_nTimeJKBreak[10];//本站集控终端通信中断计时器
    bool isJKConnected[10];//本站集控终端是否连接
    //******** 本站集控终端地址端口-TCP ********
    QString jkAddr2[10];//IP地址
    int jkPort2[10];//端口
    int m_nTimeJKBreak2[10];//本站集控终端通信中断计时器
    bool isJKConnected2[10];//本站集控终端是否连接

    //******** 本站占线图地址端口-UDP ********
    QString zxtAddr[10];//IP地址
    int zxtPort[10];//端口
    int m_nTimeZXTBreak[10];//本站集控终端通信中断计时器
    bool isZXTConnected[10];//本站集控终端是否连接
    //******** 本站集控终端地址端口-TCP ********
    QString zxtAddr2[10];//IP地址
    int zxtPort2[10];//端口
    int m_nTimeZXTBreak2[10];//本站集控终端通信中断计时器
    bool isZXTConnected2[10];//本站集控终端是否连接

public:
    typedef struct  //股道可到达的延续终端
    {
       QStringList strArrGD; //股道列表
       QStringList strArrYXEnd;//延续终d列表
       QString strDefaultEnd; //默认延续终端
    }GDYXJL_T;
    typedef struct            //延续进路定义
    {
       QString  strXHDbegin; //延续进路始端
       int      nGDYXJLCount;
       GDYXJL_T GD_YXJL[50];//股道们相应的延续终d
    }YXJLGroup_T;

    typedef struct           //接发车口定义
    {
       QString strJCKName;//接车口信号机名称//CString strName;
       int     nJCKCode;  //接车口设备号//int     nCode;
       QString strFCKName;//发车口信号机名称
       int     nFCKCode;  //发车口设备号
       QString strDirectSta;//CString strDirectSta; //接发车口方向车站名称（车站名称）
       QString strJCKjucJJQD;//接车口关联的接近区段（1个）
       QStringList strArrJckJJQD;//接车口关联的接近区段数组
       QString     strFCKjucLQQD;//发车口关联的接近区段（1个）
       QStringList strArrFckLQQD;//发车口关联的离去区段数组
       QStringList strArrJckJJQD_Temp;//接车口关联的接近区段数组
       int  nLZStationId;//邻站车站id
    }JFCK_T;

    typedef struct //ConfigInfo //车站配置信息定义结构体
    {
        bool isHavePXRJ;   //2020.7.9-BJT-记录是否有培训软件
//        bool isHaveDDFZ;   //2020.7.17-BJT-记录是否有调度仿真
        //QString strStaName;       //车站名
//        int     nCTCDevType;//CTC单机版0x00，值班员终端0x01,信号员终端0x02
//        int     nBoardType;       //T3Term占线板单机版0x00，值班员终端0x01,信号员终端0x02
        //bool    bGaoTie;          //高铁站标志
        bool    bStaSXLORR;       //上下行方向,站场下行方向对应左边1，否则为0
        bool    bChgModeNeedJSJ;  //控制模式转换是否需要教师机（中心）同意
        int     YXJLBeginNum;
        YXJLGroup_T YXJLGroup[40];//延续进路始端和默认终端信息
//        int     XF_nCode;        //XF/XN...（下反/下逆等）十进制设备编号
//        int     X_nCode;         //下行接车信号机十进制设备编号
//        QString X_JuncXHD;       //下行接车信号机名称
//        int     SF_nCode;        //SF/SN...（上反/上逆等）十进制设备编号
//        int     S_nCode;         //上行接车信号机十进制设备编号
//        QString S_JuncXHD;       //上行接车信号机名称

        int JCKCount;
        JFCK_T JFCKInfo[40];//接发车口信息
        int BTJLCount;//有变通进路的进路数目
        QString BTJLArray[300];//(基本进路|变通进路|变通进路)

        int  RouteDeleteSeconds;//进路序列删除时间秒
        QStringList XianLuSuoXHDArray;//线路所信号机 //BOOL bXianLuSuo;//线路所
        bool bQJAllLock;//区间封锁单个或全部
        bool bXHDDSBJStatus;//站场状态是否包含信号机灯丝报警状态
        //bool HaveCLPZFlag;//场联配置//机务段配置相关：0为无机务段，1为有机务段
        bool bZDBSLightExState;//自动闭塞包含指示灯扩展状态
        QStringList centrSwitchList;//中岔列表
    }StaConfig_T;
    StaConfig_T StaConfigInfo;

    QVector<YCJS> vectYCJS;//腰岔解锁
    QVector<GDQR> vectGDQR;//股道确认空闲
    XXHDBtnCmdType mXXHDBtnCmdType;//虚信号机按钮命令配置

//    struct StationLiansuo
//    {
//       QString strGDName;//股道名称
//       QString strGDName1;//股道名称（可能为无岔或道岔区段，一般为有中岔的股道配置）
//       int nCode;//股道设备号
//       QString strLeftNode;//左侧信号机名称
//       QStringList strArrLeftFCK;//左边的发车口
//       QString strRightNode;//右侧信号机名称
//       QStringList strArrRightFCK;//右边的发车口
//    }StationGDNode[100];
//    //股道数量
//    int m_nStationGDNode;
    QVector<StationGDNode> vectStationGDNode;
    bool bInitGDNodeByConfig;//股道数据根据配置文件初始化，检查是否存在文件“GDConfig.txt”

    QStringList m_ArrGD;
    //拥有进路权限（值班员默认有，信号员默认无）
    bool m_bHaveRoutePermissions;

    //车站状态模式
    struct StationModal
    {
       QString strName;
       int nStateSelect;    //0 手工排路 1 按图排路（生成进路序列）
       bool nPlanCtrl;		//计划控制
       bool nAgrRequest;	//同意转换
       int nModeState;		//当前模式-0 中心控制 1 车站控制 2 分散自律
       int nRequestMode;	//请求模式-0 中心控制 1 车站控制 2 分散自律(请求转换的模式)   //1 中心控制 2 车站控制 3 分散自律(请求转换的模式) （0为默认值，处理不便）
    }ModalSelect;//StationModalSelect
    int nOldModeState;//上一次的控制模式
    bool m_bModeChanged;
    bool m_nFCZKMode; //非常站控
    int  RoutePermit;  //进路权限 0无权限，1权限在CTC，2权限在占线板
    bool AutoSignStage;//阶段计划自动签收标志

    bool m_bAllowZH;//允许转回
    bool m_bSDJS;//上电解锁
    bool m_nDCSXAllLock;//咽喉总锁状态上行(S引导总锁)
    bool m_nDCXXAllLock;//咽喉总锁状态下行(X引导总锁)
    int  m_nPDJS180;//坡道解锁倒计时180秒计数
    int  m_nSRJ180;//S人解180s
    int  m_nSRJ60;//S人解60s
    int  m_nSRJ30;//S人解30s
    int  m_nXRJ180;//X人解180s
    int  m_nXRJ60;//X人解60s
    int  m_nXRJ30;//X人解30s

    //用于处理组合进路办理时，联锁发送的进路回执为分段办理的信号机，导致无法匹配进路序列和更新状态
    //使用此配置文件则判断信号机的开放状态
    typedef struct
    {
       QString strRoute;//进路描述
       QVector<QString> vectXHD;//检查的信号机
    }RouteCheckXhd;
    //进路序列状态需要判断信号机
    bool m_bRouteStateCheckXHD;
    //进路序列信号机数据
    QVector<RouteCheckXhd> m_vectRouteCheckXhd;

private:
    //进路办理按下的按钮数组临时变量
    QStringList routeBtnTempArray;

    //临时限速配置信息（临时限速使用）
    typedef struct  //线路公里标定义
    {
        int     XLNum;  //线路号
        QString XLName;//线路名
        QString strStartGLB;//起始公里标字符串
        QString strFinishGLB;//结束公里标字符串
        int StartGLB;//起始公里标
        int FinishGLB;//结束公里标
        QVector<CBaseData *> DevArray;//线路设备数组（轨道+道岔）
    }XLGLB_T;
    QVector<XLGLB_T> XLGLBArray;//线路公里标数组

private:
//    //进路报警类型(临时变量)
//    int JLWarningType = 0;
//    //进路报警信息(临时变量)
//    QString JLWarningMsg = "";

public:
    //读取站场设备数据
    bool readStationDev(QString fileName, MyStation *pMyStation);
    //读取站场配置数据
    bool readStationConfig(QString fileName, MyStation *pMyStation);
    //读取联锁表
    bool readInterlockTable(QString fileName, MyStation *pStation);
    //读取股道配置
    bool readGDConfigInfo(QString fileName, MyStation *pStation);
    //读取进路序列信号机判断配置文件
    void readTempRouteXHD(QString fileName, MyStation *pStation);
    //读取线路公里标配置信息
    void readXLGLBConfigInfo(QString fileName, MyStation *pStation);
    //初始化数据
    void InitData();
    //根据设备号获取设备名称
    QString GetStrNameByCode(int nCode);
    //根据设备名称获取设备号
    int GetCodeByStrName(QString devName);
    //股道在站场数组中的索引
    int GetGDPosInzcArray(int nCode);
    //根据设备号获取设备
    CGDDC* GetDCQDByCode(int nCode);
    //信号机是否属于线路所
    bool IsXHDinXianLuSuo(QString _strXHD);
    //根据名称获取设备索引
    int GetIndexByStrName(QString devName);
    //设备是否是股道
    bool DevIsGDByCode(int nCode);
public:
    //接发车口数据初始化
    void initJFCK();
    //接近区段数据初始化
    void initJJQD();
    //获取（接车口）信号机关联的接近区段("XHD.h"循环调用修改)
    QString GetXHDjucJJQDName(QPoint xhdP1,QPoint xhdP2);
    //根据第一个接近区段名称查找下一个接近区段
    QString FindNextJJQD(int index, QString strJJQD);
    //在接车口接近区段能否找到该接近区段
    bool FindJJQDFromJCK(int index, QString strJJQD);
    //闭塞数据初始化
    void initBSData();
    //股道接点数据初始化
    void initGDNode();
    //根据坐标点获取关联的信号机名称
    CXHD* GetJucXHDbyPoint(QPoint point);
    //根据坐标点获取关联的道岔是否中岔，并继续向下查找关联信号机名称
    QString judgeCentrSwitchbyPoint(QPoint point, StationGDNode* gdNode, bool bLeft);
    //道岔是否为中岔
    bool isCentrSwitch(QString dcName);
    //根据坐标点获取关联的股道
    CGD* GetJucGDbyPoint(QPoint point);
    //变通进路的进路信息
    QVector<BTJL> vectBTJL;
    //初始化本站变通进路基础信息
    void initBTJLinfo();
    //初始化进路预告窗口信息
    void initRoutePreWnd();
    //组合进路的进路信息
    QVector<ZHJL> vectZHJL;
    QVector<TrainRouteOrder*> m_ArrayRouteOrderSonTemp;//列车进路子序列数组-临时变量
    //正线通过进路信息
    QVector<TGJL> vectTGJL;


public:
    //重置站场状态
    void ResetStationDevStatus();
    //联锁状态解析
    void updateDevStateOfLS(unsigned char *array);
    void updateGDDC_StateOfLS(CGDDC *gddc,int state);
    void updateXHD_StateOfLS(CXHD *xhd,int state1);
    void updateGD_StateOfLS(CGD *gd,int state);
    void updateQD_StateOfLS(CQD *qd,int state);
    void setQD_Color(CQD *qd,QColor color,int state=0);
    int updateDCJGXGStatus(unsigned char *array, int count);
    int updateXHDTimeCount(unsigned char *array, int count);
    int updateYXJSGDRQ(unsigned char *array, int count);//更新腰岔解锁+股道确认

    //联锁站场停电故障
    bool m_bLSPowerFailure = false;
    //判断是否联锁状态停电故障
    bool CheckStationPowerFailureOfLS(unsigned char *array);

    //处理功能按钮命令,处理成功则返回true
    bool dealFuncBtnCmd(unsigned char* StatusArray, int nLength);
    //根据设备号获取设备类型（道岔、信号、股道）
    QString getTypeByCode(int nCode);
    //设置所有区间封锁解封
    void setAllQJLock(bool bLock, QString strQj);
    //控制模式转换时取消分路不良区段确认空闲状态(站内区段、道岔)
    void CancleFlblKXFalg();
    //处理发给联锁的功能按钮命令,处理成功则返回0，否则返回错误码
    int dealFuncBtnCmdToLS(unsigned char* StatusArray, int nLength, int currCtcIndex=-1);

    ////发送站场状态给CTC、占线板终端
    //void sendStationStatusToCTC();
    //打包站场状态给CTC终端，返回数据包大小 发送站场状态给CTC、占线板终端
    QByteArray packStationStatusToCTC(); //QByteArray
    //打包进路预告窗的信息
    QByteArray packRoutePreWndToCTC(RoutePreWnd *_pRoutePreWnd); //QByteArray
    //制作数据帧头帧尾和关键信息
    void packHeadAndTail(QByteArray *pDataArray, int length);

public:
    //进路按钮按下
    void recvCTCData_RouteBtnClick(unsigned char *array, int count);
    //终端闪烁功能-清除所有按钮闪烁
    void clearXHDBtnFlash();
    //终端闪烁功能-闪烁下一个可按下信号机的按钮
    void setNextXHDBtnFlash(QStringList btnArray);
    //设置独立的信号机按钮闪烁
    bool setAloneXHDBtnFlash(QString _strName);
    //获取联锁表进路按钮的信号机名称
    QString getLSBRouteBtnXhdName(QString rtBtnName);
    //根据设备号获取设备名称独立信号按钮数组
    QString GetStrNameByCodeInXhBtnArray(int nCode, int btnType);
    //根据设备名称获取设备号
    //进路表中的区段是否空闲
    bool isQDKXInLSB(QStringList pQDArray);

public:
    //自动清除按钮功能
    int m_nCountDown;//倒计时
    bool m_bBeginCount;//是否开始计数
    QDateTime m_timeCmdRecord;//命令按下时的记录时间（15-（当前时间-命令记录时间）=倒计时）
    void setCmdCountDown();//设置命令倒计时（界面操作读秒提示，15s读秒结束后清除操作）
    void clearCmdCountDown();//清除命令倒计时读秒
    void computeCmdCountDown();//定时自动计算读秒
    void sendClearCmdToCTC();//发送清除命令给CTC
    void clearCmd();//进行清除命令操作

public:
    //更新收到的阶段计划信息
    StagePlan* updateStagePlan(unsigned char *recvArray, int len);
    //更新收到的调度命令信息
    DispatchOrderStation* updateDisorderSta(unsigned char *recvArray, int len);

public:
    QVector<TrainNumType*> v_TrainNumType;
    QVector<QString> v_TrainRunType;
    //根据索引索取列车类型（管内动车组、通勤列车等几十种）
    QString GetTrainType(int index);
    //根据索引获取客货类型
    QString GetLHFlg(int index);
    //根据名称获取列车类型索引
    int GetTrainTypeIndex(QString _strType);
    //根据索引索取列车运行类型（动车组、快速旅客列车等几种）
    QString GetTrainRunType(int index);
    //根据名称获取列车运行类型索引
    int GetTrainRunTypeIndex(QString _strRunType);
    //根据索引索取超限级别
    QString GetChaoXianLevel(int index);
    //根据名称获取超限级别索引
    int GetChaoXianLevelIndex(QString _strChaoXian);

public:
    // 获取车次在行车日志数组中的索引值
    int GetIndexInTrafficArray(QString strTrainNum);
    // 根据车次查找行车日志，找不到则返回nullprt
    TrafficLog* GetTrafficLogByCheCi(QString strTrainNum);
    // 获取进路序列的索引
    int GetTrainRouteIndex(QString _strCheci, int _JFCType);
    // 获取进路序列的索引,不是组合进路的主序列(即查找子序列)
    int GetTrainRouteIndexNotZHJL(QString _strCheci, int _JFCType);
    // 获取进路序列的索引,置顶的等待自触的组合进路的子序列
    int GetPopTrainRouteIndexOfZHJL(QString _strCheci, int _JFCType);
    //获取接发车口对应的方向站名， nCode 接车、发车设备号
    QString GetJFCKDirectByCode(int nCode);
    // nCode接车、发车设备号, nFlg出发1 到达0 //阶段计划传来该设备号是否是上行
    bool GetSXByCode(int nCode, int nFlg);
    //根据阶段计划更新进路序列
    void UpdatTrainRouteOrderByStagePlan(TrainRouteOrder* _pTrainRouteOrder,
           StagePlan *_StagePlan, int _JFCType, bool _bConfirmed=false);
    //获取方向 nFlg,出发1 接收0
    QString GetDirectByCode(int nCode,int nFlg);
    //获取设备号
    int GetCodeByRecvEquipGD(int nCodeReachStaEquip, int nTrackCode, int jfType=0);
    //重置或初始化进路序列的延续进路
    bool InitRouteYXJL(TrainRouteOrder* _pTrainRouteOrder);
    //重置或初始化变通进路
    void InitRouteBtjl(TrainRouteOrder* pTrainRouteOrder);
    //初始化组合进路
    void InitRouteZhjl(TrainRouteOrder* pTrainRouteOrder);

public:
    //发送系统报警信息给CTC(级别123，类型1系统2行车，描述)
    void sendWarningMsgToCTC(int nLevel, int nType, QString strMsg);
    //消息记录
    QVector<MsgRecord> vectMsgRecord;
    //检查报警信息发送是否在30秒内发送过
    bool CheckWarningMsgSendTime(int nLevel, int nType, QString strMsg);

public:
    //接收联锁数据-列车位置信息
    void recvLSData_TrainPos(unsigned char *recvArray, int recvlen);
    //设置车次信息
    void SetCheCiInfo(Train* pTrainTemp);
    void SetQDCheCi();//设置区段车次
    void UpdateCheCiInfo();//更新车次信息
    void DeleteCheCi();//删除没有的车次
    void ChangeCheCiInfo(QString strCheci, bool bElectric);//修改车次电力属性
    void DeleteOneCheCi(QString strCheci);//删除某个车次
    void ChangeCheCiNum(QString strOldCheci, QString strNewCheci);//修改车次号
    void SetCheCiStop(QString strCheci, bool bStop);//设置车次停稳
    //更新列车早晚点时间
    bool updateTrainOvertime(QString _strCheci, int _overtime);
    //根据计划更新列车早晚点时间
    void updateTrainOvertimeByPlan(QString _strCheci, QDateTime _time);
    //根据计划更新列车早晚点时间
    void updateTrainOvertimeByPlan();
    //更新激活车次信息
    void UpdateStationCheCiInfo(int nElaps);
    //接收联锁数据-列车报点
    void recvLSData_ReportTime(unsigned char *recvArray, int recvlen);
    //接收联锁数据-调度命令
    void recvLSData_DDML(unsigned char *recvArray, int recvlen);
    //接收联锁数据-阶段计划
    void recvLSData_JDJH(unsigned char *recvArray, int recvlen);
    //接收联锁数据-邻站预告
    void recvLSData_LZYG(unsigned char *recvArray, int recvlen);
    //接收联锁数据-进路办理回执信息
    void recvLSData_RouteReturn(unsigned char *recvArray, int recvlen);

public:
    // 签收阶段计划
    void SignStagePlan(bool bSign, bool bTrain = false);

public:
    //接收车次操作命令，联锁车次时返回true，否则返回false
    bool recvCheciCmd(unsigned char *recvArray, int recvlen);
    //合并车次（CTC车次和联锁车次）
    void mergeCheci();
    //打包和向CTC发送车次
    void packAndSendCheci(Train* pTrain);
    //向CTC发送车次
    void sendCheciToCTC();

public:
    // 解释签收的阶段计划内容转化到进路指令类数组
    void StagePlanToRouteOrder(StagePlan* pStagePlan);
    // 更新或删除组合进路
    void UpdateRouteOrderOfZHJL(TrainRouteOrder* pTrainRouteOrderFather, bool bDelete=false);
    // 阶段计划同步解析至行车日志
    void StagePlanDataToTrafficLog(StagePlan* pStagePlan);
    // 根据阶段计划更新行车日志信息
    void UpdatTrafficLogByStagePlan(TrafficLog* _pTrafficLog, StagePlan *_StagePlan, bool _bConfirmed=false);
    //根据行车日志删除进路序列
    void DeleteTrainRouteByTrafficlog(TrafficLog *pTrafficLog);

public:
    //自动设置车站进路序列权限
    void AutoSetRoutePermission();
    //更新关联列车进路序列为确认状态
    TrainRouteOrder* UpdateRouteConfirmed(QString _strCheci, int _JFCType);

    //接收CTC数据-列车进路序列
    void recvCTCData_TrainRouteOrder(unsigned char *recvArray, int recvlen, int currCtcIndex = -1, int currZxbIndex = -1);
    //更新关联行车日志的股道信息,当_JFCType=-1时都更新接发车进路
    void UpdateTrafficLogTrack(QString _strCheci, QString _strNewGd, int _JFCType = -1);
    //发送修改后过的计划给LS
    bool SendModifiedPlanToLS(MyStation* pStation);
    //组织和发送计划的信息（00修改或01删除）
    void MakeAndSendPlanUDP(MyStation* pStation, TrafficLog *pTrafficLog, int _cmdType);
    //处理进路取消命令
    void DealRouteCancleCmd(unsigned char *recvArray, int nLength);
    //处理取消进路
    void QXJL_TrainRoute(int XHDBeginCode);

    //接收CTC数据-修改计划
    void recvCTCData_ChangePlan(unsigned char *StatusArray, int recvlen);
    //自动处理接收到的CTC/占线板修改后的计划
    void AutoRecvModifiedPlan();
    //更新修改的计划进路信息
    void ModifyTrainRouteOrder(StagePlan *pStagePlanChg);
    //void ModifyTrainRouteOrder2(StagePlan *pStagePlanChg);
    //更新修改的计划日志信息
    void ModifyTrafficLog(StagePlan *pStagePlanChg);
    //根据新的计划增加相应的进路信息
    void AddNewRouteOrder(StagePlan* pStagePlanChg);
    //根据新的计划增加相应的行车日志
    void AddNewTrafficLog(StagePlan* pStagePlanChg);

    //********** 列车进路序列 **********
    //自动检查和设置计划的完成标志
    void AutoCheckTrafficExecuteFlag();
    //自动删除已出清的进路序列
    void AutoDeleleFinishedTrainRoute();
    //进路是否出清
    bool RouteIsClear(int _btJFC, QString strcheci);
    //更新进路序列的开始时间(type-0x11发车报点,0x22-停车报点,0x33通过报点)
    void UpdateRouteBeginTime(int type, QString strCheCi, QDateTime time);
    //设置进路序列的自触状态
    void SetRouteAutoTouchState(bool bAutoTouch=false);

    //进路表中的区段是否占用/进路表中是否有区段占用 QDZY
    bool IsQDZYInLSB(int _IndexRoute, int &warn, QString &msg);
    //进路表中是否有区段拥有此状态 QDZY QDSB
    bool IsQDHaveStateInLSB(int _IndexRoute, int _state, int &warn, QString &msg);
    //进路表中是否有区段是分录不良
    bool isQDHaveFLBLInLSB(int _IndexRoute, int &warn, QString &msg);
    //进路表中是否有区段是无电
    bool isQDHavePowerCutInLSB(int _IndexRoute, int &warn, QString &msg);
    //进路表中的区段是否都是此状态 QDKX QDSB
    bool IsQDStateInLSB(int _IndexRoute, int _state);
    //进路表中的区段是否封锁 FS
    bool IsQDFSInLSB(int _IndexRoute, int &warn, QString &msg);
    //进路表中的道岔是否四开 DCSK
    bool IsQDDCSKInLSB(int _IndexRoute, int &warn, QString &msg);
    //进路表中的股道防溜是否清除
    int isGDFLClear(int _IndexRoute);
    //检查股道防溜是否撤除,0无防溜，1上行，2下行
    int CheckGDFL(QString gdName);
    //进路表中的区段是否都分录不良空闲
    bool isQDFLBLKXInLSB(QStringList &pQDArray, int &warn, QString &msg);
    //进路表中的区段是否都有电
    bool isQDPowerInLSB(QStringList &pQDArray, int &warn, QString &msg);
    //检查进路可否执行（检查类型1-空闲检查，2-分路不良检查，3-接触网供电检查，0-进路是否存在）
    bool CheckRouteCanCmd(int checkType, unsigned char *pCmdUDPDate, QStringList& routeBtnTempArray);

    //获取信号机关联的区段
    QString GetXHDcorrQDName(CXHD *pxhd);
    //检查调车无电
    bool CheckRouteDCPower(unsigned char *pCmdUDPDate, QStringList &routeBtnTempArray, int &warn, QString &msg);
    //检查调车分路不良
    bool CheckRouteDC_FLBL(unsigned char *pCmdUDPDate, QStringList &routeBtnTempArray, int &warn, QString &msg);
    //检查调车封锁
    bool CheckRouteDCFS(unsigned char *pCmdUDPDate, QStringList &routeBtnTempArray, int &warn, QString &msg);

    //设置行车日志的通过报点时间（联锁延续进路通过不报点BUG在CTC此处补充报点）
    void SetTrafficLogTGTime(QString _strCheci);
    //自动检查和设置进路的状态
    void AutoCheckAndSetRouteOrderState();
    //根据配置的信号机判断设置进路的办理成功状态
    bool SetRouteSuccStateByCfgXHD(TrainRouteOrder* _pTrainRouteOrder);
    //进路序列按照开始时间自动升序排序
    void AutoSortRouteOrder();
    //定时自动发送列车计划指令到联锁，Server逻辑卡控并自动发送
    void SendOrderToInterlockSys();
    //发送进路序列办理指令给联锁
    void SendOrderToInterlock(TrainRouteOrder* pTrainRouteOrder);
    //获取进路序列的联锁表进路索引并组织进路命令
    int GetTrainRouteOrderLSBRouteIndex(TrainRouteOrder* _pTrainRouteOrder);
    //获得信号机的类型 DC_XHJ CZ_XHJ JZ_XHJ
    int GetXHDType(int xhdCode);
    //查找将要办理的进路在联锁表中的索引
    int FindRouteIndexInLSB(QString JLType,QStringList &routeBtnArray);//xhdNameArray

    //更新出入口进路预告信息
    void UpdateRoutePreWndInfo();

    //检查股道车次及信号状态，并发送发车命令
    void CheckGdTrainStatus();
    //自动发车 发送发车命令
    void SendTrainStartCmd(QString checi, int gdCode);

    //列车位置信息处理
    //CTC3.0占线板列车接近信息实时更新
    void SetTrainPosStatusInTrafficLog();
    //设置计划的列车位置状态,返回是否已更新
    bool SetPlanTrainPosStatus(TrafficLog *pTrafficLog);
    //获取股道车次的占用情况
    bool GetGdStatusGDZY(QString strGdname, QString strCheCi);
    //获取列车在接近区段的索引
    int GetTrainJJQDindex(TrafficLog *pTrafficLog);
    //查找车次是否存在
    bool FindCheCiInTrainArr(QString strCheCi);
    //判断进路序列是否已执行完毕
    bool GetJLXLState_JC(QString strGdname, QString strCheCi);
    //判断进路序列是否已执行完毕
    bool GetJLXLState_FC(QString strGdname, QString strCheCi);
    //判断接车进路或者发车进路//0 为接车,1为发车，2为不动作
    int GetJLXLState_JC_FC(QString strGdname, QString strCheCi);

    //由非常站控转回到车站控制时，自触标志全部清除。
    void ClearTrainRouteAutoTouch();

public:
    //接收联锁数据-非常站控转换
    void recvLSData_FCZK(unsigned char *recvArray, int recvlen);
    //更新计划的进路办理状态
    void UpdateTrafficlogJLSuccessed(bool bFCJL, QString strcheci, bool bSucc, bool bIng=false);

public:
    //接收CTC数据-处理行车日志计划信息
    void recvCTCData_TrafficLogInfo(unsigned char *recvArray, int recvlen, int currCtcIndex = -1, int currZxbIndex = -1);
    //根据行车日志删除进路序列
    void DeleteRouteOrderByTrafficLog(TrafficLog *pTrafficLog);
    //向联锁发送车次报点信息供判定
    void SendReportTimeToLS(QString strCheCi, int type);

public:
    //接收CTC数据-处理调度命令(签收)
    void recvCTCData_DisOrderInfo(unsigned char *recvArray, int recvlen);
    //接收CTC数据-处理作业流程信息（行车日志计划）
    void recvCTCData_JobFlowInfo(unsigned char *RecvArray, int recvlen, int currCtcIndex = -1, int currZxbIndex = -1);
    //设置该站所有的股道防溜
    void SetAllGDAntiSlip();
    //清除该站所有的股道防溜
    void ClearAllGDAntiSlip();
    //接收占线板数据-计划执行命令信息
    void recvBoradData_PlanCmdInfo(unsigned char *recvArray, int recvlen);

    //进路序列中找到该车次相应类型的进路并发送进路命令（定时器自动发送）
    void SendRouteCommandBoard1(QString strCheci, int jltype);
    //查找列车进路序列并更新为未办理
    void FindAndUpdateRouteWaite(QString strCheci, int jltype);
    //修改进路变通进路
    void ChangeRouteCommand(QString strCheci, int jltype, int nIndexOfBTJL);

public:
    //根据线路号和公里标设置临时限速
    void SetTempLimitSpeedByGLB(int xlNum, int glbStart, int glbFinish, int speed, bool bSet=true);
    //接收教师的临时限速数据
    void recvTeacherData_LimitSpeed(unsigned char *StatusArray, int nLength);
    //接收新版教师的临时限速数据
    void recvTeacherData_LimitSpeedNew(QByteArray recvArray, int nLength);
    //检查列车在限速区域的运行
    void CheckTrainInLimitSpeed();
    //获取区段的限速值
    int GetQDLimitSpeed(int qdCode);
    //给联锁发送车次限速信息
    void SendLimitSpeedToLS(QString strCheci, int posCode, int speed);
    //接收版教师的邻站模拟进出站信息
    void recvTeacherData_LZNMJCZ(QByteArray recvArray, int nLength);
    //设置邻站模拟进出站(信号机设备号，进出站类型)
    void SetLZMNJCZ(int xhdCode, int jczType);
    //增加邻站模拟的进路序列
    void AddLZMNRouteOrder(TrainRouteOrder* _pTrainRouteOrder);
    //自动触发邻站模拟的进路序列
    void AutoTouchLZMNRouteOrder();
    //邻站模拟执行的进路序列（不按照实际时间执行）
    QVector<TrainRouteOrder*> vectLZMNTrainRouteOrder;

public:
    //发送语音播报文字（播报内容，播报次数-默认1次）
    void SendSpeachText(QString strText, int count=1);
public:
    //进路检查结果数组（需要与前端交互的结果）
    QVector<CheckResult*> vectCheckResult;
    //发送进路检查结果-防错办
    void SendRouteCheckResult(CheckResult* checkResult,int currCtcIndex=-1);
    //检查车次是否是电力
    bool CheckCheciElectric(QString _strCheci);
    //列车进路序列防错办检查-联锁条件，返回0可办理，非0表示有不满足的情况
    CheckResult* CheckPreventConditionInterlock(TrainRouteOrder* _pTrainRouteOrder);
    //列车进路序列防错办检查-时序，返回0可办理，非0表示有不满足的情况
    CheckResult* CheckPreventConditionSequence(TrainRouteOrder* _pTrainRouteOrder);
    //列车进路序列防错办检查-站细，返回0可办理，非0表示有不满足的情况
    CheckResult* CheckPreventConditionStaDetails(TrainRouteOrder* _pTrainRouteOrder);
    //列车进路序列防错办检查-（站细+时序+联锁条件），返回0可办理，非0表示有不满足的情况
    CheckResult* CheckPreventConditionAll(TrainRouteOrder* _pTrainRouteOrder);
    //检查进路的办理条件并自动设置触发标记，进路正常则返回true，否则返回false
    bool CheckTrainRouteOrder(TrainRouteOrder* _pTrainRouteOrder);
    //检查进路及其兄弟进路的防错办信息，检查通过则返回true，否则返回false
    CheckResult* CheckBrotherRouteOrder(TrainRouteOrder* _pTrainRouteOrder, QString &msg);
    //根据报警类型获取报警信息内容
    QString GetWariningMsgByType(TrainRouteOrder* pTrainRouteOrder, CheckResult* ckResult);
    //更新检查信息数组
    void UpdateCheckResultArray(CheckResult* checkResult, bool bDelete = false);
    //发送提示信息-QMessageBox显示内容(type=1基本信息information,2疑问question,3警告warning,4错误critical)
    void SendMessageBoxMsg(int type, QString strMsg,int currCtcIndex=-1);
    //接收CTC数据-强制执行操作
    void recvCTCData_ForceExecute(QByteArray recvArray);
    //强制执行组合进路的兄弟进路
    void ForceExecuteZHJL(TrainRouteOrder* pTrainRouteOrderSon, int excut);
    //根据id获取列车进路序列
    TrainRouteOrder* GetTrainRouteOrderById(int routeId);
    //根据进路指令和输入车次查找相应进路序列
    TrainRouteOrder* FindTrainRouteIndexByCmd(QByteArray recvArray, QString _strCheci);
    //检查列车接近进站信号机信号未开放报警
    void CheckTrainCloseToJZXHD();
    //列车进路序列自动触发模式下，列车接近自动办理接车进路
    void CheckAndTouchJCRouteOnAutoMode(QString strCheCi, QString strJZXHD);
//    //列车进路序列自动触发模式下，列车自动办理发车进路
//    void CheckAndTouchFCRouteOnAutoMode(QString strCheCi, QString strJZXHD);
    //检查进路是否可以触发(进路，冲突车次)
    bool CheckRouteIsFirst(TrainRouteOrder* pCheckRoute, QString& strCheciConflict);
    //检查进路是否交叉(进路，冲突车次)
    //bool CheckRouteIsCross(TrainRouteOrder* pCheckRoute, QString& strCheciConflict);
    bool CheckRouteIsCross(TrainRouteOrder* pCheckRoute, TrainRouteOrder* pCheckRouteConflict);
    //检查两条联锁表进路是否交叉
    bool CheckInterlockRouteIsCross(int _idxRoute1,int _idxRoute2);
    //设备是否在联锁表中
    bool IsDevInInterlockRoute(int _idxRoute, QString _strDevName);
    //检查进路的股道-占用并停稳(若股道有中岔或者股道为两段，则要判断另外一段区段)
    bool CheckRouteGDZYTW(TrainRouteOrder* pCheckRoute);
    //根据道岔区段名称获取道岔设备索引
    int GetDCIndexByDCQDName(QString qdname);

    //判断信号机是否灯丝断丝并报警
    void JudgXHDDSstate(QString xhdName);

signals:
    //更新邻站报点信号(邻站id，车次，报点类型-0x22到达，0x11出发，0x33通过等等，报点时间)
    void UpdateLZReportTimeSignal(int lzId,QString checi,int type, QDateTime dateTime);
public:
    //获取车站接发车口相应邻站的车站id
    int GetStationJFCKLZStationId(QString strJFCK);
    //更新邻站报点（邻站到达、邻站出发）(type类型-0x22到达，0x11出发，0x33通过)
    void UpdateLZReportTime(QString checi,int type, QDateTime dateTime);

    //重置进路的操作权限(type-1CTC,2占线板)
    void ResetRoutePermit(int type);
    //生成新的进路序列
    void MakeNewRouteOrder(int type,int nIndexRoute,QStringList devNameList,QString inputCheci);
    //根据进路序列索引找到该条联锁表中的股道名称
    QString GetGDNameInLSB(int nIndexRoute);
    //根据信号机名称在股道信号机列表中找到该信号机关联的股道名称
    QString GetGDNameInGDNodeList(QString xhdName);

public:
    //根据命令号查找临时限速命令
    LimitSpeed* FindLimitSpeedByNum(QString cmdNum);
public:
//    //整理组合进路//初始化读取数据库时调用
//    void ArrangeCombinedRouteOrder();
    //根据id查找进路序列
    TrainRouteOrder* FindTrainRouteOrderById(int id);
    //根据父id查找子进路序列（即组合进路的子序列）
    TrainRouteOrder* FindSonRouteOrderByFatherId(int fatherId);
    //组合进路的子序列是否拥有此状态（state-2触发完成/3占用/1自触/0非自触）
    bool IsSonRouteOrderHaveState(int fatherId, int state);
    //在独立的信号机按钮数组中查找按钮名称(设备号,类型0列按1调按2通按)
    QString GetBtnNameInAloneXHD(int code, int type);

public:
    //最近的一次进路办理时间
    QDateTime m_LastTimeOfRouteDo;
    //是否有进路状态为正在办理
    bool HaveRouteIsDoing();
    //针对发车进路而言，检查并设置其相应的接车进路是否是延续进路，用于接车延续发车进路的办理判定条件
    bool CheckJCRouteIsYXJL(TrainRouteOrder* _pTrainRouteOrderFC);
    //检查进路的车次和接近区段上的第一趟车次是否一致,不一致时返回false和不一致车次
    bool CheckJCRouteSameCheciInJJQD(TrainRouteOrder* _pTrainRouteOrder, QString &strUnSameCheci);
    //检查进路的车次和股道上的实际车次是否一致,不一致时返回false和不一致车次
    bool CheckFCRouteSameCheciInGD(TrainRouteOrder* _pTrainRouteOrder, QString &strUnSameCheci);

    //判断通过进路是否为正线通过
    bool CheckZXTGJL(TrainRouteOrder* pTrainRouteOrder);
    //检查进路关联的计划信息是否已经预告
    bool CheckTrafficLogIsNoticed(TrainRouteOrder* pTrainRouteOrder);

public:
    //响应CTC点击的“发送计划”
    void SendPlan();

public:
    //获取虚信号机按钮命令类型，返回值0是列车按钮，1是调车按钮
    int GetXXHDBtnCmdType(QString _xxhdName);

/************ 调度命令文件 mystationdisorder.cpp函数定义 ************/
public:
    //根据id获取车站调度命令
    DispatchOrderStation* GetDisOrderRecvById(int id);
    //根据id获取调度台调度命令
    DispatchOrderDispatcher* GetDisOrderDDTById(int id);
    //根据id获取机车调度命令
    DispatchOrderLocomotive* GetDisOrderJCById(int id);
    //更新收到的调度台调度命令信息
    DispatchOrderDispatcher* updateDisorderDDT(QByteArray recvArray);
    //更新收到的机车调度命令信息
    DispatchOrderLocomotive* updateDisorderJC(QByteArray recvArray);


/************ 数据同步文件 mystation2.cpp函数定义 ************/
signals:
    //信号-发送数据给CTC
    void sendDataToCTCSignal2(MyStation* pMyStation,QByteArray dataArray,int len,int currCtcIndex);
    //信号-发送数据给占线板
    void sendDataToBoardSignal2(MyStation* pMyStation, QByteArray dataArray,int len,int currZxbIndex);
    //信号-发送数据给集控台
    void sendDataToJKSignal2(MyStation* pMyStation, QByteArray dataArray,int len,int currIndex);
    //信号-发送数据给占线图
    void sendDataToZXTSignal2(MyStation* pMyStation, QByteArray dataArray,int len,int currIndex);

public:
    //打包[阶段计划]为数据帧数组
    QByteArray packStagePlanToArray(StagePlan* pStagePlan);
    //打包[行车日志]为数据帧数组
    QByteArray packTrafficLogToArray(TrafficLog* pTrafficLog);
    //打包[进路序列]为数据帧数组
    QByteArray packTrainRouteOrderToArray(TrainRouteOrder* pTrainRouteOrder);
    //打包[调度命令]为数据帧数组
    QByteArray packDisOrderToArray(DispatchOrderStation* pDisOrderSta);
    //打包[调度台调度命令]为数据帧数组
    QByteArray packDisOrderDDTToArray(DispatchOrderDispatcher* pDisOrderDDT);
    //打包[机车调度命令]为数据帧数组
    QByteArray packDisOrderJCToArray(DispatchOrderLocomotive* pDisOrderJC);
    //打包[股道防溜]为数据帧数组(所有股道)
    QByteArray packGDAntiSlipToArray();
    //初始化同步数据包[阶段计划]
    QByteArray initSyncPackStagePlan(StagePlan* pStagePlan,int syncFlag,int packSize,int packNum);
    //初始化同步数据包[行车日志]
    QByteArray initSyncPackTrafficLog(TrafficLog *pTrafficLog, int syncFlag, int packSize, int packNum);
    //初始化同步数据包[进路序列]
    QByteArray initSyncPackTrainRouteOrder(TrainRouteOrder *pTrainRouteOrder,int syncFlag,int packSize,int packNum);
    //初始化同步数据包[调度命令]
    QByteArray initSyncPackDisOrder(DispatchOrderStation *pDisOrderSta, int syncFlag, int packSize, int packNum);
    //初始化同步数据包[调度台调度命令]
    QByteArray initSyncPackDisOrderDDT(DispatchOrderDispatcher* pDisOrderDDT, int syncFlag, int packSize, int packNum);
    //初始化同步数据包[机车调度命令]
    QByteArray initSyncPackDisOrderJC(DispatchOrderLocomotive* pDisOrderJC, int syncFlag, int packSize, int packNum);
    //初始化同步数据包[股道防溜-所有股道]
    QByteArray initSyncPackGDAntiSlip(int syncFlag, int packSize, int packNum);

    //发送同步数据-所有的[阶段计划]给Soft
    void sendSyncAllStagePlanToSoft(int softType, int currSoftIndex = -1);
    //发送同步数据-所有的[进路序列]给Soft
    void sendSyncAllTrainRouteOrderToSoft(int softType, int currSoftIndex = -1);
    //发送同步数据-所有的[行车日志]给Soft
    void sendSyncAllTrafficLogToSoft(int softType, int currSoftIndex = -1);
    //发送同步数据-所有的[调度命令]给Soft
    void sendSyncAllDisOrderToSoft(int softType, int currSoftIndex = -1);
    //发送同步数据-所有的[股道防溜]给Soft
    void sendSyncAllGDAntiSlipToSoft(int softType, int currSoftIndex = -1);
    //发送[同步数据]给Soft
    void sendSyncDataToSoft(int softType, QByteArray qSendArray,int currSoftIndex = -1);
    //发送一个[阶段计划]给Soft
    void sendOneStagePlanToSoft(int softType, StagePlan* pStagePlan,int syncFlag,int packSize=1,int packNum=1,int currSoftIndex = -1);
    //发送一个[进路序列]给Soft
    void sendOneTrainRouteOrderToSoft(int softType, TrainRouteOrder* pTrainRouteOrder,int syncFlag,int packSize=1,int packNum=1,int currSoftIndex = -1);
    //发送一个[行车日志]给Soft
    void sendOneTrafficLogToSoft(int softType, TrafficLog* pTrafficLog,int syncFlag,int packSize=1,int packNum=1,int currSoftIndex = -1);
    //发送一个[调度命令]给Soft
    void sendOneDisOrderToSoft(int softType, DispatchOrderStation* pDisOrderSta,int syncFlag,int packSize=1,int packNum=1,int currSoftIndex = -1);
    //发送一个[调度台调度命令]给Soft
    void sendOneDisOrderDDTToSoft(int softType, DispatchOrderDispatcher* pDisOrderDDT,int syncFlag,int packSize=1,int packNum=1,int currSoftIndex = -1);
    //发送一个[机车调度命令]给Soft
    void sendOneDisOrderJCToSoft(int softType, DispatchOrderLocomotive* pDisOrderJC,int syncFlag,int packSize=1,int packNum=1,int currSoftIndex = -1);
    //发送所有[股道防溜]给Soft
    void sendAllGDAntiSlipToSoft(int softType, int syncFlag,int packSize=1,int packNum=1,int currSoftIndex = -1);

//    //接收CTC数据2-TCP通道同步数据
//    void recvCTCData2(unsigned char *recvArray, int recvlen, int currCtcIndex = -1, int currZxbIndex = -1);
    //接收终端数据2-TCP通道同步数据-集控、占线图
    void recvTerminalData2(int softType, unsigned char *recvArray, int recvlen, int currIndex = -1);

    //打包[防错办-单个股道属性]为数据帧数组(股道)
    QByteArray packGDAttrToArray(CGD* pGD);
    //打包[防错办-单个出入口属性]为数据帧数组(信号机)
    QByteArray packGatewayAttrToArray(CXHD* pXHD);
    //发送同步数据-所有的[股道防溜]给Soft
    void sendSyncAllGDAttrToSoft(int softType, int currSoftIndex = -1);
    //发送同步数据-所有的[出入口属性]给Soft
    void sendSyncAllGatewayAttrToSoft(int softType, int currSoftIndex = -1);

    void SetTrafficLogProc(TrafficLog* pTrafficLog);

    bool CheckDCRouteSameQDwithJLXL(QString SDAN, QString ZDAN,int takenMinutes,QString& m_TrainNum);
    bool CheckDCRouteTakenTime(TrainRouteOrder *pTrainRouteOrder,int takenMinutes);
    //判断接近区段是否被占用
    bool CheckJJQDZY(QString strName);
};

#endif // MYSTATION_H
