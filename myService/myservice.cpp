#include "myservice.h"
#include <QDebug>
#include <windows.h>

//MyService::MyService(QObject *parent) : QObject(parent)
//{

//}

MyService::MyService(int argc, char **argv)
    : QtService<QApplication>(argc, argv, "CTCDataService")
{
    createApplication(argc, argv); //创建Application对象
    setServiceDescription("It's CTCSim data service"); //服务名称
    setServiceFlags(QtServiceBase::CanBeSuspended);
    setStartupType(QtServiceController::AutoStartup);
}

MyService::~MyService()
{

}

//开始服务
void MyService::start()
{
    //qDebug()<<"["<<__FILE__<<"]"<<__LINE__<<__FUNCTION__<<"MyService ";
    //全局功能初始化
    //pDoc.initAllData();
    pDoc = new MyDoc();;
    if(pDoc)
    {
        qDebug()<<"Create main doc!";
        //pDoc->initAllData();
    }
    QLOG_INFO()<<"Service Start!";
}

//停止服务
void MyService::stop()
{
    //qDebug()<<"["<<__FILE__<<"]"<<__LINE__<<__FUNCTION__<<"ubuntuService ";
    QLOG_WARN()<<"Service Stop!";
    QLOG_INFO()<<QString("系统退出！");
}

//暂停服务
void MyService::pause()
{
    QLOG_WARN()<<"Service Pause!";
}

//重启服务
void MyService::resume()
{
    QLOG_WARN()<<"Service Resume!";
}
