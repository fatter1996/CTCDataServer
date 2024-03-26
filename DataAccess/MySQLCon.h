#ifndef MYSQLBASE_H
#define MYSQLBASE_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
struct DatabaseConfig
{
    QString ip;
    uint PORT = 0;
    QString DatabaseName;
    QString UserName;
    QString Password;

};

class CMySQLCon
{
public:
    CMySQLCon();
    ~CMySQLCon();

public:
    //设置数据库信息
    void setDabaBaseInfo(QString ipadd, int port, QString dbName, QString user, QString pwd);
    bool open();
    void close();
    bool exec(QString cmd);
    bool prepare(const QString &cmd);
    void bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType paramType = QSql::In);
    bool exec();
    bool next();
    QVariant value(int index);
    QVariant value(const QString &name);
    bool isOpen();


private:
public:
    DatabaseConfig con;
    QSqlDatabase* db = NULL;
    QSqlQuery* query = NULL;
};
bool MySqlLink();

#endif // MYSQLBASE_H
