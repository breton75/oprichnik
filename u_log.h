#ifndef U_LOG
#define U_LOG

//#pragma once

#include <QString>
#include <QDate>
#include <QTime>
#include <QTextEdit>

enum MsgType {
    msgDefault,
    msgInfo,
    msgSuccess,
    msgFail,
    msgError,
    msgDate
};

void assignLog(QTextEdit* appLogTE);

void logMsg(const MsgType msgType, const QString msgStr, QTextEdit* appLogTE = 0);

void closeLog();

#endif // U_LOG
