#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtMath>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDir>
#include <QtDebug>
#include <QTimer>
#include <QSettings>
#include <QSignalMapper>
#include <QtSql/QSqlDatabase>
#include <QStandardItemModel>
#include <QLabel>
#include <QDateTime>
//#include "QMessageBox"

#pragma execution_character_set("utf-8")

//软件版本号
#define VERSION  "CTC数据服务终端_V2.1.0.0"
#define COMPANY  "兰州安信铁路科技有限公司"
//方向
#define LEFT      0x5A
#define RIGHT     0xA5

//吸起落下
#define UP        0x55
#define DOWN      0xAA

//客户区坐标偏移量
#define Off_Global 0

//颜色定义
#define SkyBlue QColor(85,120,182)

//区段状态定义
#define GD_WHITE   0x0001
#define GD_BLUE    0x0002
#define GD_GREEN   0x0003
#define GD_RED     0x0004

#define GD_QD      0x000A
#define JJ_QD      0x000B
#define QD         0x000C

//信号机相关定义
#define JZ_XHJ     0x0100
#define DCJL_XHJ   0x0101
#define JZFS_XHJ   0x0102
#define SXCZ_XHJ   0x0103
#define YG_XHJ     0x0104
#define CZ_XHJ     0x0105
#define DC_XHJ     0x0106
#define DCFS_XHJ   0x0107
#define FCJL_XHJ   0x0108
#define JLFS_XHJ   0x0109
#define XHP_XHJ    0x010A

//#define XHD_DS     0x0110
//#define XHD_HD     0x0111
//#define XHD_AD     0x0112
//#define XHD_BD     0x0113
//#define XHD_LD     0x0114
//#define XHD_UD     0x0115
//#define XHD_UU     0x0116
//#define XHD_LL     0x0117
//#define XHD_YD     0x0118
//#define XHD_LU     0x0119
//#define XHD_2U     0x011A
//#define XHD_2L     0x011B
//#define XHD_BS     0x011C
//#define XHD_US     0x011D
//#define XHD_LS     0x011E
//#define XHD_USU    0x011F
//#define XHD_MD     0x0120
//#define XHD_YB     0x0121

//信号灯状态--CTC
#define XHD_DS      0x0000     //信号灯断丝
#define XHD_BD      0x0001     //信号灯白灯
#define XHD_AD      0x0002     //信号灯兰灯
#define XHD_HD      0x0004     //信号灯红灯
#define XHD_LD      0x0008     //信号灯绿灯
#define XHD_UD      0x0010     //信号灯单黄
#define XHD_UU      0x0020     //信号灯双黄
#define XHD_LL      0x0040     //信号灯双绿
#define XHD_YD      0x0080     //信号灯引导（红白）
#define XHD_LU      0x0100     //信号灯绿黄
#define XHD_2U      0x0200     //信号灯二黄
#define XHD_BS      0x0400     //信号灯白闪
#define XHD_HS      0x0800     //信号灯红闪
#define XHD_US      0x1000     //信号灯黄闪
#define XHD_LS      0x2000     //信号灯绿闪
#define XHD_USU     0x4000     //信号灯黄闪黄
#define XHD_MD      0x8000     //信号灯灭

#define LCAN       0x0126
#define DCAN       0x0127
#define YDAN       0x0128
#define TGAN       0x0129
#define GDDCAN     0x012A
#define QDAN       0x012B
#define ZDBSAN     0x012C
#define BZDBSAN    0x012D

#define ClearAllGZ 0x012F

#define XH_CZXH    0x0130
#define XH_DZXH    0x0131
#define XH_BZXH    0x0132
#define XH_JTXXH   0x0133

//设备类型相关定义
#define Dev_XH    0x0150
#define Dev_DC    0x0151
#define Dev_GD    0x0152
#define Dev_QD    0x0153
#define Dev_JTX   0x0154
#define Dev_TEXT  0x0155
#define Dev_YDQ   0x0156
#define Dev_DCQD  0x0157
#define Dev_WCQD  0x0158
#define Dev_TG    0x0159
#define Dev_ZDBS  0x015A

