QT       += core gui
QT       += network
QT       += sql
QT       += concurrent
#QT       += testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
#CONFIG += exceptions
LIBS   += -lDbgHelp
#目的是Release版也将生成“.pdb”后缀的调试信息文件。在使用WinDbg导入Dump前。指定好源代码与pdb文件的位置。就可以在错误报告内看到罪魁祸首是哪一行代码
QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
#CONFIG += console
#CONFIG -= debug_and_release
#DESTDIR = ../
#exe生产在项目根目录
DESTDIR = ./

RC_FILE=CTCDataServer_resource.rc

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#自定义模块引入
#全局变量模块
include(GlobalHeaders/GlobalHeaders.pri)
#车站基类模块
include(BaseDataPro/BaseDataPro.pri)
#自定义车站模块
include(MyStation/MyStation.pri)
#通信模块
include(SocketPro/SocketPro.pri)
#主业务逻辑模块
include(MyDoc/MyDoc.pri)
#数据库访问模块
include(DataAccess/DataAccess.pri)
#轻量级日志库
include(QsLog/QsLog.pri)
#自定义日志使用
include(Log/Log.pri)
#服务使用
include(qtservice/src/qtservice.pri)
include(myService/myService.pri)


SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Helper.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    DebugData.txt \
    DevelopLog.txt

#调试内存使用
# win64 {
#           CONFIG(debug, debug|release) {
#           #        DEFINES += _DEBUG
#           VLD_PATH = C:/Program Files (x86)/Visual Leak Detector
#           INCLUDEPATH += $VLD_PATH/include
#           LIBS += -L$VLD_PATH/lib/Win64 -lvld
#            }
#       }

#QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
#QMAKE_CXXFLAGS_STL_ON = /EHa
win* {
QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
QMAKE_CXXFLAGS_STL_ON = /EHa
}
CONFIG += exception



