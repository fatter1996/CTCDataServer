#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MyDoc/mydoc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MyDoc* pDoc;

    long nElapsed;  //定时器计数器
    double nDiploid;  //记录缩放量
    QTimer *pTimeShow; //界面绘制刷新显示定时器申明

private:
    //初始化数据
    void initData();
    void initMenu();

public:
    bool eventFilter(QObject *obj,QEvent *event);

private:
    //重绘界面
    void drawStation();

public slots:
    //定时器槽函数
    void timeOutSlot();
    //菜单点击
    void clickedStationMenuSlot(QAction* action);
};
#endif // MAINWINDOW_H