////菜单相关定义
//#define QDGZ     0x1601
//#define QDZY     0x1602
//#define QDFLBL   0x1603
//#define DCCQFLBL 0x1604
//#define DCDWFLBL 0x1605
//#define DCFWFLBL 0x1606
//#define DCWZ     0x1607
//#define YDQGZ    0x1608
//#define LEUGZ    0x1609
//#define ZCRESET  0x160A

////轨道道岔相关定义
#define GDWIDTH    2
#define JYJ12      8            //岔前绝缘节
#define JYJ34      16           //定位绝缘节
#define JYJ56      32           //反位绝缘节
//#define DCDW       0x0200       //定位
//#define DCFW       0x0201       //反位
//#define DCSK       0x0202       //四开

//文本相关定义
#define STATIONNAMETEXT   0x0300
#define DCQDNAMETEXT      0x0301
#define TEXT              0x0302

//应答器相关定义
#define YDQ_Q            0x0400
#define YDQ_FQ           0x0401
#define YDQ_DW           0x0402
#define YDQ_JZ           0x0403
#define YDQ_CZ           0x0404
#define YDQ_FCZ          0x0405
#define YDQ_FJZ          0x0406
#define YDQ_DC           0x0407
#define YDQ_DD           0x0408
#define YDQ_ZJ           0x0409
#define YDQ_RL           0x040A
#define YDQ_ZX_R         0x040B
#define YDQ_YG_R         0x040C
#define YDQ_ZC_3_2       0x040D
#define YDQ_YG_3_2       0x040E

#define YDQ_WY     0x0420
#define YDQ_YY     0x0421

//低频码相关定义
#define DMH_H    0x0500
#define DMH_HB   0x0501
#define DMH_HU   0x0502
#define DMH_UU   0x0503
#define DMH_UUS  0x0504   //双黄闪码
#define DMH_U2   0x0505   //黄2码
#define DMH_U2S  0x0506   //黄2闪码
#define DMH_U    0x0507
#define DMH_LU   0x0508
#define DMH_L    0x0509
#define DMH_L2   0x050A
#define DMH_L3   0x050B
#define DMH_L4   0x050C
#define DMH_L5   0x050D
#define DMH_JC   0x050E
#define DMH_SP   0x050F  //机车信号设备的载频锁定或切换
#define DMH_WM   0x0510

//功能按钮功能代码
#define Fun_FCZK        0x0600
#define Fun_XYDZS       0x0601
#define Fun_SYDZS       0x0602
#define Fun_ZQX         0x0603
#define Fun_ZRJ         0x0604
#define Fun_QGJ         0x0605
#define Fun_DCZD        0x0606
#define Fun_DCZF        0x0607
#define Fun_CLEAR       0x0608
#define Fun_DCDS        0x0609
#define Fun_DCDJ        0x060A
#define Fun_ANFS        0x060B
#define Fun_ANJF        0x060C
#define Fun_DCFS        0x060D
#define Fun_DCJF        0x060E
#define Fun_DD          0x060F
#define Fun_MD          0x0610
#define Fun_SDJS        0x0611
#define Fun_FZMENU      0x0612
#define Fun_MARK        0x0613
#define Fun_FLBL        0x0614
#define Fun_QRWCZY      0x0615
#define Fun_QJZJS       0x0616
#define Fun_OPENLJJC    0x0617
#define Fun_CLOSELJJC   0x0618
#define Fun_PDJS        0x0619
#define Fun_CQFLBL      0x061A
#define Fun_DWFLBL      0x061B
#define Fun_FWFLBL      0x061C
#define Fun_XHANDOWN    0x061D
#define Fun_TGANDOWN    0x061E
#define Fun_GDFS        0x061F
#define Fun_GDJF        0x0620
//#define Fun_FCZK        0x0621
#define Fun_GFAN        0x0622
#define Fun_ZFZ         0x0623
#define Fun_FCFZ        0x0624
#define Fun_JCFZ        0x0625

#define Arrow_Red       0x0701
#define Arrow_Yellow    0x0702
#define Arrow_Green     0x0703
#define Arrow_Black     0x0704
#define Arrow_White     0x0705
#define Arrow_Gray      0x0706

