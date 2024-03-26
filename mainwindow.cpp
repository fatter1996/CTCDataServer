#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->scrollAreaWidgetContents->installEventFilter(this);
    ui->scrollAreaWidgetContents->resize(1928,1080);
    //ui->scrollArea->resize(1920,1080);

    initData();

}

MainWindow::~MainWindow()
{
    delete ui;
}

//初始化数据
void MainWindow::initData()
{
    pDoc = new MyDoc();
    initMenu();

    nElapsed = 0;
    nDiploid = 1.0f;
    //界面绘制刷新显示定时器
    pTimeShow = new QTimer();
    pTimeShow->setInterval(500);
    connect(pTimeShow,SIGNAL(timeout()),this,SLOT(timeOutSlot()));
    pTimeShow->start();

//for teset
//    int *p=nullptr;
//        *p=66666666;
}

//动态初始化车站列表菜单
void MainWindow::initMenu()
{
    QMenu* stationMenu = menuBar()->addMenu("选择车站");//StationList//创建菜单  QString::fromLocal8Bit("车站")    QStringLiteral("车站")
    for(int i=0; i<pDoc->vectMyStation.count(); i++)
    {
        QString staName = pDoc->vectMyStation[i]->getStationName();
        QAction* actStation = new QAction(this);
        actStation->setText(staName);
        actStation->setStatusTip(staName);//状态栏设置
        stationMenu->addAction(actStation);  //创建菜单项
    }
    connect(stationMenu,SIGNAL(triggered(QAction*)),this, SLOT(clickedStationMenuSlot(QAction*)));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==ui->scrollAreaWidgetContents)
    {
        if(event->type() == QEvent::Paint)
        {
            drawStation();
        }
    }
    return QWidget::eventFilter(obj,event);
}

//重绘界面
void MainWindow::drawStation()
{
    QPainter painter(ui->scrollAreaWidgetContents);
    pDoc->Draw(&painter,nElapsed,nDiploid); //异常
    painter.end();
}

void MainWindow::timeOutSlot()
{
    ui->scrollAreaWidgetContents->update();
    nElapsed++;
    if(nElapsed > 99999999)
    {
        nElapsed=0;
    }
}
//菜单点击
void MainWindow::clickedStationMenuSlot(QAction *action)
{
    QString strText = action->text();    //根据text来判断具体的action操作
    qDebug()<<"Selected Station:"<<strText;
    pDoc->setCurrIndexByStaName(strText);
}

