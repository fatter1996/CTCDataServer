#ifndef MYSERVICE_H
#define MYSERVICE_H

#include <QObject>
#include <QApplication>
#include "qtservice.h"
#include "MyDoc/mydoc.h"

#pragma execution_character_set("utf-8")

class MyService : public QObject, public QtService<QApplication>
{
    Q_OBJECT
//public:
//    explicit MyService(QObject *parent = nullptr);
public:
    MyService(int argc, char **argv);
    ~MyService();

protected:
    void start()override;
    void stop()override;
    void pause()override;
    void resume()override;

signals:

public:
    MyDoc* pDoc;

};

#endif // MYSERVICE_H
