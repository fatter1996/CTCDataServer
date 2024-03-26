#ifndef ROUTEPREWND_H
#define ROUTEPREWND_H

#include <QObject>
#include <QPainter>

//进路预告窗（显示最近3个车次）
class RoutePreWnd : public QObject
{
    Q_OBJECT
public:
    explicit RoutePreWnd(QObject *parent = nullptr);

signals:

public:
    //进路信息
    typedef struct
    {
        int route_id = 0;//进路序列id
        QString CheCi = "";//车次
        QString GDName = "";//股道名称
        int m_nKHType = 0;//客货类型：1客车/0货车
        int routeType = 0;//进路类型：1接车，2发车，3通过
        int routeState = 0;//进路状态：0非自触，1自触等待，2进路办理完成/占用，3占用
    }RouteInfo;

public:
//    车次号显示青色文字为货车，红色文字为客车
//    列车进路的办理情况有字母 T（通过）、 F（发车）、 J（接车）三种，英文字
//    母后面的数字代表股道编号。
//    代表列车进路办理情况的英文字母和股道编号的显示颜色分三种：红色表
//    示列车进路序列表中对应的进路“自触”标记没有设置；黄色表示进路的“自
//    触”标记已经设置，进路处于等待触发状态；绿色表示进路已经办理完成。
    QString juncXHDName;//关联依赖的信号灯设备名称
    int     juncXHDCode;//关联依赖的信号灯设备号
    QPoint pBasePointConst;//对象显示的基点
    QPoint pBasePoint;//对象显示的基点，绘制的
    bool bUpSide;//上面/下面显示，默认上面
    bool m_bVisible;//是否可见

    //QVector<QRect> vectPreWnd;//预告窗
    QRect preWnd[3];//预告窗
    //RouteInfo routInfo[3];//进路信息
    QVector<RouteInfo> vectRouteInfo;//进路信息

    //区间逻辑检查状态(指示灯)，0隐藏/未知，1启用-绿色，2停用-红色，3异常-黄色
    int statusQJLJJC=0;

public:
    //绘制(画笔，计时器，缩放倍数，偏移量)
    void Draw(QPainter *painter, long nElapsed, double nDiploid, QPoint offset);
};

#endif // ROUTEPREWND_H
