#include "u_log.h"

QTextEdit* logTE;

void assignLog(QTextEdit* appLogTE) {
    logTE = appLogTE;
    logTE->setReadOnly(true);
}

void logMsg(const MsgType msgType, const QString msgStr, QTextEdit* appLogTE) {
    if (logTE == 0) return;
    QString strDT = QTime::currentTime().toString("HH:mm:ss.zzz ");
    if(msgType == msgDate) {
        QDate d = QDate::currentDate();
        strDT = d.toString("dd.MM.yy  ") + strDT;
    }

    QString msgColor;
    switch (msgType) {
        case msgInfo:
            msgColor = "DarkBlue";//"#08088A";//
            break;
        case msgFail:
            msgColor = "#8A0808";//"FireBrick";
            break;
        case msgError:
            msgColor = "Red";
            break;
        case msgDate:
            msgColor = "DarkBlue";//"#0B0B3B";//
            break;
        case msgSuccess:
            msgColor = "Green";//  "#0B610B";//"DarkGreen";//
            break;
        default:
            msgColor = "Black";
            break;
    }
    if(appLogTE == 0) appLogTE = logTE;
    //logTE->append(QString("<font color = \"%1\"> %2 %3</font>").arg(msgColor).arg(strDT).arg(msgStr));
    appLogTE->append(QString("<font color = \"%1\"> %2 %3</font>").arg(msgColor).arg(strDT).arg(msgStr));
}

void closeLog() {
    logTE = 0;
}