//******************* CTC服务终端 *******************
//CTC制式
#define CTC_TYPE_TDCS_CASCO  0x01 // _T("卡斯柯TDCS")
#define CTC_TYPE_CTC2_CASCO  0x02 //_T("卡斯柯CTC2.0")
#define CTC_TYPE_CTC2_TK     0x03 //_T("铁科院CTC2.0")
#define CTC_TYPE_CTC2_TH     0x04 //_T("通号CTC2.0")
#define CTC_TYPE_CTC3_CASCO  0x05 //_T("卡斯柯CTC3.0")

//接发车计划类型
#define JFC_TYPE_JF 0xAA //正常(接发)
#define JFC_TYPE_SF 0xBB //始发
#define JFC_TYPE_ZD 0xCC //终到
#define JFC_TYPE_TG 0xDD //通过

//软件类型或数据类型Byte[8]softType
#define DATATYPE_ALL   0x01 //所有终端（CTC+JK+ZXT+ZXB）
#define DATATYPE_LS    0xAA //联锁
#define DATATYPE_TCC   0xBB //教师机
#define DATATYPE_JK    0xCA //集控
#define DATATYPE_ZXT   0xDA //占线图计划终端
#define DATATYPE_CTC   0xCC //CTC
#define DATATYPE_SERVER 0xDD //服务 DATATYPE_CTRL
#define DATATYPE_BOARD 0xCD //占线板
#define DATATYPE_TRAIN 0xAB //培训软件
#define DATATYPE_QJ    0xAC //区间软件
//教师机功能码
#define TCHTYPE_DISPTCH 0x99 //调度命令
#define TCHTYPE_STAGEPL 0x33 //阶段计划
#define TCHTYPE_RESET   0xEA //站场重置
#define TCHTYPE_CHGMODE 0x2A //控制模式转换申请结果
#define TCHTYPE_TIMSYNC 0xAA //时钟同步
#define TCHTYPE_SHUTDN  0xAB //一键关机
#define TCHTYPE_LIMIT   0x0E //临时限速命令
#define TCHTYPE_LIMITSP 0x0F //临时限速命令(新版教师机)
#define TCHTYPE_LZMNJCZ 0x10 //邻站模拟进出站
//功能类型Byte[9]
#define FUNCTYPE_STAGEPLNEW 0x33 //阶段计划（CTC人工新增）
#define FUNCTYPE_DISPTCH 0x34 //调度命令
#define FUNCTYPE_STAGEPL 0x35 //阶段计划
#define FUNCTYPE_ROUTE   0x36 //进路序列
#define FUNCTYPE_TRAFFIC 0x37 //行车日志
#define FUNCTYPE_SECTION 0x38 //区间逻辑检查
#define FUNCTYPE_FLOWS   0x39 //作业流程办理
#define FUNCTYPE_CHGMODE 0x3A //控制模式转换
#define FUNCTYPE_BTNCLICK 0x3C //进路按钮按下（终端闪烁）
#define FUNCTYPE_DEVOPERA 0x3D //设备操作
#define FUNCTYPE_CMDCLEAR 0x3E //命令清除
#define FUNCTYPE_USERLOGIN 0x3F //用户登录注销
#define FUNCTYPE_SYSMSG    0x40 //系统消息
#define FUNCTYPE_SPEACH    0x42 //（Server->CTC）语音播报
//数据更新同步命令-Byte[9]-有数据变化，需要各终端重新同步数据
#define FUNCTYPE_UPDATE  0x3B
//数据同步消息定义-TCP协议
#define FUNCTYPE_SYNC    0x3B //同步类型
#define FUNCTYPE_CHECK   0x43 //（Server<->CTC）防错办进路检查
#define FUNCTYPE_TEXT    0xAA //文字显示

