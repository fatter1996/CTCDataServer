#ifndef SOCKETTCP_H
#define SOCKETTCP_H

#include <QObject>
#include <QTcpSocket>

//TCP Client端
class SocketTCP : public QObject
{
    Q_OBJECT
public:
    explicit SocketTCP(QObject *parent = nullptr);
    ~SocketTCP();

public:
    QTcpSocket* tcpSocket;
    QString ipAdress;
    int port;
    int timer = -1;

public:
    //初始化固定IP的端口(和服务端建立连接)
    bool initByIP(QString ipAdress,int port);

private:
    void timerEvent(QTimerEvent *event);
signals:
    //接收数据信号
    void recvDataSignal(QByteArray dataArray, QString clientAdd, int clientPort);
    //断开链接信号
    void onDisconnectedSignal(QString clientAdd, int clientPort);

public slots:
    //断开链接槽
    void disconnectedSlot();
    //接收数据槽
    void recvDataSlot();
    //发送数据槽
    void sendDataSlot(QByteArray dataArray,int len);
};

#endif // SOCKETTCP_H
