#ifndef LOGCATVIEWER_ADBHELPER_H
#define LOGCATVIEWER_ADBHELPER_H

#include <QList>
#include <QStringList>
#include <QVector>
#include <QtCore/QProcess>
#include <QtCore/QReadWriteLock>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include "logitem.h"
#include "config.h"

class ADBListener {

public:
    virtual void onDeviceUpdate() = 0;

    virtual void onLogUpdate() = 0;
};

class ADBHelper {
public:
    ADBHelper();

    ~ADBHelper();

    void setListener(ADBListener *l);

    void logcat(QString &serial);

    int findDevice(QString &serial);

private:
    void close(QProcess *process);

public:
    QString adb_absolute_path = Config::GetInstance().mADB;

    std::mutex logs_lock;
    QVector<LogItem> logs;
    int logs_current_index;

    QStringList device_list;
    QStringList device_status;
    QString device_current;
    std::mutex device_current_lock;
    std::condition_variable device_current_cond;

private:
    bool thread_exit;
    std::thread *device_thread;
    QProcess *device_process;

    std::thread *logcat_thread;
    QProcess *logcat_process;

    ADBListener *listener;
};

#endif //LOGCATVIEWER_ADBHELPER_H