//功能按钮
#define FUNCTYPE_FUNCS   0x88
#define FUNCTYPE_DDMLQS  0x04 //（CTC->联锁）调度命令签收
#define FUNCTYPE_XCRZ    0x05 //（CTC->联锁）行车日志操作
#define FUNCTYPE_MDYCC   0xAC //（CTC->联锁）车次操作 0x44 车次号修正
#define FUNCTYPE_DDMLZF  0x07 //（CTC->联锁）调度命令转发司机
#define FUNCTYPE_CHGCC   0x61 //车次计划修改（行车日志）
#define FUNCTYPE_LIMITCC 0x67 //（CTC->联锁）车次限速
//更新数据类型(子分类码)
#define UPDATETYPE_ALL  0x00 //全部
#define UPDATETYPE_JDJH 0x01 //阶段计划
#define UPDATETYPE_XCRZ 0x02 //行车日志
#define UPDATETYPE_JLXL 0x03 //进路序列
#define UPDATETYPE_GDFL 0x04 //股道防溜
#define UPDATETYPE_DDML 0x05 //调度命令
#define UPDATETYPE_KZMS 0x06 //控制模式
#define UPDATETYPE_JLQX 0x07 //进路权限
//同步数据类型(子分类码)-TCP协议
#define SYNC_ALL  0xAA //全部
#define SYNC_JDJH 0x01 //阶段计划
#define SYNC_XCRZ 0x02 //行车日志
#define SYNC_JLXL 0x03 //进路序列
#define SYNC_GDFL 0x04 //股道防溜
#define SYNC_DDML 0x05 //调度命令
#define SYNC_FCB  0x06 //防错办基础数据
//同步数据标志(子分类码)-TCP协议
#define SYNC_FLAG_REQUEST 0xAA //请求
#define SYNC_FLAG_ADD     0x11 //增加
#define SYNC_FLAG_DELETE  0x22 //删除
#define SYNC_FLAG_UPDATE  0x33 //更新
#define SYNC_FLAG_DELALL  0x44 //删除所有

//计划和进路的控制命令(和占线板通信协议保持一致)
#define PLAN_CMD_TYPE 0x51 //分类码-车次计划执行命令
#define PLAN_CMD_CHG  0x61 //分类码-车次计划修改
#define PLAN_CMD_SIGN 0x62 //分类码-车次计划签收
#define PLAN_CMD_SFQX 0x91 //分类码-进路权限释放命令
#define PLAN_CMD_FUNC 0x88 //分类码-功能命令
 //计划和进路的控制子命令(和占线板通信协议保持一致)
#define PLAN_CMD_FCYG 0x01 //发车预告
#define PLAN_CMD_TYYG 0x02 //同意预告
#define PLAN_CMD_DDBD 0x03 //到达报点
#define PLAN_CMD_CFBD 0x04 //出发报点
#define PLAN_CMD_TGBD 0x05 //通过报点
#define PLAN_CMD_LZCF 0x06 //邻站出发(邻站同意)
#define PLAN_CMD_JCJL 0x07 //接车进路
#define PLAN_CMD_FCJL 0x08 //发车进路
#define PLAN_CMD_QXJL 0x09 //取消接车进路
#define PLAN_CMD_QXFL 0x0A //取消发车进路
#define PLAN_CMD_QXJY 0x0B //取消接车预告
#define PLAN_CMD_QXFY 0x0C //取消发车预告
#define PLAN_CMD_TGJL 0x0D //通过进路
#define PLAN_CMD_QXJC 0x07 //取消接车
#define PLAN_CMD_QXBS 0x08 //取消闭塞
#define PLAN_CMD_QXFC 0x09 //取消发车
#define PLAN_CMD_QXFY 0x0A //取消发预
#define PLAN_CMD_QXJY 0x0B //取消接预
#define PLAN_CMD_LZFC 0x0C //邻站发车
#define PLAN_CMD_LZDD 0x0D //邻站到达
#define PLAN_CMD_DELE 0x1C //删除日志
#define PLAN_CMD_CHGJCBTJL 0x0E //修改接车变通进路
#define PLAN_CMD_CHGFCBTJL 0x0F //修改发车变通进路
#define PLAN_CMD_JCBTJL 0x10 //修改并办理接车变通进路
#define PLAN_CMD_FCBTJL 0x11 //修改并办理接车变通进路

