#include "mainwindow.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QDebug>
#include <QTextStream>
#include <QDir>
#include <iostream>
//#include <stdio.h>
#include <winsock2.h>
#include <QProcess>
#include "GlobalHeaders/Global.h"
#include "GlobalHeaders/GlobalFuntion.h"
//#include "vld.h" //调试内存使用
#include "Helper.h"
#include "myService/myservice.h"
#include "Log/log.h"
#include <Windows.h>
#include <DbgHelp.h>
#include <QApplication>
#include <QCoreApplication>
//#include <QProcess>
//#include <QSystemSemaphore>
//#include <QSharedMemory>
#include <QLockFile>
#include <QMessageBox>

//每次启动则生成一个带时间戳的日志文件
#define LOG_FILE_PATH QString("Logs")
#define LOG_FILE_NAME QString("Logs/SysLog_") + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + QString(".log")
QString G_LOG_FILE_NAME = LOG_FILE_NAME;

//日志-回调函数-调试信息通过标准输出流输出到终端上
void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString text;
    switch(type)
    {
    // 调试消息
    case QtDebugMsg:
        text = QString("DEBUG");
        break;
    //警告消息和可恢复的错误
    case QtWarningMsg:
        text = QString("WARN");
        break;
    //关键错误和系统错误
    case QtCriticalMsg:
        text = QString("ERROR");//CRITICAL
        break;
    // 致命错误
    case QtFatalMsg:
        text = QString("FATAL");
    // 信息消息
    case QtInfoMsg:
        text = QString("INFO");
    }

    // 截取源文件相对位置
    QString filepath = QString(context.file);
    int begin = filepath.indexOf('\\', 3);
    filepath = filepath.mid(begin + 1);

    // 设置输出信息格式
    QString context_info = QString("File:(%1) Line:(%2)").arg(filepath).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString current_date = QString("[%1]").arg(current_date_time);
    QString message = QString("%4 %1 %2 %3 ").arg(text).arg(context_info).arg(msg).arg(current_date);

    //输出显示到控制台
    std::cout<<message.toStdString().data()<< std::endl;

    static QMutex mutex;
    mutex.lock();
    // 检查目录或创建目录-LWM
    QString fullPath = GetWorkDirRoot()+LOG_FILE_PATH;
    QDir dir(fullPath);
    if(!dir.exists())
    {
        dir.mkpath(fullPath);
    }
    // 输出信息至文件中（读写、追加形式）
    // 使用全局变量G_LOG_FILE_NAME，否则使用LOG_FILE_NAME会创建2个log文件-LWM
    QFile file(G_LOG_FILE_NAME);//LOG_FILE_NAME //"log.txt"
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&file);
    textStream << message << "\r\n";
    file.flush();
    file.close();
    mutex.unlock();
}

//程序异常捕获接口
long __stdcall ExceptionCapture(EXCEPTION_POINTERS *pException)
{
    //qWarning()<<"Error:"<<pException->ContextRecord->LastExceptionToRip;
    //qWarning()<<"Error:\r\n";
    EXCEPTION_RECORD *record = pException->ExceptionRecord;
    QString errCode(QString::number(record->ExceptionCode, 16));
    QString errAddr(QString::number((uint)record->ExceptionAddress, 16));
    QString errFlag(QString::number(record->ExceptionFlags, 16));
    QString errPara(QString::number(record->NumberParameters, 16));
    qDebug() << "errCode: " << errCode;
    qDebug() << "errAddr: " << errAddr;
    qDebug() << "errFlag: " << errFlag;
    qDebug() << "errPara: " << errPara;

    //千万不能加在此处，
    //qDebug() << "start application:" << QProcess::startDetached(qApp->applicationFilePath(), QStringList());//重启

    return EXCEPTION_EXECUTE_HANDLER;
}

LONG CreateCrashHandler(EXCEPTION_POINTERS *pException){
    //创建 Dump 文件
    QDateTime CurDTime = QDateTime::currentDateTime();
    QString current_date = CurDTime.toString("yyyy_MM_dd_hh_mm_ss");
    //dmp文件的命名
    QString dumpText = "Dump_"+current_date+".dmp";
    EXCEPTION_RECORD *record = pException->ExceptionRecord;
    QString errCode(QString::number(record->ExceptionCode, 16));
    QString errAddr(QString::number((uint)record->ExceptionAddress, 16));
    QString errFlag(QString::number(record->ExceptionFlags, 16));
    QString errPara(QString::number(record->NumberParameters, 16));
    HANDLE DumpHandle = CreateFile((LPCWSTR)dumpText.utf16(),
             GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(DumpHandle != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;
        //将dump信息写入dmp文件
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),DumpHandle, MiniDumpNormal, &dumpInfo, NULL, NULL);
        CloseHandle(DumpHandle);
    }
//    //创建消息提示
//    QMessageBox::warning(NULL,"Dump",QString("ErrorCode%1  ErrorAddr：%2  ErrorFlag:%3 ErrorPara:%4").arg(errCode).arg(errAddr).arg(errFlag).arg(errPara),
//        QMessageBox::Ok);
    return EXCEPTION_EXECUTE_HANDLER;
}

//程序是否在运行
bool processIsRunning()
{
    bool isRunning = false;
    //方法：文件锁判定程序是否已启动
    //判定同目录、同名称的程序是否在运行
    QString appFilePath = QCoreApplication::applicationFilePath();//qApp->applicationFilePath();
    qDebug()<<appFilePath;
    QString appName = qApp->applicationName();//+".exe";
    qDebug()<<appName;
    QString lockFilePath;
    lockFilePath = appFilePath+".singleApp.lock";
    qDebug() << "lockFilePath: " + lockFilePath;
    QLockFile *lockFile = new QLockFile(lockFilePath);
    if(!lockFile->tryLock(1000))
    {
        qDebug() << "上锁失败，不能启动！"+appName;
        QMessageBox::warning(NULL, "警告", "程序已经在运行！"+appName);
        isRunning = true;
    }
    else
    {
        qDebug() << "上锁成功，可以启动！"+appName;
        isRunning = false;
    }
    return isRunning;
}

int main(int argc, char *argv[])
{
//    // 注册异常捕获函数
//    //SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ExceptionCapture);
//    SetUnhandledExceptionFilter(ExceptionCapture);
//    //输出dump+pdb文件记录异常位置
//    //SetUnhandledExceptionFilter(ExceptionFilter);

    //获取系统编码
    //QTextCodec *codec = QTextCodec::codecForLocale();
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    //注冊异常捕获函数
    //SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)CreateCrashHandler);

//    QApplication a(argc, argv);
//    //日志文件记录到全局变量
//    G_LOG_FILE_NAME = LOG_FILE_NAME;
//    qDebug()<<G_LOG_FILE_NAME;
//    // 注册MessageHandler
//    //qInstallMessageHandler(outputMessage);

//    MainWindow w;
//    w.setWindowTitle("CTCDataServer2.0-服务终端");
//    w.show();
//    //w.showMaximized();
//    qInfo()<<"(*^_^*) System Started!";
//    //qDebug()<<"(*^_^*) System Started!";
//    //qWarning()<<"(*^_^*) System Started!";
//    //qCritical()<<"(*^_^*) System Started!";
//    return a.exec();

//    //日志
//    QMyLog log;
//    //日志初始化
//    log.initLogger();
    //主程序
    MyService service(argc, argv);
    //重复启动判断
    if(processIsRunning())
    {
        return -1;
    }
    //日志
    QMyLog log;
    //日志初始化
    log.initLogger();
    //启动运行
    return service.exec();
}
