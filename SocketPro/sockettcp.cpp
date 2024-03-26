#include "sockettcp.h"
#include <QHostAddress>
#include <QTimerEvent>

SocketTCP::SocketTCP(QObject *parent) : QObject(parent)
{
    ipAdress = "";
    port = 0;
    tcpSocket = new QTcpSocket;
}

SocketTCP::~SocketTCP()
{

}

//初始化固定IP的端口(和服务端建立连接)
bool SocketTCP::initByIP(QString ipAdress, int port)
{
    this->ipAdress = ipAdress;
    this->port = port;
    connect(tcpSocket, &QTcpSocket::connected, this, [=](){
        qDebug()<<QString("TCP connect to %1:%2 succeeded!").arg(ipAdress).arg(port);
        if(timer >= 0)
        {
            killTimer(timer);
            timer = -1;
        }
    });
    tcpSocket->connectToHost((QHostAddress)ipAdress, port);
    //启动定时器去连接，若首次没连接则定时尝试去连接，连接成功则kill定时器
    timer = startTimer(5000);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &SocketTCP::recvDataSlot);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &SocketTCP::disconnectedSlot);
    //tcpSocket->isOpen()
    //qDebug()<<QString("TCP connect to %1:%2 succeeded!").arg(ipAdress).arg(port);
    return true;
}

//断开链接槽
void SocketTCP::disconnectedSlot()
{
    qDebug()<<QString("TCP disconnected from %1:%2 !").arg(ipAdress).arg(port);
    //发送信号（供外部使用）
    emit onDisconnectedSignal(ipAdress, port);
    timer = startTimer(5000);
}

//接收数据槽
void SocketTCP::recvDataSlot()
{
    QString clientInfo = QString("Recv from %1:%2").arg(ipAdress).arg(QString::number(port));
    QByteArray buffer;
    //获取套接字中的内容
    buffer = tcpSocket->readAll();
    QString recvMsg = QString::fromLocal8Bit(buffer);
    //qDebug()<<clientInfo<<recvMsg;
    //发送信号（供外部使用）
    emit recvDataSignal(buffer, ipAdress, port);
}

//发送数据槽
void SocketTCP::sendDataSlot(QByteArray dataArray, int len)
{
    int sendLen = (int)tcpSocket->write(dataArray);
    if(sendLen==len)
    {
        qDebug()<<"Send data:"<<dataArray.toHex();
    }
}

//定时重连
void SocketTCP::timerEvent(QTimerEvent *event)
{
    if(timer == event->timerId())
    {
        //tcpSocket->connectToHost((QHostAddress)ipAdress, port);
    }
}
