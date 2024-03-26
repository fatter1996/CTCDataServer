#ifndef GLOBALFUNTION_H
#define GLOBALFUNTION_H
#include "GlobalStructure.h"
#include <QDateTime>
#include <QTextCodec>

static int StringToHex(QString SetString)
{
    //return SetString.toULong(0,16);
    //0xA和10都可以转为10
    return SetString.toLower().contains("0x")?SetString.toLower().toInt(0,16):SetString.toInt(0,10);
}
static QPoint StringToPoint(QString str)
{
    QPoint pt;
    str.trimmed(); //去除字符串两侧空白字符
    QString text = str.mid(2,str.length()-3);
    QStringList list = text.split(",");

    pt.setX(list[0].toInt());
    pt.setY(list[1].toInt());
    return pt;
}
static QRect StringToRect(QString str)
{
    QRect rect;
    str.trimmed(); //去除字符串两侧空白字符
    QString text = str.mid(2,str.length()-3);
    QStringList list = text.split(",");

    rect.setLeft(list[0].toInt());
    rect.setTop(list[1].toInt());
    rect.setRight(list[2].toInt());
    rect.setBottom(list[3].toInt());
    return rect;
}
static int StringGLBToInt(QString str) //公里标字符串转换为米单位整型
{
    str.trimmed(); //去除字符串两侧空白字符
    QString text = str.mid(1,str.length()-1);
    QStringList list = text.split("+");
    int Meters = ((list[0].toInt())*1000)+(list[1].toInt());
    return Meters;
}
static QString GetWorkDirRoot()
{
    QString path = QString::fromLocal8Bit("%1").arg(QDir::currentPath());
    path += "/";
    return path;
}

//自定义分隔函数
static int StringSplit(QString source, QString ch, QStringList &strarr)
{
    strarr = source.split(ch);
    return strarr.length();
}

//ByteArray数组转十六进制字符串
static QString ByteArrayToString(QByteArray data)
{
    QString ret;
    for (int i = 0; i < data.count(); ++i)
    {
        ret.append(QString("%1 ").arg((quint8)data.at(i),2,16,QLatin1Char('0')).toUpper().replace("0X","0x"));
    }
    return ret;
}
//ByteArray数组转十六进制字符串0x格式
static QString ByteArrayToString0x(QByteArray data)
{
    QString ret;
    for (int i = 0; i < data.count(); ++i)
    {
        ret.append(QString("0x%1 ").arg((quint8)data.at(i),2,16,QLatin1Char('0')).toUpper().replace("0X","0x"));
    }
    return ret;
}
//UnsignedChar转String(包括转中文)
static QString UnsignedCharArrayToString(unsigned char* array)
{
    char* strTemp = (char*)array;
//    int i=0;
//    for(;;i++)
//    {
//        if(strTemp[i]==0x76)
//        {
//            break;
//        }
//    }
//    char* strNew;
//    memcpy(strNew, strTemp, i+1);
    //QString str = QString::fromUtf8(strTemp);
    QString str = QString::fromLocal8Bit(strTemp);
    return str;
}
//unsigned char *转为QByteArray
static QByteArray UnsignedCharToQByteArray(unsigned char* array, int recvlen)
{
    QByteArray qRecvArray;
    qRecvArray.append(recvlen, char(0));//添加recvlen个字节并全部置零
    memcpy(qRecvArray.data(), array, recvlen);//copy数据
    return qRecvArray;
}
static int RecvArrayToInt(unsigned char* recvArray, int len)
{
    int valu = 0;
    for(int i=0; i<len;i++)
    {
        valu = valu | (recvArray[i]<<(i*8));
    }
    return valu;
}
//QByteArray转无符号整数
static int ByteArrayToUInt(QByteArray byteArray)
{
    int valu = 0;
    int len = byteArray.size();
    for(int i=0; i<len;i++)
    {
        valu = valu | ((byteArray[i]&0xFF)<<(i*8));
    }
    return valu;
}
//转时间全字字符串，不够补零，如"2019-03-31 12:24:36"; "yyyy-MM-dd hh:mm:ss"
static QString TransfrmFullDateTimeString(int year, int mouth, int day, int hour, int mini, int second)
{
    QString strDateTime= QString("%1-%2-%3 %4:%5:%6").arg(year,4,10,QLatin1Char('0')).arg(mouth,2,10,QLatin1Char('0')).arg(day,2,10,QLatin1Char('0'))
            .arg(hour,2,10,QLatin1Char('0')).arg(mini,2,10,QLatin1Char('0')).arg(second,2,10,QLatin1Char('0'));
    return strDateTime;
}
static QString DataTimeToMySQLString(QDateTime dateTime)
{
    if(dateTime.date().year()==0)
    {
        return "1970-01-01 08:00:00";
    }
    else
    {
        return dateTime.toString("yyyy-MM-dd hh:mm:ss");
    }
}
//获取MFC架构的日期时间默认值，用于与CTC终端同步解决
static QDateTime GetMFCDataTime1970()
{
    QDateTime mfcTime;
    mfcTime.fromString("1970-01-01 08:00:00");
    return mfcTime;
}

