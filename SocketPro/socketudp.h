#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include <QObject>
#include <QUdpSocket>

//UDP通信
class SocketUDP : public QObject
{
    Q_OBJECT
public:
    explicit SocketUDP(QObject *parent = nullptr);

public:
    //申明socket通道
    QUdpSocket *socketUDP;

public:
    //初始化固定IP的端口
    bool initByIP(QString ipAdress,int port);
    //初始化所有本机IP的端口
    bool initAnyIP(int port);
    //打包帧头帧尾标志
    void packHeadAndEnd(QByteArray &dataArray,int len);

signals:
    //接收数据信号
    void recvDataSignal(QByteArray dataArray, QString clientAdd, int clientPort);

public slots:
    //接收数据槽
    void recvDataSlot();
    //发送数据槽
    void sendDataSlot(QByteArray dataArray,QString oppAdress,int oppProt,int len);
};

#endif // SOCKETUDP_H