//Time格式化为CString
#define TIME_FORMAT_YMDHMSM "yyyy-MM-dd hh:mm:ss.zzz"
#define TIME_FORMAT_YMDHMS  "yyyy-MM-dd hh:mm:ss"
#define TIME_FORMAT_YMDHM   "yyyy-MM-dd hh:mm"
#define TIME_FORMAT_YMD     "yyyy-MM-dd"
#define TIME_FORMAT_HMS     "hh:mm:ss"
#define TIME_FORMAT_HM      "hh:mm"
#define TIME_FORMAT_HMSM    "hh:mm:ss.zzz"

#define CLEANQD 0x0FF00      //用以清除区段状态
#define CLEANDC 0x0F0FF      //用以清除道岔状态

//列车类型
#define LCTYPE_KC 0x01 //客车
#define LCTYPE_HC 0x00 //货车

#define ROUTE_JC 0x00  //接车进路
#define ROUTE_FC 0x01  //发车进路

//超限等级
#define CHAOXIAN_0 "正常"
#define CHAOXIAN_1 "一级超限"
#define CHAOXIAN_2 "二级超限"
#define CHAOXIAN_3 "三级超限"
#define CHAOXIAN_4 "超级超限"

#define CTCCENTER "CTC调度中心"

#define ROUTE_JC 0x00  //接车进路
#define ROUTE_FC 0x01  //发车进路
//#define ROUTE_TG 0x03  //通过进路
#define ROUTE_TONGG 0x03  //通过进路

//列车位置信息状态
#define STATUS_LCWJJ   "列车未接近"
#define STATUS_LC3FQ   "列车距离本站3个分区"
#define STATUS_LC2FQ   "列车距离本站2个分区"
#define STATUS_LC1FQ   "列车距离本站1个分区"
#define STATUS_LCYJZ   "列车进站" //列车已进站
#define STATUS_LCYKSJRGD   "列车开始进入股道"
#define STATUS_LCYWZJRGD "列车完整进入股道" //列车已进入股道
#define STATUS_LCYCZ   "列车已出站"
#define STATUS_LCYWC   "列车已完成"

////列车办理流程提示信息
//#define PROCESS_ZBJC  "准备接车"
//#define PROCESS_BLJC  "办理接车进路"
////#define PROCESS_LCDD  "列车到达(通过)报点"
//#define PROCESS_BLFC  "发车进路办理、发车报点"
//#define PROCESS_FINISH  "流程停止"
////列车办理流程提示信息
//#define PROCESS_JCBS  "办理接车闭塞"
//#define PROCESS_BLJL  "办理接车进路"
//#define PROCESS_LCDD  "列车到达(通过)报点"
//#define PROCESS_CZZY  "办理车站作业"
//#define PROCESS_FCBS  "办理发车闭塞"
//#define PROCESS_BLFL  "办理发车进路"
//#define PROCESS_LCCF  "列车出发报点"
//#define PROCESS_ALLOK "所有流程已办理"
//#define PROCESS_JCYG "办理接车预告" //new替换“准备接车”or“办理接车闭塞”
//#define PROCESS_FCYG "办理发车预告" //new替换“准备发车”or“办理发车闭塞”
//#define PROCESS_LCDD_TG  "列车到达(通过)报点"

//列车办理流程提示信息
#define PROCESS_JCYG "办理接车预告"
#define PROCESS_BLJL  "办理接车进路"
#define PROCESS_LCDDTG  "列车到达(通过)报点"
#define PROCESS_FCYG "办理发车预告"
#define PROCESS_BLFL  "办理发车进路"
#define PROCESS_LCCF  "发车报点"
#define PROCESS_FINISH "流程停止"

#define ZHCXC  0x01  //正常行车
#define LCMN   0x02  //列车模拟
#define DCMN   0x03  //调车模拟

//进路办理状态
#define STATUS_JLBL_NO   0x00 //未办理
#define STATUS_JLBL_ING  0x01  //正在办理
#define STATUS_JLBL_SUCC 0x02  //办理成功

/*指示灯类型*/
#ifndef LAMP_TYPE_H
#define	CTC_MODE_CENTER		0x0001	//中心模式
#define	CTC_MODE_STATION	0x0002	//车站控制模式
#define	CTC_MODE_NORMAL		0x0003	//分散自律模式
#define CTC_MODE_6502		0x0004	//非常站控
#define BUILDROUTE_BYPLAN	0x0005	//按图排路
#define CONTROL_BYPLAN		0x0006	//计划控制
#define COMM_TOCENTER		0x0007	//中心通信
#define COMM_TOINTERLOCK	0x0008	//自律机通信
#define PERMIT_BACK			0x0009	//允许转回
#define TRAFFIC_CONTROL		0x000A	//列控
#define AGREE     0xAA //同意
#define DISAGREE  0xAB //不同意
#endif

