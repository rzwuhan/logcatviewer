#include "adbhelper.h"
#include <QDebug>

ADBHelper::ADBHelper() :
        logs_lock(), logs(), logs_current_index(0),
        device_list(), device_status(),
        device_current(), device_current_lock(), device_current_cond(),
        thread_exit(false), listener(nullptr) {

    device_thread = new std::thread([this] {
        device_process = new QProcess;
        device_process->setProgram(adb_absolute_path);
        device_process->setArguments(QStringList() << "devices");

        while (!thread_exit) {
            for (auto &status: device_status) {
                status = "disconnect";
            }

            device_process->start();
            if (!device_process->waitForFinished()) {
                qDebug() << "wait device process finish failed:" << device_process->errorString();
                break;
            }

            bool changed = false;
            QString result = device_process->readAllStandardOutput();
            QStringList devices = result.split('\n', QString::SkipEmptyParts);
            for (int i = 1; i < devices.size(); ++i) {
                QStringList info = devices[i].split('\t', QString::SkipEmptyParts);
                int index = findDevice(info[0]);
                if (index != -1 && device_status[index] != info[1]) {
                    // update exist device status
                    device_status[index] = info[1];
                    changed = true;
                } else if (index == -1) {
                    // append new device info
                    device_list.append(info[0]);
                    device_status.append(info[1]);
                    changed = true;
                }
            }
            if (changed && listener != nullptr)
                listener->onDeviceUpdate();
            if (!thread_exit)
                std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        close(device_process);
        delete device_process;
        device_process = nullptr;

        qDebug() << "device thread exit.";
    });
    device_thread->detach();

    logcat_thread = new std::thread([this] {
        logcat_process = new QProcess;
        logcat_process->setProgram(adb_absolute_path);

        while (!thread_exit) {
            qDebug() << "wait for serial:" << device_current;
            // wait for an valid serial
            std::unique_lock<std::mutex> lock(device_current_lock);
            if (device_current.isEmpty()) {
                device_current_cond.wait(lock);
            }
            logcat_process->setArguments(
                    QStringList() << "-s" << device_current << "shell" << "logcat" << "-v" << "threadtime"
            );
            lock.unlock();

            std::unique_lock<std::mutex> loglock(logs_lock);
            logs.clear();
            logs_current_index = 0;
            loglock.unlock();
            if (listener != nullptr) {
                listener->onLogUpdate();
            }
            qDebug() << "start logcat process:" << device_current;

            logcat_process->start();
            if (logcat_process->waitForStarted(3000)) {
                char buffer[1024];
                while (true) {
                    if (logcat_process->state() != QProcess::Running) {
                        qDebug() << "logcat process state:" << logcat_process->state();
                        break;
                    }
                    if (!logcat_process->waitForReadyRead(-1)) {
                        qDebug() << "logcat process wait for ready read failed : " << logcat_process->errorString();
                        std::lock_guard<std::mutex> lock(device_current_lock);
                        device_current.clear();
                        break;
                    }
                    loglock.lock();
                    while (logcat_process->readLine(buffer, 1024) > 0) {
                        QString item = QString(buffer).trimmed();
                        if (!item.isEmpty()) {
                            logs.append(item);
                        }
                    }
                    loglock.unlock();
                    if (listener != nullptr) {
                        listener->onLogUpdate();
                    }
                }
                if (thread_exit) break;
            } else {
                qDebug() << "wait for started failed : " << logcat_process->errorString();
            }
        }

        close(logcat_process);
        delete logcat_process;
        logcat_process = nullptr;

        qDebug() << "logcat thread exit.";
    });
    logcat_thread->detach();
}

ADBHelper::~ADBHelper() {
    thread_exit = true;

    close(device_process);
    close(logcat_process);

    delete device_thread;
    device_thread = nullptr;
    delete logcat_thread;
    logcat_thread = nullptr;
}

void ADBHelper::setListener(ADBListener *l) {
    listener = l;
}


void ADBHelper::logcat(QString &serial) {
    if (serial == device_current && logcat_process->state() == QProcess::Running)
        return;
    std::lock_guard<std::mutex> lock(device_current_lock);
    close(logcat_process);
    device_current = serial;
    device_current_cond.notify_all();
}

void ADBHelper::close(QProcess *process) {
    if (process != nullptr && process->state() == QProcess::Running) {
        process->close();
    }
}

int ADBHelper::findDevice(QString &serial) {
    int index = -1;
    for (int i = 0; i < device_list.size(); ++i) {
        if (device_list[i] == serial) {
            index = i;
            break;
        }
    }
    return index;
}
