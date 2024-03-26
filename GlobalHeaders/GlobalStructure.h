#ifndef GLOBALSTRUCTURE_H
#define GLOBALSTRUCTURE_H
#include "Global.h"
typedef struct
{
    int stationID;
    QPoint offSetPt;
    QPoint staNamePt;
    int m_nCTCSoftID;
    int m_nLSSoftID;
    int m_nXLMNJSoftID;
    void init()
    {
        stationID=m_nLSSoftID=m_nCTCSoftID=m_nXLMNJSoftID=0;
        offSetPt=QPoint(0,0);
        staNamePt=QPoint(0,0);
    }
}StaInfo;

typedef struct
{
    QString str_Speech;
    int n_Count;
    void init()
    {
        str_Speech="";
        n_Count=0;
    }
}SpeechData;

typedef struct
{
    QString strName;
    int nStationID;
    QPoint pt;
    bool isRunFlag;
    int runTime;
    void init()
    {
        strName="";
        nStationID=runTime=0;
        pt=QPoint(0,0);
        isRunFlag=false;
    }
}DLB_Data;

typedef struct
{
    QString strType;//类型
    QString strTypeName;//车次类型名车
}TrainNumType;

typedef struct
{
    QVector<QString> vectBTJLChild;
}BTJL;//变通进路
typedef struct
{
    QVector<QString> vectZHJLChild;
}ZHJL;//组合进路
typedef struct
{
    QString SDXHName;//始端信号
    QString ZDXHName;//终端信号
    QString ZXTGGDName;//正线通过股道
}TGJL;//正线通过进路

//股道及左右信号机接点
typedef struct
{
   QString strGDName;//股道名称
   QString strGDName1;//股道名称（可能为无岔或道岔区段，一般为有中岔的股道配置）
   int nCode;//股道设备号
   QString strLeftNode;//左侧信号机名称
   QString strLeftNode1;//左侧信号机名称1
   QStringList strArrLeftFCK;//左边的发车口
   QString strRightNode;//右侧信号机名称
   QString strRightNode1;//右侧信号机名称1
   QStringList strArrRightFCK;//右边的发车口
   bool bIgnoreLeftSXX;//忽略左侧信号机的上下行
   bool bIgnoreRightSXX;//忽略右侧信号机的上下行
}StationGDNode;

//列车固定径路信息
struct FixedRoute{
    int id = NULL;
    int staId = NULL;
    int gdId;//股道id
    QString gdName;//股道名称
    QString arrivalnum;//到达车次
    QString arrivaltime;//到达时间
    QString departnum;//出发车次
    QString departtime;//出发时间
    int entraid;//入口ID  始发0
    QString entrXHDName;//入口信号机名称
    int exitid;//发车ID  终到0
    QString exitXHDName;//出口信号机名称
    int beforestationid;//前场车站id  始发0
    int afterstationid;//后方车站id 终到0
    int stoppoint;//1 技术停点  0非技术停点  （后续列 0、1代表是否具有属性）
    int deliveryorder;//交令
    int trianflag;//机车
    int cargocheck;//货检
    int boardflag;//乘降
    int pickhang;//摘挂
    int transferflag;//换乘
    int Loadcargo;//装卸
    int dirtabsorption;//吸污
    int crossflag;//道口
    int wagonnum;//车号
    int waterflag;//上水
    int traincheck;//列检
    int Intercontrol;//综控
    int depotflag;//站务
    int payticket;//交票
    int traintail;//列尾
};
//车次股道列表
struct TrainNumTrack{
    int id = NULL;
    int staId = NULL;
    QString trainnum;//车次号

    QVector<int> tracks;
    QString tracksStr;

    QVector<int> Tracks()
    {
        if(tracksStr == 0)
            return QVector<int>();
        tracks.clear();
        QStringList strList = tracksStr.split(',');
        for(int i = 0; i < strList.size(); i++)
        {
            tracks.append(strList.at(i).toInt());
        }
        return tracks;
    }

    QString TracksStr()
    {
        if(tracks.size() == 0)
            return "";
        tracksStr = "";
        for(int i = 0; i < tracks.size(); i++)
        {
            tracksStr.append(QString("%1").arg(tracks.at(i)));
            if(i != tracks.size() - 1)
                tracksStr.append(",");
        }
        return tracksStr;
    }
};

//进路办理检查结果
struct CheckResult{
    int  id = 0;//id(当前时间的毫秒数取后4个字节)
    int  route_id = 0;//列车进路序列id
    int  indexRoute = -1;//匹配的联锁表表索引
    bool bEnforced = false;//强制执行
    QByteArray cmdArray;//给联锁发送的进路指令信息数组
    int  check = 0;//检查结果，0为通过，大于0则包含检查结果类型
    QString checkMsg;//检查结果信息
    QByteArray dataArray;//进路命令帧
};

//进路命令按钮
struct CmdJLBtn{
    int  devCode = 0;//设备号
    int  btnType = 0;//按钮类型：0x00列车，0x01调车，0x10通过。
};

//腰岔解锁
struct YCJS{
    QString strName;
    bool bLocking;//是否锁闭
    int  countdown;//倒计时
    void init()
    {
        strName="";
        bLocking=false;
        countdown = 0;
    }
};

//股道确认空闲
struct GDQR{
    QString strName;
    int  countdown;//倒计时
    void init()
    {
        strName="";
        countdown = 0;
    }
};

//进路办理检查结果
struct XXHDBtnCmdType{
    int BtnCmdType = 1;//命令类型0列按1调按
    QStringList XXHDArray;//进路命令帧
};

//报警信息记录
struct MsgRecord{
    int nLevel;
    int nType;
    QString strMsg;
    QDateTime timeSend;
};

#endif // GLOBALSTRUCTURE_H