//作业流程定义
#define DEF_FLOW_JAL 0x01
#define DEF_FLOW_LJ 0x02
#define DEF_FLOW_SS 0x03
#define DEF_FLOW_XW 0x04
#define DEF_FLOW_JP 0x05
#define DEF_FLOW_CJ 0x06
#define DEF_FLOW_ZG 0x07
#define DEF_FLOW_LW 0x08
#define DEF_FLOW_HJ 0x09
#define DEF_FLOW_HC 0x0A
#define DEF_FLOW_ZX 0x0B
#define DEF_FLOW_JC 0x0C
#define DEF_FLOW_DK 0x0D
#define DEF_FLOW_CH 0x0E
#define DEF_FLOW_ZK 0x0F
#define DEF_FLOW_ZW 0x10

//作业按钮状态
#define BTNSTATUS_WAP  0x00 //未安排(灰色)、未办理
#define BTNSTATUS_YAP  0x01 //已安排(红色)、需要办理
#define BTNSTATUS_ZBL  0x02 //正在办理(黄色)得知
#define BTNSTATUS_YBL  0x03 //已办理(绿色)

#define  FrameHead   0xEF     //通信数据帧的帧头
#define  FrameTail   0xFE     //通信数据帧的帧尾

//区段状态定义
#define QDGZ    0x0010       //区段故障
#define QDKX    0x0001       //区段空闲
#define QDSB    0x0002       //区段锁闭
#define QDZY    0x0004       //区段占用
#define QDYSB   0x0008       //区段预锁闭
//道岔状态
#define DCDW    0x0100       //定位
#define DCFW    0x0200       //反位
#define DCSK    0x0400       //四开

#define CLEANQD 0x0FF00      //用以清除区段状态
#define CLEANDC 0x0F0FF      //用以清除道岔状态

//列车调车进路命令2字节高两位设备按钮类型定义.lwm.2021.6.22
#define JLBTN_LC   0x00   //进路按钮-列车按钮 0000 0000
#define JLBTN_DC   0x40   //进路按钮-调车按钮 0100 0000
#define JLBTN_TG   0x80   //进路按钮-调车按钮 1000 0000
#define MASK05     0x3F   //用以得到字节的0~5位

#define LC_ROUTE    0x5A          //列车进路
#define DC_ROUTE    0xA5          //调车进路
#define TG_ROUTE    0x55          //通过进路
#define ROUTE_LC    "LCJL"   //列车进路
#define ROUTE_DC    "DCJL"   //调车进路
#define ROUTE_TG    "TGJL"   //通过进路

#define GDDC_X_LOCK 0x08//X总锁闭
#define GDDC_S_LOCK 0x10//S总锁闭

#define LCJL_AUTOTOUCH_MIN 0 //1 //列车进路自触时间->分钟
#define LCJL_AUTOTOUCH_SEC 0 //60 //列车进路自触时间->秒

