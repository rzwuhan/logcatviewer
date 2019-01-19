#ifndef LOGCATVIEWER_LOGITEM_H
#define LOGCATVIEWER_LOGITEM_H

#include <QBrush>

class LogItem {
public:
    LogItem() {}

    LogItem(QString line) :
            log(line),
            level(line[31]) {
        int pos = line.indexOf(':', 33);
        tag = line.mid(33, pos - 33).trimmed();
    }

    QBrush &Color() {
        if (level == 'D') {
            return COLOR_D;
        } else if (level == 'I') {
            return COLOR_I;
        } else if (level == 'W') {
            return COLOR_W;
        } else if (level == 'E') {
            return COLOR_E;
        } else {
            return COLOR_N;
        }
    }

public:
    QChar level;
    QString tag;
    QString log;

private:
//    Assert: 9C27B0
//    Debug: 2196F3
//    Error: F44336
//    Info: 4CAF50
//    Warning: FFC107

    QBrush COLOR_D = QBrush(QColor("#000000"));
    QBrush COLOR_I = QBrush(QColor("#000000"));
    QBrush COLOR_W = QBrush(QColor("#00007F"));
    QBrush COLOR_E = QBrush(QColor("#FF0000"));
    QBrush COLOR_N = QBrush(QColor("#000000"));
};

#endif //LOGCATVIEWER_LOGITEM_H
