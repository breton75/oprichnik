#-------------------------------------------------
#
# Project created by QtCreator 2016-11-09T09:27:11
#
#-------------------------------------------------

QT       += core gui sql widgets

TARGET = oprichnik_tpo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ref_edit.cpp \
    pg_database.cpp \
    u_log.cpp \
    tanks_edit.cpp \
    universaldelegate.cpp \
    consumers_edit.cpp \
    fueltables_edit.cpp \
    net_edit.cpp

HEADERS  += mainwindow.h \
    u_duty.h \
    ref_edit.h \
    pg_database.h \
    pg_scripts.h \
    tanks_edit.h \
    universaldelegate.h \
    consumers_edit.h \
    fueltables_edit.h \
    net_edit.h \
    u_log.h

FORMS    += mainwindow.ui \
    ref_edit.ui \
    tanks_edit.ui \
    consumers_edit.ui \
    fueltables_edit.ui \
    net_edit.ui

RESOURCES += \
    tpo_res_icons.qrc
