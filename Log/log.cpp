#include "log.h"
#include <QDir>
#include "GlobalHeaders/GlobalFuntion.h"

using namespace QsLogging;

QMyLog::QMyLog()
{

}

QMyLog::~QMyLog()
{
    Logger::destroyInstance();
}

void QMyLog::initLogger()
{
    // 1. 启动日志记录机制
    Logger& logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    //设置log所在目录
    const QString sLogPath(LOG_FILE_NAME);
    //检查目录或创建目录-LWM
    QString fullPath = GetWorkDirRoot()+LOG_FILE_PATH;
    QDir dir(fullPath);
    if(!dir.exists())
    {
        dir.mkpath(fullPath);
    }

    // 2. 添加两个destination
    //DisableLogRotation-禁止日志文件分割，即日志始终往一个文件中写入,后两个参数失效
    //EnableLogRotation-启用日志文件分割，后两个参数表示每个日志文件的字节数大小限制、 旧日志文件保留(备份)个数，超过此数量，自动删除最久远文件，备份文件最多支持10个。
    DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
      sLogPath, DisableLogRotation/*EnableLogRotation*/, MaxSizeBytes(512*1024), MaxOldLogCount(5)));
    DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
    //DestinationPtr functorDestination(DestinationFactory::MakeFunctorDestination(&logFunction));
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);

    //这样和槽函数连接
//    //DestinationPtr sigsSlotDestination(DestinationFactory::MakeFunctorDestination(this, SLOT(&QMyLog::logSlot(QString,int))));
//    //logger.addDestination(functorDestination);
//    DestinationPtr sigsSlotDestination(DestinationFactory::MakeFunctorDestination(this, SLOT(logSlot(QString,int))));
//    logger.addDestination(sigsSlotDestination);

    // 3. 开始日志记录
    //日志中记录
    QLOG_INFO() << "Program started";
    //QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
    //QLOG_TRACE() << "Here's a" << QString::fromUtf8("trace") << "message";
    //QLOG_DEBUG() << "Here's a" << static_cast<int>(QsLogging::DebugLevel) << "message";
    //QLOG_WARN()  << "Uh-oh!";
    //QLOG_ERROR() << "An error has occurred";
    //QLOG_FATAL() << "Fatal error!";
    //不在日志中记录
    //qDebug() << "This message won't be picked up by the logger";
    //qWarning() << "Neither will this one";
}

void QMyLog::logSlot(const QString &message, int level)
{
    qDebug() << qUtf8Printable(message);
}
