#ifndef SERVERTCP_H
#define SERVERTCP_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

//TCP Server端
class ServerTCP : public QObject
{
    Q_OBJECT
public:
    explicit ServerTCP(QObject *parent = nullptr);
    ~ServerTCP();

public:
    //TCPServer
    QTcpServer *tcpServer = nullptr;
    //连接的客户端
    QVector<QTcpSocket*> vectTcpClient;

public:
    //初始化固定IP的端口
    bool initByIP(QString ipAdress,int port);
    //初始化所有本机IP的端口
    bool initAnyIP(int port);

signals:
    //接收数据信号
    void recvDataSignal(QByteArray dataArray, QString clientAdd, int clientPort);
    //断开链接信号
    void onDisconnectedSignal(QString clientAdd, int clientPort);

public slots:
    //和客户端建立连接的槽
    void connectToClientSlot();
    //和客户端断开连接的槽
    void onDisconnectedSlot();
    //接收数据槽
    void recvDataSlot();
    //发送数据槽
    void sendDataSlot(QByteArray dataArray,QString oppAdress,int oppProt,int len);

};

#endif // SERVERTCP_H