//获取作业状态
static int GetJobFlowStatusFlag(int _flowCmd)
{
    int btReturn;
    switch(_flowCmd)
    {
    case 0xAA://安排作业
        btReturn = BTNSTATUS_YAP;
        break;
    case 0xBB://得知作
        btReturn = BTNSTATUS_ZBL;
        break;
    case 0xCC://完成作业
        btReturn = BTNSTATUS_YBL;
        break;
    case 0xDD://取消作业
        //btReturn = BTNSTATUS_ZBL;
        btReturn = BTNSTATUS_YAP;//20220305朱宗龙修改占线板作业流程卡控取消作业回到安排状态
        break;
    case 0xEE://取消安排作业
        btReturn = BTNSTATUS_WAP;
        break;
    default:
        btReturn = BTNSTATUS_WAP;
        break;
    }
    return btReturn;
}
//接收数据转字符串（1字节长度，N字节字符串）
static QString GetStrFun(unsigned char *Rec_data, unsigned int *SetPos)
{
    int len = Rec_data[*SetPos];
    (*SetPos)++;
    unsigned char strRecv[255];
    memset(strRecv,0,sizeof(strRecv));
    memcpy(strRecv, &Rec_data[*SetPos], len);
    QString getString = UnsignedCharArrayToString(strRecv);
    (*SetPos) += len;
    return getString;
}
//接收数据转字符串（Lenth字节长度，N字节字符串）
static QString GetStrFun_Lenth(unsigned char *Rec_data, unsigned int *SetPos, int Lenth)
{
    int len = 0;
    memcpy(&len,Rec_data+(*SetPos),Lenth);
    (*SetPos)+=Lenth;
    unsigned char strRecv[2048];
    memset(strRecv,0,sizeof(strRecv));
    memcpy(strRecv, &Rec_data[*SetPos], len);
    QString getString = UnsignedCharArrayToString(strRecv);
    (*SetPos) += len;
    return getString;
}

//公里标字符串转换整数（K1000+100）
static int GLBStrToInt(QString strGLB)
{
    int GLB = 0;
    QString strGlb = strGLB;
    strGlb.replace("K", "");
    strGlb.replace("k", "");
    QStringList strArr;
    int c = StringSplit(strGlb, "+", strArr);
    if(2==c)
    {
        GLB = strArr[0].toInt()*1000;
        GLB += strArr[1].toInt();
    }
    return GLB;
}
//int转QByteArray
static QByteArray IntToByteArray(int i)
{
    QByteArray abyte0;
    abyte0.resize(4);
    abyte0[0] = (uchar) (0x000000ff & i);
    abyte0[1] = (uchar) ((0x0000ff00 & i) >> 8*1);
    abyte0[2] = (uchar) ((0x00ff0000 & i) >> 8*2);
    abyte0[3] = (uchar) ((0xff000000 & i) >> 8*3);
    return abyte0;
}
//接收数据转中文字符串
static QString ByteArrayToUnicode(QByteArray array)
{
    //state用于保存转换状态，它的成员invalidChars，可用来判断是否转换成功
    //如果转换成功，则值为0，如果值大于0，则说明转换失败
    QTextCodec::ConverterState state;
    //先尝试使用utf-8的方式把QByteArray转换成QString
    QString text = QTextCodec::codecForName("UTF-8")->toUnicode(array.constData(), array.size(), &state);
    //如果转换时无效字符数量大于0，说明编码格式不对
    if(state.invalidChars > 0)
    {
        //再尝试使用GBK的方式进行转换，一般就能转换正确(当然也可能是其它格式，但比较少见了)
        text = QTextCodec::codecForName("GBK")->toUnicode(array);
    }
    return text;
}

//车次是否是客车
static bool IsCheciKC(QString _strCheCi)
{
    if(_strCheCi == "" || _strCheCi.length()==0)
    {
        return false;
    }

    QString str = _strCheCi;
    if( ((str[0] != 'X')&&(str[0] >= 'A')&&(str[0] <= 'Z'))
        || ((str[0] != 'x')&&(str[0] >= 'a')&&(str[0] <= 'z'))
        || ((str.length()>=5)&&(str[0] >= '0')&&(str[0] <= '9')&&(str[1] >= '0')&&(str[1] <= '9')&&(str[2] >= '0')&&(str[2] <= '9')&&(str[3] >= '0')&&(str[3] <= '9')&&(((str[4] >= 'a')&&(str[4] <= 'z'))||((str[4] >= 'A')&&(str[4] <= 'Z'))))
        || ((str.length()==4)&&(str[0] >= '0')&&(str[0] <= '9')&&(str[1] >= '0')&&(str[1] <= '9')&&(str[2] >= '0')&&(str[2] <= '9')&&(str[3] >= '0')&&(str[3] <= '9'))
       )
    {
        //"客车";
        return true;
    }
    else
    {
        //"货车";
        return false;
    }

    return false;
}

#endif // GLOBALFUNTION_H