//进路报警类型
#define JLWARNING_FLBL_GD 0x0001 //分路不良-股道区段（分路不良条件： 分路不良区段必须已确认空闲。）
#define JLWARNING_FLBL_DC 0x0002 //分路不良-道岔区段（分路不良条件： 分路不良区段必须已确认空闲。）
#define JLWARNING_FS_GD 0x0004  //股道封锁（信号联锁条件： 包括检查轨道区段的占用和锁闭状态， 信号机、 道岔、股道、 区间的封锁状态， 敌对进路， 道岔单锁位置。）
#define JLWARNING_FS_DC 0x0008  //道岔封锁（信号联锁条件： 包括检查轨道区段的占用和锁闭状态， 信号机、 道岔、股道、 区间的封锁状态， 敌对进路， 道岔单锁位置。）
#define JLWARNING_LCTYPE 0x0010 //线路性质（线路性质： 需满足站细规定的股道与出入口的客货、 超限、 电力属性。）
#define JLWARNING_GDCX   0x0020  //股道超限（超限车规定： 满足站细规定的相邻两股道不能同时接超限车的规定。）
#define JLWARNING_QDPOWERCUT 0x0040 //区段无电
#define JLWARNING_QDZY 0x0080 //区段占用
#define JLWARNING_QDSB 0x0100 //区段锁闭
#define JLWARNING_DCSK 0x0200 //道岔四开
#define JLWARNING_FLBL_WCQD 0x0400 //分路不良-无岔区段（分路不良条件： 分路不良区段必须已确认空闲。）
#define JLWARNING_FLBL_GDKX 0x0800 //分路不良-股道区段确认空闲
//进路时序
#define JLWARNING_SEQU_CCCT 0x1001 //车次冲突
#define JLWARNING_SEQU_CROSS 0x1002 //进路交叉
#define JLWARNING_SEQU_TIME  0x1003 //规定时间外
//站细
#define JLWARNING_ATTR_GDTYPE    0x2001 //股道类型
#define JLWARNING_ATTR_LEVELCX   0x2002 //股道超限
#define JLWARNING_ATTR_PLATFORM1 0x2004 //无站台
#define JLWARNING_ATTR_PLATFORM2 0x2008 //站台高低不满足
#define JLWARNING_ATTR_ARMY      0x2010 //军运
#define JLWARNING_ATTR_FLOWSS    0x2020 //作业无上水/吸污
#define JLWARNING_ATTR_FLOWXW    0x2040 //作业无上水/吸污
#define JLWARNING_ENEX_LEVELCX   0x2080 //出入口超限
#define JLWARNING_ENEX_KHTYPE    0x2100 //客货类型错误
#define JLWARNING_ENEX_UNSAME    0x2200 //接发车方向与固定路径不一致/出入口不一致
//防溜
#define JLWARNING_HAVEFLDEVSX    0x3001 //上行有防溜设备
#define JLWARNING_HAVEFLDEVXX    0x3002 //下行有防溜设备
#define JLWARNING_HAVEFLDEVSXX   0x3003 //上下行有防溜设备
//侧线通过
#define JLWARNING_ROUTE_CXTG 0x4001 //列车侧线通过


//接车进路自动触发提前时间（分钟）
extern int AutoTouchReachRouteLeadMinutes;
//发车进路自动触发提前时间（分钟）
extern int AutoTouchDepartRouteLeadMinutes;
//进路序列可设置自动触发的时间范围（分钟）
extern int AutoTouchMinutes;
//进路序列尝试自动触发的最长时间范围（分钟）
extern int TryAutoTouchMaxMinutes;
//人工排路时生成进路序列
extern bool MakeRouteOrderWhenClick;
//股道分路不良确认空闲自触可办理
extern bool GDFLBLKXAutoTouch;
//非常站控模式下是否设置进路自触
extern bool FCZKSetAutoTouch;
//日志文件名称
extern QString G_LOG_FILE_NAME;
//人工签收方式，0人工签收后自动同步进路信息，1人工签收后需点击“发送计划”才同步进路信息
extern int ManSignRouteSynType;
//正在触发、触发成功的进路是否可取消自触（默认可取消）
extern bool TouchingRouteCancelAutoFlag;
//“自触”标志的进路一直“正在触发”时，每间隔30秒重新触发一次，持久时间如下（分钟）
extern int AutoTryMaxMinutesWhenTouchingRoute;
//早点列车可以办理进路的时间范围（分钟）
extern int EarlyTrainsTouchRangeMinutes;
//设置自触标记时是否需要先完成预告（终到计划除外）
extern bool SetAutoTouchNeedNotice;
//设置自触的时间，当同意邻站预告/预告后，小于0则功能不启用
extern int AutoTouchMinitesWhenNoticed;
//设置自触的时间，当邻站模拟进出站，小于0则功能不启用
extern int AutoTouchMinitesWhenLZMNJCZ;
//是否判定进路交叉
extern bool JudgeRouteCrossed;
//调度中心的车次是否可以修改
extern bool DispatchCenterTrainModify;

#endif // GLOBAL_H
