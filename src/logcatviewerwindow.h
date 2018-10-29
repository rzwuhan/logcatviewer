#ifndef LOGCATVIEWERWINDOW_H
#define LOGCATVIEWERWINDOW_H

#include <QMainWindow>
#include <QtCore/QProcess>
#include <QtCore/QReadWriteLock>
#include <QtGui/QTextCursor>
#include <QtCore/QMutex>
#include <QtWidgets/QTextEdit>
#include <QList>
#include <QtWidgets/QLabel>

namespace Ui {
    class LogcatViewerWindow;
}

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
    QBrush COLOR_D = QBrush(QColor("#000000"));
    QBrush COLOR_I = QBrush(QColor("#000000"));
    QBrush COLOR_W = QBrush(QColor("#00007F"));
    QBrush COLOR_E = QBrush(QColor("#7F0000"));
    QBrush COLOR_N = QBrush(QColor("#000000"));
};

class LogcatViewerWindow : public QMainWindow {
Q_OBJECT

public:
    explicit LogcatViewerWindow(QWidget *parent = 0);

    ~LogcatViewerWindow();

private:

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void UpdateDevices(QString devices);

    void UpdateLog();

    void SelectDevice(int index);

    void SelectFilter(int index);

    void Filtrating();

    void HighlightFinding();

    void FindPrev();

    void FindNext();

    void Clear();

Q_SIGNALS:

    void SignalUpdateLogs();

    void SignalUpdateDevices(QString devices);

private:
    QString mProgram = "/opt/androidsdk/platform-tools/adb";
    Ui::LogcatViewerWindow *ui;
    QProcess *mLogcatProcess;
    QMutex mLocker;
    QVector<LogItem> mLogs;
    /**
     * ui->logs中最后一条log在mLogs中的索引
     */
    int mCurrentIndex;
    /**
     * 当前查找结果在ui->logs->extraSelections()中的索引
     */
    int mCurrentSelection;
    /**
     * 自定义tag过滤列表
     */
    QVector<QSet<QString>> mFilters;
    /**
     * 快速过滤关键词
     */
    QString mFilterKeyword;
    /**
     * 搜索关键词
     */
    QString mFinderKeyword;
    const QColor FINDER_HIGHLIGHT_COLOR = QColor(Qt::yellow);
    /**
     * 是否自动滚动
     */
    bool mAutoScroll;


    // 状态栏组件
    QLabel *mStatusLineCount;
    QLabel *mStatusFindResult;
};

#endif // LOGCATVIEWERWINDOW_H
