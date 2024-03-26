#include "MySQLCon.h"
#include <QDebug>
#include <QSettings>
#include "Log/log.h"

CMySQLCon::CMySQLCon()
{
    db = new QSqlDatabase;
    *db = QSqlDatabase::addDatabase("QMYSQL");
    query = new QSqlQuery;
}

CMySQLCon::~CMySQLCon()
{

}

//设置数据库信息
void CMySQLCon::setDabaBaseInfo(QString ipadd, int port, QString dbName, QString user, QString pwd)
{
    con.ip = ipadd;
    con.PORT =  port;
    con.DatabaseName = dbName;
    con.UserName = user;//"root";
    con.Password = pwd;
}

bool CMySQLCon::open()
{
//    //从ipConfig.ini文件中解析IP和端口配置
//    QSettings *config = new QSettings("ipConfig.ini",QSettings::IniFormat);
//    con.ip = config->value("/SqlBaseAddress/ip").toString();
//    con.PORT =  config->value("/SqlBaseAddress/port").toUInt();
//    con.DatabaseName = config->value("/SqlBaseAddress/DatabaseName").toString();
//    con.UserName = config->value("/SqlBaseAddress/UserName").toString();
//    con.Password = config->value("/SqlBaseAddress/Password").toString();

    //当数据库断开后，自动重连
    db->setConnectOptions("MYSQL_OPT_RECONNECT=1");
    //设置数据库连接参数
    db->setHostName(con.ip);
    db->setPort(con.PORT);
    db->setDatabaseName(con.DatabaseName);
    db->setUserName(con.UserName);
    db->setPassword(con.Password);

    bool ret = db->open();
    if(ret)
    {
        qDebug() << "Open DataBase success!";
    }
    else
    {
        QLOG_ERROR() << "Open DataBase fail!  " << db->lastError().text();
    }

    return ret;
}

void CMySQLCon::close()
{
    delete query;
    return db->close();
}

bool CMySQLCon::exec(QString cmd) //执行SQL语句
{
    bool res = query->exec(cmd);
    if(!res){
        qDebug()<<query->lastError().text();
    }
    return res;
}

bool CMySQLCon::prepare(const QString &cmd) //准备SQL语句
{
    bool res = query->prepare(cmd);
    if(!res){
        qDebug()<<query->lastError().text();
    }
    return res;
}

void CMySQLCon::bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType paramType) //绑定值
{
    query->bindValue(placeholder,val);
}

bool CMySQLCon::exec()
{
    bool res = query->exec();
    if(!res){
        qDebug()<<query->lastError().text();
    }
    return res;
}

bool CMySQLCon::next() //获得下一行数据
{
    return query->next();
}

QVariant CMySQLCon::value(int index) //根据字段次序获取查询到的值
{
    return query->value(index);
}

QVariant CMySQLCon::value(const QString &name) //根据字段名获取查询到的值
{
    return query->value(name);
}

bool CMySQLCon::isOpen()
{
    return db->isOpen();
}

