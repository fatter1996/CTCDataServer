#include "servertcp.h"
#include "Log/log.h"

ServerTCP::ServerTCP(QObject *parent) : QObject(parent)
{
    tcpServer = new QTcpServer;
}

ServerTCP::~ServerTCP()
{
    if(tcpServer)
    {
        //关闭服务
        tcpServer->close();
        tcpServer = nullptr;
        delete tcpServer;
    }
    //清除
    vectTcpClient.clear();
}
//初始化固定IP的端口
bool ServerTCP::initByIP(QString ipAdress, int port)
{
    //绑定本地IP和端口
    bool res = tcpServer->listen(QHostAddress(ipAdress), port);
    if(res)
    {
        qDebug()<<QString("init TCP listen %1:%2 succeeded!").arg(ipAdress).arg(port);
        connect(tcpServer, &QTcpServer::newConnection, this, &ServerTCP::connectToClientSlot);
    }
    else
    {
        QLOG_ERROR()<<QString("init TCP listen %1:%2 failed!").arg(ipAdress).arg(port);
    }
    return res;
}
//初始化所有本机IP的端口
bool ServerTCP::initAnyIP(int port)
{
    //绑定本地IP和端口
    bool res = tcpServer->listen(QHostAddress::Any, port);
    if(res)
    {
        qDebug()<<QString("init TCP listen Any:%1 succeeded!").arg(port);
        connect(tcpServer, &QTcpServer::newConnection, this, &ServerTCP::connectToClientSlot);
    }
    else
    {
        QLOG_ERROR()<<QString("init TCP listen Any:%1 failed!").arg(port);
    }
    return res;
}
//和客户端建立连接的槽
void ServerTCP::connectToClientSlot()
{
    //取出连接好的套接字
    QTcpSocket* tcpSocketClient = tcpServer->nextPendingConnection();
    //qDebug()<<QString("socketDescriptor=%1").arg(tcpSocketClient->socketDescriptor());
    //获得通信套接字的控制信息
    QString ipAdress = tcpSocketClient->peerAddress().toString().split("::ffff:")[1];//获取连接的 ip地址
    quint16 port = tcpSocketClient->peerPort();//获取连接的 端口号
    QLOG_INFO()<<QString("%1:%2 tcp client connect succeeded!").arg(ipAdress).arg(port);
    //连接信号槽
    connect(tcpSocketClient, &QTcpSocket::readyRead, this, &ServerTCP::recvDataSlot);
    connect(tcpSocketClient, &QTcpSocket::disconnected, this, &ServerTCP::onDisconnectedSlot);
    //增加该链接
    vectTcpClient.append(tcpSocketClient);
}
//和客户端断开连接的槽
void ServerTCP::onDisconnectedSlot()
{
    //获取信号发送者
    QTcpSocket *tcpSocketClient = qobject_cast<QTcpSocket*>(sender());
    for(int i=0; i<vectTcpClient.size(); i++)
    {
        QTcpSocket* tcpSocket = vectTcpClient.at(i);
        if(tcpSocket->socketDescriptor() == tcpSocketClient->socketDescriptor())
        {
            //移除客户端链接
            vectTcpClient.removeAt(i);
            QString ipAdress = tcpSocketClient->peerAddress().toString().split("::ffff:")[1];//获取连接的 ip地址
            quint16 port = tcpSocketClient->peerPort();//获取连接的 端口号
            qDebug()<<QString("%1:%2 disconnected!").arg(ipAdress).arg(port);
            qDebug()<<QString("socketDescriptor=%1").arg(tcpSocketClient->socketDescriptor());
            emit onDisconnectedSignal(ipAdress,port);
            return;
        }
    }
}
//接收数据槽
void ServerTCP::recvDataSlot()
{
    //获取信号发送者
    QTcpSocket *tcpSocketClient = qobject_cast<QTcpSocket*>(sender());
    //获得通信套接字的控制信息
    QString ipAdress = tcpSocketClient->peerAddress().toString().split("::ffff:")[1];//获取连接的 ip地址
    quint16 port = tcpSocketClient->peerPort();//获取连接的 端口号
    QString clientInfo = QString("Recv from %1:%2").arg(ipAdress).arg(QString::number(port));
    QByteArray buffer;
    //获取套接字中的内容
    buffer = tcpSocketClient->readAll();
    QString recvMsg = QString::fromLocal8Bit(buffer);
    //qDebug()<<clientInfo<<recvMsg;
    //发送信号（供外部使用）
    emit recvDataSignal(buffer, ipAdress, port);
}
//发送数据槽
void ServerTCP::sendDataSlot(QByteArray dataArray,QString oppAdress,int oppProt,int len)
{
    //发给指定客户端
    for(int i=0; i<vectTcpClient.size(); i++)
    {
        QTcpSocket* tcpSocket = vectTcpClient.at(i);
        QString ipAdress = tcpSocket->peerAddress().toString().split("::ffff:")[1];//获取连接的 ip地址
        quint16 port = tcpSocket->peerPort();//获取连接的 端口号
        if(oppAdress==ipAdress && oppProt==port)
        {
            if(tcpSocket->write(dataArray) != len)
            {
                qDebug()<<QString("send tcp data lenth not match, len=%1").arg(len);
                //continue;
            }
            //tcpSocket->flush();//new.2023.5.11
//            qint64 wrLen = tcpSocket->write(dataArray);
//            tcpSocket->flush();//立即发送
//            if(wrLen != (qint64)len)
//            {
//                qDebug()<<QString("send tcp data lenth not match, len=%1").arg(len);
//                //continue;
//            }
            break;
        }
    }
}


